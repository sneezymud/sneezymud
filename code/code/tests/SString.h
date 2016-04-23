#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "timing.h"


class SString : public CxxTest::TestSuite
{

 public:
  void setUp(){
    freopen("code/tests/output/SString.out", "w", stderr);
  }

  void testLowerUpper(){
    sstring foo="LOREM IpSum dolor SIT aMeT";
    
    TS_ASSERT_EQUALS(foo.lower(), "lorem ipsum dolor sit amet");
    TS_ASSERT_EQUALS(foo.upper(), "LOREM IPSUM DOLOR SIT AMET");

  }

  void testRange(){
    sstring foo;
    TS_ASSERT_THROWS(foo[0]='x', std::out_of_range);

    std::string bar;
    bar[0]='x';
    
    sstring baz;
    TS_ASSERT_THROWS(baz.c_str(), std::runtime_error);
    bar[0]='\0';
  }
};
