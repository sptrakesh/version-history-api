//
// Created by Rakesh on 2019-05-14.
//

#include "config.h"

#include <iomanip>
#include <sstream>

using spt::model::Configuration;

std::string Configuration::str() const
{
  std::ostringstream ss;
  ss << '{' <<
    R"(", "logLevel": ")" << logLevel <<
    R"(", "mongo": {)" <<
    R"("host": ")" << mongoServiceHost <<
    R"(", "port": )" << mongoServicePort <<
    R"(, "database": ")" << metricsDatabase <<
    R"(", "collection": ")" << metricsCollection <<
    R"("}, "port": )" << port <<
    ", \"threads\": " << threads <<
    '}';
  return ss.str();
}
