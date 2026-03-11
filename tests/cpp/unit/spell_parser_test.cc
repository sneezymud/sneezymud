// Integration tests for parseSpellNum() — direct spell ID assertions.
// Strictly stronger than the functional port which can only infer results
// from output strings. Tests the recursive word-splitting parser against
// the in-memory discArray.

#include <gtest/gtest.h>
#include <tuple>

#include "game_fixture.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"

class SpellParserTest : public GameFixture {
  protected:
    void SetUp() override {
      GameFixture::SetUp();

      room = &makeRoom(49998);
      caster = &makeCharacter("Testcaster");
      placeInRoom(*caster, *room);
    }

    // Helper: parse a spell name and return the spell ID.
    spellNumT parse(const sstring& input) {
      auto [spell, remaining] = caster->ch->parseSpellNum(input, "");
      return spell;
    }

    // Helper: parse a spell name and return both spell ID and remaining args.
    std::tuple<spellNumT, sstring> parseWithArgs(const sstring& input) {
      return caster->ch->parseSpellNum(input, "");
    }

    TRoom* room = nullptr;
    TestCharacter* caster = nullptr;
};

TEST_F(SpellParserTest, AbbreviatedMultiWord) {
  // "d i" is ambiguous (matches both "detect invisibility" and
  // "dispel invisible"), so use "det i" for a unique match.
  EXPECT_EQ(parse("det i"), SPELL_DETECT_INVISIBLE)
    << "'det i' should match detect invisibility";
}

TEST_F(SpellParserTest, PartialName) {
  EXPECT_EQ(parse("detect invis"), SPELL_DETECT_INVISIBLE)
    << "'detect invis' should match detect invisible";
}

TEST_F(SpellParserTest, FullName) {
  // Spell is "detect invisibility", not "detect invisible"
  EXPECT_EQ(parse("detect invisibility"), SPELL_DETECT_INVISIBLE)
    << "Full spell name should match";
}

TEST_F(SpellParserTest, SingleWord) {
  const spellNumT result = parse("flight");
  EXPECT_EQ(result, SPELL_FLY) << "'flight' should match fly";
}

TEST_F(SpellParserTest, NonExistentSpell) {
  EXPECT_EQ(parse("blarg"), TYPE_UNDEFINED)
    << "Non-existent spell should return TYPE_UNDEFINED";
}

TEST_F(SpellParserTest, EmptyInput) {
  EXPECT_EQ(parse(""), TYPE_UNDEFINED)
    << "Empty input should return TYPE_UNDEFINED";
}

TEST_F(SpellParserTest, WhitespaceOnly) {
  EXPECT_EQ(parse("   "), TYPE_UNDEFINED)
    << "Whitespace-only input should return TYPE_UNDEFINED";
}

TEST_F(SpellParserTest, SingleWordWithTarget) {
  auto [spell, remaining] = parseWithArgs("flight victim");
  EXPECT_EQ(spell, SPELL_FLY) << "'flight victim' should match fly";
  EXPECT_NE(remaining.find("victim"), sstring::npos)
    << "Remaining args should contain 'victim', got: " << remaining;
}

TEST_F(SpellParserTest, MultiWordWithTarget) {
  auto [spell, remaining] = parseWithArgs("true sight victim");
  EXPECT_EQ(spell, SPELL_TRUE_SIGHT)
    << "'true sight victim' should match true sight";
  EXPECT_NE(remaining.find("victim"), sstring::npos)
    << "Remaining args should contain 'victim', got: " << remaining;
}

TEST_F(SpellParserTest, ThreeWordSpell) {
  EXPECT_EQ(parse("protection from fire"), SPELL_PROTECTION_FROM_FIRE)
    << "Three-word spell should match";
}

TEST_F(SpellParserTest, ThreeWordSpellWithTarget) {
  auto [spell, remaining] = parseWithArgs("protection from fire victim");
  EXPECT_EQ(spell, SPELL_PROTECTION_FROM_FIRE)
    << "'protection from fire victim' should match protection from fire";
  EXPECT_NE(remaining.find("victim"), sstring::npos)
    << "Remaining args should contain 'victim', got: " << remaining;
}

TEST_F(SpellParserTest, MultiWordWithSurroundingWhitespace) {
  EXPECT_EQ(parse("  true sight  "), SPELL_TRUE_SIGHT)
    << "Leading/trailing whitespace should be tolerated";
}

TEST_F(SpellParserTest, ResourcefulnessSkillIsRegistered) {
  EXPECT_EQ(parse("resourcefulness"), SKILL_RESOURCEFULNESS)
    << "Newly added Looting skill should be parseable by name";
}

TEST_F(SpellParserTest, ScrutinySkillIsRegistered) {
  EXPECT_EQ(parse("scrutiny"), SKILL_SCRUTINY)
    << "Newly added Looting skill should be parseable by name";
}
