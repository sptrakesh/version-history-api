//
// Created by Rakesh on 22/11/2020.
//

#include "output.h"
#include "db/storage.h"
#include "http/common.h"
#include "http/context.h"
#include "log/NanoLog.h"
#include "util/hostname.h"
#include "util/split.h"

#include <memory>
#include <unordered_set>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/exception/exception.hpp>

void spt::http::bson::handleRetrieve( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  const auto st = std::chrono::steady_clock::now();
  auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };
  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );
  if ( req.method() == "OPTIONS" ) return cors( res );

  try
  {
    auto context = http::Context{};
    context.bearer = authorise( req );
    context.compress = shouldCompress( req );
    context.correlationId = correlationId( req );

    auto format = outputFormat( req );
    if ( format.empty() ) return error( 400, "Bad request", res );

    const auto parts = util::split( req.uri().path, 6, "/" );
    if ( parts.size() != 6 ) return error( 404, "Not found", res );

    LOG_DEBUG << "Handling request for " << req.uri().path;

    const auto value = urlDecode( parts[5] );
    const auto& [doc, status] = db::retrieve( parts[2], parts[3], parts[4], value );
    if ( status != 200 ) return error( status, "Error retrieving documents", res );

    auto sv = std::string_view{
        reinterpret_cast<const char *>( doc->view().data() ),
        doc->view().length() };
    auto [data, compressed] = http::compress( sv );

    auto ip = ipaddress( req );
    auto compress = shouldCompress( req );

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    auto metric = model::Metric{ bsoncxx::oid{},
        req.method(), req.uri().path, util::hostname(), ip, format,
        context.correlationId, status, int32_t( data.size() ),
        std::chrono::system_clock::now(), delta.count(), compress };
    if ( !compressed ) metric.outputSize = int32_t( sv.size() );
    db::save( metric );

    if ( compressed ) write( status, std::move( data ), res, compressed );
    else write( status, std::string{ sv.data(), sv.size() }, res, compressed );
  }
  catch ( const bsoncxx::exception& b )
  {
    LOG_WARN << "BSON error processing " << req.uri().path << ". " << b.what();
    return error( 400, "Bad request", res );
  }
  catch ( const std::exception& ex )
  {
    LOG_WARN << "Error processing request " << ex.what();
    return error( 500, "Internal server error", res );
  }
}
