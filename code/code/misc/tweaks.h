//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __TWEAK_H
#define __TWEAK_H

#include <map>

#include "sstring.h"


enum tweakTypeT {
  TWEAK_NONE  = 0,
  TWEAK_LOADRATE,
  TWEAK_BURNRATE,
  //TWEAK_BOSS,
  MAX_TWEAK_TYPES
};

class tweakEntry {
  public:
  double cvalue;
  double tvalue;
  double rate;

  tweakEntry(double, double, double);
  ~tweakEntry();

  private:
  tweakEntry();
};

class tweakInfoT {

  std::vector<tweakEntry *> tweaks;
  bool loaded;
  
 public:
  int loadTweaks();
  bool isLoaded(){ return loaded; }
  const sstring getTweakName(tweakTypeT);

  tweakEntry *operator[] (const tweakTypeT);

  tweakInfoT();
  ~tweakInfoT();  
};

extern tweakInfoT tweakInfo;
extern tweakTypeT & operator++(tweakTypeT &c, int);

#endif