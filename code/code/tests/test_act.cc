#include "MockDb.h"
#include "charfile.h"
#include "configuration.h"
#include "connect.h"
#include "extern.h"
#include "person.h"
#include "player_data.h"
#include "socket.h"

#include <gtest/gtest.h>

class ActTest : public testing::Test {
  public:
    Descriptor* testDesc;
    Descriptor* testDesc2;
    Descriptor* testHorseDesc;
    TPerson* testPerson;
    TPerson* testPerson2;
    TPerson* testHorse;
    TRoom* testRoom;
    charFile st;

    ActTest() {
      Config::doConfiguration();

      buildSpellArray();
      chdir("../lib");
      Races[RACE_NORACE] = new Race(RACE_NORACE);
      Races[RACE_HUMAN] = new Race(RACE_HUMAN);

      testRoom = new TRoom(100);
      testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

      testDesc = new Descriptor(new TSocket());
      testPerson = new TPerson(testDesc);
      load_char("test", &st, std::make_unique<MockDb>());
      strcpy(st.name, "test");
      testPerson->loadFromSt(&st, std::make_unique<MockDb>());
      testPerson->in_room = 0;
      *testRoom += *testPerson;

      testDesc2 = new Descriptor(new TSocket());
      testPerson2 = new TPerson(testDesc2);
      load_char("testone", &st, std::make_unique<MockDb>());
      strcpy(st.name, "testone");
      testPerson2->loadFromSt(&st, std::make_unique<MockDb>());
      testPerson2->in_room = 0;
      *testRoom += *testPerson2;

      testHorseDesc = new Descriptor(new TSocket());
      testHorse = new TPerson(testHorseDesc);
      load_char("testtwo", &st, std::make_unique<MockDb>());
      strcpy(st.name, "TestHorse");
      testHorse->loadFromSt(&st, std::make_unique<MockDb>());
      testHorse->in_room = 0;
      *testRoom += *testHorse;
    }

    ~ActTest() {
      delete testHorse;
      delete testPerson2;
      delete testPerson;
      delete testDesc2;
      delete testDesc;
      delete testHorseDesc;
      delete testRoom;
      delete Races[RACE_HUMAN];
    }
};

TEST_F(ActTest, simple) {
  CommPtr c;

  act("You start riding $N.", FALSE, testPerson, 0, testHorse, TO_CHAR,
      NULL, -1);
  act("$n starts riding $N.", FALSE, testPerson, 0, testHorse, TO_NOTVICT,
      NULL, -1);
  act("$n hops on your back!", FALSE, testPerson, 0, testHorse, TO_VICT,
      NULL, -1);

  EXPECT_TRUE(!testPerson->desc->output.empty());
  EXPECT_STREQ("You start riding TestHorse.\n\r", testPerson->desc->output.front()->getComm().c_str());

  EXPECT_TRUE(!testPerson2->desc->output.empty());
  EXPECT_STREQ("Test starts riding TestHorse.\n\r", testPerson2->desc->output.front()->getComm().c_str());

  EXPECT_TRUE(!testHorse->desc->output.empty());
  EXPECT_STREQ("Test hops on your back!\n\r", testHorse->desc->output.front()->getComm().c_str());
}
