//
// Replaced by Rakesh on 21/11/2020.
//

#include "basetest.h"

namespace spt
{
  class ReplaceTest : public BaseTest
  {
  Q_OBJECT

  public:
    ReplaceTest( QObject* parent = nullptr ) : BaseTest( parent ) {}

  private slots:
    void initTestCase();
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
    QString url{ "http://localhost:6106/crud/replace/itest/test" };
    QString entityId;
  };
}

DECLARE_TEST( spt::ReplaceTest )
