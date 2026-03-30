// Integration tests for the "low mv*" commands (mvMob, mvObj, mvRoom,
// mvResponse). These functions copy builder-owned data from the immortal
// database to the sneezy (live) database using INSERT ON DUPLICATE KEY UPDATE.
//
// Test vnums are dynamically allocated above the current max to avoid
// colliding with real game data.
// Requires a live MariaDB with both sneezy and immortal databases.

#include <format>
#include <gtest/gtest.h>

#include "database.h"
#include "database_fixture.h"
#include "parse.h"
#include "person.h"

// mv* functions are free functions in cmd_low.cc with no header.
extern void mvMob(TPerson& ch, int playerId, const sstring& rooms);
extern void mvObj(TPerson& ch, int playerId, const sstring& rooms);
extern void mvRoom(TPerson& ch, int playerId, int block, const sstring& rooms);
extern void mvResponse(TPerson& ch, int playerId, const sstring& builderName,
  const sstring& vnumsString);

class LowMvTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      tc = &makeCharacter("TestBuilder");
      room = &makeRoom(49999);
      placeInRoom(*tc, *room);

      // Create a test account+player for mv* functions. CI seeds the
      // schema but not player/account data.
      {
        TDatabase cleanup(DB_SNEEZY);
        cleanup.query("DELETE FROM player WHERE name = 'Testbuilder_mv'");
        cleanup.query("DELETE FROM account WHERE name = 'testbuilder_mv_acct'");
      }
      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd, email, last_logon) "
        "VALUES ('testbuilder_mv_acct', 'x', '', 0)");
      auto acctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                       "VALUES ('Testbuilder_mv', {})",
                             acctId));
      auto pidStr = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(pidStr.empty()) << "Failed to create test player";
      playerId = convertTo<int>(pidStr);

      // Clean up test player after all other cleanup (parent last)
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM player WHERE id = {}", playerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM account WHERE account_id = {}", acctId));

      // Allocate test vnums above the current max across both databases.
      // mv* commands copy from immortal to sneezy, so vnums must be
      // unique in both.
      testMobVnum = std::max(convertTo<int>(dbQueryScalar(DB_SNEEZY,
                               "SELECT COALESCE(MAX(vnum), 0) FROM mob")),
                      convertTo<int>(dbQueryScalar(DB_IMMORTAL,
                        "SELECT COALESCE(MAX(vnum), 0) FROM mob"))) +
                    1;

      testObjVnum = std::max(convertTo<int>(dbQueryScalar(DB_SNEEZY,
                               "SELECT COALESCE(MAX(vnum), 0) FROM obj")),
                      convertTo<int>(dbQueryScalar(DB_IMMORTAL,
                        "SELECT COALESCE(MAX(vnum), 0) FROM obj"))) +
                    1;

      testRoomVnum = std::max(convertTo<int>(dbQueryScalar(DB_SNEEZY,
                                "SELECT COALESCE(MAX(vnum), 0) FROM room")),
                       convertTo<int>(dbQueryScalar(DB_IMMORTAL,
                         "SELECT COALESCE(MAX(vnum), 0) FROM room"))) +
                     1;
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;
    int testMobVnum = 0;
    int testObjVnum = 0;
    int testRoomVnum = 0;

    // Helper: register cleanup for both databases for test vnums.
    // Children first, parents last. Call at the TOP of each test.
    void registerMobCleanup() {
      auto sneezyDel = [&](const char* table) {
        // trophy uses 'mobvnum' not 'vnum'
        if (std::string_view(table) == "trophy") {
          return std::format("DELETE FROM trophy WHERE mobvnum IN ({}, {})",
            testMobVnum, testMobVnum + 1);
        }
        return std::format("DELETE FROM {} WHERE vnum IN ({}, {})", table,
          testMobVnum, testMobVnum + 1);
      };
      auto immortalDel = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN ({}, {}) AND player_id = {}", table,
          testMobVnum, testMobVnum + 1, playerId);
      };

      // sneezy children then parent
      dbCleanupLater(DB_SNEEZY, sneezyDel("trophy"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob_extra"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob_imm"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mobresponses"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob"));

      // immortal children then parent
      dbCleanupLater(DB_IMMORTAL, immortalDel("mob_extra"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("mob_imm"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("mobresponses"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("mob"));
    }

    void registerObjCleanup() {
      auto sneezyDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum = {}", table,
          testObjVnum);
      };
      auto immortalDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum = {} AND player_id = {}",
          table, testObjVnum, playerId);
      };

      dbCleanupLater(DB_SNEEZY, sneezyDel("objaffect"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("objextra"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("obj"));

      dbCleanupLater(DB_IMMORTAL, immortalDel("objextra"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("objaffect"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("obj"));
    }

    void registerRoomCleanup() {
      auto sneezyDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum IN ({}, {})", table,
          testRoomVnum, testRoomVnum + 1);
      };
      auto immortalDel = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN ({}, {}) AND player_id = {}", table,
          testRoomVnum, testRoomVnum + 1, playerId);
      };

      dbCleanupLater(DB_SNEEZY, sneezyDel("roomexit"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("roomextra"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("room"));

      dbCleanupLater(DB_IMMORTAL, immortalDel("roomexit"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("roomextra"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("room"));
    }

    // Helper: insert a minimal mob into the immortal database. Uses
    // TDatabase %s for string interpolation (safe against SQL-special
    // characters like quotes).
    void insertImmortalMob(int vnum, const char* shortDesc = "a test mob") {
      TDatabase db(DB_IMMORTAL);
      db.query(
        "INSERT INTO mob (vnum, name, short_desc, long_desc, description, "
        "actions, affects, faction, fact_perc, letter, attacks, class, level, "
        "tohit, ac, hpbonus, damage_level, damage_precision, gold, race, "
        "weight, height, str, bra, con, dex, agi, intel, wis, foc, per, cha, "
        "kar, spe, pos, def_position, sex, spec_proc, skin, vision, "
        "can_be_seen, max_exist, local_sound, adjacent_sound, player_id) "
        "VALUES (%i, 'test mob keywords', '%s', 'A test mob stands here.', "
        "'This is a test mob.', 0, 0, 0, 0, 'L', 1.0, 0, 10, 0, 0.0, 0.0, "
        "0.0, 0, 0, 1, 100, 72, 100, 100, 100, 100, 100, 100, 100, 100, 100, "
        "100, 100, 100, 8, 8, 0, 0, 0, 0, 0, 1, '', '', %i)",
        vnum, shortDesc, playerId);
    }

    void insertImmortalMob(const char* shortDesc = "a test mob") {
      insertImmortalMob(testMobVnum, shortDesc);
    }

    void insertImmortalObj(const char* shortDesc = "a test object") {
      TDatabase db(DB_IMMORTAL);
      db.query(
        "INSERT INTO obj (vnum, name, short_desc, long_desc, action_desc, "
        "type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, "
        "can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, "
        "volume, material, player_id) "
        "VALUES (%i, 'test obj keywords', '%s', "
        "'A test object lies here.', '', 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "
        "1, 100, 100, 0, 1, 0, %i)",
        testObjVnum, shortDesc, playerId);
    }

    void insertImmortalRooms() {
      int v1 = testRoomVnum;
      int v2 = testRoomVnum + 1;

      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO room (vnum, x, y, z, name, description, zone, "
          "room_flag, sector, teletime, teletarg, telelook, river_speed, "
          "river_dir, capacity, height, spec, player_id, block) "
          "VALUES ({}, 0, 0, 0, 'Test Room Alpha', 'A test room.', 0, "
          "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
          v1, playerId));
      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO room (vnum, x, y, z, name, description, zone, "
          "room_flag, sector, teletime, teletarg, telelook, river_speed, "
          "river_dir, capacity, height, spec, player_id, block) "
          "VALUES ({}, 0, 0, 0, 'Test Room Beta', 'Another test room.', "
          "0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
          v2, playerId));

      // v1 south (direction 2) to v2
      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO roomexit (vnum, direction, name, description, type, "
          "condition_flag, lock_difficulty, weight, key_num, destination, "
          "player_id, block) "
          "VALUES ({}, 2, '', '', 0, 0, 0, 0, 0, {}, {}, 1)",
          v1, v2, playerId));
      // v2 north (direction 0) to v1
      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO roomexit (vnum, direction, name, description, type, "
          "condition_flag, lock_difficulty, weight, key_num, destination, "
          "player_id, block) "
          "VALUES ({}, 0, '', '', 0, 0, 0, 0, 0, {}, {}, 1)",
          v2, v1, playerId));
    }
};

// -------------------------------------------------------------------
// mvMob tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvMobInsertsNewMobWithChildData) {
  registerMobCleanup();

  insertImmortalMob();
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mob_extra (vnum, keyword, description, player_id) "
                "VALUES ({}, 'test keyword', 'A test extra description.', {})",
      testMobVnum, playerId));

  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM mob WHERE vnum = {}", testMobVnum)),
    "a test mob");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT keyword FROM mob_extra WHERE vnum = {}",
                testMobVnum)),
    "test keyword");
}

TEST_F(LowMvTest, MvMobUpdatesExistingMobAndReplacesChildData) {
  registerMobCleanup();

  // Initial insert + move
  insertImmortalMob();
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mob_extra (vnum, keyword, description, player_id) "
                "VALUES ({}, 'test keyword', 'A test extra description.', {})",
      testMobVnum, playerId));
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  // Modify immortal data
  dbExecute(DB_IMMORTAL,
    std::format("UPDATE mob SET short_desc = 'an updated test mob' "
                "WHERE vnum = {} AND player_id = {}",
      testMobVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("DELETE FROM mob_extra WHERE vnum = {} AND player_id = {}",
      testMobVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO mob_extra (vnum, keyword, description, player_id) "
      "VALUES ({}, 'updated keyword', 'Updated extra description.', {})",
      testMobVnum, playerId));

  // Move again - should update, not fail
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM mob WHERE vnum = {}", testMobVnum)),
    "an updated test mob");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT keyword FROM mob_extra WHERE vnum = {}",
                testMobVnum)),
    "updated keyword");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM mob_extra WHERE vnum = {}",
                testMobVnum)),
    "1");
}

TEST_F(LowMvTest, MvMobPreservesExternalFkDuringUpdate) {
  registerMobCleanup();

  insertImmortalMob();
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  // Trophy row references this mob via FK
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO trophy (player_id, mobvnum, count, totalcount) "
                "VALUES ({}, {}, 1, 1)",
      playerId, testMobVnum));

  // Update the mob in immortal and re-move
  dbExecute(DB_IMMORTAL,
    std::format("UPDATE mob SET short_desc = 'post-trophy update' "
                "WHERE vnum = {} AND player_id = {}",
      testMobVnum, playerId));
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM mob WHERE vnum = {}", testMobVnum)),
    "post-trophy update");

  // Trophy survived - proves UPSERT, not DELETE+INSERT
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT count FROM trophy WHERE player_id = {} AND mobvnum = {}",
        playerId, testMobVnum)),
    "1");
}

TEST_F(LowMvTest, MvMobHandlesSpecialCharacters) {
  registerMobCleanup();

  insertImmortalMob();
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  // Use TDatabase %s format for safe escaping of the apostrophe
  TDatabase updater(DB_IMMORTAL);
  updater.query(
    "UPDATE mob SET short_desc='%s', name='%s' "
    "WHERE vnum=%i AND player_id=%i",
    "the baker's apprentice", "baker's apprentice test", testMobVnum, playerId);

  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM mob WHERE vnum = {}", testMobVnum)),
    "the baker's apprentice");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT name FROM mob WHERE vnum = {}", testMobVnum)),
    "baker's apprentice test");
}

TEST_F(LowMvTest, MvMobTransfersMobImmChildData) {
  registerMobCleanup();

  insertImmortalMob();
  TDatabase db(DB_IMMORTAL);
  db.query(
    "INSERT INTO mob_imm (vnum, type, amt, player_id) "
    "VALUES (%i, 5, 100, %i)",
    testMobVnum, playerId);

  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT amt FROM mob_imm WHERE vnum = {} AND type = 5",
        testMobVnum)),
    "100");
}

TEST_F(LowMvTest, MvMobHandlesMultipleVnums) {
  registerMobCleanup();

  insertImmortalMob(testMobVnum, "first mob");
  insertImmortalMob(testMobVnum + 1, "second mob");

  mvMob(*tc->ch, playerId, std::format("{} {}", testMobVnum, testMobVnum + 1));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM mob WHERE vnum = {}", testMobVnum)),
    "first mob");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT short_desc FROM mob WHERE vnum = {}",
                testMobVnum + 1)),
    "second mob");
}

// -------------------------------------------------------------------
// mvObj tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvObjInsertsNewObjWithChildData) {
  registerObjCleanup();

  insertImmortalObj();
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objextra (vnum, name, description, player_id) "
                "VALUES ({}, 'test label', 'A test extra description.', {})",
      testObjVnum, playerId));

  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM obj WHERE vnum = {}", testObjVnum)),
    "a test object");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM objextra WHERE vnum = {}", testObjVnum)),
    "test label");
}

TEST_F(LowMvTest, MvObjUpdatesExistingObjAndReplacesChildData) {
  registerObjCleanup();

  // Initial insert + move
  insertImmortalObj();
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objextra (vnum, name, description, player_id) "
                "VALUES ({}, 'test label', 'A test extra description.', {})",
      testObjVnum, playerId));
  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  // Modify immortal data
  dbExecute(DB_IMMORTAL,
    std::format("UPDATE obj SET short_desc = 'an updated test object' "
                "WHERE vnum = {} AND player_id = {}",
      testObjVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("DELETE FROM objextra WHERE vnum = {} AND player_id = {}",
      testObjVnum, playerId));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO objextra (vnum, name, description, player_id) "
                "VALUES ({}, 'updated label', 'Updated extra.', {})",
      testObjVnum, playerId));

  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM obj WHERE vnum = {}", testObjVnum)),
    "an updated test object");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM objextra WHERE vnum = {}", testObjVnum)),
    "updated label");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM objextra WHERE vnum = {}",
                testObjVnum)),
    "1");
}

TEST_F(LowMvTest, MvObjTransfersObjaffectChildData) {
  registerObjCleanup();

  insertImmortalObj();
  TDatabase db(DB_IMMORTAL);
  db.query(
    "INSERT INTO objaffect (vnum, type, mod1, mod2, player_id) "
    "VALUES (%i, 3, 10, 0, %i)",
    testObjVnum, playerId);

  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT mod1 FROM objaffect WHERE vnum = {} AND type = 3",
        testObjVnum)),
    "10");
}

TEST_F(LowMvTest, MvObjHandlesSpecialCharacters) {
  registerObjCleanup();

  insertImmortalObj();
  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  TDatabase updater(DB_IMMORTAL);
  updater.query(
    "UPDATE obj SET short_desc='%s', name='%s' "
    "WHERE vnum=%i AND player_id=%i",
    "the baker's rolling pin", "baker's rolling pin test", testObjVnum,
    playerId);

  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM obj WHERE vnum = {}", testObjVnum)),
    "the baker's rolling pin");
}

TEST_F(LowMvTest, MvObjPreservesExternalFkDuringUpdate) {
  // Allocate a rent_id above the current max to avoid collisions
  auto rentIdStr =
    dbQueryScalar(DB_SNEEZY, "SELECT COALESCE(MAX(rent_id), 0) + 1 FROM rent");
  ASSERT_FALSE(rentIdStr.empty());
  auto rentId = convertTo<int>(rentIdStr);

  // Rent has FK RESTRICT on obj.vnum - clean up rent BEFORE obj
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM rent WHERE rent_id = {}", rentId));
  registerObjCleanup();

  insertImmortalObj();
  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  // Insert a rent row referencing this obj (rent.vnum FK)
  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO rent (rent_id, owner, owner_type, vnum, val0, val1, val2, "
      "val3, extra_flags, weight, bitvector, cur_struct, max_struct, "
      "decay, material, volume, price) "
      "VALUES ({}, 0, 'player', {}, 0, 0, 0, 0, 0, 1, 0, 100, 100, "
      "0, 0, 1, 0)",
      rentId, testObjVnum));

  dbExecute(DB_IMMORTAL,
    std::format("UPDATE obj SET short_desc = 'post-rent update' "
                "WHERE vnum = {} AND player_id = {}",
      testObjVnum, playerId));
  mvObj(*tc->ch, playerId, std::to_string(testObjVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT short_desc FROM obj WHERE vnum = {}", testObjVnum)),
    "post-rent update");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT COUNT(*) FROM rent WHERE rent_id = {}", rentId)),
    "1")
    << "Rent row should survive obj UPSERT (proves UPSERT, not DELETE+INSERT)";
}

// -------------------------------------------------------------------
// mvRoom tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvRoomInsertsWithCrossReferencingExitsAndRoomextra) {
  registerRoomCleanup();

  insertImmortalRooms();
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO roomextra (vnum, name, description, player_id, block) "
      "VALUES ({}, 'test sign', 'A sign reads: Testing!', {}, 1)",
      testRoomVnum, playerId));

  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 1));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM room WHERE vnum = {}", testRoomVnum)),
    "Test Room Alpha");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM room WHERE vnum = {}", testRoomVnum + 1)),
    "Test Room Beta");

  // Cross-referencing exits work because mvRoom inserts rooms before exits
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT destination FROM roomexit WHERE vnum = {} AND direction = 2",
        testRoomVnum)),
    std::to_string(testRoomVnum + 1));
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT destination FROM roomexit WHERE vnum = {} AND direction = 0",
        testRoomVnum + 1)),
    std::to_string(testRoomVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM roomextra WHERE vnum = {}", testRoomVnum)),
    "test sign");
}

TEST_F(LowMvTest, MvRoomUpdatesExistingRoom) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 1));

  // Update room name in immortal
  dbExecute(DB_IMMORTAL,
    std::format("UPDATE room SET name = 'Updated Room Alpha' "
                "WHERE vnum = {} AND player_id = {}",
      testRoomVnum, playerId));

  mvRoom(*tc->ch, playerId, 1, std::to_string(testRoomVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM room WHERE vnum = {}", testRoomVnum)),
    "Updated Room Alpha");
}

TEST_F(LowMvTest, MvRoomDropsExitsToNonExistentRooms) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 1));

  // Add an exit east (direction 1) to a room that doesn't exist in sneezy
  int nonExistentRoom = testRoomVnum + 100;
  dbExecute(DB_IMMORTAL,
    std::format(
      "INSERT INTO roomexit (vnum, direction, name, description, type, "
      "condition_flag, lock_difficulty, weight, key_num, destination, "
      "player_id, block) "
      "VALUES ({}, 1, '', '', 0, 0, 0, 0, 0, {}, {}, 1)",
      testRoomVnum, nonExistentRoom, playerId));

  mvRoom(*tc->ch, playerId, 1, std::to_string(testRoomVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT name FROM room WHERE vnum = {}", testRoomVnum)),
    "Test Room Alpha");

  // Valid south exit survived despite invalid east exit failing
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT destination FROM roomexit WHERE vnum = {} AND direction = 2",
        testRoomVnum)),
    std::to_string(testRoomVnum + 1));

  // Invalid east exit was NOT created (FK RESTRICT rejects it)
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT COUNT(*) FROM roomexit WHERE vnum = {} AND direction = 1",
        testRoomVnum)),
    "0");
}

TEST_F(LowMvTest, MvRoomSkipsVnumsNotInImmortalDb) {
  registerRoomCleanup();

  insertImmortalRooms();
  // Move a range including testRoomVnum+2, which doesn't exist in immortal
  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 2));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT COUNT(*) FROM room WHERE vnum = {}", testRoomVnum)),
    "1");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM room WHERE vnum = {}",
                testRoomVnum + 1)),
    "1");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM room WHERE vnum = {}",
                testRoomVnum + 2)),
    "0")
    << "Room not in immortal DB should not appear in sneezy";
}

TEST_F(LowMvTest, MvRoomUpdatesExistingExits) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 1));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT type FROM roomexit WHERE vnum = {} AND direction = 2",
        testRoomVnum)),
    "0");

  dbExecute(DB_IMMORTAL,
    std::format(
      "UPDATE roomexit SET type = 1 WHERE vnum = {} AND direction = 2 "
      "AND player_id = {}",
      testRoomVnum, playerId));

  mvRoom(*tc->ch, playerId, 1,
    std::format("{}-{}", testRoomVnum, testRoomVnum + 1));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT type FROM roomexit WHERE vnum = {} AND direction = 2",
        testRoomVnum)),
    "1");
}

TEST_F(LowMvTest, MvRoomPreservesNullZone) {
  registerRoomCleanup();

  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO room (vnum, x, y, z, name, description, zone, "
                "room_flag, sector, teletime, teletarg, telelook, river_speed, "
                "river_dir, capacity, height, spec, player_id, block) "
                "VALUES ({}, 0, 0, 0, 'Zoneless Room', 'No zone.', null, "
                "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
      testRoomVnum, playerId));

  mvRoom(*tc->ch, playerId, 1, std::to_string(testRoomVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT COALESCE(zone, 'null') FROM room WHERE vnum = {}",
        testRoomVnum)),
    "null")
    << "Room with no zone should store null, not -1";
}

TEST_F(LowMvTest, MvRoomStoresValidZone) {
  registerRoomCleanup();

  auto validZone = dbQueryScalar(DB_SNEEZY, "SELECT MIN(zone_nr) FROM zone");
  ASSERT_FALSE(validZone.empty());

  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO room (vnum, x, y, z, name, description, zone, "
                "room_flag, sector, teletime, teletarg, telelook, river_speed, "
                "river_dir, capacity, height, spec, player_id, block) "
                "VALUES ({}, 0, 0, 0, 'Zoned Room', 'Has zone.', {}, "
                "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
      testRoomVnum, validZone, playerId));

  mvRoom(*tc->ch, playerId, 1, std::to_string(testRoomVnum));

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT zone FROM room WHERE vnum = {}", testRoomVnum)),
    validZone);
}

// -------------------------------------------------------------------
// mvResponse tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvResponseInsertsNewResponse) {
  registerMobCleanup();

  // Prerequisite: mob must exist in sneezy (FK)
  insertImmortalMob();
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));

  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mobresponses (vnum, response, player_id) "
                "VALUES ({}, 'say Hello, test!', {})",
      testMobVnum, playerId));

  mvResponse(*tc->ch, playerId, "TestBuilder", std::to_string(testMobVnum));

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT response FROM mobresponses WHERE vnum = {}",
                testMobVnum)),
    "say Hello, test!");
}

TEST_F(LowMvTest, MvResponseUpdatesExistingResponse) {
  registerMobCleanup();

  // Create mob + initial response
  insertImmortalMob();
  mvMob(*tc->ch, playerId, std::to_string(testMobVnum));
  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO mobresponses (vnum, response, player_id) "
                "VALUES ({}, 'say Hello, test!', {})",
      testMobVnum, playerId));
  mvResponse(*tc->ch, playerId, "TestBuilder", std::to_string(testMobVnum));

  // Update response in immortal
  dbExecute(DB_IMMORTAL,
    std::format("UPDATE mobresponses SET response = 'say Updated response!' "
                "WHERE vnum = {} AND player_id = {}",
      testMobVnum, playerId));

  mvResponse(*tc->ch, playerId, "TestBuilder", std::to_string(testMobVnum));

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT response FROM mobresponses WHERE vnum = {}",
                testMobVnum)),
    "say Updated response!");
}

// -------------------------------------------------------------------
// FK constraint verification
// -------------------------------------------------------------------

TEST_F(LowMvTest, WorldDataFkConstraintsExist) {
  auto countStr = dbQueryScalar(DB_SNEEZY,
    "SELECT COUNT(*) FROM information_schema.KEY_COLUMN_USAGE "
    "WHERE TABLE_SCHEMA = 'sneezy' "
    "AND REFERENCED_TABLE_NAME IN ('mob', 'obj', 'room') "
    "AND REFERENCED_COLUMN_NAME = 'vnum'");

  ASSERT_FALSE(countStr.empty()) << "information_schema query returned no rows";

  // 7 pre-existing + 13 new = 20 minimum
  EXPECT_GE(convertTo<int>(countStr), 20);
}
