#pragma once

#include "autotest.h"

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace spt
{
  class BaseTest : public QObject
  {
    Q_OBJECT
  public:
    using ReplyPointer = std::unique_ptr<QNetworkReply>;

  protected:
    BaseTest( QObject* parent = nullptr ) : QObject( parent ) {}

    ReplyPointer get( const QString& url, QNetworkRequest* req );

    ReplyPointer custom( const QString& url, const QByteArray& verb,
        QNetworkRequest* req, const QByteArray& data = {} );

    ReplyPointer post( const QString& url, const QByteArray& data,
        QNetworkRequest* req );

    ReplyPointer put( const QString& url, const QByteArray& data,
        QNetworkRequest* req );

  private:
    QNetworkAccessManager mgr;
  };
}
