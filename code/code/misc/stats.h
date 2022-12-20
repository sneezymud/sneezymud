//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

// Stats.h
//
// A class for defining the stat system.

#pragma once

#include "enum.h"

class sstring;
class TBeing;

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
  MAX_STATS,
  MIN_STAT = STAT_STR,
  MAX_STATS_USED = STAT_LUC
};

extern statTypeT& operator++(statTypeT& c, int);

extern int territory_adjustment(territoryT, statTypeT);
extern sstring statToString(statTypeT whichStat);

double plotLevelDiff(int levelDiff, double minResult, double maxResult,
  double averageResult, double power = 1.4);

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
    short values[MAX_STATS]{0};

  public:
    Stats();

    Stats operator+(const Stats& operand);
    Stats operator-(const Stats& operand);

    short get(statTypeT stat) const;
    short set(statTypeT stat, short val);
    short add(statTypeT stat, short mod);

    void zero();
    int total() const;
    bool isDefault() const;

    sstring showStats(TBeing* caller);

    const sstring printStatHeader() const;
    const sstring printRawStats(const TBeing* caller) const;
};
