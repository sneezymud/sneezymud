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

#include <iostream>

class AllocateDeallocateTPersonDescriptorAndDb : public CxxTest::TestSuite {
  public:
    void testSimple() {
      using namespace std;
      Config::doConfiguration();

      buildSpellArray();
      chdir("../lib");
      Races[RACE_HUMAN] = new Race(RACE_HUMAN);
      TRoom* testRoom = new TRoom(100);
      testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);
      Descriptor* testDesc =
        new Descriptor(new TSocket());  // Descriptor deallocates the socket
      TPerson* testPerson = new TPerson(testDesc);
      charFile st;
      load_char("test", &st, std::unique_ptr<MockDb>());
      testPerson->loadFromSt(&st);
      testPerson->in_room = 0;

      // adding and removing someone into a room:
      // whoever thought it's a neat idea to use operator-- to signify removing
      // item from room?
      *testRoom += *testPerson;
      --(*testPerson);

      delete testPerson;
      delete testDesc;
      delete testRoom;
      delete Races[RACE_HUMAN];

      // no asserts. Run with asan to check for memory leaks.
      return;
    }
};
