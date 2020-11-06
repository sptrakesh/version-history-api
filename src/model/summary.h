//
// Created by Rakesh on 06/11/2020.
//

#pragma once

#include <chrono>
#include <ostream>
#include <vector>

#include <bsoncxx/oid.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/array/value.hpp>

namespace spt::model
{
  struct Summary
  {
    ~Summary() = default;
    Summary( Summary&& ) = default;
    Summary& operator=( Summary&& ) = default;
    Summary( const Summary& ) = delete;
    Summary& operator=( const Summary& ) = delete;

    [[nodiscard]] std::string json() const;
    [[nodiscard]] bsoncxx::document::value bson() const;

    bsoncxx::oid id;
    std::string action;
    std::chrono::time_point<std::chrono::system_clock> created;
  };

  using Summaries = std::vector<Summary>;
  bsoncxx::array::value bson( const Summaries & summaries );
  std::ostream& operator<<( std::ostream& stream, const Summary& summary );
  std::ostream& operator<<( std::ostream& stream, const Summaries& summaries );
}
