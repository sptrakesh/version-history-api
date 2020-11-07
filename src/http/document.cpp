//
// Created by Rakesh on 06/11/2020.
//

#include "common.h"
#include "handlers.h"
#include "db/storage.h"
#include "log/NanoLog.h"
#include "model/config.h"
#include "model/metric.h"
#include "util/hostname.h"
#include "util/split.h"

#include <unordered_set>
#include <utility>

#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/exception/exception.hpp>

namespace spt::http::internal
{
  template <typename Function, typename... Args>
  void handle( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res, const Function& fn,
      Args&&... args )
  {
    const auto st = std::chrono::steady_clock::now();
    LOG_DEBUG << "Handling request for " << req.uri().path;

    auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };
    if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );
    if ( req.method() == "OPTIONS" ) return cors( res );

    try
    {
      auto bearer = authorise( req );
      auto compress = shouldCompress( req );
      auto ip = ipaddress( req );

      auto format = outputFormat( req );
      if ( format.empty() ) return error( 400, "Bad request", res );

      const auto& [doc, code] = fn( std::forward<Args>( args )... );
      if ( code != 200 ) return error( code, "Error retrieving version history", res );

      if ( format == "application/bson" )
      {
        auto sv = std::string_view{
            reinterpret_cast<const char *>( doc->view().data() ),
            doc->view().length() };
        const auto& [data, compressed] = http::compress( sv );

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        auto metric = model::Metric{ bsoncxx::oid{},
            req.method(), req.uri().path, util::hostname(), ip, format,
            200, int32_t( data.size() ),
            std::chrono::system_clock::now(), delta.count(), compress };
        db::save( metric );
        writeBson( 200, data, res, compressed );
      }
      else
      {
        const auto js = bsoncxx::to_json( doc->view() );
        const auto& [data, compressed] = http::compress( js );

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        auto metric = model::Metric{ bsoncxx::oid{},
            req.method(), req.uri().path, util::hostname(), ip, format,
            200, int32_t( data.size() ),
            std::chrono::system_clock::now(), delta.count(), compress };
        db::save( metric );
        write( 200, data, res, compressed );
      }
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
      return error( 400, "Bad request", res );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error procesing request " << ex.what();
      return error( 500, "Internal server error", res );
    }
  }

  template <typename Function>
  void handleDocument( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res, const Function& fn )
  {
    try
    {
      const auto parts = util::split( req.uri().path, 4, "/" );
      if ( parts.size() < 4 ) return error( 404, "Not found", res );

      const auto oid = bsoncxx::oid{ parts[3] };
      handle( req, res, fn, oid );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
      return error( 400, "Bad request", res );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error procesing request " << ex.what();
      return error( 500, "Internal server error", res );
    }
  }
}

void spt::http::handleDocument( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  return internal::handleDocument( req, res, &db::document );
}

void spt::http::handleEntity( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  return internal::handleDocument( req, res, &db::entity );
}

void spt::http::handleRevert( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  try
  {
    const auto parts = util::split( req.uri().path, 7, "/" );
    if ( parts.size() < 7 ) return error( 404, "Not found", res );

    const auto vid = bsoncxx::oid{ parts[3] };
    const auto eid = bsoncxx::oid{ parts[6] };
    internal::handle( req, res, &db::revert, vid, parts[4], parts[5], eid );
  }
  catch ( const bsoncxx::exception& b )
  {
    LOG_WARN << "Invalid document id " << req.uri().path << ". " << b.what();
    return error( 400, "Bad request", res );
  }
  catch ( const std::exception& ex )
  {
    LOG_WARN << "Error procesing request " << ex.what();
    return error( 500, "Internal server error", res );
  }
}
