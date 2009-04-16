#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "person.h"
#include "extern.h"
#include "charfile.h"
#include "code/tests/ValueTraits.h"
#include "connect.h"
#include "socket.h"

class Act : public CxxTest::TestSuite
{
 public:
  TSocket *testSocket;
  Descriptor *testDesc;
  Descriptor *testDesc2;
  Descriptor *testHorseDesc;
  TPerson *testPerson;
  TPerson *testPerson2;
  TPerson *testHorse;
  TRoom *testRoom;
  charFile st;

  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Act.out", "w", stderr);

    buildSpellArray();
    chdir("lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);    

    testSocket=new TSocket();
    testRoom=new TRoom(100);
    testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

    testDesc=new Descriptor(testSocket);
    testPerson=new TPerson(testDesc);
    load_char("dante", &st);
    testPerson->loadFromSt(&st);
    testPerson->in_room=0;
    *testRoom += *testPerson;

    testDesc2=new Descriptor(testSocket);
    testPerson2=new TPerson(testDesc2);
    load_char("milton", &st);
    testPerson2->loadFromSt(&st);
    testPerson2->in_room=0;
    *testRoom += *testPerson2;

    testHorseDesc=new Descriptor(testSocket);
    testHorse=new TPerson(testHorseDesc);
    load_char("chaucer", &st);
    testHorse->loadFromSt(&st);
    testHorse->in_room=0;
    *testRoom += *testHorse;
  }

  void testSimple(){
    Comm *c;

    act("You start riding $N.", FALSE, testPerson, 0, testHorse, TO_CHAR, NULL, -1);
    act("$n starts riding $N.", FALSE, testPerson, 0, testHorse, TO_NOTVICT, NULL, -1);
    act("$n hops on your back!", FALSE, testPerson, 0, testHorse, TO_VICT, NULL, -1);

    if(!(c=testPerson->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), "You start riding Chaucer.\n\r");

    if(!(c=testPerson2->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), "Dante starts riding Chaucer.\n\r");

    if(!(c=testHorse->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), "Dante hops on your back!\n\r");

  }
};
