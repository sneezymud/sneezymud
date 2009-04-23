
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "obj_LOW.cc" - routines related to checking stats on objects.
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>

#include "statistics.h"
#include "combat.h"
#include "obj_base_weapon.h"
#include "obj_base_clothing.h"
#include "obj_worn.h"
#include "obj_jewelry.h"
#include "obj_low.h"

/*-------------------------------------------------------------------
  New LOW classification rules for gear

  Here is how it works:
    Gear is set to a 'tier' based on its type and flags
    Everything (stats, armor) is given a 'point' value
    Some points are weighted based on tier of the object (to skew a stat towards a class)
    
    We add in three categories for armor: struct, stats, main (armor for eq, damage for weapons)
    All of these point totals are calculated.  Then added.
    (min point total for any one is 1/2 the max)

    Add them all up, then determine the 'level' of the item based on a non-linear curve
 ------------------------------------------------------------------*/

ObjectEvaluator::ObjectEvaluator(const TObj *o)
{
  m_obj = o;
  m_stat =  -1;
  m_gotStats = false;
}

sstring ObjectEvaluator::getTierString()
{
  static const char * tiers[Tier_Max+1] = { "Clothing", "Light Armor", "Medium Armor", "Heavy Armor", "Jewelry",
    "Common Weapon", "Simple Weapon", "Martial Weapon", "Unknown" };
  return tiers[getTier()];
}

// ensures that main and stat points are within tolerances of each other
// you can have 0 stats, but your 'main' has to be within tolerances, else
// some gear will possibly become just empty 'stat containers'.
// not sure that statement made a ton of sense, but oh well
int ObjectEvaluator::getPointValue(PointType pt)
{
  int rawStat = getStatPointsRaw();
  int rawMain = getMainPointsRaw();
  // we don't allow negative main points ever for valuation
  int realMain = rawMain < 0 ? 0 : rawMain;

  // make sure the main points are at least 1/2 the stat points
  // so you can't have swords with +100 mana and no dmg, or helmets with +100 str and no armor
  if (rawStat > 0 && realMain < (rawStat/2))
    realMain = rawStat/2;

  // we don't allow negative stats to powerup the main points
  if (rawStat < 0)
    vlogf(LOG_LOW, format("ObjectEvaluator found negative stat points on %s (vnum %i)") % m_obj->getName() % m_obj->objVnum());

  if (PointType_Stats == pt)
    return rawStat;
  if (PointType_Main == pt)
    return realMain;
  if (PointType_All == pt)
    return rawStat + realMain;
  return 0;
}

/*
  UNUSED for now, we will put struct into its own valuation or something
int ObjectEvaluator::getStructPointsRaw()
{
  if (m_struct >= 0)
    return m_struct;

  static float structPointsPerTier[Tier_Max] = {
          6.0, // clothing
          5.0, // light
          4.0, // medium
          3.0, // heavy
          7.0, // jewelry
          4.0, // weapon common
          3.5, // weapon simple
          3.0, // weapon martial
        };
  static float slotValues[MAX_ITEM_WEARS] = {
          0.00, // ITEM_TAKE
          0.01, // ITEM_WEAR_FINGERS
          0.04, // ITEM_WEAR_NECK
          0.26, // ITEM_WEAR_BODY
          0.07, // ITEM_WEAR_HEAD
          0.03, // ITEM_WEAR_LEGS
          0.02, // ITEM_WEAR_FEET
          0.03, // ITEM_WEAR_HANDS
          0.05, // ITEM_WEAR_ARMS
          0.00, // ITEM_WEAR_UNUSED1
          0.10, // ITEM_WEAR_BACK
          0.05, // ITEM_WEAR_WAIST
          0.03, // ITEM_WEAR_WRISTS
          0.00, // ITEM_WEAR_UNUSED2
          0.07, // ITEM_HOLD
          0.00, // ITEM_THROW
          };

  int rawStruct = m_obj->getMaxStructPoints();
  float structMultiplier = structPointsPerTier[getTier()];
  float slotWeight = 1.0;

  for(unsigned int iSlot = 0; iSlot < MAX_ITEM_WEARS; iSlot++)
    if (m_obj->canWear((1 << iSlot)) && slotWeight < slotValues[iSlot])
      slotWeight = slotValues[iSlot];

  if (m_obj->getObjStat() & ITEM_PAIRED)
    slotWeight *= 2.0;

  structMultiplier /= slotWeight;

  return m_struct = int(float(rawStruct) * structMultiplier);
}*/

int ObjectEvaluator::getStatPointsRaw()
{
  if (m_gotStats)
    return m_stat;

  // used to be low_acPerLevel/(low_exchangeRate*.55)+bonus but we want to skew to allow clothing to have more stats
  static float pointsPerStat[Tier_Max] = {
    ((low_acPerLevel*(low_exchangeRate*0.55)) + 1.0)      /10.0, // clothing
    ((low_acPerLevel*(low_exchangeRate*0.65)) + (2.0/3.0))/10.0, // light
    ((low_acPerLevel*(low_exchangeRate*0.75)) + (1.0/3.0))/10.0, // medium
    ((low_acPerLevel*(low_exchangeRate*0.85)) + 0)        /10.0, // heavy
    ((low_acPerLevel*(low_exchangeRate*0.75)) + (1.0/3.0))/10.0, // jewelry
    ((low_acPerLevel*(low_exchangeRate*0.75)) + (1.0/3.0))/10.0, // weapon common
    ((low_acPerLevel*(low_exchangeRate*0.75)) + (1.0/3.0))/10.0, // weapon simple
    ((low_acPerLevel*(low_exchangeRate*0.75)) + (1.0/3.0))/10.0, // weapon martial
    };
  static float slotValues[MAX_ITEM_WEARS-1] = {
      0.00, // ITEM_TAKE
      0.05, // ITEM_WEAR_FINGERS
      0.05, // ITEM_WEAR_NECK
      0.08, // ITEM_WEAR_BODY
      0.06, // ITEM_WEAR_HEAD
      0.05, // ITEM_WEAR_LEGS
      0.05, // ITEM_WEAR_FEET
      0.05, // ITEM_WEAR_HANDS
      0.05, // ITEM_WEAR_ARMS
      0.00, // ITEM_WEAR_UNUSED1
      0.06, // ITEM_WEAR_BACK
      0.06, // ITEM_WEAR_WAIST
      0.05, // ITEM_WEAR_WRISTS
      0.00, // ITEM_WEAR_UNUSED2
      0.09, // ITEM_HOLD
      //0.00, // ITEM_THROW
      };
  static float affectsCost[AFF_MAX] = {
      0.0, //AFF_BLIND
      0.0, //AFF_INVISIBLE
      0.0, //AFF_SWIM
      6.0, //AFF_DETECT_INVISIBLE
      3.0, //AFF_DETECT_MAGIC
      5.0, //AFF_SENSE_LIFE
      6.0, //AFF_LEVITATING
      0.0, //AFF_SANCTUARY
      0.0, //AFF_GROUP
      0.0, //AFF_WEB
      0.0, //AFF_CURSE
      14.0, //AFF_FLYING
      0.0, //AFF_POISON
      0.0, //AFF_STUNNED
      0.0, //AFF_PARALYSIS
      3.0, //AFF_INFRAVISION
      10.0, //AFF_WATERBREATH
      0.0, //AFF_SLEEP
      0.0, //AFF_SCRYING
      14.0, //AFF_SNEAK
      0.0, //AFF_HIDE
      0.0, //AFF_SHOCKED
      0.0, //AFF_CHARM
      0.0, //AFF_SYPHILIS
      0.0, //AFF_SHADOW_WALK
      20.0, //AFF_TRUE_SIGHT
      0.0, //AFF_MUNCHING_CORPSE
      0.0, //AFF_RIPOSTE
      0.0, //AFF_SILENT
      0.0, //AFF_ENGAGER
      0.0, //AFF_AGGRESSOR
      20.0, //AFF_CLARITY
      0.0, //AFF_FLIGHTWORTHY
      };
  static float immunityCost[MAX_IMMUNES] = {
      0.5,  //IMMUNE_HEAT
      0.5,  //IMMUNE_COLD
      0.5,  //IMMUNE_ACID
      0.5,  //IMMUNE_POISON
      0.5,  //IMMUNE_SLEEP
      0.5,  //IMMUNE_PARALYSIS
      0.5,  //IMMUNE_CHARM
      2.0,  //IMMUNE_PIERCE
      2.0,  //IMMUNE_SLASH
      3.0,  //IMMUNE_BLUNT
      4.0,  //IMMUNE_NONMAGIC
      0.5,  //IMMUNE_PLUS1
      1.0,  //IMMUNE_PLUS2
      1.5,  //IMMUNE_PLUS3
      0.5,  //IMMUNE_AIR
      0.5,  //IMMUNE_ENERGY
      0.5,  //IMMUNE_ELECTRICITY
      0.5,  //IMMUNE_DISEASE
      0.5,  //IMMUNE_SUFFOCATION
      0.5,  //IMMUNE_SKIN_COND
      0.5,  //IMMUNE_BONE_COND
      0.5,  //IMMUNE_BLEED
      0.5,  //IMMUNE_WATER
      0.5,  //IMMUNE_DRAIN
      0.5,  //IMMUNE_FEAR
      0.5,  //IMMUNE_EARTH
      0.5,  //IMMUNE_SUMMON
      0.5,  //IMMUNE_UNUSED2
      };
  static float flagCost[MAX_OBJ_STAT] = {
      0.5, // ITEM_GLOW
      0.0, // ITEM_HUM
      0.0, // ITEM_STRUNG
      2.0, // ITEM_SHADOWY
      0.0, // ITEM_PROTOTYPE
      0.0, // ITEM_INVISIBLE 
      0.0, // ITEM_MAGIC
      0.0, // ITEM_NODROP
      0.0, // ITEM_BLESS
      0.0, // ITEM_SPIKED
      0.0, // ITEM_HOVER
      0.0, // ITEM_RUSTY
      0.0, // ITEM_ANTI_CLERIC
      0.0, // ITEM_ANTI_MAGE
      0.0, // ITEM_ANTI_THIEF
      0.0, // ITEM_ANTI_WARRIOR
      0.0, // ITEM_ANTI_SHAMAN
      0.0, // ITEM_ANTI_DEIKHAN
      0.0, // ITEM_ANTI_RANGER
      0.0, // ITEM_ANTI_MONK
      0.0, // ITEM_PAIRED
      0.0, // ITEM_NORENT
      0.0, // ITEM_FLOAT
      0.0, // ITEM_NOPURGE 
      0.0, // ITEM_NEWBIE
      0.0, // ITEM_NOJUNK_PLAYER
      0.0, // ITEM_NOT_USED2
      0.0, // ITEM_NOT_USED3
      0.0, // ITEM_ATTACHED
      0.0, // ITEM_BURNING
      0.0, // ITEM_CHARRED
      0.0, // ITEM_NOLOCATE
      };
  static float applyCost[MAX_APPLY_TYPES] = {
      0.0, //APPLY_NONE
      1.0, //APPLY_STR
      1.0, //APPLY_INT
      1.0, //APPLY_WIS
      1.0, //APPLY_DEX
      1.0, //APPLY_CON
      1.0, //APPLY_KAR
      1.0, //APPLY_SEX
      0.0, //APPLY_AGE
      5.0, //APPLY_CHAR_HEIGHT
      0.0, //APPLY_CHAR_WEIGHT
      0.35, //APPLY_ARMOR
      1.0, //APPLY_HIT
      2.0/3.0, //APPLY_MANA
      0.5, //APPLY_MOVE
      3.0, //APPLY_HITROLL
      3.0, //APPLY_DAMROLL
      3.0, //APPLY_HITNDAM
      0.0, //APPLY_IMMUNITY
      2.0, //APPLY_SPELL
      0.0, //APPLY_SPELL_EFFECT
      0.5, //APPLY_LIGHT
      -0.5, //APPLY_NOISE
      2.0, //APPLY_CAN_BE_SEEN
      1.0, //APPLY_VISION
      0.0, //APPLY_PROTECTION
      1.0, //APPLY_BRA
      1.0, //APPLY_AGI
      1.0, //APPLY_FOC
      1.0, //APPLY_SPE
      1.0, //APPLY_PER
      1.0, //APPLY_CHA
      5.0, //APPLY_DISCIPLINE
      1.0, //APPLY_SPELL_HITROLL
      1.0, //APPLY_CURRENT_HIT
      4.0, //APPLY_CRIT_FREQUENCY
      0.0, //APPLY_GARBLE
      };

  double positiveStats = 0;
  double negativeStats = 0;
  double slotWeight = 1.0;

  // add up all the weighted stat points
  for (int iAff = 0; iAff < MAX_OBJ_AFFECT; iAff++)
  {
    if (IgnoreApply(m_obj->affected[iAff].location))
      continue;

    float bonus = 0;
    if (m_obj->affected[iAff].location == APPLY_IMMUNITY)
    {
      bonus = m_obj->affected[iAff].modifier2;
      bonus *= immunityCost[m_obj->affected[iAff].modifier];
    }
    else if (m_obj->affected[iAff].location == APPLY_SPELL || m_obj->affected[iAff].location == APPLY_DISCIPLINE)
    {
      bonus = m_obj->affected[iAff].modifier2;
      bonus *= applyCost[m_obj->affected[iAff].modifier];
    }
    else if (m_obj->affected[iAff].location == APPLY_SPELL_EFFECT)
    {
      for(int iSpell = 0; iSpell < AFF_MAX; iSpell++)
        if (m_obj->affected[iAff].modifier & (1 << iSpell))
          bonus += affectsCost[iSpell];
    }
    else
    {
      bonus = m_obj->affected[iAff].modifier;
      bonus *= applyCost[m_obj->affected[iAff].location];
    }

    if (bonus >= 0)
      positiveStats += bonus;
    else
      negativeStats += -bonus;
  }

  // add in object flags
  for(int iFlag = 0; iFlag < MAX_OBJ_STAT; iFlag++)
    if (m_obj->getObjStat() & (1 << iFlag))
      if (flagCost[iFlag] >= 0)
        positiveStats += flagCost[iFlag];
      else
        negativeStats += -flagCost[iFlag];

  // don't allow negative points to be > 1/2 positive
  negativeStats = min(negativeStats, positiveStats/2.0);

  // combine negative and positive
  positiveStats -= negativeStats;  // compensate negatives at 1/2 the going rate

  // before we adjust for scale, round off to the nearest 1000th
  positiveStats = 0.001 * round(positiveStats * 1000.0);

  // adjust for tier
  positiveStats *= pointsPerStat[getTier()];

  // calculate the stat weight for this object (skip ITEM_TAKE)
  for(unsigned int iSlot = 1; iSlot < cElements(slotValues); iSlot++)
    if (m_obj->canWear((1 << iSlot)))
      slotWeight = slotValues[iSlot];
  positiveStats /= slotWeight;

  // adjust for pairedness
  if (m_obj->getObjStat() & ITEM_PAIRED)
    positiveStats /= 2.0;

  m_gotStats = true;
  return m_stat = int(positiveStats);
}

// a total rewrite of Metro's point scaler.  Sorry.
// note: at this point all point are adjusted to be uniform regardless of tier/slot/etc
double ObjectEvaluator::getLoadLevel(PointType type)
{
  int points = getPointValue(type);
  const int free_points = 140; // the first N points don't count (establish newbie base)
  const double points_per_level = 7.0; // how many points each level of eq gets
  const double level_of_diminishing_returns = 35.0; // after some levels we get diminishing returns
  const int points_of_diminishing_returns = int(points_per_level * level_of_diminishing_returns) + free_points;
  const double scale_base = (1.0 - (3.0 / level_of_diminishing_returns));

  // below newbie levels
  if (points <= free_points)
    return 0.0;

  // for these levels, its a linear function
  if (points <= points_of_diminishing_returns)
    return (points-free_points) / points_per_level;

  // calc the curve - if we were cool we'd properly handle points for partial levels
  double level = level_of_diminishing_returns + 1.0;

  for(double point_total = points - points_of_diminishing_returns;point_total > 0 && level < 127.0;level += 1.0)
  {
    double scale = pow(scale_base, ((level-level_of_diminishing_returns)/level_of_diminishing_returns));
    point_total -= points_per_level * scale;
  }

  return level;
}

ArmorEvaluator::ArmorEvaluator(const TBaseClothing *o) : ObjectEvaluator(o)
{
  m_gotMain = false;
  m_clothing = o;
  m_main = -1;
}

Tier ArmorEvaluator::getTier()
{
  // mages and shaman are clothing
  static unsigned int lightFlags = ITEM_ANTI_MAGE | ITEM_ANTI_SHAMAN; // thieves and monks are light
  static unsigned int mediumFlags = ITEM_ANTI_MONK | ITEM_ANTI_THIEF | lightFlags; // clerics and rangers are med
  static unsigned int heavyFlags = ITEM_ANTI_CLERIC | ITEM_ANTI_RANGER | mediumFlags; // diekhan and warriors are heavy
  unsigned int objStat = m_clothing->getObjStat() & heavyFlags;

  // some classes impose even more restrictions than simple obj flags
  if (m_clothing->monkRestrictedItem(NULL))
    objStat |= ITEM_ANTI_MONK;
  if (m_clothing->shamanRestrictedItem(NULL))
    objStat |= ITEM_ANTI_SHAMAN;
  if (m_clothing->rangerRestrictedItem(NULL))
    objStat |= ITEM_ANTI_RANGER;

  if (NULL != dynamic_cast<const TJewelry *>(m_clothing))
    return Tier_Jewelry;
  else if ((objStat & heavyFlags) == heavyFlags)
    return  Tier_Heavy;
  else if ((objStat & mediumFlags) == mediumFlags)
    return  Tier_Medium;
  else if ((objStat & lightFlags) == lightFlags)
    return  Tier_Light;
  return Tier_Clothing;
}

int ArmorEvaluator::getMainPointsRaw()
{
  if (m_gotMain)
    return m_main;

  // slot values for weighting armor class by place where its worn
  static float slotValues[MAX_ITEM_WEARS-1] = {
      0.00, // ITEM_TAKE
      0.01, // ITEM_WEAR_FINGERS
      0.04, // ITEM_WEAR_NECK
      0.15, // ITEM_WEAR_BODY
      0.07, // ITEM_WEAR_HEAD
      0.05, // ITEM_WEAR_LEGS
      0.02, // ITEM_WEAR_FEET
      0.03, // ITEM_WEAR_HANDS
      0.04, // ITEM_WEAR_ARMS
      0.00, // ITEM_WEAR_UNUSED1
      0.07, // ITEM_WEAR_BACK
      0.08, // ITEM_WEAR_WAIST
      0.02, // ITEM_WEAR_WRISTS
      0.00, // ITEM_WEAR_UNUSED2
      0.25, // ITEM_HOLD
      //0.00, // ITEM_THROW
  };
  static float pointsPerAC[Tier_Max] = {
      low_exchangeRate*0.85, // clothing
      low_exchangeRate*0.75, // light
      low_exchangeRate*0.65, // medium
      low_exchangeRate*0.55, // heavy
      low_exchangeRate*0.85, // jewelry
      low_exchangeRate*0.85, // weapon common
      low_exchangeRate*0.85, // weapon simple
      low_exchangeRate*0.85, // weapon martial
  };

  double points = 0;
  double neg_points = 0;
  double slotWeight = 1.0;

  // add up all armor (turn it positive)
  for (int iAff = 0; iAff < MAX_OBJ_AFFECT; iAff++)
    if (m_clothing->affected[iAff].location == APPLY_ARMOR)
      if (m_clothing->affected[iAff].modifier < 0) // armor bonus
        points += -(m_clothing->affected[iAff].modifier);
      else // armor penalty
        neg_points += m_clothing->affected[iAff].modifier;

  // don't allow negative points to be > 1/2 positive
  neg_points = min(neg_points, points/2.0);

  // combine negative and positive points
  points -= neg_points;

  // before we adjust for scale, round off to the nearest 1000th
  points = 0.001 * round(points * 1000.0);

  // adjust for tier
  points *= pointsPerAC[getTier()];

  // calculate what slot this is used for (skip ITEM_TAKE)
  for(unsigned int iSlot = 1; iSlot < cElements(slotValues); iSlot++)
    if (m_clothing->canWear((1 << iSlot)))
      slotWeight = slotValues[iSlot];
  points /= slotWeight;

  // account for pairedness with slot
  if (m_clothing->getObjStat() & ITEM_PAIRED)
    points /= 2.0;

  m_gotMain = true;
  return m_main = int(points);
}

WeaponEvaluator::WeaponEvaluator(const TBaseWeapon *o) : ObjectEvaluator(o)
{
  m_main = -1;
  m_weap = 0;
}

Tier WeaponEvaluator::getTier()
{
  // mages, shaman, monks use common weapons
  static unsigned int simpleFlags = ITEM_ANTI_SHAMAN | ITEM_ANTI_MAGE | ITEM_ANTI_MONK; // clerics use simple weapons
  static unsigned int martialFlags = ITEM_ANTI_CLERIC | simpleFlags; // ranger, thief, diekhan and warriors are heavy

  if ((m_weap->getObjStat() & martialFlags) == martialFlags)
    return Tier_Martial;
  if ((m_weap->getObjStat() & simpleFlags) == simpleFlags)
    return Tier_Simple;
  return Tier_Common;
}


int WeaponEvaluator::getMainPointsRaw()
{
  return 0;
}


