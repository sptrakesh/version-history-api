//
// Created by Rakesh on 21/11/2020.
//

#include "basetest.h"

namespace spt
{
  class CreateTest : public BaseTest
  {
  Q_OBJECT

  public:
    CreateTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void getRequestJson();
    void getRequestBson();
    void optionsRequest();
    void postRequestJson();
    void postRequestBson();
    void invalidRequestJson();
    void invalidRequestBson();
    void putRequestJson();
    void putRequestBson();
    void deleteRequestJson();
    void deleteRequestBson();
    void cleanupTestCase();

  private:
    QString url{ "http://localhost:6106/crud" };
    QString entityId1;
    QString entityId2;
  };
}

DECLARE_TEST( spt::CreateTest )
