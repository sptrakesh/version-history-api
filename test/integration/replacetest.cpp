//
// Replaced by Rakesh on 21/11/2020.
//

#include "replacetest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::ReplaceTest;

void ReplaceTest::initTestCase()
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

void ReplaceTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = get( url, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET request not rejected by create endpoint" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ReplaceTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = get( url, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET method allowed on create endpoint" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  qDebug() << QString::fromStdString( bsoncxx::to_json( *option ) );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ReplaceTest::optionsRequest()
{
  QNetworkRequest req;

  const auto reply = custom( url, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void ReplaceTest::postRequestJson()
{
  auto json = QJsonObject{
      {"filter", QJsonObject{
        {"_id", QJsonObject{
          {"$oid", entityId}
        }},
      }},
      {"replace", QJsonObject{
        {"removed", "all other properties"}
      }}
  };

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = post( url, QJsonDocument{ json }.toJson(), &req );

  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  qDebug() << obj;
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error replacing document" );
  QVERIFY2( !obj.isEmpty(), "Empty json response from replace endpoint" );
  QVERIFY2( obj.contains( "document"), "Replace did not return updated document" );
  QVERIFY2( obj.contains( "history"), "Replace did not return associated version history metadata" );
}

void ReplaceTest::postRequestBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto doc = document{} <<
    "filter" << open_document << "_id" << bsoncxx::oid{ entityId.toStdString() } << close_document <<
    "replace" << open_document << "bson" << "value" << close_document <<
    finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error creating document" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "document" ) != option->end(), "Replace response did not return replaced document" );
  QVERIFY2( option->find( "history" ) != option->end(), "Replace response did not return version history metadata" );
}

void ReplaceTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = post( url, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Empty JSON did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ReplaceTest::invalidRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = post( url, {}, &req, "application/bson" );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Invalid BSON objectId did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ReplaceTest::putRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = put( url, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ReplaceTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = put( url, {}, &req, "application/bson" );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ReplaceTest::deleteRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = custom( url, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ReplaceTest::deleteRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = custom( url, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void ReplaceTest::cleanupTestCase()
{
  remove( entityId );
}
