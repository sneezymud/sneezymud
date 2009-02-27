#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class ConvertTo : public CxxTest::TestSuite
{

 public:
  void testNumTypes(){
    TS_ASSERT_EQUALS(5, convertTo<int>("5"));
    TS_ASSERT_EQUALS(5, convertTo<unsigned int>("5"));
    TS_ASSERT_EQUALS(5.5, convertTo<float>("5.5"));
    TS_ASSERT_EQUALS(5.5, convertTo<double>("5.5"));

  }
};
