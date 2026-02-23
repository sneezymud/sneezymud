// Integration tests for findComponent() — component lookup across
// equipment slots, inventory, and containers.
// Ports the legacy CxxTest Cast.h tests with direct construction
// instead of read_object() database calls.

#include <gtest/gtest.h>

#include "defs.h"
#include "disc_mage_wizardry.h"
#include "disc_shaman_ritualism.h"
#include "discipline.h"
#include "game_fixture.h"
#include "limbs.h"
#include "obj_component.h"
#include "obj.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"

// The spell we'll use for all component tests — arbitrary choice.
// Must exist in discArray (populated by buildSpellArray).
constexpr spellNumT TEST_SPELL = SPELL_DETECT_INVISIBLE;
constexpr int TEST_CHARGES = 5;

class FindComponentTest : public GameFixture {
  protected:
    void SetUp() override {
      GameFixture::SetUp();

      room = &makeRoom(49997);
    }

    // Create a mage character with the given wizardry skill level.
    // Wizardry level thresholds (from gaining.cc):
    //   0-14: WIZ_LEV_COMP_PRIM_OTHER_FREE (primary hand only)
    //   15-29: WIZ_LEV_COMP_EITHER_OTHER_FREE
    //   30-39: WIZ_LEV_COMP_EITHER
    //   40-49: WIZ_LEV_COMP_INV (inventory search)
    //   50-59: WIZ_LEV_NO_GESTURES
    //   60-74: WIZ_LEV_NO_MANTRA (belt + inventory)
    //   75-89: WIZ_LEV_COMP_BELT (belt + neck)
    //   90-99: WIZ_LEV_COMP_NECK (+ wrist)
    //   100:   WIZ_LEV_COMP_WRIST
    TestCharacter& makeMage(const sstring& charName, int wizardrySkill) {
      auto& tc = makeCharacter(charName);
      tc.ch->setClass(CLASS_MAGE);
      placeInRoom(tc, *room);

      // Set up discipline system for wizardry
      tc.ch->discs = new CMasterDiscipline();
      auto* wiz = new CDWizardry();
      // Disc learnedness must be high enough for getMaxSkillValue to allow
      // the skill value. Setting to MAX_DISC_LEARNEDNESS is simplest.
      wiz->setLearnedness(MAX_DISC_LEARNEDNESS);
      wiz->skWizardry.setLearnedness(wizardrySkill);
      wiz->skWizardry.setNatLearnedness(wizardrySkill);
      tc.ch->discs->disc[DISC_WIZARDRY] = wiz;

      return tc;
    }

    // Create a shaman character with the given ritualism skill level.
    // Ritualism level thresholds mirror wizardry (from gaining.cc).
    TestCharacter& makeShaman(const sstring& charName, int ritualismSkill) {
      auto& tc = makeCharacter(charName);
      tc.ch->setClass(CLASS_SHAMAN);
      placeInRoom(tc, *room);

      tc.ch->discs = new CMasterDiscipline();
      auto* rit = new CDRitualism();
      rit->setLearnedness(MAX_DISC_LEARNEDNESS);
      rit->skRitualism.setLearnedness(ritualismSkill);
      rit->skRitualism.setNatLearnedness(ritualismSkill);
      tc.ch->discs->disc[DISC_RITUALISM] = rit;

      return tc;
    }

    TRoom* room = nullptr;
};

TEST_F(FindComponentTest, ComponentInPrimaryHand) {
  auto& mage = makeMage("Lowmage", 10);  // WIZ_LEV_COMP_PRIM_OTHER_FREE
  auto* comp = makeComponent(TEST_SPELL, TEST_CHARGES);

  // Place component in primary hand (right hand for right-handed characters)
  mage.ch->equipChar(comp, HOLD_RIGHT);

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, comp) << "Should find component held in primary hand";
}

TEST_F(FindComponentTest, NoMatchingComponent) {
  auto& mage = makeMage("Emptymage", 10);

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, nullptr) << "Should return nullptr with no components";
}

TEST_F(FindComponentTest, WrongSpellComponent) {
  auto& mage = makeMage("Wrongmage", 10);
  auto* comp = makeComponent(SPELL_FLY, TEST_CHARGES);  // wrong spell

  mage.ch->equipChar(comp, HOLD_RIGHT);

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, nullptr)
    << "Should not find component for a different spell";
}

TEST_F(FindComponentTest, ComponentInOpenContainer) {
  // Wizardry >= 40 allows inventory search, which includes containers
  auto& mage = makeMage("Bagmage", 45);
  auto* comp = makeComponent(TEST_SPELL, TEST_CHARGES);
  auto* chest = makeContainer();

  // Put component inside the open chest, then put chest in inventory
  *chest += *comp;
  *mage.ch += *chest;

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, comp) << "Should find component inside open container";
}

TEST_F(FindComponentTest, ComponentInClosedContainer) {
  auto& mage = makeMage("Closedmage", 45);
  auto* comp = makeComponent(TEST_SPELL, TEST_CHARGES);
  auto* chest = makeContainer();

  // Close the chest, put component in, then put in inventory
  chest->addContainerFlag(CONT_CLOSED);
  *chest += *comp;
  *mage.ch += *chest;

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, nullptr)
    << "Should NOT find component inside closed container";
}

TEST_F(FindComponentTest, LowWizardryCannotSearchInventory) {
  auto& mage = makeMage("Newmage", 10);  // WIZ_LEV_COMP_PRIM_OTHER_FREE
  auto* comp = makeComponent(TEST_SPELL, TEST_CHARGES);

  // Place component in inventory (not in hand)
  *mage.ch += *comp;

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, nullptr)
    << "Low wizardry mage should only find components in primary hand";
}

TEST_F(FindComponentTest, ShamanFindsInInventory) {
  // Ritualism >= 40 (RIT_LEV_COMP_INV) allows inventory search
  auto& shaman = makeShaman("Ritshaman", 45);
  auto* comp = makeComponent(TEST_SPELL, TEST_CHARGES);

  *shaman.ch += *comp;

  TComponent* found = shaman.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, comp)
    << "Shaman with sufficient ritualism should find component in inventory";
}

TEST_F(FindComponentTest, PicksBestComponent) {
  // When multiple matching components exist, findComponent should pick
  // the one with the fewest charges remaining.
  auto& mage = makeMage("Pickymage", 45);
  auto* compMany = makeComponent(TEST_SPELL, 10);
  auto* compFew = makeComponent(TEST_SPELL, 2);

  *mage.ch += *compMany;
  *mage.ch += *compFew;

  TComponent* found = mage.ch->findComponent(TEST_SPELL);
  EXPECT_EQ(found, compFew) << "Should pick the component with fewest charges";
}
