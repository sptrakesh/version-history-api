//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class QueryTest : public BaseTest
  {
    Q_OBJECT

  public:
    QueryTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void initTestCase();
    void getRequestJson();
    void getRequestBson();
    void optionsRequest();
    void queryIdJson();
    void queryIdBson();
    void queryPropertyJson();
    void queryPropertyBson();
    void queryIdProjectionJson();
    void queryIdProjectionBson();
    void queryProjectionJson();
    void queryProjectionBson();
    void invalidRequestJson();
    void invalidRequestBson();
    void putRequestJson();
    void putRequestBson();
    void deleteRequestJson();
    void deleteRequestBson();
    void cleanupTestCase();

  private:
    QString url{ "http://localhost:6106/crud/query/itest/test" };
    QString entityId;
  };
}

DECLARE_TEST( spt::QueryTest )
