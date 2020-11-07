//
// Created by Rakesh on 06/11/2020.
//

#pragma once

#include "basetest.h"

namespace spt
{
  class ListTest : public BaseTest
  {
  Q_OBJECT

  public:
    ListTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void getRequest();
    void optionsRequest();
    void postRequest();
    void invalidRequest();
    void putRequest();
    void deleteRequest();

  private:
    QString url{ "http://localhost:6106/version/history/list/" };
  };
}

DECLARE_TEST( spt::ListTest )