//
// Created by Rakesh on 29/04/2020.
//

#pragma once

#include <string_view>
#include <vector>

namespace spt::util
{
  std::vector<std::string_view> split( std::string_view csv,
      std::size_t sizehint = 8, std::string_view delims = "," );
}
