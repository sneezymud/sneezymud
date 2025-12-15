#include "cmd_run.h"

#include <gtest/gtest.h>

TEST(Run, addition) {
  std::deque<std::pair<int, char>> res;
  EXPECT_TRUE(parse("ne n e swud1n2e10s11w20u21d1000n", res));
  EXPECT_EQ(res.front(), std::make_pair(1, 'A'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'n'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'e'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'C'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'u'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'd'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1, 'n'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(2, 'e'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(10, 's'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(11, 'w'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(20, 'u'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(21, 'd'));
  res.pop_front();
  EXPECT_EQ(res.front(), std::make_pair(1000, 'n'));
  res.pop_front();
  EXPECT_TRUE(res.empty());
}
