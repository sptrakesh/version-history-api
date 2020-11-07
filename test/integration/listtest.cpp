//
// Created by Rakesh on 06/11/2020.
//

#include "listtest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using spt::ListTest;

void ListTest::getRequest()
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

void ListTest::optionsRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "OPTIONS", &req );
  QVERIFY2( reply->error() == QNetworkReply::NoError, "Error making OPTIONS request" );
}

void ListTest::postRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = post( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "POST allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::invalidRequest()
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

void ListTest::putRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = put( endpoint, {}, &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "PUT allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}

void ListTest::deleteRequest()
{
  QNetworkRequest req;
  req.setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );

  const QString entityId = "5f3bc9e2502422053e08f9f1";
  const auto endpoint = QString( "%1itest/test/%4" ).arg( url ).arg( entityId );
  const auto reply = custom( endpoint, "DELETE", &req );

  QVERIFY2( reply->error() != QNetworkReply::NoError, "DELETE allowed on api endpoint" );
  const auto doc = QJsonDocument::fromJson( reply->readAll() );
  const auto obj = doc.object();
  QVERIFY2( !obj.isEmpty(), "Empty error response for api endpoint" );
  QVERIFY2( !obj["cause"].toString().isEmpty(), "Error response does not have cause" );
}
