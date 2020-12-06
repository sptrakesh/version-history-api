//
// Created by Rakesh on 01/10/2019.
//

#pragma once

#include "model/metric.h"
#include "model/summaryresponse.h"

#include <bsoncxx/oid.hpp>
#include <bsoncxx/array/value.hpp>
#include <bsoncxx/document/value.hpp>

#include <optional>
#include <tuple>

namespace spt::db
{
  void init();

  spt::model::SummaryResponse summary( std::string_view database,
      std::string_view collection, const bsoncxx::oid& id );

  using Document = std::tuple<std::optional<bsoncxx::document::value>, int32_t>;
  Document document( const bsoncxx::oid& id );
  Document entity( const bsoncxx::oid& id );
  Document revert( const bsoncxx::oid& history,
      std::string_view database, std::string_view collection, const bsoncxx::oid& entity );

  void save( const model::Metric& metric );

  Document create( std::string_view database, std::string_view collection,
      const bsoncxx::document::view& document );
  Document retrieve( std::string_view database, std::string_view collection,
      std::string_view property, std::string_view value );
  Document query( std::string_view database, std::string_view collection,
      const bsoncxx::document::view& document );
  Document remove( std::string_view database, std::string_view collection,
      const bsoncxx::oid& id );
}
