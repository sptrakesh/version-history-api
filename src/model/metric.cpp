//
// Created by Rakesh on 13/03/2020.
//

#include "metric.h"
#include "log/NanoLog.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

using spt::model::Metric;

bsoncxx::document::value spt::model::Metric::bson() const
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      date.time_since_epoch() );
  auto nms = std::chrono::duration_cast<std::chrono::nanoseconds>(
      date.time_since_epoch() ).count();

  auto doc = document{};
  doc <<
    "_id" << id <<
    "method" << method <<
    "path" << path <<
    "host" << host <<
    "status" << status <<
    "contentType" << contentType <<
    "compressed" << compressed <<
    "outputSize" << outputSize <<
    "time" << bsoncxx::types::b_int64{ time } <<
    "timestamp" << bsoncxx::types::b_int64{ nms } <<
    "date" << bsoncxx::types::b_date{ ms };

  if ( !ipaddress.empty() ) doc << "ipaddress" << ipaddress;
  if ( !correlationId.empty() ) doc << "correlationId" << correlationId;

  return doc << finalize;
}

std::ostream& spt::model::operator<<( std::ostream& stream, const Metric& metric )
{
  stream << bsoncxx::to_json( metric.bson() );
  return stream;
}
