//
// Created by Rakesh on 21/11/2020.
//

#include "createtest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::CreateTest;

void CreateTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET request not rejected by create endpoint" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void CreateTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET method allowed on create endpoint" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  qDebug() << QString::fromStdString( bsoncxx::to_json( *option ) );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void CreateTest::optionsRequest()
{
  QNetworkRequest req;

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void CreateTest::postRequestJson()
{
  auto id = bsoncxx::oid{};
  entityId1 = QString::fromStdString( id.to_string() );
  auto json = QJsonObject{
      {"_id", QJsonObject{
          {"$oid", entityId1}
      }},
      {"string", "string value 1"},
      {"integer", 1},
      {"double", 1.0},
      {"boolean", false},
      {"nested", QJsonObject{
          {"key1", "value1"},
          {"key2", "value1"}
      }}
  };

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = post( endpoint, QJsonDocument{ json }.toJson(), &req );

  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating document" );
  QVERIFY2( !obj.isEmpty(), "Empty json response from create endpoint" );
  QVERIFY2( obj.contains( "_id"), "Create did not return id" );
}

void CreateTest::postRequestBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto id = bsoncxx::oid{};
  entityId2 = QString::fromStdString( id.to_string() );

  auto doc = document{} <<
      "_id" << id <<
      "string" << "string value 2" <<
      "integer" << 2 <<
      "double" << 2.0 <<
      "boolean" << true <<
      "nested" <<
        open_document <<
          "key1" << "value2" <<
          "key2" << "value2" <<
        close_document <<
      finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = post( endpoint,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating document" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "_id" ) != option->end(), "Create response did not return id" );
}

void CreateTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Empty JSON did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void CreateTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = post( endpoint, {}, &req, "application/bson" );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void CreateTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void CreateTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = put( endpoint, {}, &req, "application/bson" );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void CreateTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void CreateTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/create/itest/test" ).arg( url );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void CreateTest::cleanupTestCase()
{
  const auto remove1 = [this]()
  {
    QNetworkRequest req;
    req.setRawHeader( "accept", "application/json" );

    const auto endpoint = QString( "%1/delete/itest/test/%2" ).arg( url ).arg( entityId1 );
    const auto reply = custom( endpoint, "DELETE", &req );

    const auto doc = QJsonDocument::fromJson( reply->readAll() );
    const auto obj = doc.object();
    qDebug() << obj;
    QVERIFY2( reply->error() == QNetworkReply::NoError, "Delete endpoint returned error" );
    QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  };

  const auto remove2 = [this]()
  {
    QNetworkRequest req;
    req.setRawHeader( "accept", "application/bson" );

    const auto endpoint = QString( "%1/delete/itest/test/%2" ).arg( url ).arg( entityId2 );
    const auto reply = custom( endpoint, "DELETE", &req );

    QVERIFY2( reply->error() == QNetworkReply::NoError, "Delete endpoint returned error" );
    const auto body = reply->readAll();
    const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
    QVERIFY2( option.has_value(), "Response not BSON" );
    qDebug() << QString::fromStdString( bsoncxx::to_json( *option ) );
    //QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
  };

  remove1();
  remove2();
}
