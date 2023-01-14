#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "charfile.h"
#include "person.h"
#include "account.h"
#include "socket.h"
#include "code/tests/ValueTraits.h"
#include "code/tests/MockDb.h"
#include "player_data.h"

class Tell : public CxxTest::TestSuite {
  public:
    TSocket* testSocket;
    Descriptor* testDesc;
    Descriptor* testDesc2;
    Descriptor* testDesc3;
    TPerson* testPerson;
    TPerson* testPerson2;
    TPerson* testPerson3;
    TRoom* testRoom;
    charFile st;

    void setUp() {
      Config::doConfiguration();
      freopen("code/tests/output/Tell.out", "w", stderr);
      buildSpellArray();
      chdir("../lib");
      Races[RACE_HUMAN] = new Race(RACE_HUMAN);

      testSocket = new TSocket();
      testRoom = new TRoom(100);
      testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

      // 2 accounts
      // 1 account has 2 users: testa, testb. testa is logged in, testb is not
      // logged in As immortal, send tell to testb. Get message about testa
      // being logged in instead. As mortal, send tell to testb. Get no message
      // about testa being logged in instead.

      testDesc = new Descriptor(testSocket);
      testDesc->connected = CON_PLYNG;
      testDesc->account = new TAccount();
      testPerson = new TPerson(testDesc);
      testDesc->character = testPerson;
      load_char("anotherone", &st, std::make_unique<MockDb>());
      testPerson->loadFromSt(&st);
      testPerson->in_room = 0;
      testPerson->next = character_list;
      character_list = testPerson;
      *testRoom += *testPerson;

      testDesc2 = new Descriptor(testSocket);
      testDesc2->connected = CON_PLYNG;
      testDesc2->account = new TAccount();
      testPerson2 = new TPerson(testDesc2);
      testDesc2->character = testPerson2;
      load_char("imm", &st, std::make_unique<MockDb>());
      testPerson2->loadFromSt(&st);
      testPerson2->in_room = 0;
      testPerson2->next = character_list;
      character_list = testPerson2;
      *testRoom += *testPerson2;

      testDesc3 = new Descriptor(testSocket);
      testDesc3->connected = CON_PLYNG;
      testDesc3->account = new TAccount();
      testPerson3 = new TPerson(testDesc3);
      testDesc3->character = testPerson3;
      load_char("test", &st, std::make_unique<MockDb>());
      testPerson3->loadFromSt(&st);
      testPerson3->in_room = 0;
      testPerson3->next = character_list;
      character_list = testPerson3;
      *testRoom += *testPerson3;
    }

    void testImmNotFound() {
      testPerson2->doTell("AnotherTwo",
        "not logged in user but alt is logged in");

      TS_ASSERT(!testPerson2->desc->output.empty());
      TS_ASSERT_EQUALS("You fail to tell to 'AnotherTwo'\n\r",
        testPerson2->desc->output.front()->getComm());

      testPerson2->desc->output.pop();

      TS_ASSERT(!testPerson2->desc->output.empty());
      TS_ASSERT_EQUALS(
        "The player 'AnotherOne' is logged in under the same account.\n\r",
        testPerson2->desc->output.front()->getComm());
    }

    void testMortalNotFound() {
      // now make sure it doesn't work for mortals
      testPerson3->doTell("anothertwo",
        "not logged in user but alt is logged in");

      TS_ASSERT(!testPerson3->desc->output.empty());
      TS_ASSERT_EQUALS("You fail to tell to 'anothertwo'\n\r",
        testPerson3->desc->output.front()->getComm());

      testPerson3->desc->output.pop();

      if (!testPerson3->desc->output.empty())
        TS_FAIL((format("received data from output queue: '%s'") %
                 testPerson3->desc->output.front()->getComm())
                  .str());
    }

    void testNotLoggedIn() {
      // make sure it works with a player who isn't logged in at all
      testPerson2->doTell("testone", "test message");

      TS_ASSERT(!testPerson2->desc->output.empty());
      TS_ASSERT_EQUALS("You fail to tell to 'testone'\n\r",
        testPerson2->desc->output.front()->getComm());

      testPerson2->desc->output.pop();

      TS_ASSERT(testPerson2->desc->output.empty());
    }

    void testDoesntExist() {
      // and with a non-existent player
      testPerson2->doTell("notaplayer", "test message");

      TS_ASSERT(!testPerson2->desc->output.empty());
      TS_ASSERT_EQUALS("You fail to tell to 'notaplayer'\n\r",
        testPerson2->desc->output.front()->getComm());

      testPerson2->desc->output.pop();

      TS_ASSERT(testPerson2->desc->output.empty());
    }
};
