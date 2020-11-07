//
// Created by Rakesh on 07/11/2020.
//

#include "client.h"

#include <QtCore/QDebug>

using spt::test::MongoClient;

MongoClient::MongoClient( QObject* parent ) : QObject( parent )
{
  socket = new QTcpSocket( this );
}

QByteArray MongoClient::execute( const QByteArray& payload )
{
  socket->connectToHost( "localhost", 2000 );
  if ( socket->waitForConnected( 1000 ) )
  {
    socket->write( payload );
    socket->waitForBytesWritten( 1000 );
    socket->waitForReadyRead( 1000 );
    auto data = socket->readAll();
    socket->close();
    return data;
  }
  else
  {
    qDebug() << Q_FUNC_INFO << "Unable to connect to mongo service";
  }

  return {};
}

