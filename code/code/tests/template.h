#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"

class template : public CxxTest::TestSuite
{

 public:
  void testAddition(){
    TS_ASSERT_EQUALS(1+1, 2);
  }
};
