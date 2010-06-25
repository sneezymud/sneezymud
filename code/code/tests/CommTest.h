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

  void testOutputQ(){
    //    outputQ q;
    Comm *c;

    // make sure the basic functions work on an empty queue
    {
      TS_ASSERT(testPerson->desc->output.takeFromQ()==NULL);
      TS_ASSERT(testPerson->desc->output.getBegin()==NULL);
      TS_ASSERT(testPerson->desc->output.getEnd()==NULL);
    }

    // basic queue test
    {
      testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
      testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
      testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[0]);
      
      // stick something in out of order
      testPerson->desc->output.putInQ(new UncategorizedComm(testString[3]));
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[1]);
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[2]);
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[3]);
    }

    // queue should be empty now (after operations)
    TS_ASSERT(testPerson->desc->output.takeFromQ()==NULL);

    // check if clear() works
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[0]);
    
    testPerson->desc->output.clear();
    TS_ASSERT(testPerson->desc->output.takeFromQ()==NULL);

  }

  void testUncategorizedComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT), testString[0]);
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML), 
		     (format("<uncategorized>%s</uncategorized>") % testString[0]).str());
  }

  void testColoredComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm("this is <r>a<1> test"));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT),
		     "this is <r>a<1> test");
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML),
		     "<uncategorized>this is <font color=\"norm\" /><font color=\"red\" />a<font color=\"norm\" /> test</uncategorized>");
  }

  void testAnsiColoredComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm(format("this is %sa%s test") % ANSI_RED % ANSI_NORMAL));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT),
		     (format("this is %sa%s test") % ANSI_RED % ANSI_NORMAL).str());
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML),
		     "<uncategorized>this is <font color=\"red\" />a<font color=\"norm\" /> test</uncategorized>");
  }


  void testSystemLogComm(){
    testPerson->desc->output.putInQ(new SystemLogComm(time(0), LOG_PIO, testString[1]));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT),
		     (format("// Player I/O: %s\n\r") % testString[1]).str());
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML),
		     (format("<log>\n  <time>%i</time>\n  <type>Player I/O</type>\n  <msg>%s</msg>\n</log>\n") % time(0) % testString[1]).str());
  }

  void testTellFromComm(){
    testPerson->desc->output.putInQ(new TellFromComm("Deirdre", "Peel",
						     format("%s, %s, %s") %
			    testString[0] % testString[1] % testString[2],
						     true, false));
    Comm *comm=testPerson->desc->output.takeFromQ();
    
    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT),
		     (format("<p>Peel<z> tells you, \"<c>%s, %s, %s<z>\"\n\r") %
    		     testString[0] % testString[1] % testString[2]).str());
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML),
		     (format("<tellfrom>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <drunk>true</drunk>\n  <mob>false</mob>\n  <tell>%s, %s, %s</tell>\n</tellfrom>\n") % testString[0] % testString[1] % testString[2]).str());
  }

  void testTellToComm(){
    testPerson->desc->output.putInQ(new TellToComm("Deirdre", "Peel",
						   (format("%s, %s, %s") %
						    testString[0] % testString[1] % testString[2]).str()));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT),
		     (format("<G>You tell Deirdre<z>, \"%s, %s, %s\"\n\r") %
    		     testString[0] % testString[1] % testString[2]).str());
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML),
		     (format("<tellto>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <tell>%s, %s, %s</tell>\n</tellto>\n") % testString[0] % testString[1] % testString[2]).str());
  }

  void testWhoListRemoveComm(){
    testPerson->desc->output.putInQ(new WhoListComm("Peel", false, 60, 100,
	 true, "The Freshmaker", "Peel keeps her warm but they never kiss."));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT), "");
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML), 
		     "<wholist>\n  <online>false</online>\n  <level>60</level>\n  <idle>100</idle>\n  <linkdead>true</linkdead>\n  <name>Peel</name>\n  <prof>The Freshmaker</prof>\n  <title>Peel keeps her warm but they never kiss.</title>\n</wholist>\n");

  }

  void testWhoListAddComm(){
    testPerson->desc->output.putInQ(new WhoListComm("Peel", false, 60, 100,
	false, "The Freshmaker", "Peel keeps her warm but they never kiss."));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(Comm::TEXT), "");
    TS_ASSERT_EQUALS(comm->getComm(Comm::XML), 
		     "<wholist>\n  <online>false</online>\n  <level>60</level>\n  <idle>100</idle>\n  <linkdead>false</linkdead>\n  <name>Peel</name>\n  <prof>The Freshmaker</prof>\n  <title>Peel keeps her warm but they never kiss.</title>\n</wholist>\n");

  }

  void testPagedOutput(){
    Comm *c;

    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));
    
    testSocket->port=Config::Port::PROD_XML; // makeOutputPaged checks this
    testPerson->makeOutputPaged();

    // for XML mode it shouldn't do anything
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[0]);
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[1]);
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), testString[2]);


    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));

    testSocket->port=Config::Port::PROD; // makeOutputPaged checks this
    testPerson->makeOutputPaged();
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), (format("%s%s%s") %
		     testString[0] % testString[1] % testString[2]).str());

  }


  void testPromptComm(){
    Comm *c;
    PromptComm *pc=new PromptComm(time(0), 100, 100, 100, 100, 100, 100, 100, "prompt> ");

    testPerson->sendTo(pc);

    c=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(c->getComm(Comm::XML), (format("<prompt time=\"%i\" hp=\"100\" mana=\"100\" piety=\"100.000000\" lifeforce=\"100\" moves=\"100\" money=\"100\" room=\"100\">prompt&#62; </prompt>") % time(0)).str());

  }

  void testRoomExitComm(){
    Comm *c;
    RoomExitComm *rec=new RoomExitComm();

    for(dirTypeT dir=MIN_DIR;dir<MAX_DIR;dir++){
      rec->exits[dir].exit=false;
      rec->exits[dir].door=false;
      rec->exits[dir].open=false;
    }

    rec->exits[DIR_NORTH].exit=true;
    rec->exits[DIR_NORTH].door=true;
    rec->exits[DIR_NORTH].open=true;
    
    testPerson->sendTo(rec);

    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(Comm::XML), "<roomexits>\n  <exit>\n    <direction>north</direction>\n    <door>\n      <open>true</open>\n    </door>\n  </exit>\n</roomexits>\n");

    
  }

  void testSoundComm(){
    Comm *c;
    SoundComm *sound=new SoundComm("sound", "", "cackle.wav", "socials", -1, -1, -1, -1);

    testPerson->sendTo(sound);
    c=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), "!!SOUND(cackle.wav T=socials)\n\r");
    TS_ASSERT_EQUALS(c->getComm(Comm::XML), "<sound type=\"sound\">\n  <file>cackle.wav</file>\n  <type>socials</type>\n</sound>\n");

    sound=new SoundComm("sound", "http://sneezymud.com/sounds/", "Off", "", -1,-1,-1, -1);
    testPerson->sendTo(sound);
    c=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(c->getComm(Comm::TEXT), "!!SOUND(Off U=http://sneezymud.com/sounds/)\n\r");
    TS_ASSERT_EQUALS(c->getComm(Comm::XML), "<sound type=\"sound\">\n  <file>Off</file>\n  <url>http://sneezymud.com/sounds/</url>\n</sound>\n");

  }
  

};
