//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////



#include "obj_casino_chip.h"
#include "shop.h"

TCasinoChip::TCasinoChip() :
  TObj()
{
}

TCasinoChip::TCasinoChip(const TCasinoChip &a) :
  TObj(a)
{
}

TCasinoChip & TCasinoChip::operator=(const TCasinoChip &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TCasinoChip::~TCasinoChip()
{
}

void TCasinoChip::assignFourValues(int, int, int, int)
{
}

void TCasinoChip::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TCasinoChip::statObjInfo() const
{
  sstring a("");
  return a;
}



int TCasinoChip::sellPrice(int num, int shop_nr, float, const TBeing *ch)
{
  return (int)(obj_flags.cost * shop_index[shop_nr].getProfitSell(this, ch));
}

int TCasinoChip::shopPrice(int num, int shop_nr, float, const TBeing *ch) const
{
  return (int)(obj_flags.cost * shop_index[shop_nr].getProfitBuy(this, ch));
}

