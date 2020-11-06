//
// Created by Rakesh on 06/11/2020.
//

#pragma once

#include <optional>
#include <bsoncxx/array/value.hpp>

namespace spt::model
{
  struct SummaryResponse
  {
    ~SummaryResponse() = default;
    SummaryResponse( SummaryResponse&& ) = default;
    SummaryResponse& operator=( SummaryResponse&& ) = default;

    SummaryResponse( const SummaryResponse& ) = delete;
    SummaryResponse& operator=( const SummaryResponse& ) = delete;

    std::optional<bsoncxx::array::value> data{ std::nullopt };
    int64_t time{ 0 };
    int code{ 200 };
  };
}
