#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"
#include "configuration.h"
#include "socket.h"
#include "charfile.h"
#include "person.h"
#include "account.h"
#include "extern.h"

class Pfile : public CxxTest::TestSuite
{
 public:
  TSocket *tSocket;
  Descriptor *tDesc;
  TPerson *tPerson;
  charFile st;

  static Pfile *createSuite(){ 
    Config::doConfiguration();
    freopen("code/tests/output/Pfile.out", "w", stderr);
    buildSpellArray();
    chdir("lib");
    Races[RACE_HUMAN] = new Race(RACE_HUMAN);    

    return new Pfile;
  }
  static void destroySuite(Pfile *suite){ 
    delete suite; 
  }

  void setUp(){
    CxxTest::setAbortTestOnFail(true);
  }

  void testSocket(){
    tSocket=new TSocket();
    TS_ASSERT(tSocket);
  }    

  void testDescriptor(){
    tDesc=new Descriptor(tSocket);
    TS_ASSERT(tDesc);

    tDesc->connected=CON_PLYNG;
  }

  void testAccount(){
    tDesc->account=new TAccount();
    TS_ASSERT(tDesc);
  }

  void testPerson(){
    tPerson=new TPerson(tDesc);
    TS_ASSERT(tPerson);
   
    tDesc->character=tPerson;
  }

  void testLoadChar(){
    TS_ASSERT(load_char("peel", &st));
  }

    /*
      // loadFromSt has no error checking apparently
    tPerson2->loadFromSt(&st);
    tPerson2->in_room=0;
    tPerson2->next=character_list;
    character_list=tPerson2;
    *testRoom += *tPerson2;
    */
};
