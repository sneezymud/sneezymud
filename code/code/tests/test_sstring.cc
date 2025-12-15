#include "sstring.h"
#include <gtest/gtest.h>

TEST(SString, lowerUpper) {
  sstring foo = "LOREM IpSum dolor SIT aMeT";

  EXPECT_EQ("lorem ipsum dolor sit amet", foo.lower());
  EXPECT_EQ("LOREM IPSUM DOLOR SIT AMET", foo.upper());
}

TEST(SString, Trim) {
      EXPECT_EQ("LOREM IpSum dolor SIT aMeT", sstring("LOREM IpSum dolor SIT aMeT").trim());
      EXPECT_EQ("LOREM IpSum dolor SIT aMeT", sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trim());
      EXPECT_EQ("", sstring("\r \n").trim());
      EXPECT_EQ("word", sstring("\r\n word").trim());
      EXPECT_EQ("word", sstring("word\r\n ").trim());
      EXPECT_EQ("", sstring("").trim());
    }

TEST(SString, TrimLeft) {
      EXPECT_EQ("LOREM IpSum dolor SIT aMeT", sstring("LOREM IpSum dolor SIT aMeT").trimLeft());
      EXPECT_EQ("LOREM IpSum dolor SIT aMeT\v", sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimLeft());
      EXPECT_EQ("", sstring("\r \n").trimLeft());
      EXPECT_EQ("word", sstring("\r\n word").trimLeft());
      EXPECT_EQ("word\r\n ", sstring("word\r\n ").trimLeft());
      EXPECT_EQ("", sstring("").trimLeft());
    }

TEST(SString, TrimRight) {
      EXPECT_EQ("LOREM IpSum dolor SIT aMeT", sstring("LOREM IpSum dolor SIT aMeT").trimRight());
      EXPECT_EQ("\r\n  LOREM IpSum dolor SIT aMeT",  sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimRight());
      EXPECT_EQ("", sstring("\r \n").trimRight());
      EXPECT_EQ("\r\n word", sstring("\r\n word").trimRight());
      EXPECT_EQ("word", sstring("word\r\n ").trimRight());
      EXPECT_EQ("", sstring("").trimRight());
    }

TEST(SString, DropLastWord) {
      EXPECT_EQ("LOREM IpSum dolor SIT", sstring("LOREM IpSum dolor SIT aMeT").dropLastWord());
      EXPECT_EQ("two", sstring("two\t  --!/\\words").dropLastWord());
      EXPECT_EQ("", sstring("oneword").dropLastWord());
      EXPECT_EQ("", sstring(" oneword").dropLastWord());
      EXPECT_EQ("", sstring("").dropLastWord());
    }

TEST(SString, LastWord) {
      EXPECT_EQ("aMeT", sstring("LOREM IpSum dolor SIT aMeT").lastWord());
      EXPECT_EQ("--!/\\words", sstring("two\t  --!/\\words").lastWord());
      EXPECT_EQ("oneWord", sstring("oneWord").lastWord());
      EXPECT_EQ("oneWord", sstring(" oneWord").lastWord());
      EXPECT_EQ("", sstring("").lastWord());
    }

TEST(SString, Words) {
      sstring foo = "   LOREM IpSum   \n    dolor SIT aMeT ";
      auto words = foo.words();
      EXPECT_EQ(5, words.size());
      EXPECT_EQ("LOREM", foo.words()[0]);
      EXPECT_EQ("IpSum", foo.words()[1]);
      EXPECT_EQ("dolor", foo.words()[2]);
      EXPECT_EQ("SIT", foo.words()[3]);
      EXPECT_EQ("aMeT", foo.words()[4]);

      auto empty = sstring().words();
      EXPECT_TRUE(empty.empty());

      auto oneword = sstring("bla").words();
      EXPECT_EQ(1, oneword.size());
      EXPECT_EQ("bla", oneword[0]);

      auto onewordWs = sstring(" bla ").words();
      EXPECT_EQ(1, onewordWs.size());
      EXPECT_EQ("bla", onewordWs[0]);
    }

TEST(SString, Word) {
      sstring foo = "LOREM IpSum dolor SIT aMeT";
      EXPECT_EQ("", foo.word(-1));
      EXPECT_EQ("LOREM", foo.word(0));
      EXPECT_EQ("IpSum", foo.word(1));
      EXPECT_EQ("dolor", foo.word(2));
      EXPECT_EQ("SIT", foo.word(3));
      EXPECT_EQ("aMeT", foo.word(4));
      EXPECT_EQ("", foo.word(5));
      EXPECT_EQ("", foo.word(6));
    }

TEST(SString, Range) {
      sstring foo;
      // unfortunately, this hangs
      // TS_ASSERT_THROWS(foo[0]='x', std::out_of_range const&);
    }
