//
// Created by Rakesh on 07/11/2020.
//

#include "querytest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <bsoncxx/types.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using spt::QueryTest;

void QueryTest::initTestCase()
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

void QueryTest::getRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto reply = get( url, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void QueryTest::getRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = get( url, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "GET allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void QueryTest::optionsRequest()
{
  QNetworkRequest req;

  const auto reply = custom( url, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void QueryTest::queryIdJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto query = QJsonObject{
      {"query", QJsonObject{
          {"_id", QJsonObject{
                {"$oid", entityId}
          }},
      }}
  };
  const auto reply = post( url, QJsonDocument{ query }.toJson(), &req );

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

void QueryTest::queryIdBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = document{} <<
    "query" << open_document << "_id" << bsoncxx::oid{ entityId.toStdString() } << close_document <<
    finalize;
  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "result" ) != option->end(), "result not returned" );

  const auto obj = (*option)["result"].get_document().value;
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) != obj.end(), "prop2 not returned" );
  QVERIFY2( obj.find( "prop3" ) != obj.end(), "prop3 not returned" );
  QVERIFY2( obj.find( "prop4" ) != obj.end(), "prop4 not returned" );
}

void QueryTest::queryPropertyJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto query = QJsonObject{
      {"query", QJsonObject{
          {"prop1", "value1"},
      }}
  };
  const auto reply = post( url, QJsonDocument{ query }.toJson(), &req );

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
  const auto entity = arr[0].toObject();
  QVERIFY2( entity.contains( "_id" ), "id not returned" );
  QVERIFY2( entity.contains( "prop1" ), "prop1 not returned" );
  QVERIFY2( entity.contains( "prop2" ), "prop2 not returned" );
  QVERIFY2( entity.contains( "prop3" ), "prop3 not returned" );
  QVERIFY2( entity.contains( "prop4" ), "prop4 not returned" );
}

void QueryTest::queryPropertyBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = document{} <<
    "query" << open_document << "prop1" << "value1" << close_document <<
    finalize;
  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "results not returned" );

  const auto arr = (*option)["results"].get_array().value;
  const auto obj = arr[0].get_document().value;
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) != obj.end(), "prop2 not returned" );
  QVERIFY2( obj.find( "prop3" ) != obj.end(), "prop3 not returned" );
  QVERIFY2( obj.find( "prop4" ) != obj.end(), "prop4 not returned" );
}

void QueryTest::queryIdProjectionJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto query = QJsonObject{
      {"query", QJsonObject{
          {"_id", QJsonObject{
              {"$oid", entityId}
          }},
      }},
      {"options", QJsonObject{
          {"projection", QJsonObject{
              {"prop1", 1}
          }}
      }}
  };
  const auto reply = post( url, QJsonDocument{ query }.toJson(), &req );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();
  const auto doc = QJsonDocument::fromJson( body );
  QVERIFY2( doc.isObject(), "Empty json response for api endpoint" );
  const auto obj = doc.object();
  qDebug() << obj;

  QVERIFY2( !obj.isEmpty(), "Empty json response for api endpoint" );
  QVERIFY2( obj.contains( "result" ), "result not returned" );
  QVERIFY2( obj["result"].isObject(), "result not document" );

  const auto entity = obj["result"].toObject();
  QVERIFY2( entity.contains( "_id" ), "id not returned" );
  QVERIFY2( entity.contains( "prop1" ), "prop1 not returned" );
  QVERIFY2( !entity.contains( "prop2" ), "prop2 returned" );
  QVERIFY2( !entity.contains( "prop3" ), "prop3 returned" );
  QVERIFY2( !entity.contains( "prop4" ), "prop4 returned" );
}

void QueryTest::queryIdProjectionBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = document{} <<
    "query" << open_document << "_id" << bsoncxx::oid{ entityId.toStdString() } << close_document <<
    "options" <<
    open_document <<
      "projection" << open_document << "prop1" << 1 << close_document <<
    close_document <<
    finalize;
  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "result" ) != option->end(), "results not returned" );

  const auto obj = (*option)["result"].get_document().value;
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) == obj.end(), "prop2 returned" );
  QVERIFY2( obj.find( "prop3" ) == obj.end(), "prop3 returned" );
  QVERIFY2( obj.find( "prop4" ) == obj.end(), "prop4 returned" );
}

void QueryTest::queryProjectionJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto query = QJsonObject{
      {"query", QJsonObject{
          {"prop1", "value1"},
      }},
      {"options", QJsonObject{
          {"projection", QJsonObject{
            {"prop1", 1}
          }}
      }}
  };
  const auto reply = post( url, QJsonDocument{ query }.toJson(), &req );

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
  const auto entity = arr[0].toObject();
  QVERIFY2( entity.contains( "_id" ), "id not returned" );
  QVERIFY2( entity.contains( "prop1" ), "prop1 not returned" );
  QVERIFY2( !entity.contains( "prop2" ), "prop2 returned" );
  QVERIFY2( !entity.contains( "prop3" ), "prop3 returned" );
  QVERIFY2( !entity.contains( "prop4" ), "prop4 returned" );
}

void QueryTest::queryProjectionBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = document{} <<
    "query" << open_document << "prop1" << "value1" << close_document <<
    "options" <<
      open_document <<
        "projection" << open_document << "prop1" << 1 << close_document <<
      close_document <<
    finalize;
  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error retrieving api response" );
  const auto body = reply->readAll();

  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "results" ) != option->end(), "results not returned" );

  const auto arr = (*option)["results"].get_array().value;
  const auto obj = arr[0].get_document().value;
  QVERIFY2( obj.find( "_id" ) != obj.end(), "id not returned" );
  QVERIFY2( obj.find( "prop1" ) != obj.end(), "prop1 not returned" );
  QVERIFY2( obj.find( "prop2" ) == obj.end(), "prop2 returned" );
  QVERIFY2( obj.find( "prop3" ) == obj.end(), "prop3 returned" );
  QVERIFY2( obj.find( "prop4" ) == obj.end(), "prop4 returned" );
}

void QueryTest::invalidRequestJson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/json" );

  const auto query = QJsonObject{
      {"options", QJsonObject{
          {"limit", 5}
      }}
  };
  const auto reply = post( url, QJsonDocument{ query }.toJson(), &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Omitting query did not return error" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void QueryTest::invalidRequestBson()
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  auto doc = document{} <<
    "options" << open_document << "limit" << 5 << close_document <<
    finalize;
  const auto reply = post( url,
      { reinterpret_cast<const char*>( doc.view().data() ), static_cast<int>( doc.view().length() ) },
      &req, "application/bson" );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "Omitting query did not return error" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void QueryTest::putRequestJson()
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

void QueryTest::putRequestBson()
{
  QNetworkRequest req;
  req.setRawHeader( "accept", "application/bson" );

  const auto reply = put( url, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto body = reply->readAll();
  const auto option = bsoncxx::validate( reinterpret_cast<const uint8_t*>( body.data() ), body.size() );
  QVERIFY2( option.has_value(), "Response not BSON" );
  QVERIFY2( option->find( "cause" ) != option->end(), "Error response does not have cause" );
}

void QueryTest::deleteRequestJson()
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

void QueryTest::deleteRequestBson()
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

void QueryTest::cleanupTestCase()
{
  remove( entityId );
}
