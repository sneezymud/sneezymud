#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "charfile.h"
#include "db.h"
#include "pfile_helper.h"
#include "database_fixture.h"

namespace fs = std::filesystem;

class PfileHelperTest : public DatabaseFixture {};

TEST_F(PfileHelperTest, WritePfileCreatesFileAtLowercasePath) {
  PfileHelper pfile("Testchar", 10, RACE_HUMAN);

  // load_char uses LOWER(name[0]) / name.lower() for the path.
  fs::path expected = fs::path(Path::MUTABLE_PLAYER) / "t" / "testchar";
  EXPECT_TRUE(fs::exists(expected));

  auto size = fs::file_size(expected);
  EXPECT_EQ(size, sizeof(charFile));
}

TEST_F(PfileHelperTest, WritePfileContainsCorrectData) {
  PfileHelper pfile("Testchar", 25, RACE_ELVEN);

  std::ifstream in(pfile.path(), std::ios::binary);
  charFile cf;
  in.read(reinterpret_cast<char*>(&cf), sizeof(cf));

  EXPECT_STREQ(cf.name, "Testchar");
  EXPECT_EQ(cf.level[0], 25);
  EXPECT_EQ(cf.race, RACE_ELVEN);
}

TEST_F(PfileHelperTest, DestructorCleansUpFile) {
  fs::path expected;
  {
    PfileHelper pfile("Tempchar", 1, RACE_HUMAN);
    expected = pfile.path();
    ASSERT_TRUE(fs::exists(expected));
  }
  EXPECT_FALSE(fs::exists(expected));
}
