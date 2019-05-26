#include <cxxtest/TestSuite.h>

#include "cmd_run.h"


// semi-private to cmd_run
class Run : public CxxTest::TestSuite
{

  public:
    void testAddition(){
      std::deque<std::pair<int, char> > res;
      TS_ASSERT(parse("ne n e swud1n2e10s11w20u21d1000n", res));
      TS_ASSERT_EQUALS(std::make_pair(1, 'A'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'n'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'e'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'C'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'u'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'd'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1, 'n'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(2, 'e'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(10, 's'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(11, 'w'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(20, 'u'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(21, 'd'), res.front()); res.pop_front();
      TS_ASSERT_EQUALS(std::make_pair(1000, 'n'), res.front()); res.pop_front();
      TS_ASSERT(res.empty());
    }
};
