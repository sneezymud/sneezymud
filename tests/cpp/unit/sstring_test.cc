// Ported from legacy CxxTest suite at code/code/tests/SString.h

#include <gtest/gtest.h>

#include "sstring.h"

TEST(SString, LowerUpper) {
  sstring foo = "LOREM IpSum dolor SIT aMeT";

  EXPECT_EQ(foo.lower(), "lorem ipsum dolor sit amet");
  EXPECT_EQ(foo.upper(), "LOREM IPSUM DOLOR SIT AMET");
}

TEST(SString, Trim) {
  EXPECT_EQ(sstring("LOREM IpSum dolor SIT aMeT").trim(),
    "LOREM IpSum dolor SIT aMeT");
  EXPECT_EQ(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trim(),
    "LOREM IpSum dolor SIT aMeT");
  EXPECT_EQ(sstring("\r \n").trim(), "");
  EXPECT_EQ(sstring("\r\n word").trim(), "word");
  EXPECT_EQ(sstring("word\r\n ").trim(), "word");
  EXPECT_EQ(sstring("").trim(), "");
}

TEST(SString, TrimLeft) {
  EXPECT_EQ(sstring("LOREM IpSum dolor SIT aMeT").trimLeft(),
    "LOREM IpSum dolor SIT aMeT");
  EXPECT_EQ(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimLeft(),
    "LOREM IpSum dolor SIT aMeT\v");
  EXPECT_EQ(sstring("\r \n").trimLeft(), "");
  EXPECT_EQ(sstring("\r\n word").trimLeft(), "word");
  EXPECT_EQ(sstring("word\r\n ").trimLeft(), "word\r\n ");
  EXPECT_EQ(sstring("").trimLeft(), "");
}

TEST(SString, TrimRight) {
  EXPECT_EQ(sstring("LOREM IpSum dolor SIT aMeT").trimRight(),
    "LOREM IpSum dolor SIT aMeT");
  EXPECT_EQ(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimRight(),
    "\r\n  LOREM IpSum dolor SIT aMeT");
  EXPECT_EQ(sstring("\r \n").trimRight(), "");
  EXPECT_EQ(sstring("\r\n word").trimRight(), "\r\n word");
  EXPECT_EQ(sstring("word\r\n ").trimRight(), "word");
  EXPECT_EQ(sstring("").trimRight(), "");
}

TEST(SString, DropLastWord) {
  EXPECT_EQ(sstring("LOREM IpSum dolor SIT aMeT").dropLastWord(),
    "LOREM IpSum dolor SIT");
  EXPECT_EQ(sstring("two\t  --!/\\words").dropLastWord(), "two");
  EXPECT_EQ(sstring("oneword").dropLastWord(), "");
  EXPECT_EQ(sstring(" oneword").dropLastWord(), "");
  EXPECT_EQ(sstring("").dropLastWord(), "");
}

TEST(SString, LastWord) {
  EXPECT_EQ(sstring("LOREM IpSum dolor SIT aMeT").lastWord(), "aMeT");
  EXPECT_EQ(sstring("two\t  --!/\\words").lastWord(), "--!/\\words");
  EXPECT_EQ(sstring("oneWord").lastWord(), "oneWord");
  EXPECT_EQ(sstring(" oneWord").lastWord(), "oneWord");
  EXPECT_EQ(sstring("").lastWord(), "");
}

TEST(SString, Words) {
  sstring foo = "   LOREM IpSum   \n    dolor SIT aMeT ";
  auto words = foo.words();
  ASSERT_EQ(words.size(), 5u);
  EXPECT_EQ(words[0], "LOREM");
  EXPECT_EQ(words[1], "IpSum");
  EXPECT_EQ(words[2], "dolor");
  EXPECT_EQ(words[3], "SIT");
  EXPECT_EQ(words[4], "aMeT");

  auto empty = sstring().words();
  EXPECT_TRUE(empty.empty());

  auto oneword = sstring("bla").words();
  ASSERT_EQ(oneword.size(), 1u);
  EXPECT_EQ(oneword[0], "bla");

  auto onewordWs = sstring(" bla ").words();
  ASSERT_EQ(onewordWs.size(), 1u);
  EXPECT_EQ(onewordWs[0], "bla");
}

TEST(SString, Word) {
  sstring foo = "LOREM IpSum dolor SIT aMeT";
  EXPECT_EQ(foo.word(-1), "");
  EXPECT_EQ(foo.word(0), "LOREM");
  EXPECT_EQ(foo.word(1), "IpSum");
  EXPECT_EQ(foo.word(2), "dolor");
  EXPECT_EQ(foo.word(3), "SIT");
  EXPECT_EQ(foo.word(4), "aMeT");
  EXPECT_EQ(foo.word(5), "");
  EXPECT_EQ(foo.word(6), "");
}
