//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_potion.h"
#include "shop.h"
#include "database.h"
#include "liquids.h"

TPotion::TPotion() :
  TBaseCup()
{
}

TPotion::TPotion(const TPotion &a) :
  TBaseCup(a)
{
}

TPotion & TPotion::operator=(const TPotion &a)
{
  if (this == &a) return *this;
  TBaseCup::operator=(a);
  return *this;
}

TPotion::~TPotion()
{
}


bool TPotion::isSimilar(const TThing *t) const
{
  const TPotion *pot = dynamic_cast<const TPotion *>(t);
  if (!pot)
    return FALSE;

  if (!getDescr() || !pot->getDescr() ||
      strcmp(getDescr(), pot->getDescr())){
    return false;
  }

  if (!name || !pot->name ||
      !is_exact_name(name, pot->name)){
    return false;
  }
  
  // not same if drink types are different, unless both are empty
  if(getDrinkType() != pot->getDrinkType() &&
     !(getDrinkUnits()==0 && getDrinkUnits()==0)){
    return false;
  }

  return true;
}


int TPotion::getValue() const
{
  int cost_per, value;
  cost_per = liquidInfo[getDrinkType()]->price;
  value = (int) (getDrinkUnits() * cost_per);
  
  if (obj_flags.cost <= 1) {
    value = max(0, value);
  } else {
    value = max(1, value);  
  }

  return value;
}

int TPotion::objectSell(TBeing *ch, TMonster *keeper)
{
  sstring buf;

  if(!liquidInfo[getDrinkType()]->potion){
    keeper->doTell(ch->getName(), "Hey, that's not a potion!.");
    return TRUE;
  }

  return FALSE;
}


int TPotion::sellPrice(int, int shop_nr, float chr, const TBeing *ch)
{
  // adjust cost based on structure
  double cost = getValue();

  // adjust cost based on shop pricing
  cost *= shop_index[shop_nr].getProfitSell(this, ch);


  // adjust for charisma/swindle modifier
  if (chr != -1 && chr!=0)
    cost /= chr;

  // make sure we don't have a negative cost
  cost = max(1.0, cost);

  return (int) cost;
}


int TPotion::shopPrice(int num, int shop_nr, float chr, const TBeing *ch) const
{
  // adjust cost based on structure
  double cost = getValue();

  // adjust cost based on shop pricing
  cost *= shop_index[shop_nr].getProfitBuy(this, ch);

  // adjust for charisma/swindle modifier
  if(chr != -1)
    cost *= chr;

  // multiply by the number of items
  cost *= num;

  // make sure we don't have a negative cost
  cost = max(1.0, cost);

  return (int) cost;
}


// return the liquid associated with the shaman spell
// or LIQ_WATER if it is not an allowed potion to brew
liqTypeT spell_to_liq(spellNumT which)
{
  switch(which){
    case SPELL_CELERITE:
      return LIQ_POT_CELERITE;
    case SPELL_SHIELD_OF_MISTS:
      return LIQ_POT_SHIELD_OF_MISTS;
    case SPELL_SENSE_LIFE_SHAMAN:
      return LIQ_POT_SENSE_PRESENCE;
    case SPELL_CHEVAL:
      return LIQ_POT_CHEVAL;
    case SPELL_DJALLA:
      return LIQ_POT_DJALLAS_PROTECTION;
    case SPELL_LEGBA:
      return LIQ_POT_LEGBAS_GUIDANCE;
    case SPELL_DETECT_SHADOW:
      return LIQ_POT_DETECT_SHADOW;
    case SPELL_CLARITY:
      return LIQ_POT_CLARITY;
    case SPELL_BLOOD_BOIL:
      return LIQ_POT_BOILING_BLOOD;
    case SPELL_STUPIDITY:
      return LIQ_POT_STUPIDITY;
    default:
      return LIQ_WATER;
  }

  return LIQ_WATER;
}
