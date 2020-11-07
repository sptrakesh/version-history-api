//
// Created by Rakesh on 07/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class EntityTest : public BaseTest
  {
    Q_OBJECT

  public:
    EntityTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

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

  private:
    QString url{ "http://localhost:6106/version/history/entity/" };
    QString listUrl{ "http://localhost:6106/version/history/list/" };
    QString historyId;
  };
}

DECLARE_TEST( spt::EntityTest )
