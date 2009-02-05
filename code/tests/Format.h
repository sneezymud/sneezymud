#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class Format : public CxxTest::TestSuite
{

 public:
  void testXML(){
    TS_ASSERT_EQUALS(fmt("%x") % "<r><red><z>",
		     "<font color=\"norm\" /><font color=\"red\" />&#60;red&#62;<font color=\"norm\" />");
  }
};
