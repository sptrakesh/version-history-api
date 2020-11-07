//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include <QtCore/QByteArray>
#include <QtNetwork/QTcpSocket>

namespace spt::test
{
  class MongoClient : public QObject
  {
    Q_OBJECT

  public:
    explicit MongoClient( QObject* parent = nullptr );

    MongoClient( const MongoClient& ) = delete;
    MongoClient& operator=( const MongoClient& ) = delete;

    QByteArray execute( const QByteArray& payload );

  private:
    QTcpSocket* socket;
  };
}
