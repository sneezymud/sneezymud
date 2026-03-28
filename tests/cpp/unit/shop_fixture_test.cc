#include <gtest/gtest.h>

#include "extern.h"
#include "shop.h"
#include "shop_fixture.h"

class ShopFixtureTest : public ShopFixture {};

TEST_F(ShopFixtureTest, InsertTestShopAddsToShopIndex) {
  int keeper_rnum = insertTestMob(99010, "a test shopkeeper", 20);
  auto& room = makeRoom(49980);

  insertTestShop(99010, keeper_rnum, room.in_room);

  bool found = false;
  for (const auto& shop : shop_index) {
    if (shop.shop_nr == 99010) {
      found = true;
      EXPECT_EQ(shop.keeper, keeper_rnum);
      break;
    }
  }
  EXPECT_TRUE(found) << "Test shop not found in shop_index";
}

TEST_F(ShopFixtureTest, LoadRealShopsPopulatesFromDatabase) {
  loadRealMobIndex();
  loadRealObjIndex();
  loadRealShops();
  EXPECT_GT(shop_index.size(), 0u);
}
