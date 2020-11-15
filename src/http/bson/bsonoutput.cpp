//
// Created by Rakesh on 08/11/2020.
//

#include "output.h"
#include <unordered_set>

#include <bsoncxx/builder/stream/document.hpp>

void spt::http::bson::handleRoot(
    const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };
  auto static const paths = std::unordered_set<std::string>{ "/", "/version/history/" };

  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );

  return req.uri().path == "/" ?
      output( "ok", res ) : error( 404, "Not found", res );

}

void spt::http::bson::unsupported( const nghttp2::asio_http2::server::response& res )
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  auto doc = document{} <<
    "status" << 405 <<
    "cause" << "Method Not Allowed" <<
    finalize;
  auto body = std::string{ reinterpret_cast<const char*>( doc.view().data() ), doc.view().length() };
  write( 405, body, res );
}

void spt::http::bson::error( int code, std::string message, const nghttp2::asio_http2::server::response& res )
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  auto doc = document{} <<
    "status" << code <<
    "cause" << std::move( message ) <<
    finalize;
  auto body = std::string{ reinterpret_cast<const char*>( doc.view().data() ), doc.view().length() };
  write( code, std::move( body ), res );
}

void spt::http::bson::output( std::string message, const nghttp2::asio_http2::server::response& res )
{
  static constexpr auto code = 200;
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  auto doc = document{} <<
    "status" << code <<
    "cause" << std::move( message ) <<
    finalize;
  auto body = std::string{ reinterpret_cast<const char*>( doc.view().data() ), doc.view().length() };
  write( code, std::move( body ), res );
}

void spt::http::bson::write( int code, std::string bson,
    const nghttp2::asio_http2::server::response& res, bool compress )
{
  auto headers = nghttp2::asio_http2::header_map{
      {"content-type", { "application/bson", false } },
      { "content-length", { std::to_string( bson.size() ), false } }
  };
  if ( compress )
  {
    headers.emplace( "content-encoding", nghttp2::asio_http2::header_value{ "gzip", false } );
  }
  res.write_head( code, std::move( headers ) );
  res.end( std::move( bson ) );
}

