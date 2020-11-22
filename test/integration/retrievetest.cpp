//
// Created by Rakesh on 07/11/2020.
//

#include "retrievetest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/types.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::RetrieveTest;

void RetrieveTest::initTestCase()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

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
}

void RetrieveTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
  QVERIFY2( obj.contains( "result" ), "result not returned" );
  QVERIFY2( obj["result"].isObject(), "result not object" );

  const auto entity = obj["result"].toObject();
  QVERIFY2( entity.contains( "_id" ), "id not returned" );
  QVERIFY2( entity.contains( "prop1" ), "prop1 not returned" );
  QVERIFY2( entity.contains( "prop2" ), "prop2 not returned" );
  QVERIFY2( entity.contains( "prop3" ), "prop3 not returned" );
  QVERIFY2( entity.contains( "prop4" ), "prop4 not returned" );
}

void RetrieveTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "result" ) != option->end(), "result not returned" );

  const auto obj = (*option)["result"].get_document().view();
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) != obj.end(), "prop2 not returned" );
  QVERIFY2( obj.find( "prop3" ) != obj.end(), "prop3 not returned" );
  QVERIFY2( obj.find( "prop4" ) != obj.end(), "prop4 not returned" );
}

void RetrieveTest::getByPropertyJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/prop1/value1" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
  QVERIFY2( obj.contains( "results" ), "results not returned" );
  QVERIFY2( obj["results"].isArray(), "results not array" );

  const auto arr = obj["results"].toArray();
  QVERIFY2( !arr.empty(), "results array empty" );
  const auto entity = arr[0].toObject();
  QVERIFY2( entity.contains( "_id" ), "id not returned" );
  QVERIFY2( entity.contains( "prop1" ), "prop1 not returned" );
  QVERIFY2( entity.contains( "prop2" ), "prop2 not returned" );
  QVERIFY2( entity.contains( "prop3" ), "prop3 not returned" );
  QVERIFY2( entity.contains( "prop4" ), "prop4 not returned" );
}

void RetrieveTest::getByPropertyBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/prop1/value1" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "results not returned" );

  const auto arr = (*option)["results"].get_array().value;
  QVERIFY2( !arr.empty(), "results array empty" );
  const auto obj = arr[0].get_document().value;
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) != obj.end(), "prop2 not returned" );
  QVERIFY2( obj.find( "prop3" ) != obj.end(), "prop3 not returned" );
  QVERIFY2( obj.find( "prop4" ) != obj.end(), "prop4 not returned" );
}

void RetrieveTest::optionsRequest()
{
  QNetworkRequest req;

  const auto endpoint = QString( "%1%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void RetrieveTest::postRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RetrieveTest::postRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RetrieveTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/_id/abc123" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RetrieveTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/_id/abc123" ).arg( url );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RetrieveTest::nonexistentRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const QString eid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( eid );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RetrieveTest::nonexistentRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const QString eid = QString::fromStdString( bsoncxx::oid{}.to_string() );
  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( eid );
  const auto reply = get( endpoint, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Non-existent BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RetrieveTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RetrieveTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RetrieveTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void RetrieveTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto endpoint = QString( "%1/_id/%2" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void RetrieveTest::cleanupTestCase()
{
  remove( entityId );
}
