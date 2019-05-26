#include <cxxtest/TestSuite.h>
#include "code/tests/ValueTraits.h"
#include "configuration.h"
#include "person.h"
#include "connect.h"
#include "socket.h"

class CommTest : public CxxTest::TestSuite
{
  public:
    sstring testString[4];
    TSocket *testSocket;
    Descriptor *testDesc;
    TPerson *testPerson;

    void setUp(){
      Config::doConfiguration();
      freopen("code/tests/output/CommTest.out", "w", stderr);

      testString[0]="holding up my";
      testString[1]="purring cat to the moon";
      testString[2]="I sighed.";
      testString[3]="C-C-C-C-C-Combo breaker!";

      testSocket=new TSocket();
      testDesc=new Descriptor(testSocket);
      testPerson=new TPerson(testDesc);
    }

    void tearDown(){
    }

    void testUncategorizedComm(){
      testPerson->desc->output.push(CommPtr(new UncategorizedComm(testString[0])));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(), testString[0]);
    }

    void testColoredComm(){
      testPerson->desc->output.push(CommPtr(new UncategorizedComm("this is <r>a<1> test")));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(),
          "this is <r>a<1> test");
    }

    void testAnsiColoredComm(){
      testPerson->desc->output.push(CommPtr(new UncategorizedComm(format("this is %sa%s test") % ANSI_RED % ANSI_NORMAL)));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(),
          (format("this is %sa%s test") % ANSI_RED % ANSI_NORMAL).str());
    }


    void testSystemLogComm(){
      testPerson->desc->output.push(CommPtr(new SystemLogComm(time(0), LOG_PIO, testString[1])));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(),
          (format("// Player I/O: %s\n\r") % testString[1]).str());
    }

    void testTellFromComm(){
      testPerson->desc->output.push(CommPtr(new TellFromComm("Deirdre", "Peel",
            format("%s, %s, %s") %
            testString[0] % testString[1] % testString[2],
            true, false)));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(),
          (format("<p>Peel<z> tells you, \"<c>%s, %s, %s<z>\"\n\r") %
           testString[0] % testString[1] % testString[2]).str());
    }

    void testTellToComm(){
      testPerson->desc->output.push(CommPtr(new TellToComm("Deirdre", "Peel",
            (format("%s, %s, %s") %
             testString[0] % testString[1] % testString[2]).str())));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(),
          (format("<G>You tell Deirdre<z>, \"%s, %s, %s\"\n\r") %
           testString[0] % testString[1] % testString[2]).str());
    }

    void testWhoListRemoveComm(){
      testPerson->desc->output.push(CommPtr(new WhoListComm("Peel", false, 60, 100,
            true, "The Freshmaker", "Peel keeps her warm but they never kiss.")));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(), "");

    }

    void testWhoListAddComm(){
      testPerson->desc->output.push(CommPtr(new WhoListComm("Peel", false, 60, 100,
            false, "The Freshmaker", "Peel keeps her warm but they never kiss.")));
      CommPtr comm=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(comm->getComm(), "");

    }

    void testPagedOutput(){
      testPerson->desc->output.push(CommPtr(new UncategorizedComm(testString[0])));
      testPerson->desc->output.push(CommPtr(new UncategorizedComm(testString[1])));
      testPerson->desc->output.push(CommPtr(new UncategorizedComm(testString[2])));

      testSocket->port=Config::Port::PROD; // makeOutputPaged checks this
      testPerson->makeOutputPaged();

      TS_ASSERT_EQUALS(testPerson->desc->output.front()->getComm(), (format("%s%s%s") %
            testString[0] % testString[1] % testString[2]).str());

    }


    void testSoundComm(){
      CommPtr c;
      auto sound = CommPtr(new SoundComm("sound", "", "cackle.wav", "socials", -1, -1, -1, -1));

      testPerson->sendTo(sound);
      c=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(c->getComm(), "!!SOUND(cackle.wav T=socials)\n\r");

      auto sound2 = CommPtr(new SoundComm("sound", "http://sneezymud.com/sounds/", "Off", "", -1,-1,-1, -1));
      testPerson->sendTo(sound2);
      c=testPerson->desc->output.front();
      testPerson->desc->output.pop();

      TS_ASSERT_EQUALS(c->getComm(), "!!SOUND(Off U=http://sneezymud.com/sounds/)\n\r");
    }


};
