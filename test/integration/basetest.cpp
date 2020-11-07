#include "basetest.h"

#include <QtCore/QEventLoop>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtTest/QTest>

using spt::BaseTest;

BaseTest::ReplyPointer BaseTest::get( const QString& url, QNetworkRequest* req )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.get( *req ) };
  eventLoop.exec();
  return rptr ;
}

BaseTest::ReplyPointer BaseTest::custom( const QString& url, const QByteArray& verb,
    QNetworkRequest* req, const QByteArray& data )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.sendCustomRequest( *req, verb, data ) };
  eventLoop.exec();
  return rptr ;
}

BaseTest::ReplyPointer BaseTest::post( const QString& url, const QByteArray& data,
    QNetworkRequest* req )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.post( *req, data ) };
  eventLoop.exec();
  return rptr ;
}

BaseTest::ReplyPointer BaseTest::put( const QString& url, const QByteArray& data,
    QNetworkRequest* req )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.put( *req, data ) };
  eventLoop.exec();
  return rptr ;
}
