//
// Created by Rakesh on 2019-05-29.
//

#pragma once

#include <sstream>

namespace spt::util
{
  bool isLeapYear( int16_t year );

  // Convert microseconds since UNIX epoch to ISO 8601
  std::string isoDate( int64_t epoch );
}
