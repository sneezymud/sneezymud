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



int TPotion::sellPrice(int, int shop_nr, float)
{
//  int cost_per;
  int price;

//  cost_per = DrinkInfo[getDrinkType()]->price;
//  price = (int) (getDrinkUnits() * cost_per * shop_index[shop_nr].profit_sell);
  price = (int) (getValue() * shop_index[shop_nr].profit_sell);
  
  if (obj_flags.cost <= 1) {
    price = max(0, price);
  } else {
    price = max(1, price);
  }

  return price;
}

int TPotion::shopPrice(int num, int shop_nr, float) const
{
//  int cost_per;
  int price;

//  cost_per = DrinkInfo[getDrinkType()]->price;
//  price = (int) (num * cost_per * getDrinkUnits() * shop_index[shop_nr].profit_buy);
  price = (int) (num * getValue() * shop_index[shop_nr].profit_buy);
  price = max(1, price);

  return price;
}

int TPotion::getValue() const
{
  int cost_per, value;
  cost_per = DrinkInfo[getDrinkType()]->price;
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

  if(!DrinkInfo[getDrinkType()]->potion){
    keeper->doTell(ch->getName(), "Hey, that's not a potion!.");
    return TRUE;
  }

  return FALSE;
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
