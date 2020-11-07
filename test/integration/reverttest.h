//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class RevertTest : public BaseTest
  {
    Q_OBJECT

  public:
    RevertTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void initTestCase();
    void getRequest();
    void getBson();
    void optionsRequest();
    void postRequest();
    void invalidRequest();
    void nonexistentRequest();
    void putRequest();
    void deleteRequest();
    void cleanupTestCase();

  private:
    QString url{ "http://localhost:6106/version/history/revert/" };
    QString listUrl{ "http://localhost:6106/version/history/list/" };
    QString historyId;
    const QString entityId = "5f3bc9e2502422053e08f9f1";
  };
}

DECLARE_TEST( spt::RevertTest )
