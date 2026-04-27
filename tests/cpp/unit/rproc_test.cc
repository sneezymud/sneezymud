#include <gtest/gtest.h>

#include "spec_rooms.h"

TEST(RoomSpecials, NonExistentIdReturnsNull) {
  EXPECT_EQ(findRoomSpec(0), nullptr);
  EXPECT_EQ(findRoomSpec(999), nullptr);
}

TEST(RoomSpecials, GetNameReturnsNoneForUnknownId) {
  EXPECT_EQ(getRoomSpecName(999), "None");
}
