#include <cxxtest/TestSuite.h>

#include "configuration.h"
#include "extern.h"
#include "code/tests/ValueTraits.h"

#include "obj_fruit.h"

class Fruit : public CxxTest::TestSuite
{
 public:
  TRoom *testRoom;
  TFruit *fruit;

  void setUp(){
    Config::doConfiguration();
    freopen("code/tests/output/Fruit.out", "w", stderr);


    generate_obj_index();

    testRoom=new TRoom(100);

    fruit=dynamic_cast<TFruit *>(read_object(8936, VIRTUAL));
  }

  void testSeedVal(){
    // check that get/set seed vnum work
    int seedN=fruit->getSeedVNum();
    fruit->setSeedVNum(100);
    TS_ASSERT_EQUALS(fruit->getSeedVNum(), 100);
    fruit->setSeedVNum(seedN);
  }


  void testCreateSeeds(){
    // put the fruit in a room
    *testRoom += *fruit;
    
    // check that create seeds actually puts seeds in the room
    fruit->createSeeds();
    TS_ASSERT_EQUALS(obj_index[testRoom->stuff.front()->number].virt, 
		     fruit->getSeedVNum());

  }

};
