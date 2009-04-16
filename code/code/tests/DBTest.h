#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "database.h"
#include "timing.h"
#include "sstring.h"
#include "parse.h"

class DBTest : public CxxTest::TestSuite
{
 public:
  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/DBTest.out", "w", stderr);
  }

  void testRetrieve(){
    TDatabase db(DB_SNEEZY);

    db.query("select vnum, name, price from obj where vnum=13711");
    
    TS_ASSERT(db.fetchRow());

    TS_ASSERT_EQUALS(db["vnum"], "13711");
    TS_ASSERT_EQUALS(db["name"], "mace dark bloodspike spike");
    TS_ASSERT_EQUALS(db["price"], "37618");

    TS_ASSERT_EQUALS(db[0], "13711");
    TS_ASSERT_EQUALS(db[1], "mace dark bloodspike spike");
    TS_ASSERT_EQUALS(db[2], "37618");

  }

  void testProdConnection(){
    TDatabase db(DB_SNEEZY);
    
    db.query("select 42 as answer");
    db.fetchRow();

    TS_ASSERT_EQUALS(convertTo<int>(db["answer"]), 42);
  }
};
