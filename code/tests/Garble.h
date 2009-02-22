#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "tests/ValueTraits.h"

class Garble : public CxxTest::TestSuite
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


 public:
  void testNoExceptions(){
    TS_ASSERT_THROWS_NOTHING(garble_olddrunk(testPerson1, testPerson2, "some test string", SPEECH_WHISPER));
  }
};
