//
// Created by Rakesh on 08/11/2020.
//

#pragma once

#include <nghttp2/asio_http2_server.h>

namespace spt::http::json
{
  void handleRoot( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleList( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleDocument( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleEntity( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleRevert( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );

  void handleCreate( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleRetrieve( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleQuery( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleDelete( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );

  void unsupported( const nghttp2::asio_http2::server::response& res );
  void error( int code, std::string_view message, const nghttp2::asio_http2::server::response& res );
  void output( std::string_view message, const nghttp2::asio_http2::server::response& res );

  void write( int code, std::string bson,
      const nghttp2::asio_http2::server::response& res, bool compress = false );
}
