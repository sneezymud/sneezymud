#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "charfile.h"
#include "person.h"
#include "account.h"
#include "extern.h"
#include "code/tests/ValueTraits.h"
#include "socket.h"

class Cast : public CxxTest::TestSuite
{

 public:
  TSocket *testSocket;
  Descriptor *testDesc;
  TPerson *testPerson;
  TRoom *testRoom;
  charFile st;

  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Cast.out", "w", stderr);

    buildSpellArray();
    chdir("lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);    

    generate_obj_index();

    testSocket=new TSocket();
    testRoom=new TRoom(100);
    testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

    testDesc=new Descriptor(testSocket);
    testDesc->connected=CON_PLYNG;
    testDesc->account=new TAccount();
    testPerson=new TPerson(testDesc);
    testDesc->character=testPerson;
    load_char("peel", &st);
    testPerson->loadFromSt(&st);
    testPerson->in_room=0;
    testPerson->next=character_list;
    character_list=testPerson;
    *testRoom += *testPerson;
  }

  void testFindCompInventory(){
    TObj *junk=read_object(23641, VIRTUAL);
    TObj *comp=read_object(203, VIRTUAL);
    TObj *comp2=read_object(269, VIRTUAL);
    TObj *bag=read_object(323, VIRTUAL);

    TS_ASSERT(bag);
    TS_ASSERT(comp);
    TS_ASSERT(junk);

    *testPerson += *junk;

    *testPerson += *comp;
    TS_ASSERT(testPerson->findComponent(SPELL_TORNADO));

    *bag += *comp2;
    *testPerson += *bag;
    TS_ASSERT(testPerson->findComponent(SPELL_TSUNAMI));

  }
};
