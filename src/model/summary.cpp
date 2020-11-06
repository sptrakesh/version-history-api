//
// Created by Rakesh on 06/11/2020.
//

#include "summary.h"
#include "util/date.h"

#include <iomanip>
#include <sstream>

#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::model::Summary;

std::string Summary::json() const
{
  std::ostringstream ss;
  ss << *this;
  return ss.str();
}

bsoncxx::document::value Summary::bson() const
{
  return bsoncxx::builder::stream::document{} <<
    "_id" << id <<
    "action" << action <<
    "created" << bsoncxx::types::b_date{ created } <<
    bsoncxx::builder::stream::finalize;
}

bsoncxx::array::value spt::model::bson( const spt::model::Summaries& summaries )
{
  auto arr = bsoncxx::builder::basic::array{};
  for ( const auto& sum : summaries ) arr.append( sum.bson() );
  return arr.extract();
}

std::ostream& spt::model::operator<<( std::ostream& stream, const Summary& summary )
{
  auto micros = std::chrono::duration_cast<std::chrono::microseconds>( summary.created.time_since_epoch() );
  stream << '{' <<
    "id" << std::quoted( summary.id.to_string() ) <<
    "action" << std::quoted( summary.action ) <<
    "created" << std::quoted( util::isoDate( micros.count() ) ) <<
    '}';
  return stream;
}

std::ostream& spt::model::operator<<( std::ostream& stream,
    const spt::model::Summaries& summaries )
{
  stream << '[';
  for ( const auto& sum : summaries ) stream << sum;
  stream << ']';
  return stream;
}
