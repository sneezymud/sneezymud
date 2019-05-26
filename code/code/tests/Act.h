#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "person.h"
#include "extern.h"
#include "charfile.h"
#include "code/tests/ValueTraits.h"
#include "code/tests/MockDb.h"
#include "connect.h"
#include "socket.h"
#include "player_data.h"

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
    chdir("../lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);

    testSocket=new TSocket();
    testRoom=new TRoom(100);
    testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

    testDesc=new Descriptor(testSocket);
    testPerson=new TPerson(testDesc);
    load_char("test", &st, std::unique_ptr<MockDb>());
    testPerson->loadFromSt(&st);
    testPerson->in_room=0;
    *testRoom += *testPerson;

    testDesc2=new Descriptor(testSocket);
    testPerson2=new TPerson(testDesc2);
    load_char("testone", &st, std::unique_ptr<MockDb>());
    testPerson2->loadFromSt(&st);
    testPerson2->in_room=0;
    *testRoom += *testPerson2;

    testHorseDesc=new Descriptor(testSocket);
    testHorse=new TPerson(testHorseDesc);
    load_char("testtwo", &st, std::unique_ptr<MockDb>());
    testHorse->loadFromSt(&st);
    testHorse->in_room=0;
    *testRoom += *testHorse;
  }

  void testSimple(){
    CommPtr c;

    act("You start riding $N.", FALSE, testPerson, 0, testHorse, TO_CHAR, NULL, -1);
    act("$n starts riding $N.", FALSE, testPerson, 0, testHorse, TO_NOTVICT, NULL, -1);
    act("$n hops on your back!", FALSE, testPerson, 0, testHorse, TO_VICT, NULL, -1);

    TS_ASSERT(!testPerson->desc->output.empty());
    TS_ASSERT_EQUALS(testPerson->desc->output.front()->getComm(), "You start riding Testone.\n\r");

    TS_ASSERT(!testPerson2->desc->output.empty());
    TS_ASSERT_EQUALS(testPerson2->desc->output.front()->getComm(), "Test starts riding Testone.\n\r");

    TS_ASSERT(!testHorse->desc->output.empty());
    TS_ASSERT_EQUALS(testHorse->desc->output.front()->getComm(), "Test hops on your back!\n\r");

  }
};
