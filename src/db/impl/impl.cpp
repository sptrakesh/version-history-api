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
    std::ostringstream ss;
    ss << "Error retrieving version history for " << database << ':' <<
      collection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  const auto arr = util::bsonValueIfExists<bsoncxx::array::view>( "results", view );
  if ( !arr )
  {
    std::ostringstream ss;
    ss << "Query results for version history for " << database << ':' <<
      collection << ':' << id.to_string() <<
      " did not return results. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 404 };
  }

  if ( arr->empty() )
  {
    std::ostringstream ss;
    ss << "No version history for " << database << ':' <<
      collection << ':' << id.to_string() <<
      ". " << bsoncxx::to_json( reqv );
    LOG_INFO << ss.str();
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
    std::ostringstream ss;
    ss << "Error retrieving version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  const auto doc = util::bsonValueIfExists<bsoncxx::document::view>( "result", view );
  if ( !doc )
  {
    std::ostringstream ss;
    ss << "Query for version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() <<
      " did not return result. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
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
    std::ostringstream ss;
    ss << "Error retrieving version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  const auto doc = util::bsonValueIfExists<bsoncxx::document::view>( "result", view );
  if ( !doc )
  {
    std::ostringstream ss;
    ss << "Query for version history document " << conf.versionDatabase << ':' <<
      conf.versionCollection << ':' << id.to_string() <<
      " did not return result. " << bsoncxx::to_json( view ) <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
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
    std::ostringstream ss;
    ss << "Unable to revert " << database << ':' << collection <<
      ':' << entityId.to_string() << " as history document " <<
      historyId.to_string() << " not found.";
    LOG_WARN << ss.str();
    return { std::nullopt, code };
  }

  auto req = document{} <<
    "action" << "update" <<
    "database" << database <<
    "collection" << collection <<
    "document" <<
      open_document <<
        "filter" << open_document << "_id" << entityId << close_document <<
        "replace" << vh->view() <<
      close_document <<
    "options" << open_document << "upsert" << true << close_document <<
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
    std::ostringstream ss;
    ss << "Error reverting document " << database << ':' <<
      collection << ':' << entityId.to_string() << ". " << *err <<
      ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
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

auto spt::db::impl::createEntity( Connection& connection,
    std::string_view database, std::string_view collection,
    const bsoncxx::document::view& entity ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto req = document{} <<
    "action" << "create" <<
    "database" << database <<
    "collection" << collection <<
    "document" << entity <<
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
    std::ostringstream ss;
    ss << "Error creating entity " <<
       ". " << *err <<
       ". " << bsoncxx::to_json( entity ) <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  return { std::move( opt ), 200 };
}

auto spt::db::impl::retrieve( Connection& connection,
    std::string_view database, std::string_view collection,
    std::string_view property, std::string_view value ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto byId = [database, collection, value]()
  {
    return document{} <<
      "action" << "retrieve" <<
      "database" << database <<
      "collection" << collection <<
      "document" << open_document << "_id" << bsoncxx::oid{ value } << close_document <<
      "application" << "version-history-api" << finalize;
  };

  const auto byName = [database, collection, property, value]()
  {
    return document{} <<
      "action" << "retrieve" <<
      "database" << database <<
      "collection" << collection <<
      "document" << open_document << property << value << close_document <<
      "application" << "version-history-api" << finalize;
  };

  auto req = ( property == "_id" ) ? byId() : byName();
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
    std::ostringstream ss;
    ss << "Error retrieving from " << database << ':' << collection <<
       ". " << *err <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  return { std::move( opt ), 200 };
}

auto spt::db::impl::deleteEntity( Connection& connection,
    std::string_view database, std::string_view collection,
    const bsoncxx::oid& id ) -> Document
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto req = document{} <<
    "action" << "delete" <<
    "database" << database <<
    "collection" << collection <<
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
    std::ostringstream ss;
    ss << "Error deleting entity " << database << ':' << collection << ':' << id.to_string() <<
       ". " << *err <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  const auto arr = util::bsonValueIfExists<bsoncxx::array::view>( "success", view );
  if ( !arr )
  {
    std::ostringstream ss;
    ss << "No entity deleted for " << database << ':' << collection << ':' << id.to_string() <<
       ". " << bsoncxx::to_json( view ) <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  const auto hist = util::bsonValueIfExists<bsoncxx::array::view>( "history", view );
  if ( !hist )
  {
    std::ostringstream ss;
    ss << "No version history created for entity deleted " << database << ':' << collection << ':' << id.to_string() <<
       ". " << bsoncxx::to_json( view ) <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 417 };
  }

  if ( arr->length() < 1 || hist->length() < 1 )
  {
    std::ostringstream ss;
    ss << "No entity deleted " << database << ':' << collection << ':' << id.to_string() <<
       ". " << bsoncxx::to_json( view ) <<
       ". " << bsoncxx::to_json( reqv );
    LOG_WARN << ss.str();
    return { std::nullopt, 404 };
  }

  auto d = document{} <<
    "_id" << (*arr)[0].get_oid().value <<
    "history" << (*hist)[0].get_document().value <<
    finalize;

  return { d, 200 };
}
