//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class RetrieveTest : public BaseTest
  {
    Q_OBJECT

  public:
    RetrieveTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void initTestCase();
    void getRequestJson();
    void getRequestBson();
    void getByPropertyJson();
    void getByPropertyBson();
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
    QString url{ "http://localhost:6106/crud/retrieve/itest/test" };
    QString entityId;
  };
}

DECLARE_TEST( spt::RetrieveTest )
