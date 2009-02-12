#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class Format : public CxxTest::TestSuite
{

 public:
  void testXML(){
    TS_ASSERT_EQUALS(fmt("%x") % "<r><red><z>",
		     "<font color=\"norm\" /><font color=\"red\" />&#60;red&#62;<font color=\"norm\" />");
    TS_ASSERT_EQUALS(fmt("%x") % "<<C>onnect> <r>red<z> <<C>onnect",
		     "&#60;C&#62;onnect&#62; <font color=\"norm\" /><font color=\"red\" />red<font color=\"norm\" /> &#60;C&#62;onnect");
  }
};
