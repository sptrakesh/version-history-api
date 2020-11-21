//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include <nghttp2/asio_http2_server.h>

namespace spt::http
{
  void handleRoot( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  void handleSpec( const nghttp2::asio_http2::server::request& req,
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
  void handleDelete( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
}
