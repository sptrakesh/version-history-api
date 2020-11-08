//
// Created by Rakesh on 08/11/2020.
//

#include "output.h"
#include "db/storage.h"
#include "http/common.h"
#include "log/NanoLog.h"
#include "model/config.h"
#include "model/metric.h"
#include "util/hostname.h"
#include "util/split.h"

#include <unordered_set>
#include <utility>

#include <bsoncxx/oid.hpp>
#include <bsoncxx/exception/exception.hpp>

namespace spt::http::bson::internal
{
  template <typename Function, typename... Args>
  void handle( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res,
      const std::unordered_set<std::string>& methods,
      const Function& fn, Args&&... args )
  {
    const auto st = std::chrono::steady_clock::now();
    LOG_DEBUG << "Handling " << req.method() << " request for " << req.uri().path;

    if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );
    if ( req.method() == "OPTIONS" ) return cors( res );

    try
    {
      auto bearer = authorise( req );
      auto compress = shouldCompress( req );
      auto ip = ipaddress( req );
      auto corId = correlationId( req );
      auto format = outputFormat( req );

      const auto& [doc, code] = fn( std::forward<Args>( args )... );
      if ( code != 200 ) return error( code, "Error retrieving version history", res );

      auto sv = std::string_view{
          reinterpret_cast<const char *>( doc->view().data() ),
          doc->view().length() };
      auto [data, compressed] = http::compress( sv );

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      auto metric = model::Metric{ bsoncxx::oid{},
          req.method(), req.uri().path, util::hostname(), ip, format,
          corId, 200, int32_t( data.size() ),
          std::chrono::system_clock::now(), delta.count(), compress };
      if ( !compressed ) metric.outputSize = int32_t( sv.size() );
      db::save( metric );

      if ( compressed ) write( 200, std::move( data ), res, compressed );
      else write( 200, std::string{ sv.data(), sv.size() }, res, compressed );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
      return error( 400, "Bad request", res );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << ex.what();
      return error( 500, "Internal server error", res );
    }
  }

  template <typename Function>
  void handleDocument( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res, const Function& fn )
  {
    auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };

    try
    {
      const auto parts = util::split( req.uri().path, 4, "/" );
      if ( parts.size() < 4 ) return error( 404, "Not found", res );

      const auto oid = bsoncxx::oid{ parts[3] };
      handle( req, res, methods, fn, oid );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
      return error( 400, "Bad request", res );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << ex.what();
      return error( 500, "Internal server error", res );
    }
  }
}

void spt::http::bson::handleDocument( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  return internal::handleDocument( req, res, &db::document );
}

void spt::http::bson::handleEntity( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  return internal::handleDocument( req, res, &db::entity );
}

void spt::http::bson::handleRevert( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  auto static const methods = std::unordered_set<std::string>{ "PUT", "OPTIONS" };
  try
  {
    const auto parts = util::split( req.uri().path, 7, "/" );
    if ( parts.size() < 7 ) return error( 404, "Not found", res );

    const auto vid = bsoncxx::oid{ parts[3] };
    const auto eid = bsoncxx::oid{ parts[6] };
    internal::handle( req, res, methods, &db::revert, vid, parts[4], parts[5], eid );
  }
  catch ( const bsoncxx::exception& b )
  {
    LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
    return error( 400, "Bad request", res );
  }
  catch ( const std::exception& ex )
  {
    LOG_WARN << "Error processing request " << ex.what();
    return error( 500, "Internal server error", res );
  }
}

