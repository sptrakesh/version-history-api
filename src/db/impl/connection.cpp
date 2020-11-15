//
// Created by Rakesh on 22/09/2020.
//

#include "connection.h"
#include "log/NanoLog.h"
#include "model/config.h"

#include <bsoncxx/json.hpp>
#include <bsoncxx/validate.hpp>

using spt::db::impl::Connection;

Connection::Connection( boost::asio::io_context& ioc, std::string_view h,
    std::string_view p ) : s{ ioc }, resolver{ ioc },
    host{ h.data(), h.size() }, port{ p.data(), p.size() }
{
  boost::asio::connect( s, resolver.resolve( host, port ) );
}

Connection::~Connection()
{
  boost::system::error_code ec;
  s.close( ec );
  if ( ec ) LOG_DEBUG << "Error closing socket connection " << ec.message();
}

std::optional<bsoncxx::document::value> Connection::execute(
    const bsoncxx::document::view_or_value& document, std::size_t bufSize )
{
  std::ostream os{ &buffer };
  os.write( reinterpret_cast<const char*>( document.view().data() ), document.view().length() );

  const auto isize = socket().send( buffer.data() );
  buffer.consume( isize );

  const auto documentSize = [this]( std::size_t length )
  {
    if ( length < 5 ) return length;

    const auto data = reinterpret_cast<const uint8_t*>( buffer.data().data() );
    uint32_t len;
    memcpy( &len, data, sizeof(len) );
    return std::size_t( len );
  };

  auto osize = socket().receive( buffer.prepare( bufSize ) );
  buffer.commit( osize );
  std::size_t read = osize;

  const auto docSize = documentSize( osize );
  while ( read < docSize )
  {
    osize = socket().receive( buffer.prepare( bufSize ) );
    buffer.commit( osize );
    read += osize;
  }

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( buffer.data().data() ), docSize );
  buffer.consume( buffer.size() );
  if ( option ) return bsoncxx::document::value{ option.value() };

  LOG_INFO << "Invalid BSON with size " << int(osize) << " in response to " <<
           bsoncxx::to_json( document.view() );
  return std::nullopt;
}

tcp::socket& spt::db::impl::Connection::socket()
{
  if ( ! s.is_open() )
  {
    LOG_DEBUG << "Re-opening closed connection.";
    boost::asio::connect( s, resolver.resolve( host, port ) );
  }

  if ( !v )
  {
    boost::system::error_code ec;
    s.close( ec );
    boost::asio::connect( s, resolver.resolve( host, port ) );
    v = true;
  }

  return s;
}

std::unique_ptr<Connection> spt::db::impl::create()
{
  static boost::asio::io_context ioc;
  const auto& conf = model::Configuration::instance();
  return std::make_unique<Connection>( ioc, conf.mongoServiceHost, conf.mongoServicePort );
}

