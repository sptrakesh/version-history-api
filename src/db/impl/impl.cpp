//
// Created by Rakesh on 06/11/2020.
//

#include "impl.h"
#include "log/NanoLog.h"
#include "model/config.h"
#include "util/bson.h"

#include <sstream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

auto spt::db::impl::summary( Connection& connection,
    std::string_view database, std::string_view collection, const bsoncxx::oid& id ) -> Summary
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto& conf = model::Configuration::instance();

  auto req = document{} <<
    "action" << "retrieve" <<
    "database" << conf.versionDatabase <<
    "collection" << conf.versionCollection <<
    "document" <<
      open_document <<
        "database" << database <<
        "collection" << collection <<
        "entity._id" << id <<
      close_document <<
    "options" <<
      open_document <<
        "projection" <<
          open_document <<
            "_id" << 1 <<
            "action" << 1 <<
            "created" << 1 <<
          close_document <<
        "sort" << open_document << "_id" << 1 << close_document <<
      close_document <<
    "application" << "version-history-api" <<
    finalize;
  const auto reqv = req.view();

  auto opt = connection.execute( reqv );

  if ( !opt )
  {
    LOG_WARN << "No or invalid response from service.";
    connection.setValid( false );
    return { std::nullopt, 500 };
  }

  const auto view = opt->view();
  const auto err = util::bsonValueIfExists<std::string>( "error", view );
  if ( err )
  {
    LOG_WARN << "Error retrieving version history for " << database << ':' <<
      collection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 417 };
  }

  const auto arr = util::bsonValueIfExists<bsoncxx::array::view>( "results", view );
  if ( !arr )
  {
    LOG_WARN << "Query results for version history for " << database << ':' <<
      collection << ':' << id.to_string() <<
      " did not return results. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 404 };
  }

  return { opt, 200 };
}

auto spt::db::impl::document( Connection& connection, const bsoncxx::oid& id ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto& conf = model::Configuration::instance();

  auto req = document{} <<
    "action" << "retrieve" <<
    "database" << conf.versionDatabase <<
    "collection" << conf.versionCollection <<
    "document" << open_document << "_id" << id << close_document <<
    "application" << "version-history-api" <<
    finalize;
  const auto reqv = req.view();

  auto opt = connection.execute( reqv );

  if ( !opt )
  {
    LOG_WARN << "No or invalid response from service.";
    connection.setValid( false );
    return { std::nullopt, 500 };
  }

  const auto view = opt->view();
  const auto err = util::bsonValueIfExists<std::string>( "error", view );
  if ( err )
  {
    LOG_WARN << "Error retrieving version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 417 };
  }

  const auto doc = util::bsonValueIfExists<bsoncxx::array::view>( "result", view );
  if ( !doc )
  {
    LOG_WARN << "Query for version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() <<
      " did not return result. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 404 };
  }

  return { bsoncxx::document::value{ *doc }, 200 };
}

auto spt::db::impl::entity( Connection& connection, const bsoncxx::oid& id ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto& conf = model::Configuration::instance();

  auto req = document{} <<
    "action" << "retrieve" <<
    "database" << conf.versionDatabase <<
    "collection" << conf.versionCollection <<
    "document" << open_document << "_id" << id << close_document <<
    "options" <<
      open_document <<
        "projection" << open_document << "entity" << 1 << close_document <<
      close_document <<
    "application" << "version-history-api" <<
    finalize;
  const auto reqv = req.view();

  auto opt = connection.execute( reqv );

  if ( !opt )
  {
    LOG_WARN << "No or invalid response from service.";
    connection.setValid( false );
    return { std::nullopt, 500 };
  }

  const auto view = opt->view();
  const auto err = util::bsonValueIfExists<std::string>( "error", view );
  if ( err )
  {
    LOG_WARN << "Error retrieving version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 417 };
  }

  const auto doc = util::bsonValueIfExists<bsoncxx::array::view>( "result", view );
  if ( !doc )
  {
    LOG_WARN << "Query for version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() <<
      " did not return result. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 404 };
  }

  return { bsoncxx::document::value{ util::bsonValue<bsoncxx::document::view>( "entity", *doc ) }, 200 };
}

auto spt::db::impl::revert( Connection& connection, const bsoncxx::oid& historyId,
    std::string_view database, std::string_view collection,
    const bsoncxx::oid& entityId ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto [vh, code] = entity( connection, historyId );
  if ( !vh )
  {
    LOG_WARN << "Unable to revert " << database << ':' << collection <<
      ':' << entityId.to_string() << " as history document " <<
      historyId.to_string() << " not found.";
    return { std::nullopt, code };
  }

  auto req = document{} <<
    "action" << "update" <<
    "database" << database <<
    "collection" << collection <<
    "document" << vh->view() <<
    "application" << "version-history-api" <<
    "metadata" << open_document << "revertedFrom" << historyId << close_document <<
    finalize;
  const auto reqv = req.view();

  auto opt = connection.execute( reqv );

  if ( !opt )
  {
    LOG_WARN << "No or invalid response from service.";
    connection.setValid( false );
    return { std::nullopt, 500 };
  }

  const auto view = opt->view();
  const auto err = util::bsonValueIfExists<std::string>( "error", view );
  if ( err )
  {
    LOG_WARN << "Error reverting document " << database << ':' <<
      collection << ':' << entityId.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    return { std::nullopt, 417 };
  }

  return { std::move( vh ), 200 };
}

void spt::db::impl::save( Connection& connection, const model::Metric& metric )
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto& conf = model::Configuration::instance();
  auto req = document{} <<
    "action" << "create" <<
    "database" << conf.metricsDatabase <<
    "collection" << conf.metricsCollection <<
    "document" << metric.bson() <<
    "application" << "version-history-api" <<
    "skipVersion" << true <<
    finalize;
  const auto reqv = req.view();

  auto opt = connection.execute( reqv );

  if ( !opt )
  {
    LOG_WARN << "No or invalid response from service.";
    connection.setValid( false );
    return;
  }

  const auto view = opt->view();
  const auto err = util::bsonValueIfExists<std::string>( "error", view );
  if ( err )
  {
    std::ostringstream ss;
    ss << "Error saving metric " << metric <<
      ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
  }
}
