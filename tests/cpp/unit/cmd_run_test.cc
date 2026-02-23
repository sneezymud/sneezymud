// Ported from legacy CxxTest suite at code/code/tests/Run.h

#include <gtest/gtest.h>

#include <deque>
#include <utility>

#include "cmd_run.h"

// Helper to make assertions more readable than repeated front()/pop_front()
namespace {
void expectDirections(std::deque<std::pair<int, char>>& res,
  std::initializer_list<std::pair<int, char>> expected) {
  auto it = expected.begin();
  for (size_t i = 0; it != expected.end(); ++it, ++i) {
    ASSERT_FALSE(res.empty()) << "ran out of results at index " << i;
    EXPECT_EQ(res.front(), *it)
      << "mismatch at index " << i << ": expected {" << it->first << ", '"
      << it->second << "'} got {" << res.front().first << ", '"
      << res.front().second << "'}";
    res.pop_front();
  }
  EXPECT_TRUE(res.empty()) << "unexpected extra directions in result";
}
}  // namespace

TEST(RunParser, CompoundDirectionsAndCounts) {
  // Compound directions: ne→'A' (northeast), sw→'C' (southwest)
  // Repeat counts: 2e means east twice, 1000n means north 1000 times
  std::deque<std::pair<int, char>> res;
  ASSERT_TRUE(parse("ne n e swud1n2e10s11w20u21d1000n", res));

  expectDirections(res,
    {
      {1, 'A'},     // ne
      {1, 'n'},     // n
      {1, 'e'},     // e
      {1, 'C'},     // sw
      {1, 'u'},     // u
      {1, 'd'},     // d
      {1, 'n'},     // 1n
      {2, 'e'},     // 2e
      {10, 's'},    // 10s
      {11, 'w'},    // 11w
      {20, 'u'},    // 20u
      {21, 'd'},    // 21d
      {1000, 'n'},  // 1000n
    });
}

TEST(RunParser, RejectsInvalidInput) {
  std::deque<std::pair<int, char>> res;

  // Literal A/B/C/D in input are rejected (reserved for diagonal encoding)
  EXPECT_FALSE(parse("nAe", res));
  EXPECT_FALSE(parse("B", res));

  // Non-direction characters are rejected
  EXPECT_FALSE(parse("nxe", res));

  // Trailing count with no direction is rejected
  EXPECT_FALSE(parse("n5", res));
}

TEST(RunParser, AllDiagonals) {
  std::deque<std::pair<int, char>> res;
  ASSERT_TRUE(parse("ne se sw nw", res));

  expectDirections(res,
    {
      {1, 'A'},  // ne (northeast)
      {1, 'B'},  // se (southeast)
      {1, 'C'},  // sw (southwest)
      {1, 'D'},  // nw (northwest)
    });
}

TEST(RunParser, EmptyInput) {
  std::deque<std::pair<int, char>> res;
  EXPECT_TRUE(parse("", res));
  EXPECT_TRUE(res.empty());
}
