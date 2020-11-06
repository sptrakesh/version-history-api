//
// Created by Rakesh on 13/03/2020.
//

#pragma once

#include <chrono>
#include <ostream>
#include <string>

#include <bsoncxx/oid.hpp>
#include <bsoncxx/document/value.hpp>

namespace spt::model
{
  struct Metric
  {
    ~Metric() = default;
    Metric( const Metric& ) = delete;
    Metric& operator=( const Metric& ) = delete;

    bsoncxx::oid id;
    std::string method;
    std::string path;
    std::string host;
    std::string ipaddress;
    std::string contentType;
    int32_t status;
    int32_t outputSize;
    std::chrono::time_point<std::chrono::system_clock> date;
    int64_t time;
    bool compressed;

    [[nodiscard]] bsoncxx::document::value bson() const;
  };

  std::ostream& operator<<( std::ostream& stream, const Metric& metric );
}
