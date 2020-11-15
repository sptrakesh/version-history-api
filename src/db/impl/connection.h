//
// Created by Rakesh on 22/09/2020.
//

#pragma once

#include <optional>
#include <string_view>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <bsoncxx/document/view_or_value.hpp>

using tcp = boost::asio::ip::tcp;

namespace spt::db::impl
{
  struct Connection
  {
    Connection( boost::asio::io_context& ioc, std::string_view host, std::string_view port );

    Connection( const Connection& ) = delete;
    Connection& operator=( const Connection& ) = delete;

    ~Connection();

    std::optional<bsoncxx::document::value> execute(
        const bsoncxx::document::view_or_value& document, std::size_t bufSize = 4 * 1024 );

    [[nodiscard]] bool valid() const { return v; }
    void setValid( bool valid ) { this->v = valid; }

  private:
    tcp::socket& socket();

    tcp::socket s;
    tcp::resolver resolver;
    boost::asio::streambuf buffer;
    std::string host;
    std::string port;
    bool v{ true };
  };

  std::unique_ptr<Connection> create();
}
