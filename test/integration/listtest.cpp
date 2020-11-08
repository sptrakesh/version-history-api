//
// Created by Rakesh on 06/11/2020.
//

#include "listtest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/validate.hpp>
#include <bsoncxx/types.hpp>

using spt::ListTest;

void ListTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void ListTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e0";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
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

  const QString entityId = "5f3bc9e2502422053e0";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
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

  const QString entityId = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
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

  const QString entityId = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
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

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}
