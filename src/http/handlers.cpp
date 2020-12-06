//
// Created by Rakesh on 10/10/2020.
//

#include "common.h"
#include "bson/output.h"
#include "json/output.h"
#include "handlers.h"

void spt::http::handleRoot( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  if ( req.method() == "OPTIONS" ) return cors( res );

  const auto format = outputFormat( req );
  return format == "application/bson" ?
    bson::handleRoot( req, res ) : json::handleRoot( req, res );
}

void spt::http::handleSpec( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  if ( req.method() == "OPTIONS" ) return cors( res );
  constexpr const char* yaml =
#include "/tmp/version-history-api.yaml"
    ;
  auto yamlstr = std::string{ yaml };
  auto sc = shouldCompress( req );

  auto headers = nghttp2::asio_http2::header_map{
      {"content-type", { "text/yaml; charset=utf-8", false } },
      { "content-length", { std::to_string( yamlstr.size() ), false } }
  };

  if ( sc )
  {
    auto [out, co] = compress( yamlstr );
    if ( co )
    {
      headers.emplace( "content-encoding", nghttp2::asio_http2::header_value{ "gzip", false } );
      res.write_head( 200, std::move( headers ) );
      return res.end( std::move( out ) );
    }
  }

  res.write_head( 200, std::move( headers ) );
  res.end( std::move( yamlstr ) );
}

void spt::http::handleList( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleList( req, res ) : json::handleList( req, res );
}

void spt::http::handleDocument( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleDocument( req, res ) : json::handleDocument( req, res );
}

void spt::http::handleEntity( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleEntity( req, res ) : json::handleEntity( req, res );
}

void spt::http::handleRevert( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleRevert( req, res ) : json::handleRevert( req, res );
}

void spt::http::handleCreate( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleCreate( req, res ) : json::handleCreate( req, res );
}

void spt::http::handleRetrieve( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleRetrieve( req, res ) : json::handleRetrieve( req, res );
}

void spt::http::handleQuery( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleQuery( req, res ) : json::handleQuery( req, res );
}

void spt::http::handleDelete( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleDelete( req, res ) : json::handleDelete( req, res );
}
