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
    void getRequestJson();
    void getRequestBson();
    void optionsRequest();
    void postRequestJson();
    void postRequestBson();
    void invalidRequestJson();
    void invalidRequestBson();
    void nonexistentRequestJson();
    void nonexistentRequestBson();
    void putRequestJson();
    void putRequestBson();
    void deleteRequestJson();
    void deleteRequestBson();
    void cleanupTestCase();

  private:
    QString url{ "http://localhost:6106/version/history/revert/" };
    QString listUrl{ "http://localhost:6106/version/history/list/" };
    QString historyId;
    QString entityId;
  };
}

DECLARE_TEST( spt::RevertTest )
