//
// Created by Rakesh on 08/11/2020.
//

#include "output.h"
#include "../common.h"

#include <iomanip>
#include <unordered_set>

#include <boost/algorithm/string/predicate.hpp>

void spt::http::json::handleRoot(
    const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };
  auto static const paths = std::unordered_set<std::string>{ "/", "/version/history/" };

  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );

  if ( req.method() == "OPTIONS" ) return cors( res );

  return ( req.uri().path == "/" ||
    boost::algorithm::starts_with( req.uri().path, "/version/history/" ) ) ?
      output( "ok", res ) : error( 404, "Not found", res );

}

void spt::http::json::unsupported( const nghttp2::asio_http2::server::response& res )
{
  auto body = std::string{ R"({"status": 405, "cause": "Method Not Allowed"}
)" };
  write( 405, body, res );
}

void spt::http::json::error( int code, const std::string_view message, const nghttp2::asio_http2::server::response& res )
{
  std::ostringstream oss;
  oss << "{\"status\": " << code << R"(, "cause": )" << std::quoted( message ) << "}\n";
  write( code, oss.str(), res );
}

void spt::http::json::output( const std::string_view message, const nghttp2::asio_http2::server::response& res )
{
  std::ostringstream oss;
  oss << "{\"status\": " << 200 << R"(, "cause": )" << std::quoted( message ) << "}\n";
  write( 200, oss.str(), res );
}

void spt::http::json::write( int code, std::string json,
    const nghttp2::asio_http2::server::response& res, bool compress )
{
  auto headers = nghttp2::asio_http2::header_map{
      {"content-type", { "application/json; charset=utf-8", false } },
      { "content-length", { std::to_string( json.size() ), false } }
  };
  if ( compress )
  {
    headers.emplace( "content-encoding", nghttp2::asio_http2::header_value{ "gzip", false } );
  }
  res.write_head( code, std::move( headers ) );
  res.end( std::move( json ) );
}
