//
// Created by Rakesh on 07/11/2020.
//

#include "reverttest.h"
#include "client.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/json.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::RevertTest;

void RevertTest::initTestCase()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1itest/test/%4" ).arg( listUrl ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "Matching entities not returned" );

  QVERIFY2( (*option)["results"].type() == bsoncxx::type::k_array, "Results not array" );

  const auto doc = (*option)["results"][1].get_document();
  qDebug() << QString::fromStdString( bsoncxx::to_json( doc ) );
  historyId = QString::fromStdString( doc.value["_id"].get_oid().value.to_string() );
  qDebug() << historyId;
}

void RevertTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RevertTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RevertTest::optionsRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void RevertTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RevertTest::postRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RevertTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( "abc123" );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RevertTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( "abc123" );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RevertTest::nonexistentRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( QString::fromStdString( bsoncxx::oid{}.to_string() ) );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RevertTest::nonexistentRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( QString::fromStdString( bsoncxx::oid{}.to_string() ) );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RevertTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
}

void RevertTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "err" ) == option->end(), "Retrieving history returned error" );
}

void RevertTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RevertTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1%2/itest/test/%3" ).arg( url ).arg( historyId ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RevertTest::cleanupTestCase()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto client = test::MongoClient{};
  auto req = document{} <<
      "action" << "delete" <<
      "database" << "itest" <<
      "collection" << "test" <<
      "document" << open_document << "_id" << bsoncxx::oid{ entityId.toStdString() } << close_document <<
      finalize;
  const auto reqv = req.view();

  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );
  auto response = client.execute( payload );
  auto opt = bsoncxx::validate( reinterpret_cast<const uint8_t*>( response.data() ), response.size() );
  QVERIFY2( opt, "Error deleting reverted document" );
  QVERIFY2( opt->find( "err" ) == opt->end(), "Deleting document returned error" );
}