// Tests for SoundComm MSP (MUD Sound Protocol) wire format.
//
// SoundComm formats sound/music triggers for MSP-capable clients.
// The wire format is: !!SOUND(filename T=type U=url V=vol P=pri L=loops
// C=cont)\n\r Optional fields are omitted when set to -1 (volume, priority,
// repeats, cont) or empty string (type, url).

#include <gtest/gtest.h>

#include "comm.h"

TEST(SoundComm, AllFieldsPresent) {
  SoundComm comm("sound", "http://example.com/sounds/", "combat_01", "combat",
    80, 5, 3, 1);
  sstring result = comm.getComm();

  EXPECT_EQ(result,
    "!!SOUND(combat_01 T=combat U=http://example.com/sounds/ V=80 P=5 L=3 "
    "C=1)\n\r");
}

TEST(SoundComm, MinimalFields) {
  // -1 for numeric fields and empty string for type/url means omit them
  SoundComm comm("sound", "", "beep", "", -1, -1, -1, -1);
  EXPECT_EQ(comm.getComm(), "!!SOUND(beep)\n\r");
}

TEST(SoundComm, MusicSoundtype) {
  // soundtype is uppercased in output
  SoundComm comm("music", "", "Off", "", -1, -1, -1, -1);
  EXPECT_EQ(comm.getComm(), "!!MUSIC(Off)\n\r");
}

TEST(SoundComm, SoundtypeUppercased) {
  SoundComm comm("sound", "", "test", "", -1, -1, -1, -1);
  sstring result = comm.getComm();
  // "sound" becomes "SOUND"
  EXPECT_TRUE(result.find("!!SOUND(") == 0);
}

TEST(SoundComm, TypeOnlyOptionalField) {
  SoundComm comm("sound", "", "hit", "combat", -1, -1, -1, -1);
  EXPECT_EQ(comm.getComm(), "!!SOUND(hit T=combat)\n\r");
}

TEST(SoundComm, UrlOnlyOptionalField) {
  SoundComm comm("sound", "http://sneezymud.org/sounds/", "hit", "", -1, -1, -1,
    -1);
  EXPECT_EQ(comm.getComm(), "!!SOUND(hit U=http://sneezymud.org/sounds/)\n\r");
}

TEST(SoundComm, VolumeZero) {
  // Volume of 0 is valid and distinct from -1 (omitted)
  SoundComm comm("sound", "", "quiet", "", 0, -1, -1, -1);
  EXPECT_EQ(comm.getComm(), "!!SOUND(quiet V=0)\n\r");
}

TEST(SoundComm, AllNumericFieldsPresent) {
  SoundComm comm("sound", "", "test", "", 100, 10, 5, 0);
  EXPECT_EQ(comm.getComm(), "!!SOUND(test V=100 P=10 L=5 C=0)\n\r");
}
