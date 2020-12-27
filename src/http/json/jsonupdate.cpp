//
// Created by Rakesh on 24/12/2020.
//

#include "output.h"
#include "db/storage.h"
#include "http/common.h"
#include "http/context.h"
#include "log/NanoLog.h"
#include "util/bson.h"
#include "util/hostname.h"
#include "util/split.h"

#include <memory>
#include <unordered_set>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/exception/exception.hpp>

void spt::http::json::handleUpdate( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  const auto st = std::chrono::steady_clock::now();
  auto static const methods = std::unordered_set<std::string>{ "PATCH", "POST", "PUT", "OPTIONS" };
  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );
  if ( req.method() == "OPTIONS" ) return cors( res );

  try
  {
    auto format = outputFormat( req );
    if ( format.empty() ) return error( 400, "Bad request", res );

    auto context = std::make_shared<http::Context>();
    context->bearer = authorise( req );
    context->compress = shouldCompress( req );
    context->correlationId = correlationId( req );
    context->body.reserve( 2048 );

    LOG_DEBUG << "Handling request for " << req.uri().path;

    req.on_data([context, st, &req, &res](const uint8_t* chars, std::size_t size)
    {
      if (size)
      {
        context->body.append( reinterpret_cast<const char*>( chars ), size );
        return;
      }

      if ( context->body.empty() ) return error( 400, "No payload", res );

      const auto parts = util::split( req.uri().path, 5, "/" );
      if ( parts.size() < 4 || parts.size() > 5 ) return error( 404, "Not found", res );

      if ( parts.size() == 4 )
      {
        if ( req.method() != "POST" )
        {
          LOG_WARN << "Invalid request method " << req.method() << " for " << req.uri().path;
          return error( 405, "Method Not Allowed", res );
        }
      }
      else if ( req.method() == "POST" )
      {
        LOG_WARN << "Invalid request method " << req.method() << " for " << req.uri().path;
        return error( 405, "Method Not Allowed", res );
      }

      auto idoc = bsoncxx::from_json( context->body );

      const auto vfn = [&]()
      {
        if ( parts.size() == 4 )
        {
          const auto filter = util::bsonValueIfExists<bsoncxx::document::view>( "filter", idoc );
          const auto payload = util::bsonValueIfExists<bsoncxx::document::view>( "update", idoc );
          if ( !filter || !payload )
          {
            LOG_WARN << "Missing filter or payload";
            return false;
          }

          return true;
        }

        if ( req.method() == "PUT" )
        {
          const auto oid = util::bsonValueIfExists<bsoncxx::oid>( "_id", idoc );
          if ( !oid )
          {
            LOG_WARN << "Missing object id in payload";
            return false;
          }
        }

        return true;
      };

      const auto fn = [&]()
      {
        if ( parts.size() == 4 )
        {
          const auto filter = util::bsonValueIfExists<bsoncxx::document::view>( "filter", idoc );
          const auto payload = util::bsonValueIfExists<bsoncxx::document::view>( "update", idoc );
          return db::update( parts[2], parts[3], *filter, *payload );
        }

        const auto id = bsoncxx::oid{ parts[4] };
        if ( req.method() == "PUT" )
        {
          return db::replace( parts[2], parts[3], id, idoc );
        }

        return db::update( parts[2], parts[3], id, idoc );
      };

      if ( !vfn() ) return error( 400, "Invalid update document", res );

      const auto& [doc, status] = fn();
      if ( status != 200 ) return error( status, "Error updating document", res );

      auto json = bsoncxx::to_json( doc->view() );
      auto [data, compressed] = http::compress( json );

      auto ip = ipaddress( req );
      auto format = outputFormat( req );
      auto compress = shouldCompress( req );

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      auto metric = model::Metric{ bsoncxx::oid{},
          req.method(), req.uri().path, util::hostname(), ip, format,
          context->correlationId, status, int32_t( data.size() ),
          std::chrono::system_clock::now(), delta.count(), compress };
      if ( !compressed ) metric.outputSize = int32_t( json.size() );
      db::save( metric );

      if ( compressed ) write( status, std::move( data ), res, compressed );
      else write( status, std::move( json ), res, compressed );
    });
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

void spt::http::json::handleReplace( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  const auto st = std::chrono::steady_clock::now();
  auto static const methods = std::unordered_set<std::string>{ "POST", "OPTIONS" };
  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );
  if ( req.method() == "OPTIONS" ) return cors( res );

  try
  {
    auto format = outputFormat( req );
    if ( format.empty() ) return error( 400, "Bad request", res );

    auto context = std::make_shared<http::Context>();
    context->bearer = authorise( req );
    context->compress = shouldCompress( req );
    context->correlationId = correlationId( req );
    context->body.reserve( 2048 );

    LOG_DEBUG << "Handling request for " << req.uri().path;

    req.on_data([context, st, &req, &res](const uint8_t* chars, std::size_t size)
    {
      if ( size )
      {
        context->body.append( reinterpret_cast<const char*>( chars ), size );
        return;
      }

      if ( context->body.empty() ) return error( 400, "No payload", res );

      const auto parts = util::split( req.uri().path, 4, "/" );
      if ( parts.size() != 4 ) return error( 404, "Not found", res );

      auto idoc = bsoncxx::from_json( context->body );

      const auto filter = util::bsonValueIfExists<bsoncxx::document::view>( "filter", idoc );
      if ( !filter ) return error( 400, "Query filter not specified", res );

      const auto entity = util::bsonValueIfExists<bsoncxx::document::view>( "replace", idoc );
      if ( !entity ) return error( 400, "Replace document not specified", res );

      const auto& [doc, status] = db::replace( parts[2], parts[3], *filter, *entity );
      if ( status != 200 ) return error( status, "Error replacing document", res );

      auto json = bsoncxx::to_json( doc->view() );
      auto [data, compressed] = http::compress( json );

      auto ip = ipaddress( req );
      auto format = outputFormat( req );
      auto compress = shouldCompress( req );

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      auto metric = model::Metric{ bsoncxx::oid{},
          req.method(), req.uri().path, util::hostname(), ip, format,
          context->correlationId, status, int32_t( data.size() ),
          std::chrono::system_clock::now(), delta.count(), compress };
      if ( !compressed ) metric.outputSize = int32_t( json.size() );
      db::save( metric );

      if ( compressed ) write( status, std::move( data ), res, compressed );
      else write( status, std::move( json ), res, compressed );
    });
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
