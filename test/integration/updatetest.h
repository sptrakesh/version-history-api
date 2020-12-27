//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class UpdateTest : public BaseTest
  {
    Q_OBJECT

  public:
    UpdateTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

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
    void updateJson();
    void updateBson();
    void putRequestJson();
    void putRequestBson();
    void patchRequestJson();
    void patchRequestBson();
    void deleteRequestJson();
    void deleteRequestBson();
    void cleanupTestCase();

  private:
    QString url{ "http://localhost:6106/crud/update/itest/test" };
    QString entityId;
  };
}

DECLARE_TEST( spt::UpdateTest )
