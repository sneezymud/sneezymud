#pragma once

// Integration test fixture for testing the shopping pipeline.
// Extends WorldFixture with shop_index management.

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

#include "shop.h"
#include "structs.h"  // mud_str_dup
#include "world_fixture.h"

class ShopFixture : public WorldFixture {
  protected:
    // Create a minimal shop and add to shop_index. The keeper_rnum must
    // be a valid index into mob_index (use insertTestMob first).
    // Cleaned up in TearDown.
    void insertTestShop(int shop_nr, int keeper_rnum, int room_vnum,
      float profit_buy = 1.1f, float profit_sell = 0.9f) {
      shopData sd;
      sd.shop_nr = shop_nr;
      sd.keeper = keeper_rnum;
      sd.in_room = room_vnum;
      sd.profit_buy = profit_buy;
      sd.profit_sell = profit_sell;
      sd.owned = false;
      sd.flags = 0;
      sd.open1 = 0;
      sd.close1 = 28;  // Always open
      sd.open2 = 0;
      sd.close2 = 0;
      sd.temper1 = 0;
      sd.temper2 = 0;

      // mud_str_dup uses new char[] - matching ~shopData's delete[].
      sd.no_such_item1 = mud_str_dup("I don't have that.");
      sd.no_such_item2 = mud_str_dup("You don't have that.");
      sd.missing_cash1 = mud_str_dup("I can't afford that.");
      sd.missing_cash2 = mud_str_dup("You can't afford that.");
      sd.do_not_buy = mud_str_dup("I don't buy that.");
      sd.message_buy = mud_str_dup("That'll be %d talens.");
      sd.message_sell = mud_str_dup("I'll give you %d talens.");

      shop_index.push_back(sd);
      testShopNrs.push_back(shop_nr);
    }

    // Load all real shops from the database. Requires mob_index and
    // obj_index to be populated first. Guarded against double-init.
    void loadRealShops() {
      if (!realShopsLoaded) {
        bootTheShops();
        realShopsLoaded = true;
      }
    }

    void TearDown() override {
      // Remove synthetic shops from shop_index.
      for (int nr : testShopNrs) {
        auto it = std::find_if(shop_index.begin(), shop_index.end(),
          [nr](const shopData& s) { return s.shop_nr == nr; });
        if (it != shop_index.end()) {
          shop_index.erase(it);
        }
      }
      testShopNrs.clear();

      WorldFixture::TearDown();
    }

  private:
    static inline bool realShopsLoaded = false;
    std::vector<int> testShopNrs;
};
