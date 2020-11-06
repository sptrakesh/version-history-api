//
// Created by Rakesh on 06/11/2020.
//

#include "split.h"

#include <algorithm>

std::vector<std::string_view> spt::util::split( std::string_view csv,
    std::size_t sizehint, std::string_view delims )
{
  std::vector<std::string_view> output;
  output.reserve( sizehint );
  auto first = csv.cbegin();

  while ( first != csv.cend() )
  {
    const auto second = std::find_first_of( first, std::cend( csv ),
        std::cbegin( delims ), std::cend( delims ) );

    if ( first != second )
    {
      output.emplace_back(
          csv.substr( std::distance( csv.begin(), first ),
              std::distance( first, second ) ) );
    }

    if ( second == csv.cend() ) break;
    first = std::next( second );
  }

  return output;
}

