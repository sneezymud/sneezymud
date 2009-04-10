#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "configuration.h"
#include "code/tests/ValueTraits.h"
#include "socket.h"

class Garble : public CxxTest::TestSuite
{
 public:
  sstring testString[4];
  TSocket *testSocket;
  Descriptor *testDesc;
  TPerson *testPerson;
  TRoom *testRoom;
  charFile st;

  void setUp(){
    doConfiguration();
    freopen("code/tests/output/Garble.out", "w", stderr);
    testString[0]="holding up my";
    testString[1]="purring cat to the moon";
    testString[2]="I sighed.";
    testString[3]="C-C-C-C-C-Combo breaker!";

    buildSpellArray();
    chdir("lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);    

    testSocket=new TSocket();
    testDesc=new Descriptor(testSocket);
    testPerson=new TPerson(testDesc);

    load_char("killer", &st);
    testPerson->loadFromSt(&st);
    testRoom=new TRoom(100);
    *testRoom += *testPerson;
  }

  void testGarbles(){
    for(int i=0;i<(int)SPEECH_MAX;++i){
      for(int j=0;j<(int)GARBLE_MAX;++j){
	testPerson->toggleGarble((GARBLETYPE)j);

	TSM_ASSERT_THROWS_NOTHING(format("SPEECHTYPE = %i, GARBLETYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (SPEECHTYPE)i, GARBLE_SCOPE_INDIVIDUAL));
	TSM_ASSERT_THROWS_NOTHING(format("SPEECHTYPE = %i, GARBLETYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (SPEECHTYPE)i, GARBLE_SCOPE_SELF));
	TSM_ASSERT_THROWS_NOTHING(format("SPEECHTYPE = %i, GARBLETYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (SPEECHTYPE)i, GARBLE_SCOPE_EVERYONEANDSELF));
	TSM_ASSERT_THROWS_NOTHING(format("SPEECHTYPE = %i, GARBLETYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (SPEECHTYPE)i, GARBLE_SCOPE_ALL));
	TSM_ASSERT_THROWS_NOTHING(format("SPEECHTYPE = %i, GARBLETYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (SPEECHTYPE)i, GARBLE_SCOPE_EVERYONE));
      }
    }
  }
};
