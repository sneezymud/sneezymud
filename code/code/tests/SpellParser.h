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
#include "spells.h"

#include <iostream>

class SpellParser : public CxxTest::TestSuite
{
  TPerson* testPerson = nullptr;
  Descriptor* testDesc = nullptr;
  TRoom* testRoom = nullptr;
 public:

  void testParseSpellNum()
  {
    TS_ASSERT_EQUALS(TYPE_UNDEFINED, testPerson->parseSpellNum(sstring("blarg").words())); // TODO: something's broken, it should return "no such spell"

    TS_ASSERT_EQUALS(TYPE_UNDEFINED, testPerson->parseSpellNum(sstring("yada blarg").words())); // TODO: something's broken, it should return "no such spell"

    // without target
    TS_ASSERT_EQUALS(SPELL_FLY, testPerson->parseSpellNum(sstring("flight").words())); // 1 word
    TS_ASSERT_EQUALS(SPELL_TRUE_SIGHT, testPerson->parseSpellNum(sstring("  true sight  ").words())); // 2 words
    TS_ASSERT_EQUALS(SPELL_PROTECTION_FROM_FIRE, testPerson->parseSpellNum(sstring("protection from fire   ").words())); // 3 words

    // with target
    TS_ASSERT_EQUALS(SPELL_FLY, testPerson->parseSpellNum(sstring("flight victim").words())); // 1 word
    TS_ASSERT_EQUALS(SPELL_TRUE_SIGHT, testPerson->parseSpellNum(sstring("  true sight   victim").words())); // 2 words
    TS_ASSERT_EQUALS(SPELL_PROTECTION_FROM_FIRE, testPerson->parseSpellNum(sstring("protection from fire    victim").words())); // 3 words
  }

  void setUp()
  {
    using namespace std;
    Config::doConfiguration();

    buildSpellArray();
    chdir("../lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);
    testRoom = new TRoom(100);
    testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);
    testDesc = new Descriptor(new TSocket()); // Descriptor deallocates the socket
    testPerson = new TPerson(testDesc);
    charFile st;
    load_char("test", &st, std::unique_ptr<MockDb>());
    testPerson->loadFromSt(&st);
    testPerson->in_room=0;
    testPerson->discs->disc[DISC_AIR]->setLearnedness(MAX_DISC_LEARNEDNESS);
    testPerson->discs->disc[DISC_SPIRIT]->setLearnedness(MAX_DISC_LEARNEDNESS);
    testPerson->discs->disc[DISC_MAGE]->setLearnedness(MAX_DISC_LEARNEDNESS);
  }

  void tearDown()
  {
    delete testPerson;
    delete testDesc;
    delete testRoom;
    delete Races[RACE_HUMAN];
  }
};
