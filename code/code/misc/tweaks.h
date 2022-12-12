//////////////////////////////////////////////////////////////////////////
//
//  tweaks.h
//
//
//  A tweak is generally a coeffecient that gets slapped in the code
//  that can be modified in-game.  To add a new tweak 1) add the tweak
//  type to the enum tweakTypeT right above MAX_TWEAK_TYPES.  2)  Modify
//  tweakInfoT::getTweakName to provide a name (no spaces).  3) Multiply
//  by tweaks[YOUR_TWEAK_TYPE]->current somewhere in the code.  4)  The
//  tweak value defaults to 1.0.  Adjust the value with a trusted
//  immortal in game using info tweak.  If it is unnacceptable for the
//  value to start at 1.0, add a migration to migrations.cc inserting
//  the desired value.
//
//  Note that if you are changed the value of related tweaks at the same
//  time it might be hard to figure out the effect in game.
//
//////////////////////////////////////////////////////////////////////////


#pragma once

#include <map>

#include "extern.h"
#include "sstring.h"
#include "being.h"


enum tweakTypeT {
  TWEAK_LOADRATE = 0,
  TWEAK_BURNRATE,
  TWEAK_FREEZEDAMRATE,
  MAX_TWEAK_TYPES
};

class tweakEntry {
  public:
  int id = 0;
  double current = 1.0;
  double target = 1.0;
  double rate = 0.0;

  tweakEntry();
};

class tweakInfoT {

  std::vector<tweakEntry *> tweaks;
  bool loaded = false;
  void handleTweak(TBeing *, tweakTypeT, sstring);
  void showTweaks(TBeing *, tweakTypeT = MAX_TWEAK_TYPES);
  void showTweakOptions(TBeing *);

 public:
  int loadTweaks();
  bool isLoaded(){ return loaded; }
  const sstring getTweakName(tweakTypeT);
  void doTweak(TBeing *, sstring);
  tweakEntry *operator[] (const tweakTypeT);

  tweakInfoT();
  ~tweakInfoT();
};

extern tweakInfoT tweakInfo;
extern tweakTypeT & operator++(tweakTypeT &c, int);
