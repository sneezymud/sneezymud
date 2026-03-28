// Integration tests for the "low mv*" commands (mvMob, mvObj, mvRoom,
// mvResponse). These functions copy builder-owned data from the immortal
// database to the sneezy (live) database using INSERT ON DUPLICATE KEY UPDATE.
//
// Test vnums: 99901, 99902 - high enough to avoid colliding with real data.
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

      auto pidStr =
        dbQueryScalar(DB_SNEEZY, "SELECT MIN(id) FROM player");
      ASSERT_FALSE(pidStr.empty()) << "No player rows in sneezy.player";
      playerId = convertTo<int>(pidStr);
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;

    // Helper: register cleanup for both databases for test vnums.
    // Children first, parents last. Call at the TOP of each test.
    void registerMobCleanup() {
      auto sneezyDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum IN (99901, 99902)", table);
      };
      auto immortalDel = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN (99901, 99902) AND player_id = {}",
          table, playerId);
      };

      // sneezy children then parent
      dbCleanupLater(DB_SNEEZY, sneezyDel("trophy"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob_extra"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob_imm"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mobresponses"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("mob"));

      // immortal children then parent
      dbCleanupLater(DB_IMMORTAL, immortalDel("mob_extra"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("mobresponses"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("mob"));
    }

    void registerObjCleanup() {
      auto sneezyDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum = 99901", table);
      };
      auto immortalDel = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum = 99901 AND player_id = {}",
          table, playerId);
      };

      dbCleanupLater(DB_SNEEZY, sneezyDel("objaffect"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("objextra"));
      dbCleanupLater(DB_SNEEZY, sneezyDel("obj"));

      dbCleanupLater(DB_IMMORTAL, immortalDel("objextra"));
      dbCleanupLater(DB_IMMORTAL, immortalDel("obj"));
    }

    void registerRoomCleanup() {
      auto sneezyDel = [&](const char* table) {
        return std::format("DELETE FROM {} WHERE vnum IN (99901, 99902)", table);
      };
      auto immortalDel = [&](const char* table) {
        return std::format(
          "DELETE FROM {} WHERE vnum IN (99901, 99902) AND player_id = {}",
          table, playerId);
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
    void insertImmortalMob(int vnum = 99901,
      const char* shortDesc = "a test mob") {
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

    void insertImmortalObj(const char* shortDesc = "a test object") {
      TDatabase db(DB_IMMORTAL);
      db.query(
        "INSERT INTO obj (vnum, name, short_desc, long_desc, action_desc, "
        "type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, "
        "can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, "
        "volume, material, player_id) "
        "VALUES (99901, 'test obj keywords', '%s', "
        "'A test object lies here.', '', 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, "
        "1, 100, 100, 0, 1, 0, %i)",
        shortDesc, playerId);
    }

    void insertImmortalRooms() {
      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO room (vnum, x, y, z, name, description, zone, "
          "room_flag, sector, teletime, teletarg, telelook, river_speed, "
          "river_dir, capacity, height, spec, player_id, block) "
          "VALUES (99901, 0, 0, 0, 'Test Room Alpha', 'A test room.', 0, "
          "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
          playerId));
      dbExecute(DB_IMMORTAL,
        std::format(
          "INSERT INTO room (vnum, x, y, z, name, description, zone, "
          "room_flag, sector, teletime, teletarg, telelook, river_speed, "
          "river_dir, capacity, height, spec, player_id, block) "
          "VALUES (99902, 0, 0, 0, 'Test Room Beta', 'Another test room.', "
          "0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
          playerId));

      // Room 99901 south (direction 2) to 99902
      dbExecute(DB_IMMORTAL, std::format(
        "INSERT INTO roomexit (vnum, direction, name, description, type, "
        "condition_flag, lock_difficulty, weight, key_num, destination, "
        "player_id, block) "
        "VALUES (99901, 2, '', '', 0, 0, 0, 0, 0, 99902, {}, 1)",
        playerId));
      // Room 99902 north (direction 0) to 99901
      dbExecute(DB_IMMORTAL, std::format(
        "INSERT INTO roomexit (vnum, direction, name, description, type, "
        "condition_flag, lock_difficulty, weight, key_num, destination, "
        "player_id, block) "
        "VALUES (99902, 0, '', '', 0, 0, 0, 0, 0, 99901, {}, 1)",
        playerId));
    }
};

// -------------------------------------------------------------------
// mvMob tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvMobInsertsNewMobWithChildData) {
  registerMobCleanup();

  insertImmortalMob();
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO mob_extra (vnum, keyword, description, player_id) "
    "VALUES (99901, 'test keyword', 'A test extra description.', {})",
    playerId));

  mvMob(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM mob WHERE vnum = 99901"),
    "a test mob");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT keyword FROM mob_extra WHERE vnum = 99901"),
    "test keyword");
}

TEST_F(LowMvTest, MvMobUpdatesExistingMobAndReplacesChildData) {
  registerMobCleanup();

  // Initial insert + move
  insertImmortalMob();
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO mob_extra (vnum, keyword, description, player_id) "
    "VALUES (99901, 'test keyword', 'A test extra description.', {})",
    playerId));
  mvMob(*tc->ch, playerId, "99901");

  // Modify immortal data
  dbExecute(DB_IMMORTAL, std::format(
    "UPDATE mob SET short_desc = 'an updated test mob' "
    "WHERE vnum = 99901 AND player_id = {}", playerId));
  dbExecute(DB_IMMORTAL, std::format(
    "DELETE FROM mob_extra WHERE vnum = 99901 AND player_id = {}",
    playerId));
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO mob_extra (vnum, keyword, description, player_id) "
    "VALUES (99901, 'updated keyword', 'Updated extra description.', {})",
    playerId));

  // Move again - should update, not fail
  mvMob(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM mob WHERE vnum = 99901"),
    "an updated test mob");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT keyword FROM mob_extra WHERE vnum = 99901"),
    "updated keyword");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT COUNT(*) FROM mob_extra WHERE vnum = 99901"),
    "1");
}

TEST_F(LowMvTest, MvMobPreservesExternalFkDuringUpdate) {
  registerMobCleanup();

  insertImmortalMob();
  mvMob(*tc->ch, playerId, "99901");

  // Trophy row references this mob via FK
  dbExecute(DB_SNEEZY, std::format(
    "INSERT INTO trophy (player_id, mobvnum, count, totalcount) "
    "VALUES ({}, 99901, 1, 1)", playerId));

  // Update the mob in immortal and re-move
  dbExecute(DB_IMMORTAL, std::format(
    "UPDATE mob SET short_desc = 'post-trophy update' "
    "WHERE vnum = 99901 AND player_id = {}", playerId));
  mvMob(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM mob WHERE vnum = 99901"),
    "post-trophy update");

  // Trophy survived - proves UPSERT, not DELETE+INSERT
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY, std::format(
    "SELECT count FROM trophy WHERE player_id = {} AND mobvnum = 99901",
    playerId)),
    "1");
}

TEST_F(LowMvTest, MvMobHandlesSpecialCharacters) {
  registerMobCleanup();

  insertImmortalMob();
  mvMob(*tc->ch, playerId, "99901");

  // Use TDatabase %s format for safe escaping of the apostrophe
  TDatabase updater(DB_IMMORTAL);
  updater.query(
    "UPDATE mob SET short_desc='%s', name='%s' "
    "WHERE vnum=99901 AND player_id=%i",
    "the baker's apprentice", "baker's apprentice test", playerId);

  mvMob(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM mob WHERE vnum = 99901"),
    "the baker's apprentice");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM mob WHERE vnum = 99901"),
    "baker's apprentice test");
}

TEST_F(LowMvTest, MvMobTransfersMobImmChildData) {
  registerMobCleanup();

  insertImmortalMob();
  TDatabase db(DB_IMMORTAL);
  db.query(
    "INSERT INTO mob_imm (vnum, type, amt, player_id) "
    "VALUES (99901, 5, 100, %i)",
    playerId);

  mvMob(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT amt FROM mob_imm WHERE vnum = 99901 AND type = 5"),
    "100");
}

TEST_F(LowMvTest, MvMobHandlesMultipleVnums) {
  registerMobCleanup();

  insertImmortalMob(99901, "first mob");
  insertImmortalMob(99902, "second mob");

  mvMob(*tc->ch, playerId, "99901 99902");

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT short_desc FROM mob WHERE vnum = 99901"),
    "first mob");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT short_desc FROM mob WHERE vnum = 99902"),
    "second mob");
}

// -------------------------------------------------------------------
// mvObj tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvObjInsertsNewObjWithChildData) {
  registerObjCleanup();

  insertImmortalObj();
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO objextra (vnum, name, description, player_id) "
    "VALUES (99901, 'test label', 'A test extra description.', {})",
    playerId));

  mvObj(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM obj WHERE vnum = 99901"),
    "a test object");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM objextra WHERE vnum = 99901"),
    "test label");
}

TEST_F(LowMvTest, MvObjUpdatesExistingObjAndReplacesChildData) {
  registerObjCleanup();

  // Initial insert + move
  insertImmortalObj();
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO objextra (vnum, name, description, player_id) "
    "VALUES (99901, 'test label', 'A test extra description.', {})",
    playerId));
  mvObj(*tc->ch, playerId, "99901");

  // Modify immortal data
  dbExecute(DB_IMMORTAL, std::format(
    "UPDATE obj SET short_desc = 'an updated test object' "
    "WHERE vnum = 99901 AND player_id = {}", playerId));
  dbExecute(DB_IMMORTAL, std::format(
    "DELETE FROM objextra WHERE vnum = 99901 AND player_id = {}",
    playerId));
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO objextra (vnum, name, description, player_id) "
    "VALUES (99901, 'updated label', 'Updated extra.', {})",
    playerId));

  mvObj(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT short_desc FROM obj WHERE vnum = 99901"),
    "an updated test object");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM objextra WHERE vnum = 99901"),
    "updated label");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT COUNT(*) FROM objextra WHERE vnum = 99901"),
    "1");
}

TEST_F(LowMvTest, MvObjTransfersObjaffectChildData) {
  registerObjCleanup();

  insertImmortalObj();
  TDatabase db(DB_IMMORTAL);
  db.query(
    "INSERT INTO objaffect (vnum, type, mod1, mod2, player_id) "
    "VALUES (99901, 3, 10, 0, %i)",
    playerId);

  mvObj(*tc->ch, playerId, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT mod1 FROM objaffect WHERE vnum = 99901 AND type = 3"),
    "10");
}

TEST_F(LowMvTest, MvObjHandlesSpecialCharacters) {
  registerObjCleanup();

  insertImmortalObj();
  mvObj(*tc->ch, playerId, "99901");

  TDatabase updater(DB_IMMORTAL);
  updater.query(
    "UPDATE obj SET short_desc='%s', name='%s' "
    "WHERE vnum=99901 AND player_id=%i",
    "the baker's rolling pin", "baker's rolling pin test", playerId);

  mvObj(*tc->ch, playerId, "99901");

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT short_desc FROM obj WHERE vnum = 99901"),
    "the baker's rolling pin");
}

TEST_F(LowMvTest, MvObjPreservesExternalFkDuringUpdate) {
  registerObjCleanup();

  insertImmortalObj();
  mvObj(*tc->ch, playerId, "99901");

  // Insert a rent row referencing this obj (rent.vnum FK)
  dbExecute(DB_SNEEZY,
    "INSERT INTO rent (rent_id, owner, owner_type, vnum, val0, val1, val2, "
    "val3, extra_flags, weight, bitvector, cur_struct, max_struct, "
    "decay, material, volume, price) "
    "VALUES (99905, 0, 'player', 99901, 0, 0, 0, 0, 0, 1, 0, 100, 100, "
    "0, 0, 1, 0)");
  dbCleanupLater(DB_SNEEZY, "DELETE FROM rent WHERE rent_id = 99905");

  dbExecute(DB_IMMORTAL,
    std::format("UPDATE obj SET short_desc = 'post-rent update' "
                "WHERE vnum = 99901 AND player_id = {}",
      playerId));
  mvObj(*tc->ch, playerId, "99901");

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT short_desc FROM obj WHERE vnum = 99901"),
    "post-rent update");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT COUNT(*) FROM rent WHERE rent_id = 99905"),
    "1")
    << "Rent row should survive obj UPSERT (proves UPSERT, not DELETE+INSERT)";
}

// -------------------------------------------------------------------
// mvRoom tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvRoomInsertsWithCrossReferencingExitsAndRoomextra) {
  registerRoomCleanup();

  insertImmortalRooms();
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO roomextra (vnum, name, description, player_id, block) "
    "VALUES (99901, 'test sign', 'A sign reads: Testing!', {}, 1)",
    playerId));

  mvRoom(*tc->ch, playerId, 1, "99901-99902");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM room WHERE vnum = 99901"),
    "Test Room Alpha");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM room WHERE vnum = 99902"),
    "Test Room Beta");

  // Cross-referencing exits work because mvRoom inserts rooms before exits
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT destination FROM roomexit WHERE vnum = 99901 AND direction = 2"),
    "99902");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT destination FROM roomexit WHERE vnum = 99902 AND direction = 0"),
    "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM roomextra WHERE vnum = 99901"),
    "test sign");
}

TEST_F(LowMvTest, MvRoomUpdatesExistingRoom) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1, "99901-99902");

  // Update room name in immortal
  dbExecute(DB_IMMORTAL, std::format(
    "UPDATE room SET name = 'Updated Room Alpha' "
    "WHERE vnum = 99901 AND player_id = {}", playerId));

  mvRoom(*tc->ch, playerId, 1, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM room WHERE vnum = 99901"),
    "Updated Room Alpha");
}

TEST_F(LowMvTest, MvRoomDropsExitsToNonExistentRooms) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1, "99901-99902");

  // Add an exit from 99901 east (direction 1) to non-existent room 99999
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO roomexit (vnum, direction, name, description, type, "
    "condition_flag, lock_difficulty, weight, key_num, destination, "
    "player_id, block) "
    "VALUES (99901, 1, '', '', 0, 0, 0, 0, 0, 99999, {}, 1)",
    playerId));

  mvRoom(*tc->ch, playerId, 1, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT name FROM room WHERE vnum = 99901"),
    "Test Room Alpha");

  // Valid south exit survived despite invalid east exit failing
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT destination FROM roomexit WHERE vnum = 99901 AND direction = 2"),
    "99902");

  // Invalid east exit to 99999 was NOT created (FK RESTRICT rejects it)
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT COUNT(*) FROM roomexit WHERE vnum = 99901 AND direction = 1"),
    "0");
}

TEST_F(LowMvTest, MvRoomSkipsVnumsNotInImmortalDb) {
  registerRoomCleanup();

  insertImmortalRooms();
  // Move vnums 99901-99903, but 99903 doesn't exist in immortal
  mvRoom(*tc->ch, playerId, 1, "99901-99903");

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT COUNT(*) FROM room WHERE vnum = 99901"),
    "1");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT COUNT(*) FROM room WHERE vnum = 99902"),
    "1");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT COUNT(*) FROM room WHERE vnum = 99903"),
    "0")
    << "Room not in immortal DB should not appear in sneezy";
}

TEST_F(LowMvTest, MvRoomUpdatesExistingExits) {
  registerRoomCleanup();

  insertImmortalRooms();
  mvRoom(*tc->ch, playerId, 1, "99901-99902");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT type FROM roomexit WHERE vnum = 99901 AND direction = 2"),
    "0");

  dbExecute(DB_IMMORTAL,
    std::format(
      "UPDATE roomexit SET type = 1 WHERE vnum = 99901 AND direction = 2 "
      "AND player_id = {}",
      playerId));

  mvRoom(*tc->ch, playerId, 1, "99901-99902");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT type FROM roomexit WHERE vnum = 99901 AND direction = 2"),
    "1");
}

TEST_F(LowMvTest, MvRoomStoresNullZoneForNegativeOne) {
  registerRoomCleanup();

  dbExecute(DB_IMMORTAL,
    std::format("INSERT INTO room (vnum, x, y, z, name, description, zone, "
                "room_flag, sector, teletime, teletarg, telelook, river_speed, "
                "river_dir, capacity, height, spec, player_id, block) "
                "VALUES (99901, 0, 0, 0, 'Zoneless Room', 'No zone.', null, "
                "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
      playerId));

  mvRoom(*tc->ch, playerId, 1, "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT COALESCE(zone, 'null') FROM room WHERE vnum = 99901"),
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
                "VALUES (99901, 0, 0, 0, 'Zoned Room', 'Has zone.', {}, "
                "0, 0, 0, 0, 0, 0, 0, 0, 100, 0, {}, 1)",
      validZone, playerId));

  mvRoom(*tc->ch, playerId, 1, "99901");

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT zone FROM room WHERE vnum = 99901"),
    validZone);
}

// -------------------------------------------------------------------
// mvResponse tests
// -------------------------------------------------------------------

TEST_F(LowMvTest, MvResponseInsertsNewResponse) {
  registerMobCleanup();

  // Prerequisite: mob must exist in sneezy (FK)
  insertImmortalMob();
  mvMob(*tc->ch, playerId, "99901");

  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO mobresponses (vnum, response, player_id) "
    "VALUES (99901, 'say Hello, test!', {})", playerId));

  mvResponse(*tc->ch, playerId, "TestBuilder", "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT response FROM mobresponses WHERE vnum = 99901"),
    "say Hello, test!");
}

TEST_F(LowMvTest, MvResponseUpdatesExistingResponse) {
  registerMobCleanup();

  // Create mob + initial response
  insertImmortalMob();
  mvMob(*tc->ch, playerId, "99901");
  dbExecute(DB_IMMORTAL, std::format(
    "INSERT INTO mobresponses (vnum, response, player_id) "
    "VALUES (99901, 'say Hello, test!', {})", playerId));
  mvResponse(*tc->ch, playerId, "TestBuilder", "99901");

  // Update response in immortal
  dbExecute(DB_IMMORTAL, std::format(
    "UPDATE mobresponses SET response = 'say Updated response!' "
    "WHERE vnum = 99901 AND player_id = {}", playerId));

  mvResponse(*tc->ch, playerId, "TestBuilder", "99901");

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
    "SELECT response FROM mobresponses WHERE vnum = 99901"),
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
