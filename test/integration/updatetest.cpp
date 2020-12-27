//
// Created by Rakesh on 07/11/2020.
//

#include "updatetest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/builder/stream/document.hpp>

namespace spt::itest::update
{
  bsoncxx::document::value doc( const QString& id )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    return document{} <<
      "action" << "create" <<
      "database" << "itest" <<
      "collection" << "test" <<
      "document" <<
        open_document <<
          "_id" << bsoncxx::oid{ id.toStdString() } <<
          "prop1" << "value1" <<
          "prop2" << 4 <<
          "prop3" << false <<
          "prop4" << bsoncxx::types::b_null{} <<
        close_document <<
      finalize;
  }
}

using spt::UpdateTest;

void UpdateTest::initTestCase()
{
  auto oid = bsoncxx::oid{};
  entityId = QString::fromStdString( oid.to_string() );

  auto req = spt::itest::update::doc( entityId );
  const auto reqv = req.view();

  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );
  auto response = execute( payload );
  auto opt = bsoncxx::validate( reinterpret_cast<const uint8_t*>( response.data() ), response.size() );
  QVERIFY2( opt, "Error creating test document" );
  QVERIFY2( opt->find( "err" ) == opt->end(), "Creating document returned error" );
}

void UpdateTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void UpdateTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void UpdateTest::optionsRequest()
{
  QNetworkRequest req;

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void UpdateTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void UpdateTest::postRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void UpdateTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  auto bdoc = spt::itest::update::doc( entityId );
  auto payload = bsoncxx::to_json( bdoc.view() );

  const auto endpoint = QString( "%1/abc123" ).arg( url );
  const auto reply = put( endpoint, QByteArray{ payload.data(), int(payload.size()) }, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void UpdateTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = spt::itest::update::doc( entityId );
  const auto reqv = doc.view();
  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );

  const auto endpoint = QString( "%1/abc123" ).arg( url );
  const auto reply = put( endpoint, payload, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void UpdateTest::nonexistentRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  auto bdoc = spt::itest::update::doc( entityId );
  auto payload = bsoncxx::to_json( bdoc.view() );

  const QString eid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( eid );
  const auto reply = put( endpoint, QByteArray{ payload.data(), int(payload.size()) }, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void UpdateTest::nonexistentRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = spt::itest::update::doc( entityId );
  const auto reqv = doc.view();
  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );

  const QString eid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( eid );
  const auto reply = put( endpoint, payload, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void UpdateTest::updateJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  auto bdoc = QJsonObject{
      {"filter", QJsonObject{{"prop1", "value1"}}},
      {"update", QJsonObject{{"added", "another value"}}}
  };

  const auto reply = post( url, QJsonDocument{ bdoc }.toJson(), &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating document" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response from update endpoint" );
  QVERIFY2( obj.contains( "success"), "Updated document count not returned" );
  QVERIFY2( obj.contains( "history"), "Version history document metadata not returned" );
  QVERIFY2( obj.contains( "failure"), "Update document did not contained failure array" );

  const auto arr = obj["failure"].toArray();
  QVERIFY2( arr.empty(), "Failed updates in request" );
}

void UpdateTest::updateBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto bdoc = document{} <<
    "filter" << open_document << "prop1" << "value1" << close_document <<
    "update" << open_document << "added" << "another value" << close_document <<
    finalize;
  const auto reqv = bdoc.view();
  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );

  const auto reply = post( url, payload, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating document" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  qDebug() << QString::fromStdString( bsoncxx::to_json( *option ) );
  QVERIFY2( option->find( "success" ) != option->end(), "Update response did not return updated document count" );
  QVERIFY2( option->find( "history" ) != option->end(), "Update response did not return history metadata" );
  QVERIFY2( option->find( "failure" ) != option->end(), "Update response did not include failure array" );

  const auto arr = (*option)["failure"].get_array();
  QVERIFY2( std::distance( arr.value.begin(), arr.value.end() ) == 0, "Update request had failures" );
}

void UpdateTest::putRequestJson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  auto bdoc = document{} <<
    "_id" << bsoncxx::oid{ entityId.toStdString() } <<
    "moved" << spt::itest::update::doc( entityId ) <<
    finalize;
  const auto payload = bsoncxx::to_json( bdoc.view() );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, QByteArray{ payload.data(), int(payload.size()) }, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating document" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response from update endpoint" );
  QVERIFY2( obj.contains( "document"), "Updated document not returned" );
  QVERIFY2( obj.contains( "history"), "Version history document metadata not returned" );

  const auto udoc = obj["document"].toObject();
  QVERIFY2( udoc.contains( "_id"), "Update did not return id" );
  QVERIFY2( udoc.contains( "moved"), "Update did not add new moved property" );
  QVERIFY2( !udoc.contains( "prop1"), "Update did not remove old property" );
}

void UpdateTest::putRequestBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto bdoc = document{} <<
    "_id" << bsoncxx::oid{ entityId.toStdString() } <<
    "moved" << spt::itest::update::doc( entityId ) <<
    finalize;
  const auto reqv = bdoc.view();
  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, payload, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error updating document" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "document" ) != option->end(), "Update response did not return updated document" );
  QVERIFY2( option->find( "history" ) != option->end(), "Update response did not return history metadata" );

  const auto udoc = (*option)["document"].get_document().view();
  QVERIFY2( udoc.find( "_id" ) != option->end(), "Update response did not return id" );
  QVERIFY2( udoc.find( "moved" ) != option->end(), "Update did not add new moved property" );
  QVERIFY2( udoc.find( "prop1" ) == option->end(), "Update did not remove old property" );
}

void UpdateTest::patchRequestJson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  auto bdoc = document{} <<
    "_id" << bsoncxx::oid{ entityId.toStdString() } <<
    "added" << "another property" <<
    finalize;
  const auto payload = bsoncxx::to_json( bdoc.view() );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "PATCH", &req, QByteArray{ payload.data(), int(payload.size()) } );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error patching document" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response from patch endpoint" );
  QVERIFY2( obj.contains( "document"), "Updated document not returned" );
  QVERIFY2( obj.contains( "history"), "Version history document metadata not returned" );

  const auto udoc = obj["document"].toObject();
  QVERIFY2( udoc.contains( "_id"), "Patch did not return id" );
  QVERIFY2( udoc.contains( "moved"), "Patch removed old property" );
  QVERIFY2( udoc.contains( "added"), "Patch did not add new property" );
}

void UpdateTest::patchRequestBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto bdoc = document{} <<
    "_id" << bsoncxx::oid{ entityId.toStdString() } <<
    "added2" << "yet another property" <<
    finalize;
  const auto reqv = bdoc.view();
  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "PATCH", &req, payload );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error patching document" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "document" ) != option->end(), "Patch response did not return updated document" );
  QVERIFY2( option->find( "history" ) != option->end(), "Patch response did not return history metadata" );

  const auto udoc = (*option)["document"].get_document().view();
  QVERIFY2( udoc.find( "_id" ) != option->end(), "Patch response did not return id" );
  QVERIFY2( udoc.find( "moved" ) != option->end(), "Patch removed old property" );
  QVERIFY2( udoc.find( "added2" ) != option->end(), "Patch did not add property" );
}

void UpdateTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void UpdateTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void UpdateTest::cleanupTestCase()
{
  remove( entityId );
}
