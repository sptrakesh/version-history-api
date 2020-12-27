//
// Created by Rakesh on 10/10/2020.
//

#include "common.h"
#include "log/NanoLog.h"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filter/gzip.hpp>

void spt::http::cors( const nghttp2::asio_http2::server::response& res )
{
  auto headers = nghttp2::asio_http2::header_map{
      {"Access-Control-Allow-Origin", {"*", false}},
      {"Access-Control-Allow-Methods", {"GET,OPTIONS,PATCH,POST,PUT,DELETE", false}}
  };

  res.write_head(204, headers);
}

auto spt::http::compress( std::string_view data ) -> Output
{
  namespace bio = boost::iostreams;

  if ( data.size() < 128 )
  {
    LOG_INFO << "Uncompressed size: " << int( data.size() ) << " less than 128, not compressing";
    return { {}, false };
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

std::string spt::http::authorise( const nghttp2::asio_http2::server::request& req )
{
  auto iter = req.header().find( "authorization" );
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

std::string spt::http::correlationId( const nghttp2::asio_http2::server::request& req )
{
  auto iter = req.header().find( "x-spt-correlation-id" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "X-Spt-Correlation-Id" );
  if ( iter == std::cend( req.header() ) ) iter = req.header().find( "X-SPT-CORRELATION-ID" );
  return iter == std::cend( req.header() ) ? std::string{} : iter->second.value;
}

std::string spt::http::outputFormat( const nghttp2::asio_http2::server::request& req )
{
  auto& header = req.header();
  auto iter = header.find( "accept" );
  if ( iter == std::cend( header ) ) iter = header.find( "Accept" );
  if ( iter == std::cend( header ) )
  {
    if ( req.uri().path != "/" ) LOG_WARN << "No accept header in request for " << req.uri().path;
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

std::string spt::http::urlDecode( std::string_view path )
{
  std::string escaped;
  escaped.reserve( path.size() );

  for ( auto i = path.begin(), nd = path.end(); i < nd; ++i )
  {
    auto c = ( *i );

    switch(c)
    {
    case '%':
      if (i[1] && i[2])
      {
        char hs[]{ i[1], i[2] };
        escaped += static_cast<char>( std::strtol(hs, nullptr, 16) );
        i += 2;
      }
      break;
    case '+':
      escaped += ' ';
      break;
    default:
      escaped += c;
    }
  }

  return escaped;
}
