#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "person.h"
#include "extern.h"
#include "charfile.h"
#include "code/tests/ValueTraits.h"
#include "socket.h"

class GarbleTest : public CxxTest::TestSuite
{
 public:
  sstring testString[4];
  TSocket *testSocket;
  Descriptor *testDesc;
  TPerson *testPerson;
  TRoom *testRoom;
  charFile st;

  void setUp(){
    Config::doConfiguration();
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
    for(int i=0;i<(int)Garble::Garble::SPEECH_MAX;++i){
      for(int j=0;j<(int)Garble::TYPE_MAX;++j){
	testPerson->toggleGarble((Garble::TYPE)j);

	TSM_ASSERT_THROWS_NOTHING(format("Garble::SPEECHTYPE = %i, Garble::TYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (Garble::SPEECHTYPE)i, Garble::SCOPE_INDIVIDUAL));
	TSM_ASSERT_THROWS_NOTHING(format("Garble::SPEECHTYPE = %i, Garble::TYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (Garble::SPEECHTYPE)i, Garble::SCOPE_SELF));
	TSM_ASSERT_THROWS_NOTHING(format("Garble::SPEECHTYPE = %i, Garble::TYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (Garble::SPEECHTYPE)i, Garble::SCOPE_EVERYONEANDSELF));
	TSM_ASSERT_THROWS_NOTHING(format("Garble::SPEECHTYPE = %i, Garble::TYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (Garble::SPEECHTYPE)i, Garble::SCOPE_ALL));
	TSM_ASSERT_THROWS_NOTHING(format("Garble::SPEECHTYPE = %i, Garble::TYPE = %i") % i % j, testPerson->garble(testPerson, testString[0], (Garble::SPEECHTYPE)i, Garble::SCOPE_EVERYONE));
      }
    }
  }
};
