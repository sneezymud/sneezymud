#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "configuration.h"
// #include "extern.h"
// #include "toggle.h"

class template : public CxxTest::TestSuite{

  public:
    // this function is called once, before any tests are run in this file
    static template * createSuite(){Config::doConfiguration();
freopen("code/tests/output/template.out", "w", stderr);

return new template;
}
// this function is called once, after all tests in this file have been run
static void destroySuite(template* suite) { delete suite; }

// this is called before EACH test is run
void setUp() {
  // If the tests rely on each other, then this stops testing if one fails
  //    CxxTest::setAbortTestOnFail(true);
}

// tests
void testAddition() { TS_ASSERT_EQUALS(1 + 1, 2); }
}
;
