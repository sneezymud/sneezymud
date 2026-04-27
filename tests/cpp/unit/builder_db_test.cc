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

      // Create a synthetic test player to avoid borrowing a real player's ID.
      // Pre-cleanup handles leftovers from a previous crashed run.
      {
        TDatabase cleanup(DB_SNEEZY);
        cleanup.query("DELETE FROM player WHERE name = 'Testbuilder'");
        cleanup.query("DELETE FROM account WHERE name = 'builder_acct'");
      }

      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd, email, last_logon) "
        "VALUES ('builder_acct', 'x', '', 0)");
      acctId =
        convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));
      ASSERT_NE(acctId, 0);

      dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                       "VALUES ('Testbuilder', {})",
                             acctId));
      playerId =
        convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));
      ASSERT_NE(playerId, 0);
      tc->ch->player.player_id = playerId;

      // Allocate test vnums above the current max in the immortal database.
      // Builder functions write to immortal only.
      testMobVnum = convertTo<int>(dbQueryScalar(DB_IMMORTAL,
        "SELECT COALESCE(MAX(vnum), 0) + 1 FROM mob"));
      testObjVnum = convertTo<int>(dbQueryScalar(DB_IMMORTAL,
        "SELECT COALESCE(MAX(vnum), 0) + 1 FROM obj"));

      // Room vnums must also be valid for makeRoom(), which requires
      // vnum < WORLD_SIZE (50000). If the game world grows close to that
      // limit, this assertion will fail and WORLD_SIZE will need to be
      // increased or the test restructured to avoid makeRoom().
      testRoomVnum = convertTo<int>(dbQueryScalar(DB_IMMORTAL,
        "SELECT COALESCE(MAX(vnum), 0) + 1 FROM room"));
      ASSERT_LT(testRoomVnum + 1, WORLD_SIZE)
        << "Test room vnums would exceed WORLD_SIZE";
    }

    void TearDown() override {
      // Delete synthetic parent rows by ID - CASCADE cleans child tables,
      // making the base class dbCleanupLater queries no-ops (defensive).
      TDatabase db(DB_SNEEZY);
      db.query("DELETE FROM player WHERE id = %i", playerId);
      db.query("DELETE FROM account WHERE account_id = %i", acctId);

      DatabaseFixture::TearDown();
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;
    int acctId = 0;
    int testMobVnum = 0;
    int testObjVnum = 0;
    int testRoomVnum = 0;

    void registerImmortalMobCleanup() {
      auto del = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum = {} AND player_id = {}",
          table, testMobVnum, playerId);
      };
      dbCleanupLater(DB_IMMORTAL, del("mob_extra"));
      dbCleanupLater(DB_IMMORTAL, del("mob_imm"));
      dbCleanupLater(DB_IMMORTAL, del("mob"));
    }

    void registerImmortalObjCleanup() {
      auto del = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum = {} AND player_id = {}",
          table, testObjVnum, playerId);
      };
      dbCleanupLater(DB_IMMORTAL, del("objaffect"));
      dbCleanupLater(DB_IMMORTAL, del("objextra"));
      dbCleanupLater(DB_IMMORTAL, del("obj"));
    }

    void registerImmortalRoomCleanup() {
      auto del = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN ({}, {}) AND player_id = {}", table,
          testRoomVnum, testRoomVnum + 1, playerId);
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

  TBeingSave(tc->ch, mob, testMobVnum);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT short_desc FROM mob WHERE vnum = {} AND player_id = {}",
                testMobVnum, playerId)),
    "a test mob");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT player_id FROM mob WHERE vnum = {} AND player_id = {}",
                testMobVnum, playerId)),
    std::to_string(playerId));

  delete mob;
}

TEST_F(BuilderDbTest, TBeingSaveUpsertUpdatesExistingMob) {
  registerImmortalMobCleanup();

  auto* mob = makeTestMob();
  TBeingSave(tc->ch, mob, testMobVnum);

  // Modify and save again - UPSERT should update, not fail
  mob->shortDescr = "an updated test mob";
  TBeingSave(tc->ch, mob, testMobVnum);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT short_desc FROM mob WHERE vnum = {} AND player_id = {}",
                testMobVnum, playerId)),
    "an updated test mob");

  // Should still be exactly one row
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT COUNT(*) FROM mob WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "1");

  delete mob;
}

TEST_F(BuilderDbTest, MremoveCascadesToChildren) {
  registerImmortalMobCleanup();

  auto* mob = makeTestMob();
  TBeingSave(tc->ch, mob, testMobVnum);
  delete mob;

  // Verify parent row exists before testing CASCADE
  ASSERT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT COUNT(*) FROM mob WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "1")
    << "TBeingSave must create the parent mob row";

  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mob_imm (vnum, type, amt, player_id) "
                "VALUES ({}, 1, 50, {})",
      testMobVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mob_extra (vnum, keyword, description, player_id) "
                "VALUES ({}, 'test', 'A test description.', {})",
      testMobVnum, playerId));

  // Verify children exist before deletion
  ASSERT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM mob_imm WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "1")
    << "mob_imm child row must exist before testing CASCADE";
  ASSERT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM mob_extra WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "1")
    << "mob_extra child row must exist before testing CASCADE";

  mremove(tc->ch, testMobVnum);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT COUNT(*) FROM mob WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "0");
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM mob_imm WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "0")
    << "mob_imm should be cleaned up by CASCADE";
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM mob_extra WHERE vnum = {} AND player_id = {}",
        testMobVnum, playerId)),
    "0")
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

  ObjSave(tc->ch, obj, testObjVnum);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT short_desc FROM obj WHERE vnum = {} AND player_id = {}",
                testObjVnum, playerId)),
    "a test object");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT player_id FROM obj WHERE vnum = {} AND player_id = {}",
                testObjVnum, playerId)),
    std::to_string(playerId));

  delete obj;
}

TEST_F(BuilderDbTest, ObjLoadRetrievesByPlayerId) {
  registerImmortalObjCleanup();

  // Create a second player to own a decoy row at the same vnum.
  // This proves ObjLoad filters by player_id, not just vnum.
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('builder_acct2', 'x', '', 0)");
  auto decoyAcctId =
    convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO player (name, account_id) VALUES ('Testbuilder2', "
                "{})",
      decoyAcctId));
  auto decoyPlayerId =
    convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));

  dbCleanupLater(DB_IMMORTAL,
    std::format("DELETE FROM obj WHERE vnum = {} AND player_id = {}",
      testObjVnum, decoyPlayerId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id = {}", decoyPlayerId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id = {}", decoyAcctId));

  // Decoy row: same vnum, different player_id, distinct name
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO obj (vnum, name, short_desc, long_desc, type, action_flag, "
      "wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, "
      "spec_proc, max_exist, max_struct, cur_struct, decay, volume, material, "
      "player_id, action_desc) "
      "VALUES ({}, 'decoy obj', 'a decoy object', "
      "'A decoy lies here.', 13, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, "
      "1, 100, 100, 0, 1, 0, {}, '')",
      testObjVnum, decoyPlayerId));

  // Real row: same vnum, builder's player_id
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO obj (vnum, name, short_desc, long_desc, type, action_flag, "
      "wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, "
      "spec_proc, max_exist, max_struct, cur_struct, decay, volume, material, "
      "player_id, action_desc) "
      "VALUES ({}, 'loaded obj', 'a loaded test object', "
      "'A loaded object lies here.', 13, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, "
      "1, 100, 100, 0, 1, 0, {}, '')",
      testObjVnum, playerId));

  ObjLoad(tc->ch, testObjVnum);

  // ObjLoad places the object in ch's inventory
  ASSERT_FALSE(tc->ch->stuff.empty())
    << "Loaded object should be in builder's inventory";
  auto* obj = dynamic_cast<TObj*>(*tc->ch->stuff.begin());
  ASSERT_NE(obj, nullptr);
  EXPECT_EQ(obj->name, "loaded obj") << "Should load builder's row, not decoy";
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
      "VALUES ({}, 'removeme', 'remove test', 'Remove me.', 0, 0, 0, "
      "0, 0, 0, 0, 1, 0, 0, 0, 1, 100, 100, 0, 1, 0, {}, '')",
      testObjVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objextra (vnum, name, description, player_id) "
                "VALUES ({}, 'label', 'A test label.', {})",
      testObjVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objaffect (vnum, type, mod1, mod2, player_id) "
                "VALUES ({}, 1, 5, 0, {})",
      testObjVnum, playerId));

  oremove(tc->ch, testObjVnum);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT COUNT(*) FROM obj WHERE vnum = {} AND player_id = {}",
        testObjVnum, playerId)),
    "0");
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM objextra WHERE vnum = {} AND player_id = {}",
        testObjVnum, playerId)),
    "0")
    << "objextra should be cleaned up by CASCADE";
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format(
        "SELECT COUNT(*) FROM objaffect WHERE vnum = {} AND player_id = {}",
        testObjVnum, playerId)),
    "0")
    << "objaffect should be cleaned up by CASCADE";
}

TEST_F(BuilderDbTest, RoomSaveWritesWithPlayerId) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(testRoomVnum);
  testRoom.name = "Builder Test Room";
  testRoom.setDescr("A room built for testing.\n");

  RoomSave(tc->ch, testRoomVnum, testRoomVnum, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT name FROM room WHERE vnum = {} AND player_id = {}",
        testRoomVnum, playerId)),
    "Builder Test Room");
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT player_id FROM room WHERE vnum = {} AND player_id = {}",
                testRoomVnum, playerId)),
    std::to_string(playerId));
}

TEST_F(BuilderDbTest, RoomSaveWritesExitsAndExtras) {
  registerImmortalRoomCleanup();

  auto& room1 = makeRoom(testRoomVnum);
  room1.name = "Room Alpha";
  room1.setDescr("First room.\n");

  auto& room2 = makeRoom(testRoomVnum + 1);
  room2.name = "Room Beta";
  room2.setDescr("Second room.\n");

  // Create an exit from testRoomVnum south (dir 2) to testRoomVnum+1
  auto* exit = new roomDirData();
  exit->to_room = testRoomVnum + 1;
  exit->door_type = DOOR_NONE;
  room1.dir_option[2] = exit;

  // Create an extra description on testRoomVnum
  auto* extra = new extraDescription();
  extra->keyword = "sign";
  extra->description = "A test sign.\n";
  extra->next = nullptr;
  room1.ex_description = extra;

  RoomSave(tc->ch, testRoomVnum, testRoomVnum + 1, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT destination FROM roomexit "
                  "WHERE vnum = {} AND direction = 2 AND player_id = {}",
        testRoomVnum, playerId)),
    std::to_string(testRoomVnum + 1));
  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL, std::format("SELECT name FROM roomextra "
                                           "WHERE vnum = {} AND player_id = {}",
                                 testRoomVnum, playerId)),
    "sign");
}

TEST_F(BuilderDbTest, RoomSaveStoresNullForNegativeZone) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(testRoomVnum);
  testRoom.name = "Zoneless Room";
  testRoom.setDescr("No zone.\n");
  // zone is nullptr by default, getZoneNum() returns -1

  RoomSave(tc->ch, testRoomVnum, testRoomVnum, 1);

  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format("SELECT COALESCE(zone, 'null') FROM room "
                          "WHERE vnum = {} AND player_id = {}",
                testRoomVnum, playerId)),
    "null");
}

TEST_F(BuilderDbTest, RoomSaveResavesCleanly) {
  registerImmortalRoomCleanup();

  auto& testRoom = makeRoom(testRoomVnum);
  testRoom.name = "Original Name";
  testRoom.setDescr("Original.\n");

  RoomSave(tc->ch, testRoomVnum, testRoomVnum, 1);

  // Modify and re-save (DELETE+INSERT pattern)
  testRoom.name = "Updated Name";
  RoomSave(tc->ch, testRoomVnum, testRoomVnum, 1);

  EXPECT_EQ(
    dbQueryScalar(DB_IMMORTAL,
      std::format("SELECT name FROM room WHERE vnum = {} AND player_id = {}",
        testRoomVnum, playerId)),
    "Updated Name");
  // Should still be exactly one row
  EXPECT_EQ(dbQueryScalar(DB_IMMORTAL,
              std::format(
                "SELECT COUNT(*) FROM room WHERE vnum = {} AND player_id = {}",
                testRoomVnum, playerId)),
    "1");
}
