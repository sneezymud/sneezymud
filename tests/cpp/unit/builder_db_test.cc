#include <format>
#include <gtest/gtest.h>

#include "database.h"
#include "database_fixture.h"
#include "discipline.h"
#include "extern.h"
#include "monster.h"
#include "obj.h"
#include "obj_trash.h"
#include "person.h"
#include "room.h"

// These functions have no header declaration
extern void TBeingSave(TBeing* ch, TMonster* mob, int vnum);
extern void mremove(TBeing* ch, int vnum);
extern void ObjSave(TBeing* ch, TObj* o, int vnum);
extern void ObjLoad(TBeing* ch, int vnum);
extern void oremove(TBeing* ch, int vnum);
extern void RoomSave(TBeing* ch, int start, int end, int useSecond);

class BuilderDbTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      tc = &makeCharacter("TestBuilder");
      room = &makeRoom(49998);
      placeInRoom(*tc, *room);

      auto pidStr =
        dbQueryScalar(DB_SNEEZY, "SELECT MIN(id) FROM player");
      ASSERT_FALSE(pidStr.empty());
      playerId = convertTo<int>(pidStr);
      tc->ch->player.player_id = playerId;
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;

    void registerImmortalMobCleanup() {
      auto del = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum = 99901 AND player_id = {}",
          table, playerId);
      };
      dbCleanupLater(DB_IMMORTAL, del("mob_extra"));
      dbCleanupLater(DB_IMMORTAL, del("mob_imm"));
      dbCleanupLater(DB_IMMORTAL, del("mob"));
    }

    void registerImmortalObjCleanup() {
      auto del = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum = 99901 AND player_id = {}",
          table, playerId);
      };
      dbCleanupLater(DB_IMMORTAL, del("objaffect"));
      dbCleanupLater(DB_IMMORTAL, del("objextra"));
      dbCleanupLater(DB_IMMORTAL, del("obj"));
    }

    void registerImmortalRoomCleanup() {
      auto del = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN (49901, 49902) AND player_id = {}",
          table, playerId);
      };
      dbCleanupLater(DB_IMMORTAL, del("roomexit"));
      dbCleanupLater(DB_IMMORTAL, del("roomextra"));
      dbCleanupLater(DB_IMMORTAL, del("room"));
    }

    // Create a minimal TMonster with the fields TBeingSave requires.
    // TBeingSave returns early if name, shortDescr, longDesc, or descr
    // are empty, so all four must be set.
    // The mob is placed in the test room so ~TBeing can remove it cleanly.
    TMonster* makeTestMob() {
      auto* mob = new TMonster();
      mob->name = "test mob keywords";
      mob->shortDescr = "a test mob";
      mob->setDescr("This is a test mob.\n");
      mob->player.longDescr = "A test mob stands here.\n";
      mob->setRace(RACE_HUMAN);
      mob->setSex(SEX_MALE);
      mob->setWeight(150);
      mob->setHeight(72);
      mob->default_pos = POSITION_STANDING;
      mob->setPosition(POSITION_STANDING);
      mob->max_exist = 1;
      mob->discs = new CMasterDiscipline();
      *room += *mob;
      mob->next = character_list;
      character_list = mob;
      return mob;
    }
};

TEST_F(BuilderDbTest, TBeingSaveMobWithPlayerId) {
  registerImmortalMobCleanup();

  auto* mob = makeTestMob();

  TBeingSave(tc->ch, mob, 99901);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT short_desc FROM mob WHERE vnum = 99901 AND player_id = {}",
    playerId)), "a test mob");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT player_id FROM mob WHERE vnum = 99901 AND player_id = {}",
    playerId)), std::to_string(playerId));

  delete mob;
}

TEST_F(BuilderDbTest, TBeingSaveUpsertUpdatesExistingMob) {
  registerImmortalMobCleanup();

  auto* mob = makeTestMob();
  TBeingSave(tc->ch, mob, 99901);

  // Modify and save again - UPSERT should update, not fail
  mob->shortDescr = "an updated test mob";
  TBeingSave(tc->ch, mob, 99901);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT short_desc FROM mob WHERE vnum = 99901 AND player_id = {}",
    playerId)), "an updated test mob");

  // Should still be exactly one row
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT COUNT(*) FROM mob WHERE vnum = 99901 AND player_id = {}",
    playerId)), "1");

  delete mob;
}

TEST_F(BuilderDbTest, MremoveCascadesToChildren) {
  registerImmortalMobCleanup();

  auto* mob = makeTestMob();
  TBeingSave(tc->ch, mob, 99901);
  delete mob;

  TDatabase db(DB_IMMORTAL);
  db.query(
    "INSERT INTO mob_imm (vnum, type, amt, player_id) "
    "VALUES (99901, 1, 50, %i)", playerId);
  db.query(
    "INSERT INTO mob_extra (vnum, keyword, description, player_id) "
    "VALUES (99901, 'test', 'A test description.', %i)", playerId);

  mremove(tc->ch, 99901);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT COUNT(*) FROM mob WHERE vnum = 99901 AND player_id = {}",
    playerId)), "0");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT COUNT(*) FROM mob_imm WHERE vnum = 99901 AND player_id = {}",
    playerId)), "0")
    << "mob_imm should be cleaned up by CASCADE";
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL, std::format(
    "SELECT COUNT(*) FROM mob_extra WHERE vnum = 99901 AND player_id = {}",
    playerId)), "0")
    << "mob_extra should be cleaned up by CASCADE";
}

TEST_F(BuilderDbTest, ObjSaveWritesWithPlayerId) {
  registerImmortalObjCleanup();

  auto* obj = new TTrash();
  obj->name = "test obj keywords";
  obj->shortDescr = "a test object";
  obj->setDescr("A test object lies here.\n");
  obj->setWeight(5.0);
  obj->max_exist = 1;
  obj->obj_flags.struct_points = 100;
  obj->obj_flags.max_struct_points = 100;

  ObjSave(tc->ch, obj, 99901);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT short_desc FROM obj WHERE vnum = 99901 AND player_id = {}",
        playerId)),
    "a test object");
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT player_id FROM obj WHERE vnum = 99901 AND player_id = {}",
        playerId)),
    std::to_string(playerId));

  delete obj;
}

TEST_F(BuilderDbTest, ObjLoadRetrievesByPlayerId) {
  registerImmortalObjCleanup();

  // Seed obj data directly in immortal DB
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO obj (vnum, name, short_desc, long_desc, type, action_flag, "
      "wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, "
      "spec_proc, max_exist, max_struct, cur_struct, decay, volume, material, "
      "player_id, action_desc) "
      "VALUES (99901, 'loaded obj', 'a loaded test object', "
      "'A loaded object lies here.', 13, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, "
      "1, 100, 100, 0, 1, 0, {}, '')",
      playerId));

  ObjLoad(tc->ch, 99901);

  // ObjLoad places the object in ch's inventory
  ASSERT_FALSE(tc->ch->stuff.empty())
    << "Loaded object should be in builder's inventory";
  auto* obj = dynamic_cast<TObj*>(*tc->ch->stuff.begin());
  ASSERT_NE(obj, nullptr);
  EXPECT_EQ(obj->name, "loaded obj");
  EXPECT_EQ(obj->shortDescr, "a loaded test object");
}

TEST_F(BuilderDbTest, OremoveCascadesToChildren) {
  registerImmortalObjCleanup();

  // Seed obj + children in immortal DB
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO obj (vnum, name, short_desc, long_desc, type, action_flag, "
      "wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, "
      "spec_proc, max_exist, max_struct, cur_struct, decay, volume, material, "
      "player_id, action_desc) "
      "VALUES (99901, 'removeme', 'remove test', 'Remove me.', 0, 0, 0, "
      "0, 0, 0, 0, 1, 0, 0, 0, 1, 100, 100, 0, 1, 0, {}, '')",
      playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objextra (vnum, name, description, player_id) "
                "VALUES (99901, 'label', 'A test label.', {})",
      playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objaffect (vnum, type, mod1, mod2, player_id) "
                "VALUES (99901, 1, 5, 0, {})",
      playerId));

  oremove(tc->ch, 99901);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM obj WHERE vnum = 99901 AND player_id = {}",
        playerId)),
    "0");
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM objextra WHERE vnum = 99901 AND player_id = {}",
        playerId)),
    "0")
    << "objextra should be cleaned up by CASCADE";
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM objaffect WHERE vnum = 99901 AND player_id = {}",
        playerId)),
    "0")
    << "objaffect should be cleaned up by CASCADE";
}

TEST_F(BuilderDbTest, RoomSaveWritesWithPlayerId) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(49901);
  testRoom.name = "Builder Test Room";
  testRoom.setDescr("A room built for testing.\n");

  RoomSave(tc->ch, 49901, 49901, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT name FROM room WHERE vnum = 49901 AND player_id = {}",
        playerId)),
    "Builder Test Room");
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT player_id FROM room WHERE vnum = 49901 AND player_id = {}",
        playerId)),
    std::to_string(playerId));
}

TEST_F(BuilderDbTest, RoomSaveWritesExitsAndExtras) {
  registerImmortalRoomCleanup();

  auto& room1 = makeRoom(49901);
  room1.name = "Room Alpha";
  room1.setDescr("First room.\n");

  auto& room2 = makeRoom(49902);
  room2.name = "Room Beta";
  room2.setDescr("Second room.\n");

  // Create an exit from 49901 south (dir 2) to 49902
  auto* exit = new roomDirData();
  exit->to_room = 49902;
  exit->door_type = DOOR_NONE;
  room1.dir_option[2] = exit;

  // Create an extra description on 49901
  auto* extra = new extraDescription();
  extra->keyword = "sign";
  extra->description = "A test sign.\n";
  extra->next = nullptr;
  room1.ex_description = extra;

  RoomSave(tc->ch, 49901, 49902, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT destination FROM roomexit "
                  "WHERE vnum = 49901 AND direction = 2 AND player_id = {}",
        playerId)),
    "49902");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format("SELECT name FROM roomextra "
                          "WHERE vnum = 49901 AND player_id = {}",
                playerId)),
    "sign");
}

TEST_F(BuilderDbTest, RoomSaveStoresNullForNegativeZone) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(49901);
  testRoom.name = "Zoneless Room";
  testRoom.setDescr("No zone.\n");
  // zone is nullptr by default, getZoneNum() returns -1

  RoomSave(tc->ch, 49901, 49901, 1);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format("SELECT COALESCE(zone, 'null') FROM room "
                          "WHERE vnum = 49901 AND player_id = {}",
                playerId)),
    "null");
}

TEST_F(BuilderDbTest, RoomSaveResavesCleanly) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(49901);
  testRoom.name = "Original Name";
  testRoom.setDescr("Original.\n");

  RoomSave(tc->ch, 49901, 49901, 1);

  // Modify and re-save (DELETE+INSERT pattern)
  testRoom.name = "Updated Name";
  RoomSave(tc->ch, 49901, 49901, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT name FROM room WHERE vnum = 49901 AND player_id = {}",
        playerId)),
    "Updated Name");
  // Should still be exactly one row
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM room WHERE vnum = 49901 AND player_id = {}",
        playerId)),
    "1");
}
