
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "LOW.cc" - routines related to checking stats.
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>

#include "extern.h"
#include "handler.h"
#include "being.h"
#include "colorstring.h"
#include "statistics.h"
#include "combat.h"
#include "database.h"
#include "obj_player_corpse.h"
#include "obj_gas.h"
#include "obj_arrow.h"
#include "obj_base_weapon.h"
#include "obj_pool.h"
#include "obj_base_clothing.h"
#include "obj_worn.h"
#include "pathfinder.h"
#include "obj_commodity.h"
#include "obj_plant.h"
#include "obj_tooth_necklace.h"
#include "obj_trash_pile.h"
#include "obj_jewelry.h"
#include "spec_mobs.h"
#include "materials.h"
#include "monster.h"
#include "person.h"
#include "low.h"
#include "configuration.h"

double balanceCorrectionForLevel(double level)
{
  // from balance notes
  // PC damage was miscalculated and the extra hit from specializing
  // was "lost", in essence, allowing PC melee damage to be 2* what it
  // ought to be under some situations
  // this returns a value from 1.0 to 2.0 indicating how much more dam
  // a char is getting as a result of this, and we can use this factor
  // in "tweaking" balance
  double boost = min(max(((level-25) * 0.1) + 1.0, 1.00), 2.0);
  return boost;
}

double get_doubling_level(float real_lev)
{
  // doubling level is defined as the number of levels over mine at
  // which point a creature is twice as possible

  // since what follows is resource intensive, let's just do the calculations
  // once, and do lookups thereafter
  static bool loaded = false;
  static std::map<int, double>mob_double_table;
  const double RESOLUTION = .01;

  if (!loaded) {
    double k;
    for (k = 0.0; ; k += RESOLUTION) {
      double perc = (0.60 - k * 0.03)/(0.60 + k * 0.03);

      double lbla = sqrt(2.0 * perc) - 1.0;

      // recall: K = La * (sqrt(2 * Pa/Pb) - 1)
      // K = La * lbla
      // La = K / lbla
      int lookup_lev = (int) (k/lbla/RESOLUTION);

      mob_double_table[lookup_lev] = k;
      
      if ((int) (lookup_lev*RESOLUTION) >= MAX_MORT)
        break;
    }
    loaded = true;
  }
  int lookup_lev = (int) (max(real_lev, (float) 1.0)/RESOLUTION);

  double k_lev = 1.000;
  do {
    std::map<int, double>::const_iterator CT = mob_double_table.find(lookup_lev);
    if (CT == mob_double_table.end()) {
      // if we aren't an exact match, back up a bit and try again
      k_lev = 0.0;
      lookup_lev -= 1;
    } else
      k_lev = CT->second;
  } while (k_lev == 0);

  if (lookup_lev <= 0)
    return 1.0;

  return k_lev;
}

static double get_perc_level(float real_lev)
{
  // Since we know the doubling level, we postulate that at my level
  // we can calculate a percentage growth.
  // That is, if we had a 1% growth, we would double in 69.3 levels....
  // IN essence, we are turning the doubling level into a percentage growth
  // where 100% = double in 1 level, 50% = double in two levs, etc.

  // this tends also to be resource intensive, so only do it once and
  // save it for later
  static bool loaded = false;
  static std::map<int, double>mob_perc_table;
  const double RESOLUTION = .01;

  if (!loaded) {
    double lev;
    for (lev = 0.0; lev < 50.0; lev += RESOLUTION) {
      double dlev = get_doubling_level(lev);
      double one_ver = 1.0/dlev;
      double perc = pow(2.0, one_ver) - 1;

      // As a final quirk, realize the percentage growth is changing constantly
      // What I found is that if I assigned the above calculated perc to the
      // lev here, I later wound up with an XP value that was like 3-4X at
      // the doubling level.
      // to counter act this, I simply assign the percent I just calculated
      // to a level that is slightly lower.  Somewhat arbitrary, but has the
      // desired effect
      int newlev = (int) ((lev - 0.4*dlev)/RESOLUTION);

      mob_perc_table[newlev] = perc;

      if (newlev > (int) (50.0/RESOLUTION))
        break;
    }

    loaded = true;
  }

  double k_perc = 0.0;
  int lookup_lev = (int) (real_lev/RESOLUTION);
  do {
    std::map<int, double>::const_iterator CT = mob_perc_table.find(lookup_lev);
    if (CT == mob_perc_table.end()) {
      // if we aren'tan exact match, back up a bit and try again
      k_perc = 0.0;
      lookup_lev -= 1;
    } else
      k_perc = CT->second;
  } while (k_perc == 0.0 && lookup_lev > 0);

  if (lookup_lev <= 0)
    return 1.0;

  return min(k_perc, 3.0);
}

double mob_exp(float real_lev)
{
  // This is a new EXP formula devised by Batopr 12/25/98
  // It is predicated on the axiom that a mob that is twice as difficult
  // should award twice the XP.
  // let l be my level
  // let k be the level difference above mine that is twice as hard.

  // rnds = rnds it takes to kill mob
  // HP = Kh * Lm = hp of mob
  // P = percent chance of hitting mob
  // Dam = Kd * L0 = dam I do per round
  // rnds = HP / (P * Dam)
  // rnds = Kh/Kd * 1/P * Lm/L0

  // Compute doubling level as the point where rnds for A to kill B is
  // twice that of B killing A

  // Kh/Kd * 1/Pa * Lb/La = 2 * Kh/Kd * 1/Pb * La/Lb
  // 2 * (Pa/Pb) = (Lb/La)^2
  // Lb/La = sqrt(2 * Pa/Pb)

  // Realize that Lb = La + K
  // where K is the "doubling level", the number of levels OVER mine which
  // is twice as powerful
  // K/La +1 = sqrt(2 *Pa/Pb)
  // K = La * (sqrt(2 * Pa/Pb) - 1)

  // this is arbitrary, but is what we use in the hits() formula so
  // don't toy with it lightly
  // Pa = 0.60 - 0.03*K
  // Pb = 0.60 + 0.03*K

  // Beyond L50, just use a linear scale
  if (real_lev > 50.0) {
    // mobs beyond MAX_MORT are on a different scale
    // the constants used here depend critically on the other scale though
    // so tweaking should not be done in isolation...
    // the below constants are based on what the mob would have at L50
    // and giving a flat rate for each level beyond that
    // this is actually less than they would get on the other scale.
    real_lev -= 50.0;

    // I'm artificially trimming this further because of mass abuse
    // of L50+ mobs by large groups.  I can't quite put finger on the problem
    real_lev /= 3;

    double i_mob_xp = 2322144.999809;
    i_mob_xp += (327436.877146 * real_lev);
    return i_mob_xp;
  }

  // OK, given our level, we are able to calculate a percentage growth for
  // this level.
  // We thus start at a given XP value, and keep taking small slices
  // each of which is percentage growth bigger than the last.
  // effectively, causing XP to follow the percentage growth curve, which we
  // set up so that it modeled the desired doubling level.

  // Obviously, it's resource intensive, so again, do it once and save
  static bool loaded = false;
  static std::map<int, double>mob_xp_table;
  const double RESOLUTION = .01;

  if (!loaded) {
    double lev;
    double exp = 0.1;
    for (lev = 0.0; lev < 50.0; lev += RESOLUTION) {
      double dlev = get_perc_level(lev);

      // we can safely throw an arbitrary constant here, so I chose one
      // that made XP to lev L50 come out nice
      exp += 1.020 * (dlev * exp) * RESOLUTION;

      int newlev = (int) (lev/RESOLUTION);
      mob_xp_table[newlev] = exp;
    }
#if 0
    // for logging...
    for (lev = 0.0; lev < 50.0; lev += RESOLUTION) {
      int newlev = (int) (lev/RESOLUTION);
      double mob_xp;

      do {
        std::map<int, double>::const_iterator CT = mob_xp_table.find(newlev);
        if (CT == mob_xp_table.end()) {
          // if we aren'tan exact match, back up a bit and try again
          mob_xp = 0.0;
          newlev -= 1;
        } else
          mob_xp = CT->second;
      } while (mob_xp == 0);

      int newlev2 = (int) ((lev+get_doubling_level(lev))/RESOLUTION);
      double mob_xp2;

      do {
        std::map<int, double>::const_iterator CT = mob_xp_table.find(newlev2);
        if (CT == mob_xp_table.end()) {
          // if we aren'tan exact match, back up a bit and try again
          mob_xp2 = 0.0;
          newlev2 -= 1;
        } else
          mob_xp2 = CT->second;
      } while (mob_xp2 == 0);

      FILE *fp = fopen("xp_table.log", "a+");
      if (fp) {
        fprintf(fp, "%d : xp: %f, l2:%d, xp2: %f, fact: %f\n", newlev, mob_xp, newlev2, mob_xp2, mob_xp2/mob_xp);
        fclose(fp);
      }
    }
#endif
    loaded = true;
  }

  double mob_xp = 0.0;
  int lookup_lev = (int) (real_lev/RESOLUTION);
  do {
    std::map<int, double>::const_iterator CT = mob_xp_table.find(lookup_lev);
    if (CT == mob_xp_table.end()) {
      // if we aren't an exact match, back up a bit and try again
      mob_xp = 0.0;
      lookup_lev -= 1;
    } else
      mob_xp = CT->second;
  } while (mob_xp == 0);

  return mob_xp;
}

int kills_to_level(int lev)
{
  return (int) (17 + (1.25 * lev));
#if 0
  // these are alternatives to consider
  return (int) (2 + (1.6 * lev));
  return (int) (0 + (1.7 * lev));
#endif
}
  
static double hpModifier(const TMonster *ch)
{
  double sanct_modifier = (100.0 / (100.0 - min(99.0, (double) ch->getProtection())));
  double hp_modifier = ((double) ch->hitLimit());

  // adjust hp for correction made in calcHitPoints
  hp_modifier /= balanceCorrectionForLevel(ch->GetMaxLevel());

  if (ch->getHPLevel() <= 70) 
  {
    hp_modifier *= sanct_modifier * 2.0 / 31.0;
  }
  else
  // mobs over level 70 get special treatment
  {
    hp_modifier *= sanct_modifier * 1 / ( 15.5 +
      11 * (ch->getHPLevel() - 70) * (ch->getHPLevel() - 70) / 150);
  }
  return hp_modifier;
}

static double acModifier(const TBeing *ch)
{
  double armor_modifier = max(0.0, (600.0 - ((double) ch->getArmor())) / 20.0);
  return armor_modifier;
}

static double avgDam(const TBeing *ch)
{
  double av_dam = ch->baseDamage();
  av_dam += ((double) ch->getDamroll());
  av_dam = av_dam * ch->getMult();
  return av_dam;
}

static double damModifier(const TBeing *ch)
{
  // we essentially wantmobs doing 0.9 * level dam per round
  double av_dam = avgDam(ch);
  float damage_modifier = av_dam * 1.1;

  return damage_modifier;
}

#if 0
// the old formula assumed a lot of attacks made a mob easier
// rationale: absorbtion is per hit based, so spreading your damage around
// more, means more is absorbed
// this mechanism isn't a good way to trap this, and my testing didn't
// indicate that number of attacks made much difference.
// - Bat
static double numattModifier(const TBeing *ch)
{
  double av_dam = avgDam(ch);
  double hitting_level = av_dam - (ch->getMult() * ((double) ch->GetMaxLevel()) / 10.0);
  hitting_level *= 1.1;
  hitting_level = max(0.0, hitting_level);
  return hitting_level;
}
#endif

static double thacoModifier(const TBeing *ch)
{
  double thaco_level = (double) (ch->GetMaxLevel() + ch->getHitroll() / 10.0);
  return thaco_level;
}

// this function is a real low-level thing
// it basically attempts to have the mud figure out what level is
// appropriate for the mob based on its stats.
// the outcome is used in determineExp() and in some LOW warnings
//
// Making ANY CHANGES will cause massive retweaking by the LOWs so
// don't fuck with this function lightly!!!!!
// If a certain class gives out too much xp, and we are just trying
// to patch around this (until AI improves or whatever), muck with
// determineExp()  -Batopr 10/26/98
double TMonster::getRealLevel() const
{
  double hp_modifier, damage_modifier, real_level;
  double armor_modifier;

  // this should be kept so numbers increase

  hp_modifier = hpModifier(this);
  armor_modifier = acModifier(this);
  damage_modifier = damModifier(this);

  // modify damage based on absorbtion 
  // (assume 1 point dam absorbed per hit per 10 levels)
  // double hitting_level = numattModifier(this);

  // throw on a thac0 modifier too
  double thaco_level = thacoModifier(this);

  // now make a rough approximation of the level.
  // we sort of want the average of the above levels
  // however, we don't want to let one or two whacked values
  // to dominate.  So what we will do is throw out the high and low
  // and average the other two, then use this value heavily but throw
  // in the other values too.
  double best_value = max(max(max(hp_modifier, damage_modifier), thaco_level), armor_modifier);
  double worst_value = min(min(min(hp_modifier, damage_modifier), thaco_level), armor_modifier);
  double avg_value = (hp_modifier + armor_modifier + damage_modifier + thaco_level - best_value - worst_value) / 2.0;

  double rough_level = (hp_modifier + armor_modifier + damage_modifier + thaco_level + (8.0 * avg_value) ) / 12.0;

  // fine tune the range and throw whacked values into line
  hp_modifier = max(min(hp_modifier, 1.1 * rough_level), 0.8 * rough_level);
  armor_modifier = max(min(armor_modifier, 1.3 * rough_level), 0.7 * rough_level);
  damage_modifier = max(min(damage_modifier, 1.1 * rough_level), 0.8 * rough_level);
  thaco_level = max(min(thaco_level, 1.1 * rough_level), 0.8 * rough_level);

  // to get around really small levels vs really large levels, just add a large
  // constant to both and then remove it at the end.  This is TWEAK.
  const int TWEAK = 10000;
  double attack_level = sqrt((damage_modifier + TWEAK) * (thaco_level + TWEAK)) - TWEAK;
  double defense_level = sqrt((armor_modifier + TWEAK) * (hp_modifier + TWEAK)) - TWEAK;

  // testing has shown defense to be more critical, AC in particular.
  // so we weight it higher
  real_level = ((1.0 * attack_level) + (4.0 * defense_level)) / 5.0;

  // up to this point, we are basically assuming everybody is a warrior
  // make some other adjustments for things that affect overall power and
  // hence level
if (!isTestmob()) {
  // flight improves your fighting ability, so allow for this
  // L5(0.5), L30(1.0)
  if (canFly())
    real_level += min(1.00, real_level/10.0);

  // class based adjustments
  // this is based on empiracal data gathered November 1998 - bat
  // I basically took a warrior test mob, changed its class and determined
  // what level warrior test mob it could beat.
  // from there, i derived the curves.
  // also, assume at L1 all skills suck (low chance of hitting, low damage)
  // I will report 3 values, one at L15, one at L30, and one at L45
  // the values represent the number of levels over my level I was able
  // to beat, thus a value of 2.5 for L15, means that if i take the L15 
  // warrior test mob, and change its class, It will now beat an L17, but
  // lose to the L18 warrior test mobs.
  double class_adjust = 0.0;
  if (hasClass(CLASS_MAGE | CLASS_SHAMAN)) {
    // data taken by Batopr 2/2/99
    double mage_array[] = { 0.00,  // L0
      -0.02, 0.01, -0.07, -0.07, -0.12,  // L5
      2.55, 2.63, 2.57, 2.49, 1.50,  // L10
      3.55, 5.97, 4.18, 4.67, 5.17,  // L15
      5.40, 4.90, 5.41, 5.25, 4.80,  // L20
      4.55, 4.34, 4.18, 4.01, 4.00,  // L25
      3.99, 4.03, 4.04, 4.03, 4.13,  // L30
      4.94, 6.00, 5.86, 5.55, 5.69,  // L35
      5.64, 5.56, 5.60, 5.67, 5.56,  // L40
      5.54, 5.64, 5.54, 5.98    
    };
    unsigned int num_elems = (sizeof(mage_array)/sizeof(double))-1;
    if (real_level >= num_elems)
      class_adjust = mage_array[num_elems];
    else {
      int iLev = (int) real_level;
      double leftover = real_level - iLev;
      class_adjust = mage_array[iLev] + leftover * (mage_array[iLev+1] - mage_array[iLev]);
    }
  } else if (hasClass(CLASS_CLERIC)) {
    // data taken by Batopr 2/2/99
    double cleric_array[] = { 0.00,  // L0
      0.00, 0.00, 0.40, 0.60, 0.48,  // L5
      1.32, 0.94, 1.44, 1.72, 1.93,  // L10
      5.60, 8.55, 7.33, 7.54, 7.95,  // L15
      8.16, 7.11, 6.29, 5.69, 5.17,  // L20
      4.65, 4.13, 3.90, 4.11, 4.73,  // L25
      5.28, 5.63, 5.85, 5.20, 4.53,  // L30
      6.27, 6.23, 5.54, 6.64, 6.50,  // L35
      5.76, 5.33, 5.58, 6.24, 5.93,  // L40
      6.49, 7.27
    };
    unsigned int num_elems = (sizeof(cleric_array)/sizeof(double))-1;
    if (real_level >= num_elems)
      class_adjust = cleric_array[num_elems];
    else {
      int iLev = (int) real_level;
      double leftover = real_level - iLev;
      class_adjust = cleric_array[iLev] + leftover * (cleric_array[iLev+1] - cleric_array[iLev]);
    }
  } else if (hasClass(CLASS_THIEF)) {
    class_adjust = min(real_level, 15.0) * 0.033;
    class_adjust += min(max((real_level - 15.0), 0.0), 15.0) * 0;
    class_adjust += min(max((real_level - 30.0), 0.0), 15.0) * 0.033;
  } else if (hasClass(CLASS_DEIKHAN)) {
    // data taken by Batopr 3/6/99
    double deikhan_array[] = { 0.00,  // L0
      0.00, -0.01, 0.61, 1.49, 0.50,  // L5
      1.93, 1.49, 2.27, 2.31, 1.34,  // L10
      2.56, 2.55, 1.47, 1.81, 2.60,  // L15
      2.06, 1.96, 2.16, 1.56, 0.97,  // L20
      2.18, 3.13, 2.40, 1.76, 0.41,  // L25
      0.36, 1.61, 0.99, 1.14, 0.55,  // L30
      1.47, 1.51, 0.68, 1.88, 0.42,  // L35
      1.41, 2.19, 1.75, 0.60, 1.17,  // L40
      1.31, 1.21, 1.38, 1.11, 1.39,  // L45
      1.41, 2.71
    };
    unsigned int num_elems = (sizeof(deikhan_array)/sizeof(double))-1;
    if (real_level >= num_elems)
      class_adjust = deikhan_array[num_elems];
    else {
      int iLev = (int) real_level;
      double leftover = real_level - iLev;
      class_adjust = deikhan_array[iLev] + leftover * (deikhan_array[iLev+1] - deikhan_array[iLev]);
    }
  } else if (hasClass(CLASS_MONK)) {
    class_adjust = min(real_level, 15.0) * 0.067;
    class_adjust += min(max((real_level - 15.0), 0.0), 15.0) * 0.167;
    class_adjust += min(max((real_level - 30.0), 0.0), 15.0) * 0.100;
  } else if (hasClass(CLASS_RANGER)) {
    class_adjust = min(real_level, 15.0) * 0.083;
    class_adjust += min(max((real_level - 15.0), 0.0), 15.0) * 0.1;
    class_adjust += min(max((real_level - 30.0), 0.0), 15.0) * 0.0;
  }
  real_level += class_adjust;
} // !testmob

  if (real_level > 127)
    real_level = 127.;
  if (real_level < 0.0)
    real_level = 0.0;

  return real_level;
}

double TMonster::determineExp()
{
  double d_exp;
  float real_level = (float) getRealLevel();

  d_exp = mob_exp(real_level);

  if (!Config::LoadOnDeath())
    calculateGoldFromConstant();

  if (specials.act & ACT_AGGRESSIVE)
    d_exp *= 1.05;

  // what about Config::LoadOnDeath() here?
  if (!getMoney() && (GetMaxLevel() < 12))
    d_exp += (d_exp * (double) ::number(1,5)) / 100.;   // 1% to 5% increase

  // make grimhaven high level mobs crappy exp
  // due to how mobVnum works re: "number", don't trap med mobs in this
  if ((mobVnum() < 1000) && (GetMaxLevel() > 8) && number >= 0) 
    d_exp *= 0.65;

  // arbitrary increase in exp because sneezy is so hard - bat 9-12-96
  d_exp *= stats.xp_modif;

  return (d_exp);
}

bool UtilProcs(int spec)
{
  switch (spec) {
    case SPEC_ALIGN_DEITY:
    case SPEC_ENGRAVER:
    case SPEC_PET_KEEPER:
    case SPEC_HORSE_FAMINE:
    case SPEC_HORSE_WAR:
    case SPEC_CRAPSGUY:
    case SPEC_SHOPKEEPER:
    case SPEC_POSTMASTER:
    case SPEC_RECEPTIONIST:
    case SPEC_REPAIRMAN:
    case SPEC_SHARPENER:
    case SPEC_HORSE_DEATH:
    case SPEC_HORSE_PESTILENCE:
    case SPEC_FACTION_FAERY:
    case SPEC_HERO_FAERIE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool GuildProcs(int spec)
{
  switch (spec) {
    case SPEC_GM_SHAMAN:
    case SPEC_GM_MONK:
    case SPEC_GM_RANGER:
    case SPEC_GM_DEIKHAN:
    case SPEC_GM_WARRIOR:
    case SPEC_GM_CLERIC:
    case SPEC_GM_THIEF:
    case SPEC_GM_MAGE:
    case SPEC_TRAINER_AIR:
    case SPEC_TRAINER_ALCHEMY:
    case SPEC_TRAINER_EARTH:
    case SPEC_TRAINER_FIRE:
    case SPEC_TRAINER_SORCERY:
    case SPEC_TRAINER_SPIRIT:
    case SPEC_TRAINER_WATER:
    case SPEC_TRAINER_WRATH:
    case SPEC_TRAINER_AFFLICTIONS:
    case SPEC_TRAINER_CURE:
    case SPEC_TRAINER_HAND_OF_GOD:
    case SPEC_TRAINER_RANGER:
    case SPEC_TRAINER_LOOTING:
    case SPEC_TRAINER_MURDER:
    case SPEC_TRAINER_DUELING:
    case SPEC_TRAINER_ADVENTURING:
    case SPEC_TRAINER_WARRIOR:
    case SPEC_TRAINER_WIZARDRY:
    case SPEC_TRAINER_FAITH:
    case SPEC_TRAINER_SLASH:
    case SPEC_TRAINER_BLUNT:
    case SPEC_TRAINER_PIERCE:
    case SPEC_TRAINER_RANGED:
    case SPEC_TRAINER_BAREHAND:
    case SPEC_TRAINER_DEIKHAN:
    case SPEC_TRAINER_BRAWLING:
    case SPEC_TRAINER_SURVIVAL:
    case SPEC_TRAINER_SHAMAN_ARMADILLO:
    case SPEC_TRAINER_ANIMAL:
    case SPEC_TRAINER_AEGIS:
    case SPEC_TRAINER_SHAMAN:
    case SPEC_TRAINER_COMBAT:
    case SPEC_TRAINER_MAGE:
    case SPEC_TRAINER_MONK:
    case SPEC_TRAINER_CLERIC:
    case SPEC_TRAINER_THIEF:
    case SPEC_TRAINER_PLANTS:
    case SPEC_TRAINER_SOLDIERING:
    case SPEC_TRAINER_BLACKSMITHING:
    case SPEC_TRAINER_DEIKHAN_FIGHT:
    case SPEC_TRAINER_MOUNTED:
    case SPEC_TRAINER_DEIKHAN_AEGIS:
    case SPEC_TRAINER_DEIKHAN_CURES:
    case SPEC_TRAINER_DEIKHAN_WRATH:
    case SPEC_TRAINER_MEDITATION_MONK:
    case SPEC_TRAINER_MINDBODY:
    case SPEC_TRAINER_LEVERAGE:
    case SPEC_TRAINER_FOCUSED_ATTACKS:
    case SPEC_TRAINER_THIEF_FIGHT:
    case SPEC_TRAINER_POISONS:
    case SPEC_TRAINER_SHAMAN_FROG:
    case SPEC_TRAINER_SHAMAN_ALCHEMY:
    case SPEC_TRAINER_SHAMAN_SKUNK:
    case SPEC_TRAINER_SHAMAN_SPIDER:
    case SPEC_TRAINER_SHAMAN_CONTROL:
    case SPEC_TRAINER_RITUALISM:
    case SPEC_TRAINER_STEALTH:
    case SPEC_TRAINER_TRAPS: 
    case SPEC_TRAINER_LORE:
    case SPEC_TRAINER_THEOLOGY:
    case SPEC_TRAINER_DEFENSE:
    case SPEC_TRAINER_SHAMAN_HEALING:
      return TRUE;
    default:
      return FALSE;
  }
}

bool UtilMobProc(TBeing *ch)
{
  if (ch->isPc() || !ch->spec)
    return FALSE;
  if (ch->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
    return TRUE;
  if (ch->rider)
    return TRUE;
  if (ch->isShopkeeper())
    return TRUE;
  if (ch->spec == SPEC_BUTLER)
    return TRUE;
  if (IS_SET(ch->specials.act, ACT_IMMORTAL)) {
	  return TRUE;
  }
  /*
  switch (ch->mobVnum()) {
  	// february quest mobs, we don't want them picking things up...
  	case Mob::GYPSY_ROMANTIC:
  	case Mob::FLORIST_BUSY:
  	case Mob::CLOWN_SCARY:
  	case Mob::MIME_ANIMATED:
  	case Mob::GUY_LAZY:
  	case Mob::WIFE_FARMER:
  	case Mob::DRUNK_TRADER_RICH:
  		return TRUE;
  	default:
  		break;
  } 
  */ 
  return UtilProcs(ch->spec);
}

bool GuildMobProc(TBeing *ch)
{
  if (ch->isPc() || !ch->spec)
    return FALSE;
  if (ch->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
    return TRUE;
  if (ch->rider)
    return TRUE;

  return GuildProcs(ch->spec);
}

void TMonster::checkMobStats(tinyfileTypeT forReal)
{
  int sumstat;
  char *s;

  if (getLongDesc() && (strlen(getLongDesc()) > 2) &&
      ((getLongDesc()[strlen(getLongDesc()) - 1] != '\r') ||
      (getLongDesc()[strlen(getLongDesc()) - 2] != '\n'))) {
    vlogf(LOG_LOW, format("%s (%d) has bad format of longDescr") % 
          getName() % mobVnum());
  }
  if (getDescr() && (strlen(getDescr()) > 2) &&
      ((getDescr()[strlen(getDescr()) - 1] != '\r') ||
      (getDescr()[strlen(getDescr()) - 2] != '\n'))) {
    vlogf(LOG_LOW, format("%s (%d) has bad format of descr") % 
          getName() % mobVnum());
  }
  if (strchr(name, '(') || strchr(name, ')')) {
    vlogf(LOG_LOW, format("%s (%d) has illegal parenthetical in name") % 
          getName() % mobVnum());
  }
  if ((s = strchr(name, '['))) {
    for (s++;*s != ']';s++) {
      if (isalnum(*s) || *s == '_' || *s == '/')
        continue;
      vlogf(LOG_LOW, format("%s (%d) lacked contiguity in braces (%c)") % 
            getName() % mobVnum() % *s);
    }
  }
#if 0
  if (strlen(getName()) > MAX_NAME_LENGTH-1) {
    vlogf(LOG_LOW, format("%s (%d) had excessive mob name length.") % 
         getName() % mobVnum());
  }
#endif

  
  if (forReal == TINYFILE_YES) {
    if (IS_SET(specials.act, ACT_STRINGS_CHANGED))
      vlogf(LOG_LOW, format("%s (%d) strung-mob bit (%d) set") %  getName() % mobVnum() % ACT_STRINGS_CHANGED);
  }
  if (IS_SET(specials.act, ACT_DISGUISED))
    vlogf(LOG_LOW, format("%s disguise bit (%d)  set") %  getName() % ACT_DISGUISED);
  if (IS_SET(specials.act, ACT_HATEFUL))
    vlogf(LOG_LOW, format("%s hateful bit (%d) set") %  getName() % ACT_HATEFUL);
  if (IS_SET(specials.act, ACT_AFRAID))
    vlogf(LOG_LOW, format("%s afraid bit (%d) set") %  getName() % ACT_AFRAID);
  if (IS_SET(specials.act, ACT_HUNTING))
    vlogf(LOG_LOW, format("%s hunting bit (%d) set") %  getName() % ACT_HUNTING);
  if (IS_SET(specials.act, ACT_GUARDIAN))
    vlogf(LOG_LOW, format("%s guardian bit (%d) set") %  getName() % ACT_GUARDIAN);

  if (getImmunity(IMMUNE_BLUNT) < 0)
    vlogf(LOG_LOW,format("mob (%s:%d) with susc blunt.") %
	  getName() % mobVnum());
  if (getImmunity(IMMUNE_PIERCE) < 0)
    vlogf(LOG_LOW,format("mob (%s:%d) with susc pierce.") %
	  getName() % mobVnum());
  if (getImmunity(IMMUNE_SLASH) < 0)
    vlogf(LOG_LOW,format("mob (%s:%d) with susc slash.") %
	  getName() % mobVnum());
  if (getImmunity(IMMUNE_NONMAGIC) < 0)
    vlogf(LOG_LOW,format("mob (%s:%d) with susc non-magic.") %
	  getName() % mobVnum());

  if (isAffected(AFF_SLEEP)) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_SLEEP (%d) set.") %
	  getName() % mobVnum() % AFF_SLEEP);
  if (isAffected(AFF_CHARM) && !master) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_CHARM (%d) and no master set.") %
	  getName() % mobVnum() % AFF_CHARM); 
#if 0
  if (isAffected(AFF_STUNNED)) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_STUNNED set. make %d") %
	  getName() % mobVnum() % (specials.affectedBy & ~AFF_STUNNED));
#endif
  if (isAffected(AFF_SHOCKED)) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_SHOCKED set. make %d") %  
	  getName() % mobVnum() % (specials.affectedBy & ~AFF_SHOCKED));
  if (isAffected(AFF_SYPHILIS)) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_SYPHILIS (%d) set.") %
	  getName() % mobVnum() % AFF_SYPHILIS);
#ifdef JEEZ
  if (isAffected(AFF_UNDEF4)) 
    vlogf(LOG_LOW, format("mob (%s:%d) with AFF_UNDEF4 (%d) set.") %
	  getName() % mobVnum() % AFF_UNDEF4);
#endif
  sumstat = getStat(STAT_CHOSEN, STAT_STR) +
            getStat(STAT_CHOSEN, STAT_BRA) +
            getStat(STAT_CHOSEN, STAT_AGI) +
            getStat(STAT_CHOSEN, STAT_DEX) +
            getStat(STAT_CHOSEN, STAT_CON);
  if (sumstat > 0) {
    vlogf(LOG_LOW,format("mob (%s, %d) with excessive physical stats (%d).") % 
                getName() % mobVnum() % sumstat);
  }

  sumstat = getStat(STAT_CHOSEN, STAT_INT) +
            getStat(STAT_CHOSEN, STAT_WIS) +
            getStat(STAT_CHOSEN, STAT_FOC);
  if (sumstat > 0) {
    vlogf(LOG_LOW,format("mob (%s, %d) with excessive mental stats (%d).") % 
                getName() % mobVnum() % sumstat);
  }

  sumstat = getStat(STAT_CHOSEN, STAT_CHA) +
            getStat(STAT_CHOSEN, STAT_KAR) +
            getStat(STAT_CHOSEN, STAT_SPE) +
            getStat(STAT_CHOSEN, STAT_PER);
  if (sumstat > 0) {
    vlogf(LOG_LOW,format("mob (%s, %d) with excessive utility stats (%d).") % 
                getName() % mobVnum() % sumstat);
  }

  if (!isSingleClass())
    vlogf(LOG_LOW,format("multiclass mob: %s") % getName());

  /*
  // Leave this disabled.  With the new tBorn code it is now outdated.
  if (!IS_SET(specials.act, ACT_SENTINEL) && max_exist > 100) {
    vlogf(LOG_LOW,format("non-sentinel mob (%s : %d) with excessive max exist (%d)") % 
       getName() % mobVnum() % max_exist);
  }
  */
  if (isShopkeeper() && !IS_SET(specials.act, ACT_IMMORTAL)) {
    // have a lot of gold, and otherwise might get killed
    vlogf(LOG_LOW, format("Shopkeeper (%s:%d) needs to be set immortal(%d).") % 
         getName() % mobVnum() % ACT_IMMORTAL);
  }

  if (isWinged() && isAffected(AFF_FLYING)) {
    vlogf(LOG_LOW, format("Winged mob (%s:%d) given magical flight too.  Probably bad") %  getName() % mobVnum());
  }
}

// this actually sets some of the items stats (glow/chaos) so has to
// be done whenever item enters game
void TObj::checkObjStats()
{
  int i;
  char *s;

  if (getDescr() && (strlen(getDescr()) > 2) &&
      (getDescr()[strlen(getDescr()) - 1] == '\r') &&
      (getDescr()[strlen(getDescr()) - 2] == '\n')) {
    vlogf(LOG_LOW, format("%s (%d) has bad format of longDescr") % 
          getName() % objVnum());
  }
  if (strchr(name, '(') || strchr(name, ')')) {
    vlogf(LOG_LOW, format("%s (%d) has illegal parenthetical in name") % 
          getName() % objVnum());
  }
  if ((s = strchr(name, '['))) {
    for (s++;*s != ']';s++) {
      if (isalnum(*s) || *s == '_' || *s == '/')
        continue;
      vlogf(LOG_LOW, format("%s (%d) lacked contiguity in braces (%c)") % 
            getName() % objVnum() % *s);
    }
  }

  if (strlen(getName()) > MAX_NAME_LENGTH-1) {
    vlogf(LOG_LOW, format("%s (%d) had excessive obj name length.") % 
         getName() % objVnum());
  }

  sstring a=stripColorCodes(shortDescr);
  bool found=false;
  
  for(int i=0;a.word(i)!="";++i){
    if(isname(a.word(i), name)){
      found=true;
      break;
    }
  }
  if(!found){
    vlogf(LOG_LOW, format("Item '%s' (%i) lacks appropriate name keyword (%s).") %
	  shortDescr % objVnum() % name);
  }
  
  if(sstring(name) != stripColorCodes(name)){
    vlogf(LOG_LOW, format("Item '%s' (%i) has color codes in name (%s).") %
	  shortDescr % objVnum() % name);
  }


  if(!canWear(ITEM_TAKE) && obj_flags.wear_flags>0){
    vlogf(LOG_LOW, format("Item '%s' (%i) is !take but has other wear flags.") %
	  shortDescr % objVnum());
  }

  if (isObjStat(ITEM_PROTOTYPE)) {
    vlogf(LOG_LOW, format("Item %s had a prototype flag. Get rid of it in tinyworld file!\n\rRemoving bit for object going into game.") %  getName());
    remObjStat(ITEM_PROTOTYPE);
  }


  // TPool strings itself during constructor, so bypass this
  // these are all classes that implement updateDesc()
  if (isObjStat(ITEM_STRUNG) && 
      !dynamic_cast<TPool *>(this) && 
      !dynamic_cast<TGas *>(this) &&
      !dynamic_cast<TCommodity *>(this) &&
      !dynamic_cast<TBaseCup *>(this) &&
      !dynamic_cast<TTrashPile *>(this) &&
      !dynamic_cast<TPlant *>(this) &&
      !dynamic_cast<TToothNecklace *>(this) &&
      spec!=SPEC_GRAFFITI) {
    vlogf(LOG_LOW, format("Item %s has been set strung, fix! (%d)") %  getName() % objVnum());
    remObjStat(ITEM_STRUNG);
  }

  lowCheck();

  int tmp = getMaterial();
  if (!((tmp>=0 && tmp < MAX_MAT_GENERAL) ||
       (tmp>=50 && tmp < (50+MAX_MAT_NATURE)) ||
       (tmp>=100 && tmp < (100+MAX_MAT_MINERAL)) ||
       (tmp>=150 && tmp < (150+MAX_MAT_METAL))))
    vlogf(LOG_LOW, format("Illegal material type %d for %s!") % 
         tmp % getName());

  for (i=0; i<MAX_OBJ_AFFECT;i++) {
    affected[i].checkForBadness(this);
  }

  if (obj_index[number].armor == -99) {   // item not inited
    if (dynamic_cast<TBaseWeapon *>(this))
      obj_index[number].armor = -97;    // init to weapon
    else if (dynamic_cast<TBaseClothing *>(this)) {
      obj_index[number].armor = -itemAC();  // this is positive num
    } else
      obj_index[number].armor = -98;    // init to unused
  }
  if (isObjStat(ITEM_GLOW)) {
    if (isObjStat(ITEM_SHADOWY))
      vlogf(LOG_LOW,format("obj %s is glowing and shadowy.") % 
               getName());
    addGlowEffects();
  }
  if (isObjStat(ITEM_SHADOWY)) {
    canBeSeen += (1 + getVolume()/1500);
#if 0
    for (i=0; i<MAX_OBJ_AFFECT;i++) {
      if (affected[i].location == APPLY_NONE) {
        affected[i].location = APPLY_LIGHT;
        affected[i].modifier = -(1 + getVolume()/3000);
        addToLight(affected[i].modifier);
        if (affected[i].modifier < -6 && canWear(ITEM_TAKE))
          vlogf(LOG_LOW,format("Mega shadow on %s") % getName());
        break;
      } else if (i==(MAX_OBJ_AFFECT-1))
        vlogf(LOG_LOW,format("obj %s has too many affects to set shadow on it.") % 
               getName());
    }
#endif
  }
  if (canWear(ITEM_TAKE))
    if (isObjStat(ITEM_NOPURGE) && (objVnum() != Obj::CRAPS_DICE)) {
      TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(this);
      if (!tmpcorpse)
        vlogf(LOG_LOW,format("item takeable and !purge (%s)") %  getName());
    }

  lowCheckSlots(SILENT_NO);

  
  if(getMaxStructPoints()<getStructPoints()){
    vlogf(LOG_LOW, format("obj %s had greater struct than max struct.") %  getName());
  }

}

void objAffData::checkForBadness(TObj *obj)
{
  bool removeIt  = false,
       isIllegal = false;

  if (!apply_types[location].assignable) {
    isIllegal = true;

    if ((location == APPLY_HITROLL ||
         location == APPLY_DAMROLL ||
         location == APPLY_HITNDAM) &&
        (obj->itemType() == ITEM_BOW ||
         obj->itemType() == ITEM_ARROW ||
         obj_index[obj->getItemIndex()].max_exist <= 3))
      isIllegal = false;
  }

  if (isIllegal)
    vlogf(LOG_LOW,format("%s has an unassignable affect") % obj->getName());

  // changes to this list should also be added to dispelMagic()
  if ((location != APPLY_NONE) && 
      (location != APPLY_LIGHT) &&
      (location != APPLY_NOISE) &&
      (location != APPLY_HIT) &&
      (location != APPLY_CHAR_WEIGHT) &&
      (location != APPLY_CHAR_HEIGHT) &&
      (location != APPLY_MOVE) &&
      (location != APPLY_ARMOR)) {
    if (!obj->isObjStat(ITEM_MAGIC))
      vlogf(LOG_LOW,format("obj (%s:%d) should be set magic due to %s") % 
            obj->getName() % obj->objVnum() % 
            apply_types[location].name);
  }
  if (location == APPLY_SPELL &&
      (modifier < 0 ||
       modifier >= MAX_SKILL ||
       !discArray[modifier])) {
    vlogf(LOG_LOW,format("obj (%s:%d) trying to set skill %d that is not defined.  REMOVING IT!") % 
          obj->getName() % obj->objVnum() % modifier);
    // this would be bad, remove it
    removeIt = true;
  }
  if (location == APPLY_DISCIPLINE &&
      (modifier < 0 ||
       modifier >= MAX_DISCS ||
       !discNames[modifier].disc_num)) {
    vlogf(LOG_LOW,format("obj (%s:%d) trying to set discipline %d that is not defined") % 
         obj->getName() % obj->objVnum() % modifier);
    // this would be bad, remove it
    removeIt = true;
  }
  if (removeIt) {
    location = APPLY_NONE;
    modifier = 0;
    modifier2 = 0;
  }
}

void TBeing::immortalEvaluation(const TMonster * victim) const
{
  if (!isImmortal())
    return;

  sendTo("********* Immortal Evaluation *********\n\r");
  sendTo(format(" Real Level    : %.2f\n\r") % victim->getRealLevel());
  sendTo(format("    HP level      : %.2f (theo: %.1f)\n\r") % hpModifier(victim) % victim->getHPLevel());
  sendTo(format("    Dam level     : %.2f (theo: %.1f)\n\r") % damModifier(victim) % victim->getDamLevel());
  sendTo(format("    AC level      : %.2f (theo: %.1f)\n\r") % acModifier(victim) % victim->getACLevel());
  sendTo(format("    Hitroll level : %.2f\n\r") % thacoModifier(victim));
  sendTo(format("    Spell Hitroll : %d\n\r") % (int) victim->getSpellHitroll());
}

static void compare2Mobs(TBeing *ch, TBeing *mob1, TBeing *mob2)
{
  // what follows is a simplification and extension of logic in combat.cc
  // I'll mark the relavent functions

  // combat.cc hit()
  int offense = mob1->attackRound(mob2);
  int defense = mob2->defendRound(mob1);
  int mod = offense - defense;
  unsigned int num_hits = 0, tot_hits = 10000;
  unsigned int i;
  for (i = 0; i < tot_hits; i++) {
    int ret = mob1->hits(mob2, mod);
    if (ret == TRUE || ret == GUARANTEED_SUCCESS)
      num_hits++;
  }
  float hitRate = (float) num_hits / (float) tot_hits;
  ch->sendTo(COLOR_MOBS, format("%s hits %s roughly %.2f%c of the time.\n\r") %
    mob1->getName() % mob2->getName() % (hitRate * 100.0) % '%');

  double avg_dam = avgDam(mob1);
  float modDam = (float) (avg_dam * hitRate);
  ch->sendTo(COLOR_MOBS, format("Average damage: %.2f, Modified Damage: %.2f\n\r") %
       avg_dam % modDam);
  float num_rounds = mob2->hitLimit() / modDam;
  ch->sendTo(COLOR_MOBS, format("%s has %d HPs, so can survive %.2f rounds.\n\r") %
        mob2->getName() % mob2->hitLimit() % num_rounds);
}

void TBeing::doCompare(const char *arg)
{
  if (!isImmortal() || !hasWizPower(POWER_COMPARE)) {
    doMortalCompare(arg);
    return;
  }

  sstring errMsg = "Syntax: compare <mob1> <mob2>\n\r";

  char mArg1[256], mArg2[256];
  arg = one_argument(arg, mArg1, cElements(mArg1));
  one_argument(arg, mArg2, cElements(mArg2));

  TBeing *mob1, *mob2 = NULL;
  if (mArg1 && *mArg1) {
    mob1 = get_char_vis_world(this, mArg1, NULL, EXACT_YES);
    if (!mob1) {
      mob1 = get_char_vis_world(this, mArg1, NULL, EXACT_NO);
      if (!mob1) {
        sendTo(format("Can't locate '%s' here.\n\r") % mArg1);
        return;
      }
    }
  } else {
    sendTo(errMsg);  
    return;
  }

  if (mArg2 && *mArg2) {
    mob2 = get_char_vis_world(this, mArg2, NULL, EXACT_YES);
    if (!mob2) {
      mob2 = get_char_vis_world(this, mArg2, NULL, EXACT_NO);
      if (!mob2) {
        sendTo(format("Can't locate '%s' here.\n\r") % mArg2);
        return;
      }
    }
  } else {
    sendTo(errMsg);
    return;
  }

  sendTo(COLOR_MOBS, format("Comparison of %s vs %s.\n\r") % mob1->getName() % mob2->getName());

  // first check 1 v 2
  compare2Mobs(this, mob1, mob2);
  // first check 2 v 1
  compare2Mobs(this, mob2, mob1);
}

void TBeing::doLow(const sstring &)
{
  sendTo("You are a monster now, forget about this command.");
}

void TPerson::doLow(const sstring &argument)
{
  sstring buf, arg=argument;

  if (!hasWizPower(POWER_LOW)) {
    sendTo("You do not yet have the low command. If you need something, see a higher level god.");
    return;
  }

  arg=one_argument(arg, buf);

  sstring usage = "Syntax: low <mob | race | statbonus | statcharts | tasks | path room> ...\n\r";
  if (buf.empty()) {
    sendTo(usage);
    return;
  } else if (is_abbrev(buf, "objs") ||
	     is_abbrev(buf, "weapons")) {
    sendTo("The low command does not currently work for objects or weapons.\n\r");
    
#if 0
    lowObjs(arg.c_str());
    return;
  } else if (is_abbrev(buf, "weapons")) {
    lowWeaps(arg.c_str());
#endif
    return;
  } else if (is_abbrev(buf, "tasks")) {
    lowTasks(arg.c_str());
    return;
  } else if (is_abbrev(buf, "mobs")) {
    lowMobs(arg.c_str());
    return;
  } else if (is_abbrev(buf, "race")) {
    lowRace(arg.c_str());
    return;
  } else if (is_abbrev(buf, "path")){
    lowPath(arg);
  } else if (is_abbrev(buf, "statbonus")) {
    TDatabase db(DB_SNEEZY);
    db.query("select type, count(*) as count, max(mod1) as max, min(mod1) as min, avg(mod1) as avg, sum(mod1) as sum from objaffect group by type");

    sendTo(format("%13s %5s %5s %5s %10s %10s\n\r") % 
	   "Bonus       :" %"Cnt" %"Max" %"Min" %"Avg" %"Sum");

    while(db.fetchRow()){
      buf = format("%5s %5s %5s %10s %10s") % db["count"] % db["max"] %
	db["min"] % db["avg"] % db["sum"];

      switch(mapFileToApply(convertTo<int>(db["type"]))){
	case APPLY_STR:
	  sendTo(format("Strength    : %s\n\r") % buf);
	  break;
	case APPLY_BRA:
	  sendTo(format("Brawn       : %s\n\r") % buf);
	  break;
	case APPLY_CON:
	  sendTo(format("Constitution: %s\n\r") % buf);
	  break;
	case APPLY_DEX:
	  sendTo(format("Dexterity   : %s\n\r") % buf);
	  break;
	case APPLY_AGI:
	  sendTo(format("Agility     : %s\n\r") % buf);
	  break;
	case APPLY_INT:
	  sendTo(format("Intelligence: %s\n\r") % buf);
	  break;
	case APPLY_WIS:
	  sendTo(format("Wisdom      : %s\n\r") % buf);
	  break;
	case APPLY_FOC:
	  sendTo(format("Focus       : %s\n\r") % buf);
	  break;
	case APPLY_PER:
	  sendTo(format("Perception  : %s\n\r") % buf);
	  break;
	case APPLY_CHA:
	  sendTo(format("Charisma    : %s\n\r") % buf);
	  break;
	case APPLY_KAR:
	  sendTo(format("Karma       : %s\n\r") % buf);
	  break;
	case APPLY_SPE:
	  sendTo(format("Speed       : %s\n\r") % buf);
    break;
  case APPLY_CRIT_FREQUENCY:
    sendTo(format("Crit. Chance: %s\n\r") % buf);
	  break;
	default:
	  break;
      }
    }

  } else if (is_abbrev(buf, "statcharts")) {
    Stats st;
    sstring buf;
    char tmpbuf[256];
    race_t races[6]={RACE_HUMAN, RACE_GNOME, RACE_ELVEN, RACE_OGRE,
		     RACE_DWARF, RACE_HOBBIT};
    int i, total=0;

    sendTo(COLOR_BASIC, "       <c>[STR][BRA][CON][DEX][AGI][INT][WIS][FOC][PER][CHA][KAR][SPE][TOTAL]<1>\n\r");


    for(i=0;i<6;++i){
      sprintf(tmpbuf, "%-8s", Races[races[i]]->getProperName().c_str());
      buf += tmpbuf;
      st=Races[races[i]]->getBaseStats();
      for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	sprintf(tmpbuf, "%3d  ", st.get(stat));
	buf += tmpbuf;
	total+=st.get(stat);
      }
      sprintf(tmpbuf, "%5d\n\r", total);
      buf += tmpbuf;
      total=0;
    }
    sendTo(COLOR_BASIC, format("%s\n\r") % buf);

    for(territoryT terr=HOME_TER_NONE;terr<MAX_HOME_TERS;terr++){
      total=0;
      switch(terr){
	case HOME_TER_HUMAN_VILLAGER:
	  sendTo("Villager");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_URBAN:
	  sendTo("Urban   ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_PLAINS:
	  sendTo("Plains  ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_RECLUSE:
	  sendTo("Recluse ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_HILL:
	  sendTo("Hill    ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_MOUNTAIN:
	  sendTo("Mountain");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_FOREST:
	  sendTo("Forest  ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	case HOME_TER_HUMAN_MARINER:
	  sendTo("Marine  ");
	  for(statTypeT stat=MIN_STAT; stat<MAX_STATS_USED; stat++) {
	    sendTo(format("%3d  ") % territory_adjustment(terr, stat));
	    total += territory_adjustment(terr, stat);
	  }
	  sendTo(format("%5d\n\r") % total);
	  break;
	default:
	  break;
      }
    }
    

  } else {
    sendTo(usage);
    return;
  }
}


void TBeing::lowPath(const sstring &arg)
{
  dirTypeT dir;
  int here=in_room;
  int target=convertTo<int>(arg.word(0));
  TPathFinder path;

  //path.setUsePortals(false);

  if(path.findPath(here, findRoom(target)) == DIR_NONE){
    sendTo("No path found, trying again going through no-mob rooms.\n\r");
    path.setNoMob(false);
    if(path.findPath(here, findRoom(target)) == DIR_NONE){
      sendTo("No path found.\n\r");
      return;
    }
  }

  sendTo(format("Distance: %i\n\r") % path.getDist());

  
  for(unsigned int i=0;i<path.path.size();++i){
    dir=path.path[i]->direct;

    if(dir >= MAX_DIR){
      sendTo(format("{%i, %i},\n\r") % dir % path.path[i]->room);
    } else {
      sendTo(format("{DIR_%s, %i},\n\r") % 
	     (dir<0?"NONE":sstring(dirs[dir]).upper()) %
	     path.path[i]->room);
    }
  }
}




void TBeing::lowRace(const char *arg)
{
  TMonster *mob;
  char namebuf2[256], buf2[255];
  int race_num;
  sstring str;
  bool show_stat=true;

  arg = one_argument(arg, buf2, cElements(buf2));
  race_num = convertTo<int>(buf2);
  if ((race_num < 0) || (race_num > MAX_RACIAL_TYPES)) {
    sendTo("Syntax: low race <racial index>\n\r");
    sendTo("See 'show race' for valid indices.\n\r");
    return;
  }
  if (*arg)
    show_stat = false;

  str = "THIS INFORMATION IS CONSIDERED CLASSIFIED AND SECRET.  RELEASING IT TO MORTALS\n\rIS GROUNDS FOR DEMOTION OR DELETION.\n\r";
  sprintf(buf2,"List of mobs of race %s:\n\r",RaceNames[race_num]);
  str += buf2;
  sprintf(buf2,"%-23s: %76s", "<c>Name<1>", curStats.printStatHeader().c_str());
  str += buf2;

  unsigned int mobnum;
  for (mobnum = 0; mobnum < mob_index.size(); mobnum++) {
    if ((mob_index[mobnum].level == -99) || (mob_index[mobnum].spec == -99)) {
      // not inited, load to init
      if ((mob = read_mobile(mobnum, REAL)) != NULL) {
        thing_to_room(mob, Room::VOID);   // prevents extracting from "nowhere"
        delete mob;
        mob = NULL;
      } else 
        vlogf(LOG_BUG,format("BOGUS LOAD of mob %d") % mobnum);
    }
    if (mob_index[mobnum].race != race_num)
      continue;
    if (UtilProcs(mob_index[mobnum].spec))
      continue;
    if (!(mob = read_mobile(mobnum, REAL))) {
      vlogf(LOG_BUG,format("Error during doLowRace in mob rnum %d") % mobnum);
      continue;     
    }
    strcpy(namebuf2, mob->getName());
    sstring namebuf = colorString(this, desc, namebuf2, NULL, COLOR_NONE, FALSE);
    if (show_stat)
      sprintf(buf2, "%-17.17s: %60s", 
         namebuf.c_str(), mob->chosenStats.printRawStats(this).c_str());
    else
      sprintf(buf2, "%5.5d : %-27.27s : L%-3d, C%-3d, Hgt:%-3d\n\r",
         mob->mobVnum(), namebuf.c_str(), mob->GetMaxLevel(), mob->getClass(), mob->getHeight());
    str += buf2;

    thing_to_room(mob,Room::VOID);
    delete mob;
    mob = NULL;
  }

  str += "\n\r";

  desc->page_string(str);
}

void TBeing::lowTasks(const sstring &oarg)
{
  // this works a database of tasks that need doing for the low team
  // low tasks list [<name>]
  // low tasks add <priority> <task>
  // low tasks assign <task ID> <name>
  // low tasks priority <task ID> <priority>
  // low tasks status <task ID> <status>
  // low tasks delete <task ID>
  TDatabase db(DB_SNEEZY);

  sstring buf, buf2, temp, str, arg=oarg;
  int id, priority;

  arg = one_argument(arg, buf);
  if (buf.empty()){
    sendTo("Syntax: low  tasks <list | add | assign | priority | delete> ...\n\r");
    return;
  } else if (is_abbrev(buf, "list")) {
    arg = one_argument(arg, buf);
    if(buf.empty()){
      temp="select id, priority, assigned_to, task, status from lowtasks order by priority";
      vlogf(LOG_DASH, format("lowtask: %s") %  temp);
      db.query(temp.c_str());
    } else {
      temp=format("select id, priority, assigned_to, task, status from lowtasks where assign_to='%s' order by priority") % buf;
      vlogf(LOG_DASH, format("lowtask: %s") %  temp);
      db.query(temp.c_str());

    }

    str =  "ID  | P |  Assigned To  |  Task Description (Status)\n\r";
    str += "---------------------------------------------------------\n\r";

    while(db.fetchRow()){
      id = convertTo<int>(db["id"]);
      priority = convertTo<int>(db["priority"]);
      buf=format("%-4d| %d |  %-13s| %s (%s)\n\r") %
	id % priority % db["assigned_to"] % db["task"] % db["status"];
      str += buf;
    }

    if (desc)
      desc->page_string(str, SHOWNOW_NO, ALLOWREP_YES);


    return;
  } else if (is_abbrev(buf, "add")) {
    arg = one_argument(arg, buf);
    if (buf.empty() || arg.empty()){
      sendTo("Syntax: low tasks add <priority> <task>\n\r");
      return;
    } else {
      priority = convertTo<int>(buf);
      if (priority < 0 || priority > 9) {
	sendTo("Priority must be between 0 (highest) and 9 (lowest).\n\r");
	return;
      } else {
	temp="select id from lowtasks order by id desc";
	vlogf(LOG_DASH, format("lowtask: %s") %  temp);
	db.query(temp.c_str());

	if(!db.fetchRow()) {
	  id = 0;
	} else {
	  id = convertTo<int>(db["id"]) + 1;
	}
        temp=format("insert into lowtasks (id, priority, assigned_to, task, status) values(%d, %d, '%s', '%s', '')") % id % priority % getName() % arg;
        vlogf(LOG_DASH, format("lowtask: %s") %  temp);
        db.query(temp.c_str());

	sendTo("New task inserted into list.\n\r");
	return;
      }
    }
    return;
  } else if (is_abbrev(buf, "assign")) {
    arg = one_argument(arg, buf);
    arg = one_argument(arg, buf2);
    if(buf.empty() || buf2.empty()){
      sendTo("Syntax: low tasks assign <task ID> <name>\n\r");
      return;
    } else {
      id = convertTo<int>(buf);
      temp=format("update lowtasks set assigned_to='%s' where id=%d") %
	buf2 % id;
      vlogf(LOG_DASH, format("lowtask: %s") %  temp);
      db.query(temp.c_str());

      sendTo("Task assignment updated.\n\r");
      return;
    }
    return;
  } else if (is_abbrev(buf, "status")) {
    arg = one_argument(arg, buf);
    arg = one_argument(arg, buf2);
    if(buf.empty() || buf2.empty()){
      sendTo("Syntax: low tasks status <task ID> <status>\n\r");
      return;
    } else {
      id = convertTo<int>(buf);
      temp=format("update lowtasks set status='%s' where id=%d") % buf2 % id;
      vlogf(LOG_DASH, format("lowtask: %s") %  temp);
      db.query(temp.c_str());
      
      sendTo("Task status updated.\n\r");
      return;
    }
    return;
  } else if (is_abbrev(buf, "priority")) {
    arg = one_argument(arg, buf);
    arg = one_argument(arg, buf2);
    if(buf.empty() || buf2.empty()){
      sendTo("Syntax: low tasks priority <task ID> <priority>\n\r");
      return;
    } else {
      id = convertTo<int>(buf);
      priority = convertTo<int>(buf2);
      if (priority < 0 || priority > 9) {
        sendTo("Priority must be between 0 (highest) and 9 (lowest).\n\r");
        return;
      } else {
	temp=format("update lowtasks set priority=%d where id=%d") %priority %id;
	vlogf(LOG_DASH, format("lowtask: %s") %  temp);
	db.query(temp.c_str());

	sendTo("Task priority updated.\n\r");
	return;
      }
    }
    return;
  } else if (is_abbrev(buf, "delete")) {
    arg = one_argument(arg, buf);
    if(buf.empty()){
      sendTo("Syntax: low tasks delete <task ID>\n\r");
      return;
    } else {
      id = convertTo<int>(buf);

      temp=format("delete from lowtasks where id=%d") % id;
      vlogf(LOG_DASH, format("lowtask: %s") %  temp);
      db.query(temp.c_str());

      sendTo("Task deleted.\n\r");
      return;
    }
 
    return;
  }

  return;

}

void TBeing::lowMobs(const char *arg)
{
  TMonster *mob;
  char namebuf2[256],buf2[255];
  int level;
  sstring str;

  level = convertTo<int>(arg);
  if ((level <= 0) || (level > 127)) {
    sendTo("Syntax: low mobs <level>\n\r");
    sendTo("Level must be between 1 and 127\n\r");
    return;
  }
  str = "THIS INFORMATION IS CONSIDERED CLASSIFIED AND SECRET.  RELEASING IT TO MORTALS\n\rIS GROUNDS FOR DEMOTION OR DELETION.\n\r";
  sprintf(buf2,"List of level %d mobs:\n\r",level);
  str += buf2;
  sprintf(buf2,"%5s %-27s : %6s %5s %5s %5s %5s %5s %5s\n\r\n\r",
      "vnum","name","class", "hp", "AC", "tohit", "#atts", "damage", "factn");
  str += buf2;

  unsigned int mobnum;
  for (mobnum = 0; mobnum < mob_index.size();mobnum++) {
    if ((mob_index[mobnum].level == -99) || (mob_index[mobnum].spec == -99))
      // not inited, load to init
      if ((mob = read_mobile(mobnum, REAL)) != NULL) {
        thing_to_room(mob, Room::VOID);      // prevents extracting from "nowhere"
        delete mob;
        mob = NULL;
      } else 
        vlogf(LOG_BUG,format("BOGUS LOAD of mob %d") % mobnum);
    if (mob_index[mobnum].level != level)
      continue;
    if (UtilProcs(mob_index[mobnum].spec))
      continue;
    if (!(mob = read_mobile(mobnum, REAL))) {
      vlogf(LOG_BUG,format("Error during doLowMobs in mob rnum %d") % mobnum);
      continue;     
    }

    strcpy(namebuf2, mob->getName());
    sstring namebuf = colorString(this, desc, namebuf2, NULL, COLOR_NONE, FALSE);

    sprintf(buf2, "%5d %-27.27s : %6d %5.2f %5d  %2d    %.1f %5.2f+%d%% %2d\n\r", 
       mob->mobVnum(), namebuf.c_str(), mob->getClass(), mob->getHPLevel(),
       mob->getArmor(), mob->getHitroll(), mob->getMult(),
       mob->getDamLevel(), mob->getDamPrecision(),
       mob->getFaction());

    str += buf2;
    thing_to_room(mob,Room::VOID);
    delete mob;
    mob = NULL;
  }
  str += "\n\r";
  desc->page_string(str);
}

class lowObjSort {
public:
  bool operator() (const int &, const int &) const;
};

bool lowObjSort::operator() (const int &x, const int &y) const
{
  // sort in descending order of armor
  // if armor is same, sort in descending struct order

  if (x < 0 || x >= (int) obj_index.size()) {
    vlogf(LOG_BUG, format("Bogus value for x: %d in lowObjSort") %  x);
    return false;
  }
  if (y < 0 || y >= (int) obj_index.size()) {
    vlogf(LOG_BUG, format("Bogus value for y: %d in lowObjSort") %  y);
    return false;
  }

  if (obj_index[x].armor <
       obj_index[y].armor) {
    return false;
  } else if (obj_index[x].armor >
             obj_index[y].armor) {
    return true;
  } else {
    // sort based on structure
    if (obj_index[x].max_struct <
        obj_index[y].max_struct) {
      return false;
    } else {
      return true;
    }
  }
}

class lowObjSortValue {
public:
  bool operator() (const int &, const int &) const;
};

bool lowObjSortValue::operator() (const int &x, const int &y) const
{
  // sort in descending order of value
  return (obj_index[x].value >= obj_index[y].value);
}

void TBeing::lowObjs(const char *arg)
{
  TObj *obj;
  char buf2[255];
  int i;
  wearSlotT slot = WEAR_HEAD;
  unsigned int worn;
  bool val_sort = FALSE;
  int sort_race=0;
  int sort_race_low = 0;
  int sort_race_high = 0;
  double tmp;
  sstring str;
  std::vector<int>objList(0);

  arg = one_argument(arg, buf2, cElements(buf2));
  if (!*buf2) {
    sendTo("Syntax: low obj <body slot>\n\r");
    return;
  } else if (is_abbrev(buf2, "head")) {
    worn = ITEM_WEAR_HEAD;
    slot = WEAR_HEAD;
  } else if (is_abbrev(buf2, "neck")) {
    worn = ITEM_WEAR_NECK;
    slot = WEAR_NECK;
  } else if (is_abbrev(buf2, "fingers")) {
    worn = ITEM_WEAR_FINGERS;
    slot = WEAR_FINGER_R;
  } else if (is_abbrev(buf2, "body")) {
    worn = ITEM_WEAR_BODY;
    slot = WEAR_BODY;
  } else if (is_abbrev(buf2, "arms")) {
    worn = ITEM_WEAR_ARMS;
    slot = WEAR_ARM_R;
  } else if (is_abbrev(buf2, "wrists")) {
    worn = ITEM_WEAR_WRISTS;
    slot = WEAR_WRIST_R;
  } else if (is_abbrev(buf2, "hands")) {
    worn = ITEM_WEAR_HANDS;
    slot = WEAR_HAND_R;
  } else if (is_abbrev(buf2, "hold") || is_abbrev(buf2, "held")) {
    worn = ITEM_HOLD;
    slot = HOLD_RIGHT;
  } else if (is_abbrev(buf2, "waist")) {
    worn = ITEM_WEAR_WAIST;
    slot = WEAR_WAIST;
  } else if (is_abbrev(buf2, "back")) {
    worn = ITEM_WEAR_BACK;
    slot = WEAR_BACK;
  } else if (is_abbrev(buf2, "legs")) {
    worn = ITEM_WEAR_LEGS;
    slot = WEAR_LEG_R;
  } else if (is_abbrev(buf2, "foot") || is_abbrev(buf2, "feet")) {
    worn = ITEM_WEAR_FEET;
    slot = WEAR_FOOT_R;
  } else {
    sendTo("bogus body slot.\n\r");
    return;
  }
  arg=one_argument(arg, buf2, cElements(buf2));
  if (is_abbrev(buf2, "value"))
    val_sort = TRUE;
  if (is_abbrev(buf2, "race")){
    one_argument(arg, buf2, cElements(buf2));
    sort_race=convertTo<int>(buf2);

    tmp=Races[sort_race]->getBaseMaleHeight()+
	Races[sort_race]->getMaleHtNumDice();
    tmp*=0.85;
    tmp*=race_vol_constants[mapSlotToFile(slot)];
    sort_race_low=(int) tmp;

    tmp=Races[sort_race]->getBaseMaleHeight()+
	   (Races[sort_race]->getMaleHtNumDice()*
	    Races[sort_race]->getMaleHtDieSize());
    tmp*=1.15;
    tmp*=race_vol_constants[mapSlotToFile(slot)];
    sort_race_high=(int) tmp;
  }

// this loop scrolls thru all objs in dbase and creates
// objbug, an unsorted list of obj real nums meeting criteria
  unsigned int objnum = 0;
  for (objnum = 0;objnum < obj_index.size();objnum++) {
    if (obj_index[objnum].armor == -97)    // a weapon
      continue;
    if (obj_index[objnum].armor == -98)    // not a piece of armor
      continue;

    if(sort_race){
      if (!(obj = read_object(objnum, REAL))) {
	vlogf(LOG_BUG,format("Error in doLow::lowObjs.  real obj %d") %  objnum);
	continue;
      }
      
      if(obj->getVolume() <= sort_race_low ||
	 obj->getVolume() >= sort_race_high){
	delete obj;
	continue;
      }
      delete obj;
    }

    if (IS_SET(obj_index[objnum].where_worn, worn)) {
      // item is something we are looking for
      objList.push_back(objnum);
    }
  }
  
  // now lets do some sorting
  if (val_sort)
    std::sort(objList.begin(), objList.end(), lowObjSortValue());
  else
    std::sort(objList.begin(), objList.end(), lowObjSort());

// sortbuf now holds a sorted list of objects
// send that to user
  str = "THIS INFORMATION IS CONSIDERED CLASSIFIED AND SECRET.  RELEASING IT TO MORTALS\n\rIS GROUNDS FOR DEMOTION OR DELETION.\n\r";

  sprintf(buf2, "%5s %30s : %3s %5s %4s %6s %4s %5s\n\r","vnum","obj name", 
          "str", "armor", "wght", "price", "sug.pr" "size", "max #");
  str += buf2;
  for (objnum = 0; objnum < objList.size(); objnum++) {
    if (!(obj = read_object(objList[objnum], REAL))) {
      vlogf(LOG_BUG,format("Error in doLow::lowObjs.  real obj %d") %  objList[objnum]);
      return;
    }
    TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
    if (!tbc) {
      vlogf(LOG_BUG,format("Error in doLow::lowObjs.  Non-clothing (%d)") %  objList[objnum]);
      delete obj;
    }
    int size_per = 100;
    // the bogus slots are set to 0
    if (slot && race_vol_constants[mapSlotToFile( slot)]){
      size_per = (int)(100.0 * tbc->getVolume() / race_vol_constants[mapSlotToFile(slot)]);
      if (tbc->isPaired())
        size_per /= 2;
    }
    sprintf(buf2, "%s%5d %-30.30s : %3d %2dAC %4.1f %6d %6d %3d%% %4d%s\n\r", 
      green(), tbc->objVnum(),
      tbc->getNameNOC(this).c_str(),
      tbc->obj_flags.max_struct_points, 
      obj_index[tbc->getItemIndex()].armor, tbc->getWeight(),
      tbc->obj_flags.cost, tbc->suggestedPrice(), size_per, 
      obj_index[tbc->getItemIndex()].max_exist, norm());
    str += buf2;

    if (tbc->isPaired()) {
      sprintf(buf2, "   %sPAIRED%s",blue(),norm());
      str += buf2;
    }
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if (tbc->affected[i].location == APPLY_ARMOR)
        continue;
      if (tbc->affected[i].location == APPLY_NONE)
        continue;
      if (tbc->affected[i].location == APPLY_SPELL) {
        sprintf(buf2,"   %s%s by %ld%s", red(),
        discArray[tbc->affected[i].modifier]->name, tbc->affected[i].modifier2, norm());
      } else if (tbc->affected[i].location == APPLY_DISCIPLINE) {
        sprintf(buf2,"   %s%s: %s by %ld%s",red(),apply_types[tbc->affected[i].location].name, discNames[tbc->affected[i].modifier].name, tbc->affected[i].modifier2,
norm());
      } else if (tbc->affected[i].location == APPLY_IMMUNITY) {
        sprintf(buf2,"   %s%s: %s by %ld%s",red(),apply_types[tbc->affected[i].location].name,
          immunity_names[tbc->affected[i].modifier], tbc->affected[i].modifier2,norm());
      } else {
        if ((tbc->affected[i].location == APPLY_SPELL) ||
            (tbc->affected[i].location == APPLY_DISCIPLINE) ||
            (tbc->affected[i].location == APPLY_IMMUNITY) ||
            (tbc->affected[i].location == APPLY_SPELL_EFFECT))
          str += red();
        sprintf(buf2,"   %s by %ld",apply_types[tbc->affected[i].location].name,
          tbc->affected[i].modifier);
        if ((tbc->affected[i].location == APPLY_SPELL) ||
            (tbc->affected[i].location == APPLY_DISCIPLINE) ||
            (tbc->affected[i].location == APPLY_IMMUNITY) ||
            (tbc->affected[i].location == APPLY_SPELL_EFFECT))
          strcat(buf2, norm());
      }
      str += buf2;
    }
    if (!tbc->isObjStat(ITEM_ANTI_MAGE))
      str += "  MU";
    if (!tbc->isObjStat(ITEM_ANTI_CLERIC))
      str += "  CL";
    if (!tbc->isObjStat(ITEM_ANTI_MONK) && dynamic_cast<TWorn *>(tbc))
      str += "  MK";

    str += "\n\r";
    delete tbc;
    tbc = NULL;
  }
  desc->page_string(str);
}

class lowWeapSort {
public:
  bool operator() (const int &, const int &) const;
};

bool lowWeapSort::operator() (const int &x, const int &y) const
{
  // sort in descending order of damage
  // if armor is same, sort in descending struct order

  TObj *obj;
  TBaseWeapon *x_obj = NULL,
              *y_obj = NULL;

  if ((obj = read_object(x, REAL)))
    x_obj = dynamic_cast<TBaseWeapon *>(obj);

  if ((obj = read_object(y, REAL)))
    y_obj = dynamic_cast<TBaseWeapon *>(obj);

  if (!x_obj || !y_obj) {
    vlogf(LOG_BUG, format("Error in lowWeapSort(): x: %s, y: %s, x=%d, y=%d") % 
        (x_obj ? "true" : "false") %
        (y_obj ? "true" : "false") %
        x % y);
    delete x_obj;
    delete y_obj;
    return false;
  }

  double x_dam = x_obj->baseDamage();
  x_dam += x_obj->itemDamroll();
  double y_dam = y_obj->baseDamage();
  y_dam += y_obj->itemDamroll();

  delete x_obj;
  delete y_obj;

  return x_dam >= y_dam;
}

void TBeing::lowWeaps(const char *arg)
{
  char buf2[255];
  bool blunt = FALSE, pierce = FALSE, slash = FALSE, arrow = false;
  int i;
  std::vector<int>objList(0);
  bool sort_val = false;
  sstring str;

  one_argument(arg, buf2, cElements(buf2));
  if (!*buf2) {
    sendTo("Syntax: low weapons <slash | blunt | pierce | arrow>\n\r");
    return;
  } else if (is_abbrev(buf2,"pierce")) {
    pierce = TRUE;
  } else if (is_abbrev(buf2,"slash")) {
    slash = TRUE;
  } else if (is_abbrev(buf2,"blunt")) {
    blunt = TRUE;
  } else if (is_abbrev(buf2,"arrow")) {
    arrow = TRUE;
  } else {
    sendTo("Syntax: low weapons <slash | blunt | pierce | arrow>\n\r");
    return;
  }
  one_argument(arg, buf2, cElements(buf2));
  if (is_abbrev(buf2, "value"))
    sort_val = TRUE;

  // this scrolls thru all objs in dbase and creates
  // objbug, an unsorted list of objs rnums that are weapons of appropriate type
  unsigned int objnum;
  objList.clear();
  for (objnum = 0;objnum < obj_index.size();objnum++) {
    if (obj_index[objnum].armor != -97)    // not a weapon
      continue;
    TObj * obj = read_object(objnum, REAL);
    if (!obj) {
      vlogf(LOG_BUG,"Error in lowObjs : lowWeps");
      return;
    }

    TBaseWeapon * tbw = dynamic_cast<TBaseWeapon *>(obj);
    TArrow      * arr = dynamic_cast<TArrow      *>(tbw);

    if (!tbw || (arrow && !arr)) {
      delete obj;
      obj = NULL;
      continue;
    }

    if (arrow ||
        (blunt  && tbw->isBluntWeapon()) ||
        (pierce && tbw->isPierceWeapon()) ||
        (slash  && tbw->isSlashWeapon())) {
      vlogf(LOG_SILENT, format("Pushing Back Object: %d:%d") %  objnum % obj->objVnum());
      objList.push_back(objnum);
    }

    delete tbw;
    tbw = NULL;
  }

  // now lets do some sorting
  if (!sort_val)
    std::sort(objList.begin(), objList.end(), lowWeapSort());
  else
    std::sort(objList.begin(), objList.end(), lowObjSortValue());

  // objList now holds a sorted list of objects
  // send that to user
  str = "THIS INFORMATION IS CONSIDERED CLASSIFIED AND SECRET.  RELEASING IT TO MORTALS\n\rIS GROUNDS FOR DEMOTION OR DELETION.\n\r";
  sprintf(buf2, "%5s %20s %4s: %4s %4s %5s %6s %5s %5s %5s %5s\n\r","vnum","obj name", 
          "(#H)", "wgt", "vol", "level", "struct", "qual", "price", "recmd", "max #");
  str += buf2;
  for (objnum = 0; objnum < objList.size(); objnum++) {
    TBaseWeapon * weap = dynamic_cast<TBaseWeapon *>(read_object(objList[objnum], REAL));
    if (!(weap)) {
      vlogf(LOG_BUG,format("Error in doLow::lowWeaps.  real obj %d") % objList[objnum]);
      return;
    }
    sprintf(buf2, "%s%5d %-20.20s %4s: %4.1f %4d %5.2f %6d %5d %6d %6d %4d%s\n\r", 
      green(), weap->objVnum(),
      weap->getNameNOC(this).c_str(),
      (weap->isPaired()) ? "(2H)" : "    ",
      weap->getWeight(), weap->getVolume(),
      weap->damageLevel(), 
      weap->getMaxStructPoints(),
      weap->getCurSharp(),
      weap->obj_flags.cost, weap->suggestedPrice(),
      obj_index[weap->getItemIndex()].max_exist, norm());

    str += buf2;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if (weap->affected[i].location == APPLY_NONE)
        continue;
      if (weap->affected[i].location == APPLY_SPELL) {
        sprintf(buf2,"   %s%s by %ld%s", red(),
          discArray[weap->affected[i].modifier]->name, weap->affected[i].modifier2, norm());
      } else if (weap->affected[i].location == APPLY_DISCIPLINE) {
        sprintf(buf2,"   %s%s: %s by %ld%s",red(),apply_types[weap->affected[i].location].name, discNames[weap->affected[i].modifier].name, weap->affected[i].modifier2, norm());
      } else if (weap->affected[i].location == APPLY_IMMUNITY) {
        sprintf(buf2,"   %s%s: %s by %ld%s",red(),apply_types[weap->affected[i].location].name,
          immunity_names[weap->affected[i].modifier], weap->affected[i].modifier2, norm());
      } else {
       // doesnt seem to do anything cept spell effect but never know
        if ((weap->affected[i].location == APPLY_SPELL) ||
            (weap->affected[i].location == APPLY_DISCIPLINE) ||
            (weap->affected[i].location == APPLY_IMMUNITY) ||
            (weap->affected[i].location == APPLY_SPELL_EFFECT))
          str += red();
        sprintf(buf2,"   %s by %ld",apply_types[weap->affected[i].location].name,
          weap->affected[i].modifier);
        if ((weap->affected[i].location == APPLY_SPELL) ||
            (weap->affected[i].location == APPLY_DISCIPLINE) ||
            (weap->affected[i].location == APPLY_IMMUNITY) ||
            (weap->affected[i].location == APPLY_SPELL_EFFECT))
          strcat(buf2, norm());
      }
      str += buf2;
    }
    str += "\n\r";
    delete weap;
    weap = NULL;
  }
  desc->page_string(str);
}

int getSpellCasttime(spellNumT spell)
{
  int durat = discArray[spell]->lag;
  // tasked spells take an extra round, so account for this
  if (discArray[spell]->comp_types & SPELL_TASKED)
    durat++;
  return durat;
}

// returns the cost in talens for a spell cast at level and learning
// for the most part, semi-arbitrary
// it presumes we are calling this for objects
int getSpellCost(spellNumT spell, int level, int learning)
{
  if (spell < MIN_SPELL || spell >= MAX_SKILL || !discArray[spell])
    return 0;

  // first, correct the level and put in proper range
  // since it's an object, treat adv_learn=0
  int min_lev, max_lev;
  getSkillLevelRange(spell, min_lev, max_lev, 0);
  level = min(max(level, min_lev), max_lev); 

  // calculate how long it takes to cast normally
  int durat = getSpellCasttime(spell);

  // C.f. the balancing notes for why this is the way it is
  int value = (int) (level * level * 1.5 * durat / 50);
  value = max(value, 10);

  // discount defective items
  value *= learning;
  value /= 100;

  if (discArray[spell]->targets & TAR_VIOLENT)
    value *= 2;

  return value;
}
