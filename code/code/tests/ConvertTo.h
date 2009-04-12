#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "parse.h"

class ConvertTo : public CxxTest::TestSuite
{

 public:
  void setUp(){
    freopen("code/tests/output/ConvertTo.out", "w", stderr);
  }

  void testNumTypes(){
    TS_ASSERT_EQUALS(5, convertTo<int>("5"));
    TS_ASSERT_EQUALS(5, convertTo<unsigned int>("5"));
    TS_ASSERT_EQUALS(5.5, convertTo<float>("5.5"));
    TS_ASSERT_EQUALS(5.5, convertTo<double>("5.5"));

  }
};
