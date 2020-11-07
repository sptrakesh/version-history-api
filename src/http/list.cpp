//
// Created by Rakesh on 10/10/2020.
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

#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/exception/exception.hpp>

void spt::http::handleList( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto st = std::chrono::steady_clock::now();
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

    LOG_DEBUG << "Handling request for " << req.uri().path;
    const auto parts = util::split( req.uri().path, 6, "/" );
    if ( parts.size() < 6 )
    {
      LOG_INFO << "Path " << req.uri().path << " has only " << int(parts.size()) << " components";
      return error( 404, "Not found", res );
    }

    const auto oid = bsoncxx::oid{ parts[5] };
    const auto result = db::summary( parts[3], parts[4], oid );

    if ( result.code != 200 ) return error( result.code, "Error retrieving version history", res );

    if ( format == "application/bson" )
    {
      auto sv = std::string_view{
        reinterpret_cast<const char *>( result.data->view().data() ),
        result.data->view().length() };
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
      const auto js = bsoncxx::to_json( result.data->view() );
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

