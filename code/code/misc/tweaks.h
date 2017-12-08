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
  TWEAK_BURNRATE,
  //TWEAK_BOSS,
  MAX_TWEAK_TYPES
};

class tweakEntry {
  public:
  double value;
  const sstring name;
  const sstring descr;

  tweakEntry(double, const sstring, const sstring);
  ~tweakEntry();

  private:
  tweakEntry();
};


class tweakInfoT {
  
  

 public:
  std::vector<tweakEntry *> tweaks;
  bool loaded;
  
  void loadTweaks();
  
  
  
  bool isLoaded(){ return loaded; }

  tweakEntry *operator[] (const tweakTypeT);

  tweakInfoT();
  ~tweakInfoT();  
};

extern tweakInfoT tweakInfo;
extern tweakTypeT & operator++(tweakTypeT &c, int);

#endif