// stats.cc

#include "stdsneezy.h"
#include "stats.h"

#include <cmath>

Stats Stats::operator+(const Stats &operand)
{
  Stats sum;

  for(statTypeT stat=MIN_STAT; stat<MAX_STATS; stat++)
    sum.values[stat] = values[stat] + operand.values[stat];

  return sum;
}

Stats Stats::operator-(const Stats &operand)
{
  Stats diff;

  for(statTypeT stat=MIN_STAT; stat<MAX_STATS; stat++)
    diff.values[stat] = values[stat] - operand.values[stat];

  return diff;
}

sh_int Stats::get(statTypeT stat) const
{
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
        "Something tried to access an invalid stat.");

  return values[stat];
}

sh_int Stats::set(statTypeT stat, sh_int val)
{
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
        "Something tried to access an invalid stat.");

  return values[stat] = val;
}

sh_int Stats::add(statTypeT stat, sh_int mod)
{
  mud_assert(((stat >= MIN_STAT) && (stat < MAX_STATS)),
	"Something tried to access an invalid stat.");

  return values[stat] += mod;
}

void Stats::zero()
{
  for(statTypeT stat=MIN_STAT; stat<MAX_STATS; stat++)
    values[stat]=0;
}

sstring Stats::showStats(TBeing *caller)
{
  byte level = caller->GetMaxLevel();
  Stats showStat;
  char tmpbuf[80];
  sstring buf;

  if(level < GOD_LEVEL1)
    showStat = *this - caller->race->baseStats;
  else
    showStat = *this;

  buf = "<c>[STR]<z> <c>[BRA]<z> <c>[CON]<z> <c>[DEX]<z> <c>[AGI]<z> <c>[INT]<z> <c>[WIS]<z> <c>[FOC]<z> <c>[PER]<z> <c>[CHA]<z> <c>[KAR]<z> <c>[SPE]<z>\n\r";

  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
    sprintf(tmpbuf, " %3d  ", showStat.get(stat));
    buf += tmpbuf;
  }
  buf += "\n\r";

  return buf;
}

const sstring Stats::printStatHeader() const
{
  sstring header;

  // Physical Stats
  header = "<c>[STR]<z><c>[BRA]<z><c>[CON]<z><c>[DEX]<z><c>[AGI]<z>";
  // Mental Stats
  header += "<c>[INT]<z><c>[WIS]<z><c>[FOC]<z>";
  // Intangibles
  header += "<c>[PER]<z><c>[CHA]<z><c>[KAR]<z><c>[SPE]<z>\n\r";

  return header;
}

const sstring Stats::printRawStats(const TBeing *) const
{
  sstring rawStats, buf;

  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
    buf = fmt(" %3d ") % get(stat);
    rawStats += buf;
  }
  rawStats += "\n\r";

  return rawStats;
}

int age_mod_for_stat(const TBeing *tb, int age_num, statTypeT whichStat)
{
  if(!tb->hasQuestBit(TOG_REAL_AGING))
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

int territory_adjustment(territoryT ter, statTypeT whichStat)
{
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

int TBeing::getStat(statSetT fromSet, statTypeT whichStat) const
{
  int amount;
  int my_age;

  switch(fromSet){
    case(STAT_CHOSEN):
      return chosenStats.get(whichStat);
    case(STAT_CURRENT):
      return curStats.get(whichStat);
    case(STAT_NATURAL):
      // natural should be mostly based on race
      amount = race->baseStats.get(whichStat);

      // throw on the chosen stats as a modifier
      amount += chosenStats.get(whichStat);

      // and add on age modifiers
      my_age = age()->year - getBaseAge() + 17;
      if(!isVampire())
	amount += age_mod_for_stat(this, my_age, whichStat);
    
      amount += territory_adjustment(player.hometerrain, whichStat);

      // monk skill
      // this is kind of a wack place to put it I think, but it's the easiest
      // place to do a dynamic change based on skill
      if(discs && whichStat == STAT_STR && doesKnowSkill(SKILL_IRON_MUSCLES)){
	amount += getSkillValue(SKILL_IRON_MUSCLES)/8;
      }

      if(isVampire() &&
	 (whichStat == STAT_STR ||
	  whichStat == STAT_SPE ||
	  whichStat == STAT_AGI ||
	  whichStat == STAT_FOC))
	amount += 15;

      return amount;
    case(STAT_RACE):
      return race->baseStats.get(whichStat);
    case(STAT_AGE):
      if(!isVampire())
	return age_mod_for_stat(this, (age()->year - getBaseAge() + 17), whichStat);
      else
	return 0;
    case(STAT_TERRITORY):
      return territory_adjustment(player.hometerrain, whichStat);
  }
  return 0;
}

int TBeing::setStat(statSetT whichSet, statTypeT whichStat, int value)
{
  switch(whichSet){
    case(STAT_CHOSEN):
      return chosenStats.set(whichStat,value);
    case(STAT_NATURAL):
      vlogf(LOG_BUG, "some piece of code was trying to set a person's natural stats");
      return 0;
    case(STAT_CURRENT):
      return curStats.set(whichStat,value);
    case(STAT_RACE): case(STAT_AGE): case(STAT_TERRITORY):
      vlogf(LOG_BUG, "something tried to set STAT_RACE, STAT_AGE or STAT_TERRITORY");
      return 0;
  }
  return 0;
}

int TBeing::addToStat(statSetT whichSet, statTypeT whichStat, int modifier)
{
  switch(whichSet){
  case(STAT_CHOSEN):
    return chosenStats.add(whichStat,modifier);
  case(STAT_NATURAL):
    vlogf(LOG_BUG,"You should not attempt to modify Natural Stats.");
    return 0;
  case(STAT_CURRENT):
    return curStats.add(whichStat,modifier);
    case(STAT_RACE): case(STAT_AGE): case(STAT_TERRITORY):
      vlogf(LOG_BUG, "something tried to add to STAT_RACE, STAT_AGE or STAT_TERRITORY");
      return 0;
  }
  return 0;
}

int TBeing::plotStat(statSetT x, statTypeT y, int a, int b, int c, double n) const
{
  return (int) plotStat(x, y, (double) a, (double) b, (double) c, n);
}

float TBeing::plotStat(statSetT x, statTypeT y, float a, float b, float c, double n) const
{
  return (float) plotStat(x, y, (double) a, (double) b, (double) c, n);
}

double TBeing::plotStat(statSetT whichSet, statTypeT whichStat, double min_value, double max_value, double avg, double power) const
{
  // takes a stat given by whichSet/whichStat and maps it to a curved function
  // this curved function is like an s turned on its side.  its flat in the
  // middle, and curves up to the right, and down to the left.
  // to avoid using some whacky formula, model it as two separate curves

  // in general, we want this curve to be of form
  // Y(x) = A x ^ n + B
  // where A and B are known constants, and n we modify some to get nice
  // numbers.

  mud_assert(((max_value >= avg && avg >= min_value) || (max_value <= avg && avg <= min_value)),
     "Problem in assignment of values to plotStat (%.2f, %.2f, %.2f)", min_value, max_value, avg);

  // this function gets called A LOT!
  // as such, it behooves us to try and limit the impact
  // since it also uses nasty math functions, lets save results
  // as they get discovered
  static bool cleared = false;
  static double storedPlots[250];
  if (!cleared) {
    for (int i = 0; i < 250; i++)
      storedPlots[i] = -10;
    cleared = true;
  }
 
  int MAXSTAT = 205;
  int MINSTAT = 005;
  if (whichSet == STAT_CHOSEN) {
    MAXSTAT = 25;
    MINSTAT = -25;
  }
  int midline = ((MAXSTAT - MINSTAT) /2) + MINSTAT;

  // boundary conditions:
  // Y(midline) = avg
  // Y(MAXSTAT) = max_value   : flipside being Y(MINSTAT) = min_value
  // A linear formula (power = 1.0) somehow seems bad
  // A quadratic (power = 2.0) meant they had to get 165 stats to get 1/2 max_value
  // I chose (power = 1.4 by default) since it was more in middle of these two
  // lowering n causes a stat closer to midline to have a larger number

  // A little algebra and we get:
  // A = (max_value - avg)/(MAXSTAT ^ power - midline ^ power)
  // B = avg - (midline^power) * A 

  // March, 2001:
  // this is incorrect, A = (max_value - avg)/(MAXSTAT-midline)^power


  int stat = getStat(whichSet, whichStat);
  // pin the value if necessary
  stat = min(max(stat, MINSTAT), MAXSTAT);
#if 0
  if (power == 1.4 && whichSet != STAT_CHOSEN) {
    // we only store for the default value of power
    // storedPlot will represent the curve as normalized to -1,1,0
    double num = storedPlots[stat];
    if (num == -10) {
      // determine the normalized value
      double A, B;
      if (stat >= midline) {
        A = 1  / (pow(MAXSTAT, power) - pow(midline, power)); 
      } else {
        A = 1 / (pow(midline, power) - pow(MINSTAT, power)); 
      }

      // this if statment is equiv to A = abs(1 / (pow(midline,power) - pow(MAXSTAT,power)))
      B = - pow(midline, power) * A;
      num = A * pow(stat, power) + B;
      storedPlots[stat] = num;
    }

    // use the stored value, and scale based on the input
    if (num > 0)
      return (num * (max_value-avg))+avg;
    else
      return (num * (avg-min_value))+avg;
  }
#endif
  double A, B;
  double num; 

  if (stat >= midline) {
    A = (max_value - avg) / (pow(MAXSTAT - midline, power)); 
    B = avg;
    num = A * pow(stat - midline, power) + B;
  } else {
    A = (min_value - avg) / (pow(midline - MINSTAT, power)); 
    B =  avg;
    num = A * pow(midline - stat, power) + B;
  }
  return num;
}

// Some notes on these two dam formulas
// High stat should yield 1.25 * more dam, low 0.80 less dam
float TBeing::getStrDamModifier() const
{
  // the name of this function is historical, we want to use brawn (damage)
  // not strength (weight)
  // um no, we use str.
  return plotStat(STAT_CURRENT, STAT_STR, 0.8, 1.25, 1.0, 1.0);
}

float TBeing::getDexDamModifier() const
{
  // this is archaic, we don't want dex to affect damage, it is already factored in.
  return 1.0;
  //  return plotStat(STAT_CURRENT, STAT_DEX, 0.8, 1.25, 1.0, 1.0);
}

int TBeing::getDexReaction() const
{
  return plotStat(STAT_CURRENT, STAT_DEX, -4, 6, 0);
}

int TBeing::getAgiReaction() const
{
  return plotStat(STAT_CURRENT, STAT_AGI, -4, 6, 0);
}

int TBeing::getConShock() const
{
  return plotStat(STAT_CURRENT, STAT_CON, 15, 99, 65);
}

// extra HPs gotten when you level
float TBeing::getConHpModifier() const
{
  // From Balance notes:
  // High con should give 5/4 more HP than normal, and low con should be 4/5
  // assuming that warriors have 8 HP/lev, we want -1.6 and +2.0 as the
  // values

  // i think this would work more predictably if we actually just factored in
  // the values and used our standards - Dash
  // (and its still following balance notes)

  return plotStat(STAT_CURRENT, STAT_CON, (float) 4.0/5.0, (float) 5.0/4.0, (double) 1.0);
}

float TBeing::getDexMod() const
{
  return plotStat(STAT_CURRENT, STAT_DEX, 0.8, 1.25, 1.0);
}

float TBeing::getAgiMod() const
{
  return plotStat(STAT_CURRENT, STAT_AGI, 0.8, 1.25, 1.0);
}

float TBeing::getSpeMod() const
{
  return plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);
}

float TBeing::getBraMod() const
{
  return plotStat(STAT_CURRENT, STAT_BRA, 0.8, 1.25, 1.0);
}

float TBeing::getFocMod() const
{
  return plotStat(STAT_CURRENT, STAT_FOC, 0.8, 1.25, 1.0);
}

float TBeing::getIntModForPracs() const
{
  // this formula is convoluted, and we use stat natural because we don't want them
  // to carry around +int eq purely for the sake of gaining.
  return plotStat(STAT_NATURAL, STAT_INT, .8 , 1.25, 1.0);
}

float TBeing::getChaShopPenalty() const
{
  return plotStat(STAT_CURRENT, STAT_CHA, 1.3, 1.0, 1.1);
}

float TBeing::getSwindleBonus()
{
  float chr=0.0;

  if(doesKnowSkill(SKILL_SWINDLE)){
    // make 5 separate rolls so chr goes up amount based on learning
    for (int i = 0; i < 5; i++)
      if (bSuccess(SKILL_SWINDLE))
	chr += 0.02;
  }

  return chr;
}

Stats TBeing::getCurStats() const
{
  return curStats;
}

Stats::Stats()
{
  statTypeT stat;
  for(stat=MIN_STAT; stat < MAX_STATS; stat++)
    values[stat]=150;

  values[STAT_EXT] = 0;
}

Stats::Stats(const Stats &a)
{
  statTypeT stat;
  for(stat=MIN_STAT; stat < MAX_STATS; stat++)
    values[stat]=a.values[stat];
}

Stats & Stats::operator=(const Stats &a)
{
  if (this == &a) return *this;
  statTypeT stat;
  for(stat=MIN_STAT; stat < MAX_STATS; stat++)
    values[stat]=a.values[stat];
  return *this;
}

Stats::~Stats()
{
}

bool TBeing::isStrong() const
{
  return (plotStat(STAT_CURRENT, STAT_BRA, 30, 180, 105, 1.0) >= ::number(30,200));
}

bool TBeing::isPerceptive() const
{
  return (plotStat(STAT_CURRENT, STAT_PER, 30, 180, 105, 1.0) >= ::number(30,200));
}

bool TBeing::isAgile(int num) const
{
  return ((plotStat(STAT_CURRENT, STAT_AGI, 30, 180, 105, 1.0) + num) >= ::number(30,200));
}

bool TBeing::isTough() const
{
  return (plotStat(STAT_CURRENT, STAT_CON, 30, 180, 105, 1.0) >= ::number(30,200));
}

bool TBeing::isUgly() const
{
  return (plotStat(STAT_CURRENT, STAT_CHA, 30, 180, 105, 1.0) >= ::number(30,200));
}

bool TBeing::isRealUgly() const
{
  return (isUgly() && isUgly() && !::number(0,1));
}
