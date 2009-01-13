#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "database.h"

class DBTest : public CxxTest::TestSuite
{
 public:
  void testProdConnection(){
    TDatabase db(DB_SNEEZY);
    
    db.query("select 42 as answer");
    db.fetchRow();

    TS_ASSERT_EQUALS(convertTo<int>(db["answer"]), 42);
  }
};
