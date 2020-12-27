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

  if ( sc )
  {
    auto [out, co] = compress( yamlstr );
    if ( co )
    {
      auto headers = nghttp2::asio_http2::header_map{
          {"content-type", { "text/yaml; charset=utf-8", false } },
          { "content-length", { std::to_string( out.size() ), false } }
      };
      headers.emplace( "content-encoding", nghttp2::asio_http2::header_value{ "gzip", false } );
      res.write_head( 200, std::move( headers ) );
      return res.end( std::move( out ) );
    }
  }

  auto headers = nghttp2::asio_http2::header_map{
      {"content-type", { "text/yaml; charset=utf-8", false } },
      { "content-length", { std::to_string( yamlstr.size() ), false } }
  };
  res.write_head( 200, std::move( headers ) );
  res.end( std::move( yamlstr ) );
}

void spt::http::handleRedoc( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  if ( req.method() == "OPTIONS" ) return cors( res );

  static const std::string html = R"(<!DOCTYPE html>
<html>
  <head>
    <title>Version History API</title>
    <!-- needed for adaptive design -->
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://fonts.googleapis.com/css?family=Montserrat:300,400,700|Roboto:300,400,700" rel="stylesheet">

    <!--
    ReDoc doesn't change outer page styles
    -->
    <style>
      body {
        margin: 0;
        padding: 0;
      }
    </style>
  </head>
  <body>
    <redoc spec-url='/docs/openapi.yaml'></redoc>
    <script src="https://cdn.jsdelivr.net/npm/redoc@next/bundles/redoc.standalone.js"> </script>
  </body>
</html>
)";

  auto headers = nghttp2::asio_http2::header_map{
      { "content-type", { "text/html; charset=utf-8", false } },
      { "content-length", { std::to_string( html.size() ), false } }
  };
  res.write_head( 200, std::move( headers ) );
  res.end( html );
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

void spt::http::handleUpdate( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleUpdate( req, res ) : json::handleUpdate( req, res );
}

void spt::http::handleReplace( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto of = outputFormat( req );
  return of == "application/bson" ?
      bson::handleReplace( req, res ) : json::handleReplace( req, res );
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
