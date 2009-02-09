#include <cxxtest/TestSuite.h>
#include "tests/ValueTraits.h"
#include "stdsneezy.h"
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
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[0]);
      
      // stick something in out of order
      testPerson->desc->output.putInQ(new UncategorizedComm(testString[3]));
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[1]);
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[2]);
      
      c=testPerson->desc->output.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[3]);
    }

    // queue should be empty now (after operations)
    TS_ASSERT(testPerson->desc->output.takeFromQ()==NULL);

    // check if clear() works
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[0]);
    
    testPerson->desc->output.clear();
    TS_ASSERT(testPerson->desc->output.takeFromQ()==NULL);

  }

  void testUncategorizedComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT), testString[0]);
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML), 
		     fmt("<uncategorized>%s</uncategorized>") % testString[0]);
  }

  void testColoredComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm("this is <r>a<1> test"));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT),
		     fmt("this is <r>a<1> test"));
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML),
		     "<uncategorized>this is <font color=\"norm\" /><font color=\"red\" />a<font color=\"norm\" /> test</uncategorized>");
  }

  void testAnsiColoredComm(){
    testPerson->desc->output.putInQ(new UncategorizedComm(fmt("this is %sa%s test") % ANSI_RED % ANSI_NORMAL));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT),
		     fmt("this is %sa%s test") % ANSI_RED % ANSI_NORMAL);
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML),
		     "<uncategorized>this is <font color=\"red\" />a<font color=\"norm\" /> test</uncategorized>");
  }


  void testSystemLogComm(){
    testPerson->desc->output.putInQ(new SystemLogComm(time(0), LOG_PIO, testString[1]));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT),
		     fmt("// Player I/O: %s\n\r") % testString[1]);
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML),
		     fmt("<log>\n  <time>%i</time>\n  <type>Player I/O</type>\n  <msg>%s</msg>\n</log>\n") % time(0) % testString[1]);
  }

  void testTellFromComm(){
    testPerson->desc->output.putInQ(new TellFromComm("Deirdre", "Peel",
						     fmt("%s, %s, %s") %
			    testString[0] % testString[1] % testString[2],
						     true, false));
    Comm *comm=testPerson->desc->output.takeFromQ();
    
    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT),
		     fmt("<p>Peel<z> tells you, \"<c>%s, %s, %s<z>\"\n\r") %
    		     testString[0] % testString[1] % testString[2]);
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML),
		     fmt("<tellfrom>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <drunk>true</drunk>\n  <mob>false</mob>\n  <tell>%s, %s, %s</tell>\n</tellfrom>\n") % testString[0] % testString[1] % testString[2]);
  }

  void testTellToComm(){
    testPerson->desc->output.putInQ(new TellToComm("Deirdre", "Peel",
						   fmt("%s, %s, %s") %
		       testString[0] % testString[1] % testString[2]));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT),
		     fmt("<G>You tell Deirdre<z>, \"%s, %s, %s\"\n\r") %
    		     testString[0] % testString[1] % testString[2]);
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML),
		     fmt("<tellto>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <tell>%s, %s, %s</tell>\n</tellto>\n") % testString[0] % testString[1] % testString[2]);
  }

  void testWhoListRemoveComm(){
    testPerson->desc->output.putInQ(new WhoListComm("Peel", false, 60, 100,
	 true, "The Freshmaker", "Peel keeps her warm but they never kiss."));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT), "");
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML), 
		     "<wholist>\n  <online>false</online>\n  <level>60</level>\n  <idle>100</idle>\n  <linkdead>true</linkdead>\n  <name>Peel</name>\n  <prof>The Freshmaker</prof>\n  <title>Peel keeps her warm but they never kiss.</title>\n</wholist>\n");

  }

  void testWhoListAddComm(){
    testPerson->desc->output.putInQ(new WhoListComm("Peel", false, 60, 100,
	false, "The Freshmaker", "Peel keeps her warm but they never kiss."));
    Comm *comm=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(comm->getComm(COMM_TEXT), "");
    TS_ASSERT_EQUALS(comm->getComm(COMM_XML), 
		     "<wholist>\n  <online>false</online>\n  <level>60</level>\n  <idle>100</idle>\n  <linkdead>false</linkdead>\n  <name>Peel</name>\n  <prof>The Freshmaker</prof>\n  <title>Peel keeps her warm but they never kiss.</title>\n</wholist>\n");

  }

  void testPagedOutput(){
    Comm *c;

    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));
    
    testSocket->port=PROD_XMLPORT; // makeOutputPaged checks this
    testPerson->makeOutputPaged();

    // for XML mode it shouldn't do anything
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[0]);
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[1]);
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[2]);


    testPerson->desc->output.putInQ(new UncategorizedComm(testString[0]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[1]));
    testPerson->desc->output.putInQ(new UncategorizedComm(testString[2]));

    testSocket->port=PROD_GAMEPORT; // makeOutputPaged checks this
    testPerson->makeOutputPaged();
    
    c=testPerson->desc->output.takeFromQ();
    TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), fmt("%s%s%s") %
		     testString[0] % testString[1] % testString[2]);

  }


  void testPromptComm(){
    Comm *c;
    PromptComm *pc=new PromptComm(time(0), 100, 100, 100, 100, 100, 100, 100, "prompt> ");

    testPerson->sendTo(pc);

    c=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(c->getComm(COMM_XML), fmt("<prompt time=\"%i\" hp=\"100\" mana=\"100\" piety=\"100.000000\" lifeforce=\"100\" moves=\"100\" money=\"100\" room=\"100\">prompt&#62; </prompt>") % time(0));

  }

  void testRoomExitComm(){
    Comm *c;
    RoomExitComm *rec=new RoomExitComm();

    for(dirTypeT dir=MIN_DIR;dir<MAX_DIR;dir++){
      rec->exits[dir].exit=false;
    }

    rec->exits[DIR_NORTH].exit=true;
    rec->exits[DIR_NORTH].open=true;

    testPerson->sendTo(rec);
    
    c=testPerson->desc->output.takeFromQ();

    TS_ASSERT_EQUALS(c->getComm(COMM_XML), "<roomexits>\n  <exit>\n    <direction>north</direction>\n    <door>\n      <open>true</open>\n    </door>\n  </exit>\n</roomexits>\n");
    
  }
  

};
