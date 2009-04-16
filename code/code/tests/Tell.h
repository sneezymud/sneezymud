#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "charfile.h"
#include "person.h"
#include "account.h"
#include "socket.h"
#include "code/tests/ValueTraits.h"
#include "extern.h"

class Tell : public CxxTest::TestSuite
{
 public:
  TSocket *testSocket;
  Descriptor *testDesc;
  Descriptor *testDesc2;
  Descriptor *testDesc3;
  TPerson *testPerson;
  TPerson *testPerson2;
  TPerson *testPerson3;
  TRoom *testRoom;
  charFile st;

  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Tell.out", "w", stderr);
    buildSpellArray();
    chdir("lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);    

    testSocket=new TSocket();
    testRoom=new TRoom(100);
    testRoom->setRoomFlagBit(ROOM_ALWAYS_LIT);

    testDesc=new Descriptor(testSocket);
    testDesc->connected=CON_PLYNG;
    testDesc->account=new TAccount();
    testPerson=new TPerson(testDesc);
    testDesc->character=testPerson;
    load_char("dante", &st);
    testPerson->loadFromSt(&st);
    testPerson->in_room=0;
    testPerson->next=character_list;
    character_list=testPerson;
    *testRoom += *testPerson;

    testDesc2=new Descriptor(testSocket);
    testDesc2->connected=CON_PLYNG;
    testDesc2->account=new TAccount();
    testPerson2=new TPerson(testDesc2);
    testDesc2->character=testPerson2;
    load_char("peel", &st);
    testPerson2->loadFromSt(&st);
    testPerson2->in_room=0;
    testPerson2->next=character_list;
    character_list=testPerson2;
    *testRoom += *testPerson2;

    testDesc3=new Descriptor(testSocket);
    testDesc3->connected=CON_PLYNG;
    testDesc3->account=new TAccount();
    testPerson3=new TPerson(testDesc3);
    testDesc3->character=testPerson3;
    load_char("killer", &st);
    testPerson3->loadFromSt(&st);
    testPerson3->in_room=0;
    testPerson3->next=character_list;
    character_list=testPerson3;
    *testRoom += *testPerson3;


  }

  void testImmNotFound(){
    Comm *c;

    testPerson2->doTell("milton", "test message");
    
    if(!(c=testPerson2->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS("You fail to tell to 'milton'\n\r", c->getComm(Comm::TEXT));

    if(!(c=testPerson2->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS("The player 'Dante' is logged in under the same account.\n\r", c->getComm(Comm::TEXT));


    // now make sure it doesn't work for mortals
    testPerson3->doTell("milton", "test message");
    
    if(!(c=testPerson3->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS("You fail to tell to 'milton'\n\r", c->getComm(Comm::TEXT));

    if((c=testPerson3->desc->output.takeFromQ())){
      TS_FAIL(format("received data from output queue: '%s'") % c->getComm(Comm::TEXT));
    }

    // make sure it works with a player who isn't logged in at all
    testPerson2->doTell("pappy", "test message");
    
    if(!(c=testPerson2->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS("You fail to tell to 'pappy'\n\r", c->getComm(Comm::TEXT));

    if((c=testPerson3->desc->output.takeFromQ())){
      TS_FAIL(format("received data from output queue: '%s'") % c->getComm(Comm::TEXT));
    }

    // and with a non-existent player
    testPerson2->doTell("notaplayer", "test message");
    
    if(!(c=testPerson2->desc->output.takeFromQ()))
      TS_FAIL("received NULL from output queue");
    else
      TS_ASSERT_EQUALS("You fail to tell to 'notaplayer'\n\r", c->getComm(Comm::TEXT));

    if((c=testPerson3->desc->output.takeFromQ())){
      TS_FAIL(format("received data from output queue: '%s'") % c->getComm(Comm::TEXT));
    }




  }
};
