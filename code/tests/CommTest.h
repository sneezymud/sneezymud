#include <cxxtest/TestSuite.h>

#include "stdsneezy.h"
#include "connect.h"

class CommTest : public CxxTest::TestSuite
{
 public:

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
      q.putInQ(new UncategorizedComm("holding up my"));
      q.putInQ(new UncategorizedComm("purring cat to the moon"));
      q.putInQ(new UncategorizedComm("I sighed."));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), (sstring)"holding up my");
      
      // stick something in out of order
      q.putInQ(new UncategorizedComm("C-C-C-C-C-Combo breaker!"));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), (sstring)"purring cat to the moon");
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), (sstring)"I sighed.");
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), (sstring)"C-C-C-C-C-Combo breaker!");
    }

    // queue should be empty now (after operations)
    TS_ASSERT(q.takeFromQ()==NULL);

    // check if clear() works
    {
      q.putInQ(new UncategorizedComm("holding up my"));
      q.putInQ(new UncategorizedComm("purring cat to the moon"));
      q.putInQ(new UncategorizedComm("I sighed."));
      
      c=q.takeFromQ();
      TS_ASSERT_EQUALS(c->getComm(COMM_TEXT), (sstring)"holding up my");
      
      q.clear();
      TS_ASSERT(q.takeFromQ()==NULL);
    }
    

  }

  void testUncategorizedComm(){
    UncategorizedComm comm("this is <r>a<1> test");
    
    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT),
		     (sstring)"this is <r>a<1> test");
  }


  void testTellComm(){
    TellComm comm("Peel", "this is a test");
    
    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT), 
		     (sstring)"Peel tells you, \"<c>this is a test<z>\"");
  }

  void testCmdErrorComm(){
    CmdErrorComm comm(CMD_TELL, "You are a dumb animal; you can't talk!");

    TS_ASSERT_EQUALS(comm.getComm(COMM_TEXT),
		     (sstring)"You are a dumb animal; you can't talk!");
  }
};
