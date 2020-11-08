//
// Created by Rakesh on 10/10/2020.
//

#include "common.h"
#include "bson/output.h"
#include "json/output.h"
#include "handlers.h"

void spt::http::handleRoot( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  const auto format = outputFormat( req );
  return format == "application/bson" ?
    bson::handleRoot( req, res ) : json::handleRoot( req, res );
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
