//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// Stats.h
//
// A class for defining the stat system.

#ifndef __CHARACTERISTICS_H
#define __CHARACTERISTICS_H

enum statTypeT {
     STAT_STR,
     STAT_BRA,
     STAT_CON,
     STAT_DEX,
     STAT_AGI,
     STAT_INT,
     STAT_WIS,
     STAT_FOC,
     STAT_PER,
     STAT_CHA,
     STAT_KAR,
     STAT_SPE,
     STAT_LUC,
     STAT_EXT,
     MAX_STATS
};
const statTypeT MIN_STAT=	statTypeT(0);
const statTypeT MAX_STATS_USED=	statTypeT(STAT_SPE+1);
extern statTypeT & operator++(statTypeT &c, int);

extern int territory_adjustment(territoryT, statTypeT);
	
const int MAX_STAT_COMBAT=	5;
const int MAX_STAT_LEARN=	3;
const int MAX_STAT_UTIL	=4;
const int MAX_STAT_EXTS	=2;

enum statSetT {
     STAT_CHOSEN,
     STAT_NATURAL,
     STAT_CURRENT,
     STAT_RACE,
     STAT_AGE,
     STAT_TERRITORY,
};

class Stats {

  friend class Descriptor;
  friend class TPerson;

  private:
    sh_int values[MAX_STATS];

  public: 
    Stats();
    Stats(const Stats &);
    ~Stats();
    Stats & operator=(const Stats &);

  Stats operator+(const Stats &operand);
  Stats operator-(const Stats &operand);
  
  sh_int get(statTypeT stat) const;
  sh_int set(statTypeT stat, sh_int val);
  sh_int add(statTypeT stat, sh_int mod);

  void zero();

  sstring showStats(TBeing *caller);

  const sstring printStatHeader() const;
  const sstring printRawStats(const TBeing *caller) const;

};
#endif
