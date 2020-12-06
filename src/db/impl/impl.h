//
// Created by Rakesh on 06/11/2020.
//

#pragma once

#include "connection.h"
#include "model/metric.h"

#include <bsoncxx/oid.hpp>
#include <bsoncxx/document/value.hpp>

#include <optional>
#include <tuple>

namespace spt::db::impl
{
  using Summary = std::tuple<std::optional<bsoncxx::document::value>, int>;
  Summary summary( Connection& connection,
      std::string_view database, std::string_view collection, const bsoncxx::oid& id );

  using Document = std::tuple<std::optional<bsoncxx::document::value>, int32_t>;
  Document document( Connection& connection,
      const bsoncxx::oid& id );
  Document entity( Connection& connection, const bsoncxx::oid& id );
  Document revert( Connection& connection,
      const bsoncxx::oid& historyId,
      std::string_view database, std::string_view collection,
      const bsoncxx::oid& entityId );

  void save( Connection& connection, const model::Metric& metric );

  Document createEntity( Connection& connection,
      std::string_view database, std::string_view collection,
      const bsoncxx::document::view& document );
  Document retrieve( Connection& connection,
      std::string_view database, std::string_view collection,
      std::string_view property, std::string_view value );
  Document query( Connection& connection,
      std::string_view database, std::string_view collection,
      const bsoncxx::document::view& document );
  Document deleteEntity( Connection& connection,
      std::string_view database, std::string_view collection,
      const bsoncxx::oid& id );
}
