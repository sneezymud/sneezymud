#include <cxxtest/TestSuite.h>

#include "code/tests/ValueTraits.h"


class SString : public CxxTest::TestSuite
{

 public:
  void testLowerUpper(){
    sstring foo="LOREM IpSum dolor SIT aMeT";

    TS_ASSERT_EQUALS(foo.lower(), "lorem ipsum dolor sit amet");
    TS_ASSERT_EQUALS(foo.upper(), "LOREM IPSUM DOLOR SIT AMET");

  }

  void testTrim(){
    TS_ASSERT_EQUALS(sstring("LOREM IpSum dolor SIT aMeT").trim(), "LOREM IpSum dolor SIT aMeT");
    TS_ASSERT_EQUALS(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trim(), "LOREM IpSum dolor SIT aMeT");
    TS_ASSERT_EQUALS(sstring("\r \n").trim(), "");
    TS_ASSERT_EQUALS(sstring("\r\n word").trim(), "word");
    TS_ASSERT_EQUALS(sstring("word\r\n ").trim(), "word");
    TS_ASSERT_EQUALS(sstring("").trim(), "");
  }

  void testTrimLeft(){
    TS_ASSERT_EQUALS(sstring("LOREM IpSum dolor SIT aMeT").trimLeft(), "LOREM IpSum dolor SIT aMeT");
    TS_ASSERT_EQUALS(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimLeft(), "LOREM IpSum dolor SIT aMeT\v");
    TS_ASSERT_EQUALS(sstring("\r \n").trimLeft(), "");
    TS_ASSERT_EQUALS(sstring("\r\n word").trimLeft(), "word");
    TS_ASSERT_EQUALS(sstring("word\r\n ").trimLeft(), "word\r\n ");
    TS_ASSERT_EQUALS(sstring("").trimLeft(), "");
  }

  void testTrimRight(){
    TS_ASSERT_EQUALS(sstring("LOREM IpSum dolor SIT aMeT").trimRight(), "LOREM IpSum dolor SIT aMeT");
    TS_ASSERT_EQUALS(sstring("\r\n  LOREM IpSum dolor SIT aMeT\v").trimRight(), "\r\n  LOREM IpSum dolor SIT aMeT");
    TS_ASSERT_EQUALS(sstring("\r \n").trimRight(), "");
    TS_ASSERT_EQUALS(sstring("\r\n word").trimRight(), "\r\n word");
    TS_ASSERT_EQUALS(sstring("word\r\n ").trimRight(), "word");
    TS_ASSERT_EQUALS(sstring("").trimRight(), "");
  }

  void testDropLastWord(){
    TS_ASSERT_EQUALS(sstring("LOREM IpSum dolor SIT aMeT").dropLastWord(), "LOREM IpSum dolor SIT");
    TS_ASSERT_EQUALS(sstring("two\t  --!/\\words").dropLastWord(), "two");
    TS_ASSERT_EQUALS(sstring("oneword").dropLastWord(), "");
    TS_ASSERT_EQUALS(sstring(" oneword").dropLastWord(), "");
    TS_ASSERT_EQUALS(sstring("").dropLastWord(), "");
  }

  void testLastWord(){
    TS_ASSERT_EQUALS(sstring("LOREM IpSum dolor SIT aMeT").lastWord(), "aMeT");
    TS_ASSERT_EQUALS(sstring("two\t  --!/\\words").lastWord(), "--!/\\words");
    TS_ASSERT_EQUALS(sstring("oneWord").lastWord(), "oneWord");
    TS_ASSERT_EQUALS(sstring(" oneWord").lastWord(), "oneWord");
    TS_ASSERT_EQUALS(sstring("").lastWord(), "");
  }

  void testWords(){
    sstring foo="   LOREM IpSum   \n    dolor SIT aMeT ";
    auto words = foo.words();
    TS_ASSERT_EQUALS(words.size(), 5);
    TS_ASSERT_EQUALS(foo.words()[0], "LOREM");
    TS_ASSERT_EQUALS(foo.words()[1], "IpSum");
    TS_ASSERT_EQUALS(foo.words()[2], "dolor");
    TS_ASSERT_EQUALS(foo.words()[3], "SIT");
    TS_ASSERT_EQUALS(foo.words()[4], "aMeT");

    auto empty = sstring().words();
    TS_ASSERT(empty.empty());

    auto oneword = sstring("bla").words();
    TS_ASSERT_EQUALS(oneword.size(), 1);
    TS_ASSERT_EQUALS(oneword[0], "bla");

    auto onewordWs = sstring(" bla ").words();
    TS_ASSERT_EQUALS(onewordWs.size(), 1);
    TS_ASSERT_EQUALS(onewordWs[0], "bla");
  }

  void testWord(){
    sstring foo="LOREM IpSum dolor SIT aMeT";
    TS_ASSERT_EQUALS(foo.word(-1), "");
    TS_ASSERT_EQUALS(foo.word(0), "LOREM");
    TS_ASSERT_EQUALS(foo.word(1), "IpSum");
    TS_ASSERT_EQUALS(foo.word(2), "dolor");
    TS_ASSERT_EQUALS(foo.word(3), "SIT");
    TS_ASSERT_EQUALS(foo.word(4), "aMeT");
    TS_ASSERT_EQUALS(foo.word(5), "");
    TS_ASSERT_EQUALS(foo.word(6), "");
  }

  void testRange(){
    sstring foo;
    // unfortunately, this hangs
    // TS_ASSERT_THROWS(foo[0]='x', std::out_of_range const&);
  }
};
