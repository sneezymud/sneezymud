#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "socket.h"
#include "code/tests/ValueTraits.h"
#include "spec_mobs.h"
#include "person.h"
#include "extern.h"

class Exceptions : public CxxTest::TestSuite
{
  TSocket *testSocket;
  Descriptor *testDesc;
  TPerson *testPerson1;
  TPerson *testPerson2;

  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Exceptions.out", "w", stderr);
    testSocket=new TSocket();
    testDesc=new Descriptor(testSocket);
    testPerson1=new TPerson(testDesc);
    testPerson2=new TPerson(testDesc);
    gamePort=Config::Port::BETA;
  }

  // I added an exception throw to out of range access of sstrings,
  // so these tests were originally added to test bug fixes for that


 public:
  void testProdVSBeta(){
    sstring foo;

    TS_ASSERT_THROWS(foo[0], std::out_of_range);

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
