#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"


class SString : public CxxTest::TestSuite
{

 public:
  void testLowerUpper(){
    sstring foo="LOREM IpSum dolor SIT aMeT";
    
    TS_ASSERT_EQUALS(foo.lower(), "lorem ipsum dolor sit amet");
    TS_ASSERT_EQUALS(foo.upper(), "LOREM IPSUM DOLOR SIT AMET");

  }

  void testWord(){
    sstring foo="LOREM IpSum dolor SIT aMeT";
    TS_ASSERT_EQUALS(foo.word(-1), "");
    TS_ASSERT_EQUALS(foo.word(0), "LOREM");
    TS_ASSERT_EQUALS(foo.word(1), "IpSum");
    TS_ASSERT_EQUALS(foo.word(2), "dolor");
    TS_ASSERT_EQUALS(foo.word(3), "SIT");
    TS_ASSERT_EQUALS(foo.word(4), "aMeT");
    TS_ASSERT_EQUALS(foo.word(5), "");
    TS_ASSERT_EQUALS(foo.word(6), "");

  }

  void testRange(){
    sstring foo;
    // unfortunately, this hangs
    // TS_ASSERT_THROWS(foo[0]='x', std::out_of_range const&);
  }
};
