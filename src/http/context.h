//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include <string>

namespace spt::http
{
  struct Context
  {
    Context() = default;
    ~Context() = default;

    Context( const Context& ) = delete;
    Context& operator=( const Context& ) = delete;

    std::string bearer;
    std::string correlationId;
    std::string body;
    bool compress;
  };
}
