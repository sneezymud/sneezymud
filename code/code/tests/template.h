#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "configuration.h"
//#include "extern.h"
//#include "toggle.h"

class template : public CxxTest::TestSuite
{

 public:
  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/template.out", "w", stderr);
    //    generate_obj_index();
    //    toggleInfo.loadToggles();
  }

  void testAddition(){
    TS_ASSERT_EQUALS(1+1, 2);
  }
};
