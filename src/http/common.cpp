//
// Created by Rakesh on 10/10/2020.
//

#include "common.h"
#include "log/NanoLog.h"

#include <iomanip>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filter/gzip.hpp>

void spt::http::cors( const nghttp2::asio_http2::server::response& res )
{
  auto headers = nghttp2::asio_http2::header_map{
      {"Access-Control-Allow-Origin", {"*", false}},
      {"Access-Control-Allow-Methods", {"GET,POST", false}}
  };

  res.write_head(204, headers);
}

void spt::http::write( int code, const std::string& json,
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
  res.end( json );
}

void spt::http::writeBson( int code, const std::string& bson,
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
  res.end( bson );
}

void spt::http::unsupported( const nghttp2::asio_http2::server::response& res )
{
  auto body = std::string{ R"({"status": 405, "cause": "Method Not Allowed"}
)" };
  write( 405, body, res );
}

void spt::http::error( int code, const std::string_view message, const nghttp2::asio_http2::server::response& res )
{
  std::ostringstream oss;
  oss << "{\"status\": " << code << R"(, "cause": )" << std::quoted( message ) << "}\n";
  write( code, oss.str(), res );
}

auto spt::http::compress( std::string_view data ) -> Output
{
  namespace bio = boost::iostreams;

  if ( data.size() < 128 )
  {
    LOG_INFO << "Uncompressed size: " << int( data.size() ) << " less than 128, not compressing";
    return { std::string{ data.data(), data.size() }, false };
  }

  bio::stream<bio::array_source> source( data.data(), data.size() );
  std::ostringstream compressed;
  bio::filtering_streambuf<bio::input> in;
  in.push( bio::gzip_compressor( bio::gzip_params( bio::gzip::best_compression ) ) );
  in.push( source );
  bio::copy( in, compressed );

  auto str = compressed.str();
  LOG_INFO << "Uncompressed size: " << int( data.size() ) << " compressed size: " << int( str.size() );
  return { std::move( str ), true };
}

std::string spt::http::decompress( const std::string& body )
{
  namespace bio = boost::iostreams;

  bio::filtering_streambuf<bio::input> in;
  in.push( boost::iostreams::gzip_decompressor() );
  std::stringstream ss( body );
  in.push( ss );

  std::ostringstream decomp;
  boost::iostreams::copy( in, decomp );
  return decomp.str();
}

std::string spt::http::authorise( const nghttp2::asio_http2::server::request& req,
    const nghttp2::asio_http2::server::response& res )
{
  auto iter = req.header().find( "Content-Type" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "content-type" );

  if ( iter == std::cend( req.header() ) )
  {
    error( 400, "No Content-Type", res );
    return {};
  }

  if ( iter->second.value.find( "application/json" ) == std::string::npos )
  {
    error( 400, "Invalid Content-Type", res );
    return {};
  }

  iter = req.header().find( "authorization" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "Authorization" );
  return iter == std::cend( req.header() ) ? std::string{} : iter->second.value;
}

bool spt::http::shouldCompress( const nghttp2::asio_http2::server::request& req )
{
  auto iter = req.header().find( "accept-encoding" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "Accept-Encoding" );
  return !( iter == std::cend( req.header() ) ) &&
      iter->second.value.find( "gzip" ) != std::string::npos;
}

bool spt::http::isCompressed( const nghttp2::asio_http2::server::request& req )
{
  auto iter = req.header().find( "content-encoding" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "Content-Encoding" );
  return !( iter == std::cend( req.header() ) ) &&
      iter->second.value.find( "gzip" ) != std::string::npos;
}

std::string spt::http::correlationId( const nghttp2::asio_http2::server::request& req )
{
  auto iter = req.header().find( "x-wp-correlation-id" );
  return iter == std::cend( req.header() ) ? std::string{} : iter->second.value;
}

std::string spt::http::outputFormat( const nghttp2::asio_http2::server::request& req )
{
  auto& header = req.header();
  auto iter = header.find( "accept" );
  if ( iter == std::cend( header ) ) iter = header.find( "Accept" );
  if ( iter == std::cend( header ) )
  {
    LOG_INFO << "No accept header in request";
    return "";
  }

  if ( iter->second.value.find( "application/bson" ) != std::string::npos )
  {
    return "application/bson";
  }

  if ( iter->second.value.find( "application/json" ) != std::string::npos )
  {
    return "application/json";
  }

  LOG_INFO << "Unsupported accept header value " << iter->second.value;
  return "";
}

std::string spt::http::ipaddress( const nghttp2::asio_http2::server::request& req )
{
  auto& header = req.header();
  auto iter = header.find( "x-real-ip" );
  if ( iter == std::cend( header ) ) iter = header.find( "x-forwarded-for" );
  if ( iter != std::cend( header ) )
  {
    // Trim any port part from address
    const auto pos = iter->second.value.find( ':' );
    return pos != std::string::npos ? iter->second.value.substr( 0, pos ) :
        iter->second.value;
  }

  return req.remote_endpoint().address().to_string();
}
