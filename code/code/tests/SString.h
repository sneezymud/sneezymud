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

  void testEscapeXML(){
    TS_ASSERT_EQUALS(((sstring)"<r><red><z>").escape(sstring::XML),
		     "<font color=\"norm\" /><font color=\"red\" />&#60;red&#62;<font color=\"norm\" />");
    TS_ASSERT_EQUALS(((sstring)"<<C>onnect> <r>red<z> <<C>onnect").escape(sstring::XML),
		     "&#60;C&#62;onnect&#62; <font color=\"norm\" /><font color=\"red\" />red<font color=\"norm\" /> &#60;C&#62;onnect");
    TS_ASSERT_EQUALS(((sstring)"<<C>onnect <C>onnect").escape(sstring::XML),
		     "&#60;C&#62;onnect <font style=\"bold\" color=\"cyan\" />onnect");    
  }

};
