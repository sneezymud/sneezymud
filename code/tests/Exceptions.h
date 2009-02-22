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
  }

  // I added an exception throw to out of range access of sstrings,
  // so these tests were originally added to test bug fixes for that


 public:
  void testGarble(){
    TS_ASSERT_THROWS_NOTHING(garble_olddrunk(testPerson1, testPerson2, "some test string", SPEECH_WHISPER));
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
};
