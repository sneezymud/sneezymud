#pragma once

// Integration test fixture providing mob/obj index management, ItemInfo,
// and game stats constants on top of DatabaseFixture.
//
// SetUpTestSuite initializes:
// - ItemInfo[MAX_OBJ_TYPES] via assign_item_info()
// - stats constants (max_exist, xp_modif, damage mods, etc.)
//
// These globals persist for the process lifetime (same pattern as
// GameFixture's spell array and race data).

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#if __has_include(<sanitizer/lsan_interface.h>)
#include <sanitizer/lsan_interface.h>
#endif

#include <gtest/gtest.h>

#include "database_fixture.h"
#include "db.h"
#include "extern.h"
#include "low.h"
#include "monster.h"
#include "obj.h"
#include "shop.h"
#include "statistics.h"
#include "structs.h"

// RAII wrapper for a test-instantiated mob. Handles the cleanup sequence
// required by TMonster/TBeing destructors (see spec for full rationale).
struct TestMob {
    TMonster* mob = nullptr;

    TestMob() = default;
    TestMob(const TestMob&) = delete;
    TestMob& operator=(const TestMob&) = delete;

    TestMob(TestMob&& other) noexcept : mob(other.mob) { other.mob = nullptr; }

    TestMob& operator=(TestMob&& other) noexcept {
      if (this != &other) {
        destroy();
        mob = other.mob;
        other.mob = nullptr;
      }
      return *this;
    }

    ~TestMob() { destroy(); }

    void destroy() {
      if (!mob) {
        return;
      }

      mob->spec = 0;  // Prevent checkSpec(CMD_GENERIC_DESTROYED)
      mob->brtRoom =
        Room::NOWHERE;  // Prevent ~TMonster removing from birth room

      mob->reformGroup();
      DeleteHatreds(mob, nullptr);
      DeleteFears(mob, nullptr);

      delete mob;
      mob = nullptr;
    }
};

// RAII wrapper for a test-instantiated object. Only needed for objects
// that were instantiated but never placed into a container/room/character
// (placed objects are cleaned up by the game's destruction chain).
struct TestObject {
    TObj* obj = nullptr;

    TestObject() = default;
    TestObject(const TestObject&) = delete;
    TestObject& operator=(const TestObject&) = delete;

    TestObject(TestObject&& other) noexcept : obj(other.obj) {
      other.obj = nullptr;
    }

    TestObject& operator=(TestObject&& other) noexcept {
      if (this != &other) {
        destroy();
        obj = other.obj;
        other.obj = nullptr;
      }
      return *this;
    }

    ~TestObject() { destroy(); }

    void destroy() {
      if (!obj) {
        return;
      }

      obj->spec = 0;  // Prevent checkSpec(CMD_GENERIC_DESTROYED)
      --(*obj);       // Remove from room/container if placed
      delete obj;
      obj = nullptr;
    }
};

class WorldFixture : public DatabaseFixture {
  protected:
    static void SetUpTestSuite() {
      DatabaseFixture::SetUpTestSuite();

      if (!worldInitialized) {
        // Pure hardcoded metadata, no dependencies.
        assign_item_info();

        // Cannot call init_game_stats() - it reads mutable/stats and
        // forks a shell process. Set the critical balance values directly.
        stats.max_exist = 1.2;
        stats.xp_modif = 0.65;
        stats.absorb_damage_divisor[MOB_STAT] = 2;
        stats.absorb_damage_divisor[PC_STAT] = 4;
        stats.skill_damage_mod = 0.45;
        stats.heal_amount_mod = 0.65;
        stats.weapon_damage_mod = 0.33;
        stats.barehand_damage_mod = 0.36;
        stats.npc_skill_damage_mod = 0.5;
        stats.npc_heal_amount_mod = 0.65;
        stats.npc_weapon_damage_mod = 0.85;

        worldInitialized = true;
      }
    }

    // Create a minimal mob index entry at the given vnum. The entry is
    // inserted at the correct sorted position and removed in TearDown.
    // Use vnums in the 99000-99999 range to avoid collisions with real data.
    int insertTestMob(int vnum, const char* name, int level) {
      // real_mobile() crashes on empty mob_index (unsigned underflow in
      // size()-1).
      assert((mob_index.empty() || real_mobile(vnum) < 0) &&
             "Test mob vnum already exists - collision");

      mobIndexData entry;
      entry.virt = vnum;
      // mud_str_dup uses new char[] - matching ~indexData's delete[].
      entry.name = mud_str_dup(name);
      entry.short_desc = mud_str_dup(name);
      entry.long_desc = mud_str_dup("");
      entry.description = mud_str_dup("");
      entry.level = level;

      auto it = std::lower_bound(mob_index.begin(), mob_index.end(), vnum,
        [](const mobIndexData& e, int v) { return e.virt < v; });
      auto inserted = mob_index.insert(it, entry);
      int rnum = static_cast<int>(inserted - mob_index.begin());

      testMobVnums.push_back(vnum);
      return rnum;
    }

    // Create a minimal obj index entry at the given vnum. The entry is
    // inserted at the correct sorted position and removed in TearDown.
    // Use vnums in the 99000-99999 range to avoid collisions with real data.
    int insertTestObj(int vnum, const char* name, itemTypeT type) {
      assert((obj_index.empty() || real_object(vnum, true) < 0) &&
             "Test obj vnum already exists - collision");

      objIndexData entry;
      entry.virt = vnum;
      entry.name = mud_str_dup(name);
      entry.short_desc = mud_str_dup(name);
      entry.long_desc = mud_str_dup("");
      entry.description = mud_str_dup("");
      entry.itemtype = static_cast<ubyte>(type);
      entry.ex_description = nullptr;

      auto it = std::lower_bound(obj_index.begin(), obj_index.end(), vnum,
        [](const objIndexData& e, int v) { return e.virt < v; });
      auto inserted = obj_index.insert(it, entry);
      int rnum = static_cast<int>(inserted - obj_index.begin());

      testObjVnums.push_back(vnum);
      return rnum;
    }

    // Instantiate a mob from the index using read_mobile(). The mob
    // must exist in mob_index (either real or synthetic). The mob is
    // placed in the given room and cleaned up in TearDown.
    TestMob& loadMob(int vnum, TRoom& room) {
      assert(real_mobile(vnum) >= 0 && "Mob vnum not in mob_index");

      ensureMinimalShopIndex();

      auto* mob = read_mobile(vnum, VIRTUAL);
      assert(mob != nullptr && "read_mobile returned null");

      // read_mobile() calls readMobFromDB(virt, false) which allocates
      // ex_description but doesn't set ACT_STRINGS_CHANGED. ~TMonster
      // then assumes ex_description is shared prototype data and copies
      // it, leaking the original. Mark strings as owned so ~TMonster
      // skips the copy and ~TThing frees them directly.
      SET_BIT(mob->specials.act, ACT_STRINGS_CHANGED);

      room += *mob;

      auto wrapper = std::make_unique<TestMob>();
      wrapper->mob = mob;
      testMobs.push_back(std::move(wrapper));
      return *testMobs.back();
    }

    // Load the full mob index from the database. Guarded against
    // double-init (generate_mob_index appends without clearing).
    // Index data is intentionally process-lifetime; suppress leak detection.
    void loadRealMobIndex() {
      if (!realMobIndexLoaded) {
#if __has_include(<sanitizer/lsan_interface.h>)
        __lsan_disable();
#endif
        generate_mob_index();
#if __has_include(<sanitizer/lsan_interface.h>)
        __lsan_enable();
#endif
        realMobIndexLoaded = true;
      }
    }

    // Instantiate an object from the index using read_object(). The
    // object must exist in obj_index (either real or synthetic).
    // Cleaned up in TearDown.
    TestObject& loadObj(int vnum) {
      assert(real_object(vnum) >= 0 && "Obj vnum not in obj_index");

      TObj* obj = read_object(vnum, VIRTUAL);
      assert(obj != nullptr && "read_object returned null");

      auto wrapper = std::make_unique<TestObject>();
      wrapper->obj = obj;
      testObjs.push_back(std::move(wrapper));
      return *testObjs.back();
    }

    // Load the full obj index from the database. Requires stats.max_exist
    // to be set (handled by SetUpTestSuite). Guarded against double-init.
    // Index data is intentionally process-lifetime; suppress leak detection.
    void loadRealObjIndex() {
      if (!realObjIndexLoaded) {
#if __has_include(<sanitizer/lsan_interface.h>)
        __lsan_disable();
#endif
        generate_obj_index();
#if __has_include(<sanitizer/lsan_interface.h>)
        __lsan_enable();
#endif
        realObjIndexLoaded = true;
      }
    }

    void TearDown() override {
      // Destroy instantiated entities first (they reference index data).
      testObjs.clear();
      testMobs.clear();

      // Remove synthetic obj index entries (reverse order to preserve indices).
      // erase() invokes ~indexData() which delete[]s the strings.
      for (auto it = testObjVnums.rbegin(); it != testObjVnums.rend(); ++it) {
        int rnum = real_object(*it, true);
        if (rnum >= 0) {
          obj_index.erase(obj_index.begin() + rnum);
        }
      }
      testObjVnums.clear();

      // Remove synthetic mob index entries (reverse order to preserve indices).
      for (auto it = testMobVnums.rbegin(); it != testMobVnums.rend(); ++it) {
        int rnum = real_mobile(*it);
        if (rnum >= 0) {
          mob_index.erase(mob_index.begin() + rnum);
        }
      }
      testMobVnums.clear();

      DatabaseFixture::TearDown();
    }

  private:
    // read_mobile() -> readMobFromDB() -> determineExp() -> getLoadMoney()
    // unconditionally accesses shop_index[123] (the central bank). Ensure
    // the vector is large enough to avoid UB on that access.
    static void ensureMinimalShopIndex() {
      constexpr int CENTRAL_BANK_INDEX = 124;  // need indices 0..123
      if (shop_index.size() < CENTRAL_BANK_INDEX) {
        shop_index.resize(CENTRAL_BANK_INDEX);
      }
    }

    // Persists for process lifetime. assign_item_info() leaks if called
    // twice (allocates without freeing previous), so the guard is required.
    static inline bool worldInitialized = false;
    static inline bool realMobIndexLoaded = false;
    static inline bool realObjIndexLoaded = false;
    std::vector<std::unique_ptr<TestMob>> testMobs;
    std::vector<std::unique_ptr<TestObject>> testObjs;
    std::vector<int> testMobVnums;
    std::vector<int> testObjVnums;
};
