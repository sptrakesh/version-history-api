//
// Created by Rakesh on 06/11/2020.
//

#include "listtest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/validate.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::ListTest;

void ListTest::initTestCase()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  const auto create = [this]()
  {
    auto oid = bsoncxx::oid{};
    entityId = QString::fromStdString( oid.to_string() );

    auto req = document{} <<
      "action" << "create" <<
      "database" << "itest" <<
      "collection" << "test" <<
      "document" <<
        open_document <<
          "_id" << oid <<
          "prop1" << "value1" <<
          "prop2" << 4 <<
          "prop3" << false <<
          "prop4" << bsoncxx::types::b_null{} <<
        close_document <<
      finalize;
    const auto reqv = req.view();

    auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );
    auto response = execute( payload );
    auto opt = bsoncxx::validate( reinterpret_cast<const uint8_t*>( response.data() ), response.size() );
    QVERIFY2( opt, "Error creating test document" );
    QVERIFY2( opt->find( "err" ) == opt->end(), "Creating document returned error" );
  };

  const auto update = [this]()
  {
    auto req = document{} <<
      "action" << "update" <<
      "database" << "itest" <<
      "collection" << "test" <<
      "document" <<
        open_document <<
          "_id" << bsoncxx::oid{ entityId.toStdString() } <<
          "prop1" << "value1" <<
          "prop2" << 40 <<
          "prop3" << true <<
          "prop4" << bsoncxx::types::b_null{} <<
        close_document <<
      finalize;
    const auto reqv = req.view();

    auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );
    auto response = execute( payload );
    auto opt = bsoncxx::validate( reinterpret_cast<const uint8_t*>( response.data() ), response.size() );
    QVERIFY2( opt, "Error updating test document" );
    QVERIFY2( opt->find( "err" ) == opt->end(), "Updating document returned error" );
  };

  create();
  update();
  remove( entityId );
}

void ListTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
  QVERIFY2( obj.contains( "results"), "Version documents not found" );

  const auto results = obj["results"];
  QVERIFY2( results.isArray(), "Results not array for api endpoint" );
  const auto arr = results.toArray();
  QVERIFY2( arr.size() >= 3, "Not all versions returned" );
}

void ListTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "Matching entities not returned" );

  QVERIFY2( (*option)["results"].type() == bsoncxx::type::k_array, "Results not array" );
}

void ListTest::optionsRequest()
{
  QNetworkRequest req;

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void ListTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::postRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ListTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( "abc123" );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( "abc123" );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ListTest::nonexistentRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const QString oid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( oid );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::nonexistentRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const QString oid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( oid );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ListTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ListTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ListTest::cleanupTestCase()
{
  remove( entityId );
}
