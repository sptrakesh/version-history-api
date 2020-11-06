//
// Created by Rakesh on 10/10/2020.
//

#include "common.h"
#include "context.h"
#include "handlers.h"

#include <unordered_set>
#include <boost/algorithm/string/predicate.hpp>

void spt::http::handleRoot( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res )
{
  auto static const methods = std::unordered_set<std::string>{ "GET", "OPTIONS" };
  auto static const paths = std::unordered_set<std::string>{ "/", "/version/history/" };

  if ( methods.find( req.method() ) == std::cend( methods ) ) return unsupported( res );

  if ( req.method() == "OPTIONS" ) return cors( res );

  if ( req.uri().path == "/" || boost::algorithm::starts_with( req.uri().path, "/version/history/" ) )
  {
    return write( 200, R"({"status": 200, "cause": "ok"}
)", res );
  }

  return error( 404, "Not found", res );

}
