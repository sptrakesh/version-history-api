//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include <sstream>
#include <tuple>

#include <nghttp2/asio_http2_server.h>

namespace spt::http
{
  void cors( const nghttp2::asio_http2::server::response& res );
  void write( int code, const std::string& json,
      const nghttp2::asio_http2::server::response& res, bool compress = false );
  void writeBson( int code, const std::string& bson,
      const nghttp2::asio_http2::server::response& res, bool compress = false );
  void unsupported( const nghttp2::asio_http2::server::response& res );
  void error( int code, std::string_view message, const nghttp2::asio_http2::server::response& res );

  std::string authorise( const nghttp2::asio_http2::server::request& req,
      const nghttp2::asio_http2::server::response& res );
  std::string correlationId( const nghttp2::asio_http2::server::request& req );

  using Output = std::tuple<std::string,bool>;
  Output compress( std::string_view data );

  std::string decompress( const std::string& body );
  bool shouldCompress( const nghttp2::asio_http2::server::request& req );
  bool isCompressed( const nghttp2::asio_http2::server::request& req );

  std::string outputFormat( const nghttp2::asio_http2::server::request& req );
  std::string ipaddress( const nghttp2::asio_http2::server::request& req );
}

