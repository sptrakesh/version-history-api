#include "basetest.h"

#include <QtCore/QEventLoop>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QTest>

#include <bsoncxx/json.hpp>
#include <bsoncxx/validate.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>

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
    QNetworkRequest* req, const QString& contentType )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, contentType );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.post( *req, data ) };
  eventLoop.exec();
  return rptr ;
}

BaseTest::ReplyPointer BaseTest::put( const QString& url, const QByteArray& data,
    QNetworkRequest* req, const QString& contentType )
{
  QEventLoop eventLoop;
  connect( &mgr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit );

  req->setUrl( QUrl( url ) );
  req->setHeader( QNetworkRequest::ContentTypeHeader, contentType );
  req->setAttribute( QNetworkRequest::Http2DirectAttribute, {true} );
  ReplyPointer rptr{ mgr.put( *req, data ) };
  eventLoop.exec();
  return rptr ;
}

QByteArray BaseTest::execute( const QByteArray& payload )
{
  QTcpSocket socket;
  socket.connectToHost( "localhost", 2000 );
  if ( socket.waitForConnected( 1000 ) )
  {
    socket.write( payload );
    socket.waitForBytesWritten( 1000 );
    socket.waitForReadyRead( 1000 );
    auto data = socket.readAll();
    socket.close();
    return data;
  }
  else
  {
    qDebug() << Q_FUNC_INFO << "Unable to connect to mongo service";
  }

  return {};
}

void BaseTest::remove( const QString& entityId )
{
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::open_document;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::finalize;

  auto req = document{} <<
    "action" << "delete" <<
    "database" << "itest" <<
    "collection" << "test" <<
    "document" << open_document << "_id" << bsoncxx::oid{ entityId.toStdString() } << close_document <<
    finalize;
  const auto reqv = req.view();

  auto payload = QByteArray( reinterpret_cast<const char*>( reqv.data() ), reqv.length() );
  auto response = execute( payload );
  auto opt = bsoncxx::validate( reinterpret_cast<const uint8_t*>( response.data() ), response.size() );
  QVERIFY2( opt, "Error deleting test document" );
  QVERIFY2( opt->find( "err" ) == opt->end(), "Deleting document returned error" );
  qDebug() << QString::fromStdString( bsoncxx::to_json( *opt ) );
}
