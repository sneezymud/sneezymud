//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "obj_base_container.h"
#include "obj_poison.h"
#include "monster.h"
#include "shop.h"
#include "liquids.h"

TPoison::TPoison() :
  TBaseCup()
{
}

TPoison::TPoison(const TPoison &a) :
  TBaseCup(a)
{
}

TPoison & TPoison::operator=(const TPoison &a)
{
  if (this == &a) return *this;
  TBaseCup::operator=(a);
  return *this;
}

TPoison::~TPoison()
{
}



int TPoison::sellPrice(int, int shop_nr, float, const TBeing *ch)
{
  int cost_per;
  int price;

  cost_per = liquidInfo[getDrinkType()]->price;
  price = (int) (getDrinkUnits() * cost_per * shop_index[shop_nr].getProfitSell(this, ch));

  if (obj_flags.cost <= 1) {
    price = max(0, price);
  } else {
    price = max(1, price);
  }

  return price;
}

int TPoison::shopPrice(int num, int shop_nr, float, const TBeing *ch) const
{
  int cost_per;
  int price;

  cost_per = liquidInfo[getDrinkType()]->price;
  price = (int) (num * cost_per * getDrinkUnits() * shop_index[shop_nr].getProfitBuy(this,ch));
  price = max(1, price);

  return price;
}



int TPoison::objectSell(TBeing *ch, TMonster *keeper)
{
  sstring buf;

  if(!liquidInfo[getDrinkType()]->poison){
    keeper->doTell(ch->getName(), "Hey, that's not poison!.");
    return TRUE;
  }

  return FALSE;
}
