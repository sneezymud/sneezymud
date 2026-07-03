// Trap component bag: holds only trap components, and its contents feed the
// trap-set component search (TBeing::findTrapComp descends into the bag).
// The FindTrapComp* cases guard against a future refactor silently severing
// that bag descent -- setting traps from loose inventory would still pass.

#include <gtest/gtest.h>

#include "game_fixture.h"
#include "obj_trap_component.h"
#include "obj_trapcomp_bag.h"

class TrapCompBagTest : public GameFixture {
  protected:
    void SetUp() override {
      GameFixture::SetUp();
      room = &makeRoom(49996);
    }

    static TTrapComponent* makeTrapComp(const sstring& name) {
      auto* c = new TTrapComponent();
      c->name = name;
      c->shortDescr = name;
      c->setTrapComponentCharges(3);
      return c;
    }

    static TTrapCompBag* makeTrapBag() {
      auto* b = new TTrapCompBag();
      b->name = "a trap component bag";
      b->shortDescr = "a trap component bag";
      return b;
    }

    TRoom* room = nullptr;
};

TEST_F(TrapCompBagTest, RejectsNonComponent) {
  auto& tc = makeCharacter("Trapper");
  placeInRoom(tc, *room);
  auto* bag = makeTrapBag();
  auto* chest = makeContainer();  // not a trap component

  int rc = bag->putSomethingInto(tc.ch, chest);

  EXPECT_EQ(rc, 2) << "Non-component put should be halted";
  EXPECT_TRUE(bag->stuff.empty()) << "Rejected object must not enter the bag";

  delete chest;
  delete bag;
}

TEST_F(TrapCompBagTest, FindTrapCompLocatesComponentInBag) {
  auto& tc = makeCharacter("Trapper");
  placeInRoom(tc, *room);
  auto* comp = makeTrapComp("spike");
  auto* bag = makeTrapBag();

  *bag += *comp;
  *tc.ch += *bag;

  EXPECT_EQ(tc.ch->findTrapComp("spike"), comp)
    << "findTrapComp should descend into a carried trap component bag";
}

TEST_F(TrapCompBagTest, FindTrapCompIgnoresPlainContainer) {
  auto& tc = makeCharacter("Trapper");
  placeInRoom(tc, *room);
  auto* comp = makeTrapComp("spring");
  auto* chest = makeContainer();

  *chest += *comp;
  *tc.ch += *chest;

  EXPECT_EQ(tc.ch->findTrapComp("spring"), nullptr)
    << "findTrapComp must not find components in a non-trap-bag container";
}

TEST_F(TrapCompBagTest, FindTrapCompFindsLooseInInventory) {
  auto& tc = makeCharacter("Trapper");
  placeInRoom(tc, *room);
  auto* comp = makeTrapComp("tripwire");

  *tc.ch += *comp;

  EXPECT_EQ(tc.ch->findTrapComp("tripwire"), comp)
    << "findTrapComp should still find loose top-level components";
}
