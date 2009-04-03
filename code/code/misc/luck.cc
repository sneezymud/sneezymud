//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

// assumption is that mod/20 is "level" of the effect being defended against
bool TBeing::isLucky(int mod) const
{
  // this is basically a saving throw
  // we will reduce it to basically a level comparison which is basically
  // a 50% +- 1.5% per level.
  // mod will come in the range 0-1000 which we will translate to level
  // the higher mod is, the higher the effect they are saving against

  if (isImmortal())
    return TRUE;

  if (getPosition() <= POSITION_STUNNED)
    return FALSE;

  int lev = GetMaxLevel() * 20;

  // make a karma adjustment
  lev = (int) (lev * plotStat(STAT_CURRENT, STAT_KAR, 0.8, 1.25, 1.0));

  // kinda convert to a percentage, but multiply by 100 so 50%=5000 pts
  // diff = 0 should be 50%
  // if diff > 0, should be > 50% chance
  // every 20 points of diff represents 1.5%=150 pts
  int diff = lev - mod;
  int chance = 5000 + (int) (7.5 * diff);

  if (::number(0,9999) < chance)
    return true;
  return false;
}

// pass in a "level" and it will return a value suitable to pass as a mod
// for isLucky.  Pretty silly under the hood, but given history, we seem
// to change how saving throws work every 10 months, so this simplifies
// that process
int levelLuckModifier(float lev)
{
  return (int) (lev * 20);
}

// takes a spell/skill as an argument and returns a modifier appropriate
// for passing into isLucky as the "mod" argument.
// essentially, it figures what level this is casting the skill at and
// makes it a mod.
int TBeing::spellLuckModifier(spellNumT spell)
{
  if (isImmortal())
    return 5000;

  int lvl = getSkillLevel(spell);
  int mod = levelLuckModifier(lvl);

  // possible additions are possible here.
  // difficulty of spell
  // terrain and other impediments
  // focus, etc
  // karma

  return mod;
}

