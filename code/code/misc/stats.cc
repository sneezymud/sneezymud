// stats.cc

#include <boost/format.hpp>
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>

#include "being.h"
#include "defs.h"
#include "extern.h"
#include "gametime.h"
#include "log.h"
#include "race.h"
#include "spells.h"
#include "sstring.h"
#include "stats.h"
#include "structs.h"
#include "toggle.h"

Stats Stats::operator+(const Stats& operand) {
  Stats sum;

  for (statTypeT stat = MIN_STAT; stat < MAX_STATS; stat++)
    sum.values[stat] = values[stat] + operand.values[stat];

  return sum;
}

Stats Stats::operator-(const Stats& operand) {
  Stats diff;

  for (statTypeT stat = MIN_STAT; stat < MAX_STATS; stat++)
    diff.values[stat] = values[stat] - operand.values[stat];

  return diff;
}

short Stats::get(statTypeT stat) const {
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
    "Something tried to access an invalid stat.");

  return values[stat];
}

short Stats::set(statTypeT stat, short val) {
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
    "Something tried to access an invalid stat.");

  return values[stat] = val;
}

short Stats::add(statTypeT stat, short mod) {
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
    "Something tried to access an invalid stat.");

  return values[stat] += mod;
}

void Stats::zero() {
  for (statTypeT stat = MIN_STAT; stat < MAX_STATS; stat++)
    values[stat] = 0;
}

int Stats::total() const {
  int total = 0;
  for (statTypeT stat = MIN_STAT; stat < MAX_STATS; stat++)
    total += values[stat];
  return total;
}

bool Stats::isDefault() const {
  for (statTypeT stat = MIN_STAT; stat < MAX_STATS; stat++)
    if (values[stat] != 0)
      return false;
  return true;
}

sstring Stats::showStats(TBeing* caller) {
  byte level = caller->GetMaxLevel();
  Stats showStat;
  char tmpbuf[80];
  sstring buf;

  if (level < GOD_LEVEL1)
    showStat = *this - caller->race->baseStats;
  else
    showStat = *this;

  buf =
    "<c>[STR]<z> <c>[BRA]<z> <c>[CON]<z> <c>[DEX]<z> <c>[AGI]<z> <c>[INT]<z> "
    "<c>[WIS]<z> <c>[FOC]<z> <c>[PER]<z> <c>[CHA]<z> <c>[KAR]<z> "
    "<c>[SPE]<z>\n\r";

  for (statTypeT stat = MIN_STAT; stat < MAX_STATS_USED; stat++) {
    sprintf(tmpbuf, " %3d  ", showStat.get(stat));
    buf += tmpbuf;
  }
  buf += "\n\r";

  return buf;
}

const sstring Stats::printStatHeader() const {
  sstring header;

  // Physical Stats
  header = "<c>[STR]<z><c>[BRA]<z><c>[CON]<z><c>[DEX]<z><c>[AGI]<z>";
  // Mental Stats
  header += "<c>[INT]<z><c>[WIS]<z><c>[FOC]<z>";
  // Intangibles
  header += "<c>[PER]<z><c>[CHA]<z><c>[KAR]<z><c>[SPE]<z>\n\r";

  return header;
}

const sstring Stats::printRawStats(const TBeing*) const {
  sstring rawStats, buf;

  for (statTypeT stat = MIN_STAT; stat < MAX_STATS_USED; stat++) {
    buf = format(" %3d ") % get(stat);
    rawStats += buf;
  }
  rawStats += "\n\r";

  return rawStats;
}

int age_mod_for_stat(const TBeing* tb, int age_num, statTypeT whichStat) {
  if (!tb->hasQuestBit(TOG_REAL_AGING))
    return 0;

  // age_num is the "human" age, realize non-humans have been adjusted
  // to the human age, so no need to modify for race.
  switch (whichStat) {
    case STAT_STR:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_BRA:
      if (age_num < 17)
        return 5;
      else if (age_num <= 18)
        return 4;
      else if (age_num <= 20)
        return 3;
      else if (age_num <= 24)
        return 2;
      else if (age_num <= 28)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 66)
        return -1;
      else if (age_num <= 70)
        return -2;
      else if (age_num <= 74)
        return -3;
      else if (age_num <= 78)
        return -4;
      else
        return -5;
    case STAT_AGI:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_DEX:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_CON:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_INT:
      if (age_num < 17)
        return -10;
      else if (age_num <= 17)
        return -9;
      else if (age_num <= 18)
        return -8;
      else if (age_num <= 19)
        return -7;
      else if (age_num <= 20)
        return -6;
      else if (age_num <= 22)
        return -5;
      else if (age_num <= 24)
        return -4;
      else if (age_num <= 26)
        return -3;
      else if (age_num <= 28)
        return -2;
      else if (age_num <= 30)
        return -1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return 1;
      else if (age_num <= 66)
        return 2;
      else if (age_num <= 68)
        return 3;
      else if (age_num <= 70)
        return 4;
      else if (age_num <= 72)
        return 5;
      else if (age_num <= 74)
        return 6;
      else if (age_num <= 76)
        return 7;
      else if (age_num <= 78)
        return 8;
      else if (age_num <= 80)
        return 9;
      else
        return 10;
    case STAT_WIS:
      if (age_num < 17)
        return -10;
      else if (age_num <= 17)
        return -9;
      else if (age_num <= 18)
        return -8;
      else if (age_num <= 19)
        return -7;
      else if (age_num <= 20)
        return -6;
      else if (age_num <= 22)
        return -5;
      else if (age_num <= 24)
        return -4;
      else if (age_num <= 26)
        return -3;
      else if (age_num <= 28)
        return -2;
      else if (age_num <= 30)
        return -1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return 1;
      else if (age_num <= 66)
        return 2;
      else if (age_num <= 68)
        return 3;
      else if (age_num <= 70)
        return 4;
      else if (age_num <= 72)
        return 5;
      else if (age_num <= 74)
        return 6;
      else if (age_num <= 76)
        return 7;
      else if (age_num <= 78)
        return 8;
      else if (age_num <= 80)
        return 9;
      else
        return 10;
    case STAT_FOC:
      if (age_num < 17)
        return -10;
      else if (age_num <= 17)
        return -9;
      else if (age_num <= 18)
        return -8;
      else if (age_num <= 19)
        return -7;
      else if (age_num <= 20)
        return -6;
      else if (age_num <= 22)
        return -5;
      else if (age_num <= 24)
        return -4;
      else if (age_num <= 26)
        return -3;
      else if (age_num <= 28)
        return -2;
      else if (age_num <= 30)
        return -1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return 1;
      else if (age_num <= 66)
        return 2;
      else if (age_num <= 68)
        return 3;
      else if (age_num <= 70)
        return 4;
      else if (age_num <= 72)
        return 5;
      else if (age_num <= 74)
        return 6;
      else if (age_num <= 76)
        return 7;
      else if (age_num <= 78)
        return 8;
      else if (age_num <= 80)
        return 9;
      else
        return 10;
    case STAT_PER:
      if (age_num < 17)
        return -10;
      else if (age_num <= 17)
        return -9;
      else if (age_num <= 18)
        return -8;
      else if (age_num <= 19)
        return -7;
      else if (age_num <= 20)
        return -6;
      else if (age_num <= 22)
        return -5;
      else if (age_num <= 24)
        return -4;
      else if (age_num <= 26)
        return -3;
      else if (age_num <= 28)
        return -2;
      else if (age_num <= 30)
        return -1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return 1;
      else if (age_num <= 66)
        return 2;
      else if (age_num <= 68)
        return 3;
      else if (age_num <= 70)
        return 4;
      else if (age_num <= 72)
        return 5;
      else if (age_num <= 74)
        return 6;
      else if (age_num <= 76)
        return 7;
      else if (age_num <= 78)
        return 8;
      else if (age_num <= 80)
        return 9;
      else
        return 10;
    case STAT_SPE:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_KAR:
      if (age_num < 17)
        return 10;
      else if (age_num <= 17)
        return 9;
      else if (age_num <= 18)
        return 8;
      else if (age_num <= 19)
        return 7;
      else if (age_num <= 20)
        return 6;
      else if (age_num <= 22)
        return 5;
      else if (age_num <= 24)
        return 4;
      else if (age_num <= 26)
        return 3;
      else if (age_num <= 28)
        return 2;
      else if (age_num <= 30)
        return 1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return -1;
      else if (age_num <= 66)
        return -2;
      else if (age_num <= 68)
        return -3;
      else if (age_num <= 70)
        return -4;
      else if (age_num <= 72)
        return -5;
      else if (age_num <= 74)
        return -6;
      else if (age_num <= 76)
        return -7;
      else if (age_num <= 78)
        return -8;
      else if (age_num <= 80)
        return -9;
      else
        return -10;
    case STAT_CHA:
      if (age_num < 17)
        return -10;
      else if (age_num <= 17)
        return -9;
      else if (age_num <= 18)
        return -8;
      else if (age_num <= 19)
        return -7;
      else if (age_num <= 20)
        return -6;
      else if (age_num <= 22)
        return -5;
      else if (age_num <= 24)
        return -4;
      else if (age_num <= 26)
        return -3;
      else if (age_num <= 28)
        return -2;
      else if (age_num <= 30)
        return -1;
      else if (age_num <= 60)
        return 0;
      else if (age_num <= 63)
        return 1;
      else if (age_num <= 66)
        return 2;
      else if (age_num <= 68)
        return 3;
      else if (age_num <= 70)
        return 4;
      else if (age_num <= 72)
        return 5;
      else if (age_num <= 74)
        return 6;
      else if (age_num <= 76)
        return 7;
      else if (age_num <= 78)
        return 8;
      else if (age_num <= 80)
        return 9;
      else
        return 10;
    case STAT_LUC:
    case STAT_EXT:
    case MAX_STATS:
      return 0;
  }
  return 0;
}

int territory_adjustment(territoryT ter, statTypeT whichStat) {
  // This function defines how territorial choice affects natural stats.
  // I'm loosely grouping stats into 3 groups
  // dex, agi. speed
  // con, bra, str
  // int, wis, foc
  // I'm allowing free trading of points across boundaries
  // the net should be 0 (or maybe slightly neg).

  // the other stats will reside outside the above system as they
  // are not really worthwhile.
  // per:  raised if little stimulation, or highly dangerous env.
  // cha and kar will trade off directly
  // cha will raise if high contact with others
  // kar drops for same reason (damage to soul from cynacism)

  // for the later races, we just have a slot for every territory, so adjust to
  // fit the human model
  if (ter >= HOME_TER_GOBLIN_URBAN)
    ter = territoryT(1 + ((ter - HOME_TER_GOBLIN_URBAN) % 8));

  switch (ter) {
    case HOME_TER_NONE:
      return 0;
    case HOME_TER_HUMAN_VILLAGER:
    case HOME_TER_ELF_TRIBE:
    case HOME_TER_DWARF_VILLAGER:
    case HOME_TER_GNOME_VILLAGER:
    case HOME_TER_OGRE_VILLAGER:
    case HOME_TER_HOBBIT_SHIRE:
      switch (whichStat) {
        case STAT_CHA:  // moderate contact
          return 10;
        case STAT_KAR:  // moderate cynacism
          return -10;
        case STAT_INT:  // moderate education
          return 10;
        case STAT_WIS:
          return 10;
        case STAT_CON:  // poor health environment
          return -10;
        case STAT_BRA:
          return -10;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_URBAN:
    case HOME_TER_ELF_URBAN:
    case HOME_TER_DWARF_URBAN:
    case HOME_TER_GNOME_URBAN:
    case HOME_TER_HOBBIT_URBAN:
      switch (whichStat) {
        case STAT_CHA:  // high contact
          return 20;
        case STAT_KAR:  // high cynacism
          return -20;
        case STAT_INT:  // good education
          return 20;
        case STAT_WIS:
          return 20;
        case STAT_FOC:  // low attention span
          return -10;
        case STAT_SPE:  // hustle bustle
          return 20;
        case STAT_CON:  // poor health environment
          return -20;
        case STAT_BRA:
          return -20;
        case STAT_PER:  // over stimulated
          return -10;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_PLAINS:
    case HOME_TER_ELF_PLAINS:
    case HOME_TER_OGRE_PLAINS:
    case HOME_TER_HOBBIT_GRASSLANDS:
      switch (whichStat) {
        case STAT_CHA:  // low contact
          return -5;
        case STAT_KAR:  // mildly upbeat outlook
          return 5;
        case STAT_INT:  // mildly substandard education
          return -10;
        case STAT_WIS:
          return 0;
        case STAT_CON:  // mildly healthy environment
          return 5;
        case STAT_BRA:
          return 5;
        case STAT_FOC:  // there wasn't a lot to do
          return -10;
        case STAT_PER:
          return 15;
        case STAT_SPE:
          return -5;
        case STAT_AGI:
          return 0;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_RECLUSE:
    case HOME_TER_ELF_RECLUSE:
    case HOME_TER_DWARF_RECLUSE:
      switch (whichStat) {
        case STAT_CHA:  // extreme low contact
          return -30;
        case STAT_KAR:  // upbeat outlook
          return 30;
        case STAT_INT:  // substandard education
          return -25;
        case STAT_WIS:
          return -15;
        case STAT_CON:  // mildly healthy environment
          return 25;
        case STAT_BRA:
          return 15;
        case STAT_FOC:  // there wasn't a lot to do
          return 15;
        case STAT_PER:
          return -15;
        case STAT_SPE:
          return 0;
        case STAT_AGI:
          return 0;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_HILL:
    case HOME_TER_DWARF_HILL:
    case HOME_TER_GNOME_HILL:
    case HOME_TER_OGRE_HILL:
    case HOME_TER_HOBBIT_HILL:
      switch (whichStat) {
        case STAT_CHA:  // low contact
          return -10;
        case STAT_KAR:  // mildly upbeat outlook
          return 10;
        case STAT_INT:  // mildly substandard education
          return -15;
        case STAT_WIS:
          return -5;
        case STAT_CON:  // mildly healthy environment
          return 10;
        case STAT_BRA:
          return 10;
        case STAT_FOC:  // there wasn't a lot to do
          return -15;
        case STAT_PER:
          return 10;
        case STAT_SPE:
          return 0;
        case STAT_AGI:
          return 5;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_MOUNTAIN:
    case HOME_TER_DWARF_MOUNTAIN:
    case HOME_TER_ELF_SNOW:
      switch (whichStat) {
        case STAT_CHA:  // low contact
          return -20;
        case STAT_KAR:  // mildly upbeat outlook
          return 20;
        case STAT_INT:  // mildly substandard education
          return -20;
        case STAT_WIS:
          return -15;
        case STAT_CON:  // mildly healthy environment
          return 20;
        case STAT_BRA:
          return 15;
        case STAT_FOC:  // there wasn't a lot to do
          return -15;
        case STAT_PER:
          return 10;
        case STAT_SPE:
          return 0;
        case STAT_AGI:
          return 5;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_FOREST:
    case HOME_TER_ELF_WOOD:
    case HOME_TER_HOBBIT_WOODLAND:
      switch (whichStat) {
        case STAT_CHA:  // low contact
          return -15;
        case STAT_KAR:  // mildly upbeat outlook
          return 15;
        case STAT_INT:  // mildly substandard education
          return -15;
        case STAT_WIS:
          return -15;
        case STAT_CON:  // mildly healthy environment
          return 15;
        case STAT_BRA:
          return 15;
        case STAT_FOC:  // there wasn't a lot to do
          return -15;
        case STAT_PER:
          return 10;
        case STAT_SPE:
          return 0;
        case STAT_AGI:
          return 5;
        default:
          return 0;
      }
    case HOME_TER_HUMAN_MARINER:
    case HOME_TER_ELF_SEA:
    case HOME_TER_GNOME_SWAMP:
    case HOME_TER_HOBBIT_MARITIME:
      switch (whichStat) {
        case STAT_CHA:  // low contact
          return -5;
        case STAT_KAR:  // mildly upbeat outlook
          return 5;
        case STAT_INT:  // mildly substandard education
          return -5;
        case STAT_WIS:
          return -5;
        case STAT_CON:  // mildly healthy environment
          return 5;
        case STAT_BRA:
          return 5;
        case STAT_FOC:  // there wasn't a lot to do
          return -5;
        case STAT_PER:
          return 5;
        case STAT_SPE:
          return 0;
        case STAT_AGI:
          return 0;
        default:
          return 0;
      }
    default:
      return 0;
  }
}

sstring statToString(statTypeT whichStat) {
  switch (whichStat) {
    case (STAT_STR):
      return "strength";
    case (STAT_BRA):
      return "brawn";
    case (STAT_CON):
      return "constitution";
    case (STAT_DEX):
      return "dexterity";
    case (STAT_AGI):
      return "agility";
    case (STAT_SPE):
      return "speed";
    case (STAT_INT):
      return "intelligence";
    case (STAT_WIS):
      return "wisdom";
    case (STAT_FOC):
      return "focus";
    case (STAT_PER):
      return "perception";
    case (STAT_CHA):
      return "charisma";
    case (STAT_KAR):
      return "karma";
    case (STAT_LUC):
      return "luck";
    default:
      return "none";
  }
}

int TBeing::getStat(statSetT fromSet, statTypeT whichStat) const {
  int amount;
  int my_age;

  switch (fromSet) {
    case (STAT_CHOSEN):
      return chosenStats.get(whichStat);
    case (STAT_CURRENT):
      return curStats.get(whichStat);
    case (STAT_NATURAL):
      // natural should be mostly based on race
      amount = race->baseStats.get(whichStat);

      // throw on the chosen stats as a modifier
      amount += chosenStats.get(whichStat);

      // and add on age modifiers
      my_age = age()->year - getBaseAge() + 17;
      if (!isVampire())
        amount += age_mod_for_stat(this, my_age, whichStat);

      amount += territory_adjustment(player.hometerrain, whichStat);

      // monk skill
      // this is kind of a wack place to put it I think, but it's the easiest
      // place to do a dynamic change based on skill
      if (discs && whichStat == STAT_STR && doesKnowSkill(SKILL_IRON_MUSCLES)) {
        amount += getSkillValue(SKILL_IRON_MUSCLES) / 8;
      }

      if (isVampire() && (whichStat == STAT_STR || whichStat == STAT_SPE ||
                           whichStat == STAT_CHA))
        amount += 25;

      return amount;
    case (STAT_RACE):
      return race->baseStats.get(whichStat);
    case (STAT_AGE):
      if (!isVampire())
        return age_mod_for_stat(this, (age()->year - getBaseAge() + 17),
          whichStat);
      else
        return 0;
    case (STAT_TERRITORY):
      return territory_adjustment(player.hometerrain, whichStat);
  }
  return 0;
}

int TBeing::setStat(statSetT whichSet, statTypeT whichStat, int value) {
  switch (whichSet) {
    case (STAT_CHOSEN):
      return chosenStats.set(whichStat, value);
    case (STAT_NATURAL):
      vlogf(LOG_BUG,
        "some piece of code was trying to set a person's natural stats");
      return 0;
    case (STAT_CURRENT):
      return curStats.set(whichStat, value);
    case (STAT_RACE):
    case (STAT_AGE):
    case (STAT_TERRITORY):
      vlogf(LOG_BUG,
        "something tried to set STAT_RACE, STAT_AGE or STAT_TERRITORY");
      return 0;
  }
  return 0;
}

int TBeing::addToStat(statSetT whichSet, statTypeT whichStat, int modifier) {
  switch (whichSet) {
    case STAT_CHOSEN:
      return chosenStats.add(whichStat, modifier);
    case STAT_NATURAL:
      vlogf(LOG_BUG, "Illegal attempt to modify Natural Stats.");
      return 0;
    case STAT_CURRENT:
      return curStats.add(whichStat, modifier);
    case STAT_RACE:
    case STAT_AGE:
    case STAT_TERRITORY:
      vlogf(LOG_BUG,
        "Illegal attempt to modify STAT_RACE, STAT_AGE or STAT_TERRITORY");
      return 0;
  }
  return 0;
}

// Use the plotValue function to obtain a result based on a level difference
// between two beings
double plotLevelDiff(const int levelDiff, const double minResult,
  const double maxResult, const double averageResult, const double power) {
  static constexpr int MIN_LEVEL_DIFF = 0;
  static constexpr int MAX_LEVEL_DIFF = 70;

  return plotValue<int, double>(levelDiff, MIN_LEVEL_DIFF, MAX_LEVEL_DIFF,
    minResult, maxResult, averageResult, power);
}

// High stat should yield 1.25 * more dam, low 0.80 less dam
float TBeing::getStrDamModifier() const {
  return plotStat(STAT_CURRENT, STAT_STR, 0.8, 1.25, 1.0, 1.0);
}

// High stat should yield 1.25 * more dam, low 0.80 less dam
float TBeing::getWisDamModifier() const {
  return plotStat(STAT_CURRENT, STAT_WIS, 0.8, 1.25, 1.0, 1.0);
}

int TBeing::getDexReaction() const {
  return plotStat(STAT_CURRENT, STAT_DEX, -4, 6, 0);
}

int TBeing::getAgiReaction() const {
  return plotStat(STAT_CURRENT, STAT_AGI, -4, 6, 0);
}

int TBeing::getConShock() const {
  return plotStat(STAT_CURRENT, STAT_CON, 15, 99, 65);
}

// extra HPs gotten when you level
float TBeing::getConHpModifier() const {
  // From Balance notes:
  // High con should give 5/4 more HP than normal, and low con should be 4/5
  // assuming that warriors have 8 HP/lev, we want -1.6 and +2.0 as the
  // values

  // i think this would work more predictably if we actually just factored in
  // the values and used our standards - Dash
  // (and its still following balance notes)

  return plotStat(STAT_CURRENT, STAT_CON, (float)4.0 / 5.0, (float)5.0 / 4.0,
    (double)1.0);
}

double TBeing::getStatMod(statTypeT statType, int multiplier) const {
  return (
    ((plotStat(STAT_CURRENT, statType, 0.8, 1.25, 1.0) - 1) * multiplier) + 1);
}

float TBeing::getIntModForPracs() const {
  // this formula is convoluted, and we use stat natural because we don't want
  // them to carry around +int eq purely for the sake of gaining.
  return plotStat(STAT_NATURAL, STAT_INT, .8, 1.25, 1.0);
}

float TBeing::getChaShopPenalty() const {
  return plotStat(STAT_CURRENT, STAT_CHA, 1.3, 1.0, 1.1);
}

float TBeing::getSwindleBonus() {
  float chr = 0.0;

  if (doesKnowSkill(SKILL_SWINDLE)) {
    // make 5 separate rolls so chr goes up amount based on learning
    for (int i = 0; i < 5; i++)
      if (bSuccess(SKILL_SWINDLE))
        chr += 0.02;
  }

  return chr;
}

Stats TBeing::getCurStats() const { return curStats; }

Stats::Stats() {
  std::fill(std::begin(values), std::end(values), 150);
  values[STAT_EXT] = 0;
}

// Uses plotStat to convert a given stat to a percent chance of success, then
// rolls to see if success happens in this specific instance. Use a midpoint of
// 25, meaning a character with exactly average stat has a 25% chance to
// succeed. Chance increases exponentially as stat gets closer to max. As with
// attacks, have a static 5% chance to fail or succeed.
bool TBeing::statSelfCheck(statTypeT stat, int num) const {
  return percentChance(plotStat(STAT_CURRENT, stat, 5, 95, 25) + num);
}

bool TBeing::isStrong() const { return statSelfCheck(STAT_STR); }

bool TBeing::isPerceptive() const { return statSelfCheck(STAT_PER); }

bool TBeing::isAgile(int num) const { return statSelfCheck(STAT_AGI, num); }

bool TBeing::isDextrous() const { return statSelfCheck(STAT_DEX); }

bool TBeing::isTough() const { return statSelfCheck(STAT_CON); }

bool TBeing::isBrawny() const { return statSelfCheck(STAT_BRA); }

bool TBeing::isIntelligent() const { return statSelfCheck(STAT_INT); }

bool TBeing::isWise() const { return statSelfCheck(STAT_WIS); }

bool TBeing::isFast() const { return statSelfCheck(STAT_SPE); }

bool TBeing::isFocused() const { return statSelfCheck(STAT_FOC); }

bool TBeing::isCharismatic() const { return statSelfCheck(STAT_CHA); }

bool TBeing::isLucky() const { return statSelfCheck(STAT_KAR); }

bool TBeing::isUgly() const { return statSelfCheck(STAT_CHA); }

bool TBeing::isRealUgly() const { return (isUgly() && isUgly()); }
