//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_poison.h"
#include "shop.h"

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



int TPoison::sellPrice(int shop_nr, float, int *)
{
  int cost_per;
  int price;

  cost_per = DrinkInfo[getDrinkType()]->price;
  price = (int) (getDrinkUnits() * cost_per * shop_index[shop_nr].profit_sell);

  if (obj_flags.cost <= 1) {
    price = max(0, price);
  } else {
    price = max(1, price);
  }

  return price;
}

int TPoison::shopPrice(int num, int shop_nr, float, int *) const
{
  int cost_per;
  int price;

  cost_per = DrinkInfo[getDrinkType()]->price;
  price = (int) (num * cost_per * getDrinkUnits() * shop_index[shop_nr].profit_buy);
  price = max(1, price);

  return price;
}



int TPoison::objectSell(TBeing *ch, TMonster *keeper)
{
  sstring buf;

  if(!DrinkInfo[getDrinkType()]->poison){
    ssprintf(buf, "%s Hey, that's not poison!.", ch->getName());
    keeper->doTell(buf.c_str());
    return TRUE;
  }

  return FALSE;
}
