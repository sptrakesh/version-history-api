//
// Created by Rakesh on 05/11/2020.
//

#include "hostname.h"

#include <cstdlib>
#include <boost/asio/ip/host_name.hpp>

std::string spt::util::hostname()
{
  char* val = std::getenv( "HOST_NODE" );
  return val == nullptr ? boost::asio::ip::host_name() : std::string( val );
}
