//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include <tuple>

#include <nghttp2/asio_http2_server.h>

namespace spt::http
{
  void cors( const nghttp2::asio_http2::server::response& res );

  std::string authorise( const nghttp2::asio_http2::server::request& req );
  std::string correlationId( const nghttp2::asio_http2::server::request& req );

  bool shouldCompress( const nghttp2::asio_http2::server::request& req );

  std::string outputFormat( const nghttp2::asio_http2::server::request& req );
  std::string ipaddress( const nghttp2::asio_http2::server::request& req );

  using Output = std::tuple<std::string,bool>;
  Output compress( std::string_view data );
}

