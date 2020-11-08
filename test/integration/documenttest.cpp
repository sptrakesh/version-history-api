//
// Created by Rakesh on 07/11/2020.
//

#include "documenttest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/json.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/types.hpp>

using spt::DocumentTest;

void DocumentTest::initTestCase()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( listUrl ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "Matching entities not returned" );

  QVERIFY2( (*option)["results"].type() == bsoncxx::type::k_array, "Results not array" );

  const auto doc = (*option)["results"][0].get_document();
  qDebug() << QString::fromStdString( bsoncxx::to_json( doc ) );
  historyId = QString::fromStdString( doc.value["_id"].get_oid().value.to_string() );
  qDebug() << historyId;
}

void DocumentTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
  QVERIFY2( obj.contains( "database"), "database not returned" );
  QVERIFY2( obj.contains( "collection"), "collection not returned" );
  QVERIFY2( obj.contains( "action"), "action not returned" );
  QVERIFY2( obj.contains( "entity"), "entity not returned" );
}

void DocumentTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "database" ) != option->end(), "database not returned" );
  QVERIFY2( option->find( "collection" ) != option->end(), "database not returned" );
  QVERIFY2( option->find( "action" ) != option->end(), "database not returned" );
  QVERIFY2( option->find( "entity" ) != option->end(), "database not returned" );
}

void DocumentTest::optionsRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void DocumentTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void DocumentTest::postRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void DocumentTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1abc123" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void DocumentTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1abc123" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void DocumentTest::nonexistentRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const QString entityId = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void DocumentTest::nonexistentRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const QString entityId = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void DocumentTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void DocumentTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void DocumentTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void DocumentTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( historyId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}
