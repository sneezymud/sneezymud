#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class Exceptions : public CxxTest::TestSuite
{
  TSocket *testSocket;
  Descriptor *testDesc;
  TPerson *testPerson1;
  TPerson *testPerson2;

  void setUp(){
    testSocket=new TSocket();
    testDesc=new Descriptor(testSocket);
    testPerson1=new TPerson(testDesc);
    testPerson2=new TPerson(testDesc);
    gamePort=BETA_GAMEPORT;
  }

  // I added an exception throw to out of range access of sstrings,
  // so these tests were originally added to test bug fixes for that


 public:
  void testProdVSBeta(){
    sstring foo;

    gamePort=PROD_GAMEPORT;
    vlogf(LOG_BUG, "(Exceptions.h) Out-of-range error expected here:");
    TS_ASSERT_THROWS_NOTHING(foo[0]);

    gamePort=BETA_GAMEPORT;
    TS_ASSERT_THROWS_ANYTHING(foo[0]);

  }

  void testRandomMessage(){
    for(int i=0;i<100;++i)
      TS_ASSERT_THROWS_NOTHING(randommessage("Peel"));
  }

  void testToCRLF(){
    sstring foo="\n";

    TS_ASSERT_THROWS_NOTHING(foo.toCRLF());

    foo="xxx\n";

    TS_ASSERT_THROWS_NOTHING(foo.toCRLF());
  }

  void testCap(){
    sstring foo;
    
    TS_ASSERT_THROWS_NOTHING(foo.cap());
  }
};
