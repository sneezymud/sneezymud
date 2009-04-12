//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  "cmd_compare.cc"
//  All functions and routins related to the compare command.
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "shop.h"
#include "spec_mobs.h"
#include "obj_bow.h"
#include "obj_symbol.h"
#include "obj_food.h"
#include "obj_opal.h"
#include "obj_arrow.h"
#include "obj_base_weapon.h"
#include "obj_base_light.h"
#include "obj_base_clothing.h"
#include "materials.h"
#include "monster.h"

TObj * findShopObjForCompare(TBeing *ch, sstring StObject)
{
  TThing *tThing;
  unsigned int     shop_nr,
                   tValue;
           char    tString[256];

  if (!(tValue = getabunch(StObject.c_str(), tString))) {
    strcpy(tString, StObject.c_str());
    tValue = 1;
  }

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();++it) {
    tThing=*it;
    if (!dynamic_cast<TMonster *>(tThing) ||
        (mob_specials[GET_MOB_SPE_INDEX(tThing->spec)].proc != shop_keeper))
      continue;

    for (shop_nr = 0; (shop_nr < shop_index.size()) &&
                      (shop_index[shop_nr].keeper != (tThing)->number); shop_nr++);

    if (shop_nr >= shop_index.size() ||
        !shop_index[shop_nr].willTradeWith(dynamic_cast<TMonster *>(tThing), ch))
      continue;

    TThing *tObj = searchLinkedListVis(ch, tString, tThing->stuff);
    if (!tObj)
      tObj = get_num_obj_in_list(ch, convertTo<int>(tString), tThing->stuff, shop_nr);

    if (tObj)
      return (dynamic_cast<TObj *>(tObj));
  }

  return NULL;
}

TObj * findForCompare(TBeing *ch, sstring StObject)
{
  int        tCount = 0;
  wearSlotT  tSlot;
  TThing    *tObj;

  if (!(tObj = get_thing_in_equip(ch, StObject.c_str(), ch->equipment, &tSlot, TRUE, &tCount)))
    if (!(tObj = searchLinkedListVis(ch, StObject, ch->stuff, &tCount)))
      if (!(tObj = findShopObjForCompare(ch, StObject)))
        return NULL;

  return (dynamic_cast<TObj *>(tObj));
}

void TBeing::doMortalCompare(const char *tArg)
{
  sstring    StObject1(""),
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

  StObject1=StString.word(0);
  StObject2=StString.word(1);


  if (StObject1.empty() || StObject2.empty()) {
    sendTo("Syntax: compare <item> <item>\n\r");
    return;
  }

  if (!(tObj1 = findForCompare(this, StObject1)) ||
      !(tObj2 = findForCompare(this, StObject2))) {
    sendTo("You cannot find at least one of those items.\n\r");
    return;
  }

  if (tObj1 == tObj2) {
    sendTo("For some strange reason you feel these items to be equal, strange isn't it...\n\r");
    return;
  }

  StString  = "You compare ";
  StString += tObj1->getName();
  StString += " and ";
  StString += tObj2->getName();
  StString += ".\n\r";

  StString += tObj1->compareMeAgainst(this, tObj2);

  sendTo(COLOR_OBJECTS, StString);
}

int compareDetermineMessage(const int tDrift, const int tValue)
{
  return (min(6, max(0, 3 - (tValue / tDrift))));
}

sstring compareStructure(TObj *tObj1, TObj *tObj2, TBeing *ch)
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
  sstring StString("");

  tMessage = compareDetermineMessage(15, tStructDiff);

  StString += sstring(tObj1->getName()).cap();
  StString += structureLevels[tMessage];
  StString += tObj2->getName();
  StString += ".\n\r";

  return StString;
}

sstring compareNoise(TObj *tObj1, TObj *tObj2, TBeing *ch)
{
  const char * noiseLevels[] =
  {
    " is incredibly more noisier than ",
    " is much more noisier than ",
    " is slightly noisier than ",
    " makes no more noise than ",
    " makes less noise than ",
    " makes much less noise compared to ",
    " makes nowhere near the noise as "
  };

  int    tNoise1  = material_nums[tObj1->getMaterial()].noise,
         tNoise2  = material_nums[tObj2->getMaterial()].noise,
         tMessage = 0;
  int    tNoiseDiff = (tNoise1 - tNoise2);
  sstring StString("");

  tMessage = compareDetermineMessage(3, tNoiseDiff);

  StString += sstring(tObj1->getName()).cap();
  StString += noiseLevels[tMessage];
  StString += tObj2->getName();
  StString += ".\n\r";

  return StString;
}

sstring TThing::compareMeAgainst(TBeing *, TObj *)
{
  return "These two things can not be compared.";
}

sstring TBaseWeapon::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring StString("");

  tSharpDiff  = (tSharp1 - tSharp2);
  tMessage1   = compareDetermineMessage(15, tSharpDiff);
  tDamageDiff = (tDamage1 - tDamage2);
  tMessage2   = compareDetermineMessage(2, tDamageDiff);

  StString += compareStructure(this, tWeapon, ch);
  StString += compareNoise(this, tWeapon, ch);

  if (ch->getSkillValue(SKILL_EVALUATE) > 35) {
    StString += sstring(getName()).cap();
    StString += damageLevels[tMessage2];
    StString += tWeapon->getName();
    StString += ".\n\r";
  }

  if (ch->getSkillValue(SKILL_EVALUATE) > 5)
    if (isBluntWeapon() && tWeapon->isBluntWeapon()) {
      StString += sstring(getName()).cap();
      StString += bluntnessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    } else if (isSlashWeapon() && tWeapon->isSlashWeapon()) {
      StString += sstring(getName()).cap();
      StString += sharpnessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    } else if (isPierceWeapon() && tWeapon->isPierceWeapon()) {
      StString += sstring(getName()).cap();
      StString += pointednessLevels[tMessage1];
      StString += tWeapon->getName();
      StString += ".\n\r";
    }

  return StString;
}

sstring TBaseClothing::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring StString("");

  StString += compareStructure(this, tClothing, ch);
  StString += compareNoise(this, tClothing, ch);

  tArmorDiff = (tArmor1 - tArmor2);
  tMessage = compareDetermineMessage(5, tArmorDiff);

  if (ch->getSkillValue(SKILL_EVALUATE) > 50) {
    StString += sstring(getName()).cap();
    StString += armorLevels[tMessage];
    StString += tClothing->getName();
    StString += ".\n\r";
  }

  return StString;
}

sstring TBow::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring StString("");

  StString += compareStructure(this, tBow, ch);
  StString += compareNoise(this, tBow, ch);

  tRangeDiff = (tRange1 - tRange2);
  tMessage = compareDetermineMessage(2, tRangeDiff);

  if (ch->getSkillValue(SKILL_EVALUATE) > 25) {
    StString += sstring(getName()).cap();
    StString += rangeLevels[tMessage];
    StString += tBow->getName();
    StString += ".\n\r";
  }

  return StString;
}

sstring TArrow::compareMeAgainst(TBeing *ch, TObj *tObj)
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

  sstring StString("");

  if (tBow) {
    StString += sstring(getName()).cap();

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
    StString += sstring(getName()).cap();
    StString += sharpnessLevels[tMessage1];
    StString += tArrow->getName();
    StString += ".\n\r";
  }

  StString += compareStructure(this, tArrow, ch);
  StString += compareNoise(this, tArrow, ch);

  if (ch->getSkillValue(SKILL_EVALUATE) > 50) {
    StString += sstring(getName()).cap();
    StString += damageLevels[tMessage2];
    StString += tArrow->getName();
    StString += ".\n\r";
  }

  return StString;
}

sstring TFood::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring StString("");

  tFillDiff = (tFill1 - tFill2);
  tMessage  = compareDetermineMessage(4, tFillDiff);

  StString += sstring(getName()).cap();
  StString += fillLevels[tMessage];
  StString += tFood->getName();
  StString += ".\n\r";

  return StString;
}

sstring TSymbol::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring   StString("");

  tStrengthDiff  = (tStrength1 - tStrength2);
  tMessage1      = compareDetermineMessage(100, tStrengthDiff);
  tHolyWaterDiff = (tHolyWater1 - tHolyWater2);
  tMessage2      = compareDetermineMessage(30, tHolyWaterDiff);

  StString += sstring(getName()).cap();
  StString += strengthLevels[tMessage1];
  StString += tSymbol->getName();
  StString += ".\n\r";

  if (getSymbolFaction() == FACT_UNDEFINED &&
      tSymbol->getSymbolFaction() == FACT_UNDEFINED &&
      ch->getSkillValue(SKILL_EVALUATE) > 10) {
    StString += sstring(getName()).cap();
    StString += holyWaterLevels[tMessage2];
    StString += tSymbol->getName();
    StString += ".\n\r";
  }

  return StString;
}

sstring TBaseLight::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring   StString("");

  tLightDiff = (tLight1 - tLight2);
  tMessage    = compareDetermineMessage(3, tLightDiff);

  StString += sstring(getName()).cap();
  StString += lightLevels[tMessage];
  StString += tLight->getName();
  StString += ".\n\r";

  return StString;
}

sstring TOpal::compareMeAgainst(TBeing *ch, TObj *tObj)
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
  sstring StString("");

  tChargeDiff = (tCharge1 - tCharge2);
  tMessage    = compareDetermineMessage(3, tChargeDiff);

  StString += sstring(getName()).cap();
  StString += chargeLevels[tMessage];
  StString += tOpal->getName();
  StString += ".\n\r";

  return StString;
}
