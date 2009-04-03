//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// disc_cleric.cc

#include "stdsneezy.h"
#include "disc_cleric.h"

CDCleric::CDCleric() :
  CDiscipline(),
  skHealLight(),
  skHarmLight(),
  skCreateFood(),
  skCreateWater(),
  skArmor(),
  skBless(),
  skClot(),
  skRainBrimstone(),
  skHealSerious(),
  skHarmSerious(),
  skSterilize(),
  skExpel(),
  skCureDisease(),
  skCurse(),
  skRemoveCurse(),
  skCurePoison(),
  skHealCritical(),
  skSalve(),
  skPoison(),
  skHarmCritical(),
  skInfect(),
  skRefresh(),
  skNumb(),
  skDisease(),
  skFlamestrike(),
  skPlagueOfLocusts(),
  skCureBlindness(),
  skSummon(),
  skHeal(),
  skParalyzeLimb(),
  skWordOfRecall(),
  skHarm(),
  skKnitBone(),
  skBlindness()
{
}

CDCleric::CDCleric(const CDCleric &a) :
  CDiscipline(a),
  skHealLight(a.skHealLight),
  skHarmLight(a.skHarmLight),
  skCreateFood(a.skCreateFood),
  skCreateWater(a.skCreateWater),
  skArmor(a.skArmor),
  skBless(a.skBless),
  skClot(a.skClot),
  skRainBrimstone(a.skRainBrimstone),
  skHealSerious(a.skHealSerious),
  skHarmSerious(a.skHarmSerious),
  skSterilize(a.skSterilize),
  skExpel(a.skExpel),
  skCureDisease(a.skCureDisease),
  skCurse(a.skCurse),
  skRemoveCurse(a.skRemoveCurse),
  skCurePoison(a.skCurePoison),
  skHealCritical(a.skHealCritical),
  skSalve(a.skSalve),
  skPoison(a.skPoison),
  skHarmCritical(a.skHarmCritical),
  skInfect(a.skInfect),
  skRefresh(a.skRefresh),
  skNumb(a.skNumb),
  skDisease(a.skDisease),
  skFlamestrike(a.skFlamestrike),
  skPlagueOfLocusts(a.skPlagueOfLocusts),
  skCureBlindness(a.skCureBlindness),
  skSummon(a.skSummon),
  skHeal(a.skHeal),
  skParalyzeLimb(a.skParalyzeLimb),
  skWordOfRecall(a.skWordOfRecall),
  skHarm(a.skHarm),
  skKnitBone(a.skKnitBone),
  skBlindness(a.skBlindness)
{
}

CDCleric & CDCleric::operator=(const CDCleric &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skHealLight = a.skHealLight;
  skHarmLight = a.skHarmLight;
  skCreateFood = a.skCreateFood;
  skCreateWater = a.skCreateWater;
  skArmor = a.skArmor;
  skBless = a.skBless;
  skClot = a.skClot;
  skRainBrimstone = a.skRainBrimstone;
  skHealSerious = a.skHealSerious;
  skHarmSerious = a.skHarmSerious;
  skSterilize = a.skSterilize;
  skExpel = a.skExpel;
  skCureDisease = a.skCureDisease;
  skCurse = a.skCurse;
  skRemoveCurse = a.skRemoveCurse;
  skCurePoison = a.skCurePoison;
  skHealCritical = a.skHealCritical;
  skSalve = a.skSalve;
  skPoison = a.skPoison;
  skHarmCritical = a.skHarmCritical;
  skInfect = a.skInfect;
  skRefresh = a.skRefresh;
  skNumb = a.skNumb;
  skDisease = a.skDisease;
  skFlamestrike = a.skFlamestrike;
  skPlagueOfLocusts = a.skPlagueOfLocusts;
  skCureBlindness = a.skCureBlindness;
  skSummon = a.skSummon;
  skHeal = a.skHeal;
  skParalyzeLimb = a.skParalyzeLimb;
  skWordOfRecall = a.skWordOfRecall;
  skHarm = a.skHarm;
  skKnitBone = a.skKnitBone;
  skBlindness = a.skBlindness;
  return *this;
}

CDCleric::~CDCleric()
{
}

CDCleric * CDCleric::cloneMe()
{
  return new CDCleric(*this);
}
