#include <algorithm>

#include <gtest/gtest.h>

#include "extern.h"
#include "monster.h"
#include "obj.h"
#include "statistics.h"
#include "world_fixture.h"

class WorldFixtureTest : public WorldFixture {};

TEST_F(WorldFixtureTest, ItemInfoPopulated) {
  // assign_item_info() should have been called by SetUpTestSuite.
  // ITEM_WEAPON is a representative entry.
  ASSERT_NE(ItemInfo[ITEM_WEAPON], nullptr);
  EXPECT_STREQ(ItemInfo[ITEM_WEAPON]->name, "Weapon");
}

TEST_F(WorldFixtureTest, StatsInitialized) {
  EXPECT_DOUBLE_EQ(stats.max_exist, 1.2);
  EXPECT_DOUBLE_EQ(stats.xp_modif, 0.65);
  EXPECT_EQ(stats.absorb_damage_divisor[MOB_STAT], 2);
  EXPECT_EQ(stats.absorb_damage_divisor[PC_STAT], 4);
  EXPECT_DOUBLE_EQ(stats.skill_damage_mod, 0.45);
  EXPECT_DOUBLE_EQ(stats.heal_amount_mod, 0.65);
  EXPECT_DOUBLE_EQ(stats.weapon_damage_mod, 0.33);
  EXPECT_DOUBLE_EQ(stats.barehand_damage_mod, 0.36);
  EXPECT_DOUBLE_EQ(stats.npc_skill_damage_mod, 0.5);
  EXPECT_DOUBLE_EQ(stats.npc_heal_amount_mod, 0.65);
  EXPECT_DOUBLE_EQ(stats.npc_weapon_damage_mod, 0.85);
}

TEST_F(WorldFixtureTest, InsertTestMobMakesRealMobileFindIt) {
  int rnum = insertTestMob(99001, "a test goblin", 10);
  EXPECT_GE(rnum, 0);
  EXPECT_EQ(real_mobile(99001), rnum);
}

TEST_F(WorldFixtureTest, InsertTestMobSetsNameAndLevel) {
  int rnum = insertTestMob(99002, "a test dragon", 50);
  EXPECT_STREQ(mob_index[rnum].name, "a test dragon");
  EXPECT_EQ(mob_index[rnum].level, 50);
}

TEST_F(WorldFixtureTest, MultipleTestMobsMaintainSortedOrder) {
  insertTestMob(99003, "mob three", 30);
  insertTestMob(99001, "mob one", 10);
  insertTestMob(99002, "mob two", 20);

  int r1 = real_mobile(99001);
  int r2 = real_mobile(99002);
  int r3 = real_mobile(99003);
  EXPECT_LT(r1, r2);
  EXPECT_LT(r2, r3);
}

TEST_F(WorldFixtureTest, InsertTestObjMakesRealObjectFindIt) {
  int rnum = insertTestObj(99001, "a test sword", ITEM_WEAPON);
  EXPECT_GE(rnum, 0);
  EXPECT_EQ(real_object(99001, true), rnum);
}

TEST_F(WorldFixtureTest, InsertTestObjSetsItemType) {
  auto mobCountBefore = mob_index.size();
  int rnum = insertTestObj(99002, "a test potion", ITEM_POTION);
  EXPECT_EQ(mob_index.size(),
    mobCountBefore);  // obj insert doesn't affect mob_index
  EXPECT_EQ(obj_index[rnum].itemtype, ITEM_POTION);
}

TEST_F(WorldFixtureTest, LoadMobFromDatabaseCreatesInstance) {
  // loadRealMobIndex populates mob_index from the DB.
  loadRealMobIndex();

  // Find any valid mob vnum from the loaded index.
  ASSERT_FALSE(mob_index.empty());
  int vnum = mob_index[0].virt;

  auto& room = makeRoom(49990);
  auto& mob = loadMob(vnum, room);

  EXPECT_NE(mob.mob, nullptr);
  EXPECT_EQ(mob.mob->mobVnum(), vnum);
  EXPECT_EQ(mob.mob->roomp, &room);
}

TEST_F(WorldFixtureTest, LoadObjFromDatabaseCreatesInstance) {
  loadRealObjIndex();

  // Skip type 0 (ITEM_UNDEFINED) entries - makeNewObj returns null for them.
  auto it = std::find_if(obj_index.begin(), obj_index.end(),
    [](const objIndexData& e) { return e.itemtype != 0; });
  ASSERT_NE(it, obj_index.end());
  int vnum = it->virt;

  auto& obj = loadObj(vnum);

  EXPECT_NE(obj.obj, nullptr);
  EXPECT_EQ(obj.obj->objVnum(), vnum);
}

TEST_F(WorldFixtureTest, LoadRealMobIndexPopulatesFromDatabase) {
  loadRealMobIndex();
  EXPECT_GT(mob_index.size(), 0u);
}

TEST_F(WorldFixtureTest, LoadRealObjIndexPopulatesFromDatabase) {
  loadRealObjIndex();
  EXPECT_GT(obj_index.size(), 0u);
}

TEST_F(WorldFixtureTest, LoadRealMobIndexIsIdempotent) {
  loadRealMobIndex();
  auto size1 = mob_index.size();
  loadRealMobIndex();
  EXPECT_EQ(mob_index.size(), size1);
}
