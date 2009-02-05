#include <cxxtest/TestSuite.h>
#include "stdsneezy.h"
#include "connect.h"
#include "tests/ValueTraits.h"


class CommTest : public CxxTest::TestSuite
{
 public:
  sstring testString[4];

  void setUp(){
    testString[0]="holding up my";
    testString[1]="purring cat to the moon";
    testString[2]="I sighed.";
    testString[3]="C-C-C-C-C-Combo breaker!";
  }

  void testOutputQ(){
    outputQ q;
    Comm *c;

    // make sure the basic functions work on an empty queue
    {
      TS_ASSERT(q.takeFromQ()==NULL);
      TS_ASSERT(q.getBegin()==NULL);
      TS_ASSERT(q.getEnd()==NULL);
    }

    // basic queue test
    {
      q.putInQ(new UncategorizedComm(testString[0]));
      q.putInQ(new UncategorizedComm(testString[1]));
      q.putInQ(new UncategorizedComm(testString[2]));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[0]);
      
      // stick something in out of order
      q.putInQ(new UncategorizedComm(testString[3]));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[1]);
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[2]);
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[3]);
    }

    // queue should be empty now (after operations)
    TS_ASSERT(q.takeFromQ()==NULL);

    // check if clear() works
    {
      q.putInQ(new UncategorizedComm(testString[0]));
      q.putInQ(new UncategorizedComm(testString[1]));
      q.putInQ(new UncategorizedComm(testString[2]));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), testString[0]);
      
      q.clear();
      TS_ASSERT(q.takeFromQ()==NULL);
    }
    

  }

  void testUncategorizedComm(){
    UncategorizedComm comm(testString[0]);
    
    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT), testString[0]);

  }

  void testColoredComm(){
    UncategorizedComm comm("this is <r>a<1> test");

    TS_ASSERT_EQUALS(comm.getComm(COMM_XML),
		     "<uncategorized>this is <font color=\"norm\" /><font color=\"red\" />a<font color=\"norm\" /> test</uncategorized>");
  }

  void testSystemLogComm(){
    SystemLogComm comm(time(0), LOG_PIO, testString[1]);

    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT),
		     fmt("// Player I/O: %s\n\r") % testString[1]);
    TS_ASSERT_EQUALS(comm.getComm(COMM_XML),
		     fmt("<log>\n  <time>%i</time>\n  <type>Player I/O</type>\n  <msg>%s</msg>\n</log>\n") % time(0) % testString[1]);
  }

  void testTellFromComm(){
    TellFromComm comm("Deirdre", "Peel", fmt("%s, %s, %s") %
		      testString[0] % testString[1] % testString[2], 
		      true, false);
    
    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT),
		     fmt("<p>Peel<z> tells you, \"<c>%s, %s, %s<z>\"\n\r") %
    		     testString[0] % testString[1] % testString[2]);
    TS_ASSERT_EQUALS(comm.getComm(COMM_XML),
		     fmt("<tellfrom>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <drunk>true</drunk>\n  <mob>false</mob>\n  <tell>%s, %s, %s</tell>\n</tellfrom>\n") % testString[0] % testString[1] % testString[2]);
  }

  void testTellToComm(){
    TellToComm comm("Deirdre", "Peel", fmt("%s, %s, %s") %
		    testString[0] % testString[1] % testString[2]);

    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT),
		     fmt("<G>You tell Deirdre<z>, \"%s, %s, %s\"\n\r") %
    		     testString[0] % testString[1] % testString[2]);
    TS_ASSERT_EQUALS(comm.getComm(COMM_XML),
		     fmt("<tellto>\n  <to>Deirdre</to>\n  <from>Peel</from>\n  <tell>%s, %s, %s</tell>\n</tellto>\n") % testString[0] % testString[1] % testString[2]);


  }


};
