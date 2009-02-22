#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class SString : public CxxTest::TestSuite
{

 public:
  void testRange(){
    sstring foo;
    TS_ASSERT_THROWS(foo[0]='x', std::out_of_range);

    string bar;
    bar[0]='x';
    
    sstring baz;
    TS_ASSERT_THROWS(baz.c_str(), std::runtime_error);
  }
};
