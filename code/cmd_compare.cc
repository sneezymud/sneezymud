//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_compare.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_compare.cc"
  All functions and routins related to the compare command.

  Created 5/ 2/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"
#include "shop.h"

extern bool is_ok(TMonster *, TBeing *, int);

TObj * findShopObjForCompare(TBeing *ch, string StObject)
{
  TThing *tThing;
  unsigned int     shop_nr,
                   tValue;
           char    tString[256];

  if (!(tValue = getabunch(StObject.c_str(), tString))) {
    strcpy(tString, StObject.c_str());
    tValue = 1;
  }

  for (tThing = ch->roomp->stuff; tThing; tThing = tThing->nextThing) {
    if (!dynamic_cast<TMonster *>(tThing) ||
        (mob_specials[GET_MOB_SPE_INDEX(tThing->spec)].proc != shop_keeper))
      continue;

    for (shop_nr = 0; (shop_nr < shop_index.size()) &&
                      (shop_index[shop_nr].keeper != (tThing)->number); shop_nr++);

    if (shop_nr >= shop_index.size() ||
        !is_ok(dynamic_cast<TMonster *>(tThing), ch, shop_nr))
      continue;

    TThing *tObj = searchLinkedListVis(ch, tString, tThing->stuff);
    if (!tObj)
      tObj = get_num_obj_in_list(ch, atoi(tString), tThing->stuff, shop_nr);

    if (tObj)
      return (dynamic_cast<TObj *>(tObj));
  }

  return NULL;
}

TObj * findForCompare(TBeing *ch, string StObject)
{
  int        tCount = 0;
  wearSlotT  tSlot;
  TThing    *tObj;

  if (!(tObj = get_thing_in_equip(ch, StObject.c_str(), ch->equipment, &tSlot, TRUE, &tCount)))
    if (!(tObj = searchLinkedListVis(ch, StObject.c_str(), ch->stuff, &tCount)))
      if (!(tObj = findShopObjForCompare(ch, StObject)))
        return NULL;

  return (dynamic_cast<TObj *>(tObj));
}

void TBeing::doMortalCompare(const char *tArg)
{
  string     StObject1(""),
             StObject2(""),
             StString(tArg);
  TObj      *tObj1 = NULL,
            *tObj2 = NULL;

  if (!roomp || !desc)
    return;

  if (!getSkillValue(SKILL_EVALUATE)) {
    sendTo("You have no knowledge in evaluate which makes comparing things slightly difficult.\n\r");
    return;
  }

  StString = two_arg(StString, StObject1, StObject2);

  if (StObject1.empty() || StObject2.empty()) {
    sendTo("Syntax: compare <item> <item>\n\r");
    return;
  }

  if (!(tObj1 = findForCompare(this, StObject1)) ||
      !(tObj2 = findForCompare(this, StObject2))) {
    sendTo("You cannot find at least one of those items.\n\r");
    return;
  }

  StString  = "You compare ";
  StString += tObj1->getName();
  StString += " and ";
  StString += tObj2->getName();
  StString += ".\n\r";

  StString += tObj1->compareMeAgainst(this, tObj2);

  sendTo(COLOR_OBJECTS, StString.c_str());
}

int compareDetermineMessage(const int tDrift, const int tValue)
{
  return (min(6, max(0, 3 - (tValue / tDrift))));
}

string compareStructure(TObj *tObj1, TObj *tObj2, TBeing *ch)
{
  const char *structureLevels[] =
  {
    " is great deal stronger than ",
    " is much stronger than ",
    " is stronger than ",
    " looks to be as strong as ",
    " is not as strong as ",
    " is much weaker than ",
    " is nowhere near as strong as "
  };

  if (ch->getSkillValue(SKILL_EVALUATE) <= 20)
    return "";

  int    tStruct1 = tObj1->getMaxStructPoints(),
         tStruct2 = tObj2->getMaxStructPoints(),
         tMessage = 0;
  int    tStructDiff = (tStruct1 - tStruct2);
  string StString("");

  tMessage = compareDetermineMessage(15, tStructDiff);

  StString += good_cap(tObj1->getName()).c_str();
  StString += structureLevels[tMessage];
  StString += tObj2->getName();
  StString += ".\n\r";

  return StString;
}

string compareNoise(TObj *tObj1, TObj *tObj2, TBeing *ch)
{
  const char * noiseLevels[] =
  {
    " is incredibly more noisier than ",
    " is much more noisier than ",
    " is slighy noisier than ",
    " makes no more noise than ",
    " makes less noise than ",
    " makes much less noise compared to ",
    " makes nowhere near the noise as "
  };

  int    tNoise1  = material_nums[tObj1->getMaterial()].noise,
         tNoise2  = material_nums[tObj2->getMaterial()].noise,
         tMessage = 0;
  int    tNoiseDiff = (tNoise1 - tNoise2);
  string StString("");

  tMessage = compareDetermineMessage(3, tNoiseDiff);

  StString += good_cap(tObj1->getName()).c_str();
  StString += noiseLevels[tMessage];
  StString += tObj2->getName();
  StString += ".\n\r";

  return StString;
}

string TThing::compareMeAgainst(TBeing *, TObj *)
{
  return "These two things can not be compared.";
}

string TBaseWeapon::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * sharpnessLevels[] =
  {
    " is greatly sharper than ",
    " is a lot sharper than ",
    " is a little sharper than ",
    " has the same sharpness as ",
    " is a little duller than ",
    " is a lot duller than ",
    " is greatly dull compared to "
  };

  const char * pointednessLevels[] =
  {
    " is greatly pointier than ",
    " is a lot pointier than ",
    " is a little pointier than ",
    " has the same pointedness as ",
    " is a little duller than ",
    " is a lot duller than ",
    " is greatly duller than "
  };

  const char * bluntnessLevels[] =
  {
    " is greatly more blunt than ",
    " is a lot blunter than ",
    " is a little blunter than ",
    " has the same bluntness as ",
    " is not as smooth as ",
    " is no where near as smooth as ",
    " is has no surface compared to  "
  };

  const char * damageLevels[] =
  {
    " does a whole lot more damage than ",
    " does a lot more damage than ",
    " does a little more damage than ",
    " does the same amount of damage as ",
    " does a little less damage than ",
    " does a lot less damage compared to ",
    " does a whole lot less damage compared to "
  };

  TBaseWeapon *tWeapon = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tWeapon = dynamic_cast<TBaseWeapon *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";


  int    tSharp1 = getMaxSharp(),
         tSharp2 = tWeapon->getMaxSharp(),
         tSharpDiff,
         tMessage1,
         tDamage1 = (int)baseDamage(),
         tDamage2 = (int)tWeapon->baseDamage(),
         tDamageDiff,
         tMessage2;
  string StString("");

  tSharpDiff  = (tSharp1 - tSharp2);
  tMessage1   = compareDetermineMessage(15, tSharpDiff);
  tDamageDiff = (tDamage1 - tDamage2);
  tMessage2   = compareDetermineMessage(2, tDamageDiff);

  StString += compareStructure(this, tWeapon, ch);
  StString += compareNoise(this, tWeapon, ch);

  if (ch->getSkillValue(SKILL_EVALUATE) > 35) {
    StString += good_cap(getName()).c_str();
    StString += damageLevels[tMessage2];
    StString += tWeapon->getName();
    StString += ".\n\r";
  }

  if (ch->getSkillValue(SKILL_EVALUATE) > 5)
    if (isBluntWeapon() && tWeapon->isBluntWeapon()) {
      StString += good_cap(getName()).c_str();
      StString += bluntnessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    } else if (isSlashWeapon() && tWeapon->isSlashWeapon()) {
      StString += good_cap(getName()).c_str();
      StString += sharpnessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    } else if (isPierceWeapon() && tWeapon->isPierceWeapon()) {
      StString += good_cap(getName()).c_str();
      StString += pointednessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    }

  return StString;
}

string TBaseClothing::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * armorLevels[] =
  {
    " is greatly more protective than ",
    " is notably more protective than ",
    " is a little bit more protective as ",
    " has about the same ac as ",
    " does not protect you as well as ",
    " protects you far less than ",
    " offers no where near the protection compared to "
  };

  TBaseClothing *tClothing = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((((itemType() != ITEM_WORN || itemType() != ITEM_ARMOR) ||
       (tObj->itemType() != ITEM_WORN || tObj->itemType() != ITEM_ARMOR)) &&
       (itemType() != tObj->itemType())) ||
      !(tClothing = dynamic_cast<TBaseClothing *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int    tArmor1 = (int)armorLevel(ARMOR_LEV_AC),
         tArmor2 = (int)tClothing->armorLevel(ARMOR_LEV_AC),
         tArmorDiff,
         tMessage;
  string StString("");

  StString += compareStructure(this, tClothing, ch);
  StString += compareNoise(this, tClothing, ch);

  tArmorDiff = (tArmor1 - tArmor2);
  tMessage = compareDetermineMessage(5, tArmorDiff);

  if (ch->getSkillValue(SKILL_EVALUATE) > 50) {
    StString += good_cap(getName()).c_str();
    StString += armorLevels[tMessage];
    StString += tClothing->getName();
    StString += ".\n\r";
  }

  return StString;
}

string TBow::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * rangeLevels[] =
  {
    " can shoot a good ways further than ",
    " can shoot a lot further than ",
    " can shoot a little further than ",
    " has the same range as ",
    " can not shoot as far as ",
    " can not shoot nearly as far as ",
    " can not shoot no where near as far as "
  };

  TBow   *tBow   = NULL;
  TArrow *tArrow = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if (((itemType() != tObj->itemType()) ||
       !(tBow = dynamic_cast<TBow *>(tObj))) &&
      ((tObj->itemType() != ITEM_ARROW) ||
       !(tArrow = dynamic_cast<TArrow *>(tObj))))
    return "These two items cannot be compared against one another.\n\r";

  if (tArrow)
    return tArrow->compareMeAgainst(ch, this);

  int    tRange1 = getMaxRange(),
         tRange2 = tBow->getMaxRange(),
         tRangeDiff,
         tMessage;
  string StString("");

  StString += compareStructure(this, tBow, ch);
  StString += compareNoise(this, tBow, ch);

  tRangeDiff = (tRange1 - tRange2);
  tMessage = compareDetermineMessage(2, tRangeDiff);

  if (ch->getSkillValue(SKILL_EVALUATE) > 25) {
    StString += good_cap(getName()).c_str();
    StString += rangeLevels[tMessage];
    StString += tBow->getName();
    StString += ".\n\r";
  }

  return StString;
}

string TArrow::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * sharpnessLevels[] =
  {
    " is greatly sharper than ",
    " is a lot sharper than ",
    " is a little sharper than ",
    " has the same sharpness as ",
    " is a little duller than ",
    " is a lot duller than ",
    " is greatly duller than "
  };

  const char * damageLevels[] =
  {
    " does a whole lot more damage than ",
    " does a lot more damage than ",
    " does a little more damage than ",
    " does the same amount of damage as ",
    " does a little less damage than ",
    " does a lot less damage compared to ",
    " does a whole lot less damage compared to "
  };

  TArrow *tArrow = NULL;
  TBow   *tBow = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if (((itemType() != tObj->itemType()) ||
       !(tArrow = dynamic_cast<TArrow *>(tObj))) &&
      ((tObj->itemType() != ITEM_BOW) ||
       !(tBow = dynamic_cast<TBow *>(tObj))))
    return "These two items cannot be compared against one another.\n\r";

  string StString("");

  if (tBow) {
    StString += good_cap(getName()).c_str();

    if (tBow->getArrowType() == getArrowType())
      StString += " is a perfect fit for ";
    else if (tBow->getArrowType() > getArrowType())
      StString += " is too small for ";
    else
      StString += " is too big for ";


    StString += tBow->getName();
    StString += ".\n\r";

    return StString;
  }

  int tSharp1 = getMaxSharp(),
      tSharp2 = tArrow->getMaxSharp(),
      tSharpDiff,
      tMessage1,
      tDamage1 = (int)baseDamage(),
      tDamage2 = (int)tArrow->baseDamage(),
      tDamageDiff,
      tMessage2;

  tSharpDiff  = (tSharp1 - tSharp2);
  tMessage1   = compareDetermineMessage(15, tSharpDiff);
  tDamageDiff = (tDamage1 - tDamage2);
  tMessage2   = compareDetermineMessage(2, tDamageDiff);

  if (ch->getSkillValue(SKILL_EVALUATE) > 5) {
    StString += good_cap(getName()).c_str();
    StString += sharpnessLevels[tMessage1];
    StString += tArrow->getName();
    StString += ".\n\r";
  }

  StString += compareStructure(this, tArrow, ch);
  StString += compareNoise(this, tArrow, ch);

  if (ch->getSkillValue(SKILL_EVALUATE) > 50) {
    StString += good_cap(getName()).c_str();
    StString += damageLevels[tMessage2];
    StString += tArrow->getName();
    StString += ".\n\r";
  }

  return StString;
}

string TFood::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char *fillLevels[] =
  {
    " will fill you a lot more than ",
    " will fill you more than ",
    " will fill you a little more than ",
    " will fill you the same amount as ",
    " will fill you a little less than ",
    " will fill you less than ",
    " will fill you a lot less than "
  };

  TFood *tFood = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tFood = dynamic_cast<TFood *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int    tFill1 = getFoodFill(),
         tFill2 = tFood->getFoodFill(),
         tFillDiff,
         tMessage;
  string StString("");

  tFillDiff = (tFill1 - tFill2);
  tMessage  = compareDetermineMessage(4, tFillDiff);

  StString += good_cap(getName()).c_str();
  StString += fillLevels[tMessage];
  StString += tFood->getName();
  StString += ".\n\r";

  return StString;
}

string TSymbol::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * strengthLevels[] =
  {
    " has a very notable amount of strength more than ",
    " has a lot more strength than ",
    " has a little more strength than ",
    " has the same strength as ",
    " has a little less strength than ",
    " has a lot less strength than ",
    " has a very notable amount of strength less than ",
  };

  const char * holyWaterLevels[] =
  {
    " requires a great deal more holywater than ",
    " requires a lot more holywater than ",
    " requires a little more holywater than ",
    " requires the same amount of holywater as ",
    " requires a little less holywater than ",
    " requires a lot less holywater than ",
    " requires a great deal less holywater than ",
  };

  TSymbol *tSymbol = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tSymbol = dynamic_cast<TSymbol *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int      tStrength1  = getSymbolMaxStrength(),
           tStrength2  = tSymbol->getSymbolMaxStrength(),
           tStrengthDiff,
           tMessage1,
           tHolyWater1 = (obj_flags.cost / 100),
           tHolyWater2 = (tSymbol->obj_flags.cost / 100),
           tHolyWaterDiff,
           tMessage2;
  string   StString("");

  tStrengthDiff  = (tStrength1 - tStrength2);
  tMessage1      = compareDetermineMessage(100, tStrengthDiff);
  tHolyWaterDiff = (tHolyWater1 - tHolyWater2);
  tMessage2      = compareDetermineMessage(30, tHolyWaterDiff);

  StString += good_cap(getName()).c_str();
  StString += strengthLevels[tMessage1];
  StString += tSymbol->getName();
  StString += ".\n\r";

  if (getSymbolFaction() == FACT_UNDEFINED &&
      tSymbol->getSymbolFaction() == FACT_UNDEFINED &&
      ch->getSkillValue(SKILL_EVALUATE) > 10) {
    StString += good_cap(getName()).c_str();
    StString += holyWaterLevels[tMessage2];
    StString += tSymbol->getName();
    StString += ".\n\r";
  }

  return StString;
}

string TBaseLight::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * lightLevels[] =
  {
    " is a lot brighter than ",
    " is a small bit brighter than ",
    " is a little brighter than ",
    " lets off as much light as ",
    " is a little dimmer than ",
    " is a small bit dimmer than ",
    " is a lot dimmer than "
  };

  TBaseLight *tLight = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if (!(tLight = dynamic_cast<TBaseLight *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int      tLight1 = getLightAmt(),
           tLight2 = tLight->getLightAmt(),
           tLightDiff,
           tMessage;
  string   StString("");

  tLightDiff = (tLight1 - tLight2);
  tMessage    = compareDetermineMessage(3, tLightDiff);

  StString += good_cap(getName()).c_str();
  StString += lightLevels[tMessage];
  StString += tLight->getName();
  StString += ".\n\r";

  return StString;
}

string TOpal::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * chargeLevels[] =
  {
    " has a great amount more stregenth compared to ",
    " has a lot more strength than ",
    " has a little more strength than ",
    " has the same strength as ",
    " has a little less strength than ",
    " has a lot less strength than ",
    " has a great less strength comapred to "
  };

  TOpal *tOpal = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tOpal = dynamic_cast<TOpal *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int    tCharge1 = psGetCarats(),
         tCharge2 = tOpal->psGetCarats(),
         tChargeDiff,
         tMessage;
  string StString("");

  tChargeDiff = (tCharge1 - tCharge2);
  tMessage    = compareDetermineMessage(3, tChargeDiff);

  StString += good_cap(getName()).c_str();
  StString += chargeLevels[tMessage];
  StString += tOpal->getName();
  StString += ".\n\r";

  return StString;
}

string TRealContainer::compareMeAgainst(TBeing *ch, TObj *tObj)
{
  const char * sizeLevels[] =
  {
    " has a great amount of space more than ",
    " has a lot more space than ",
    " has a little bit more space than ",
    " has the same amount of space as ",
    " has a little less space than ",
    " has a lot less space than ",
    " has a great amount less space compared to "
  };

  const char *weightLevels[] = 
  {
    " can hold a great amount of weight over ",
    " can hold a lot more weight than ",
    " can hold a little more weight than ",
    " can hold the same amount of weight as ",
    " can hold less weight than ",
    " can hold a lot less weight than ",
    " can hold a great amount less weight compared to ",
  };

  TRealContainer *tRealContainer = NULL;

  if (!tObj)
    return "Could not find other item to compare.\n\r";

  if ((itemType() != tObj->itemType()) ||
      !(tRealContainer = dynamic_cast<TRealContainer *>(tObj)))
    return "These two items cannot be compared against one another.\n\r";

  int    tSize1   = carryVolumeLimit(),
         tSize2   = tRealContainer->carryVolumeLimit(),
         tSizeDiff,
         tMessage1,
         tWeight1 = (int)carryWeightLimit(),
         tWeight2 = (int)tRealContainer->carryWeightLimit(),
         tWeightDiff,
         tMessage2;
  string StString("");

  tSizeDiff   = (tSize1 - tSize2);
  tMessage1   = compareDetermineMessage(15, tSizeDiff);
  tWeightDiff = (tWeight1 - tWeight2);
  tMessage2   = compareDetermineMessage(15, tWeightDiff);

  StString += good_cap(getName()).c_str();
  StString += sizeLevels[tMessage1];
  StString += tRealContainer->getName();
  StString += ".\n\r";

  StString += good_cap(getName()).c_str();
  StString += weightLevels[tMessage2];
  StString += tRealContainer->getName();
  StString += ".\n\r";

  return StString;
}
