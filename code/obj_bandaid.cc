//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// bandaid.cc

#include "stdsneezy.h"
#include "obj_bandaid.h"

TBandaid::TBandaid() :
  TObj()
{
}

TBandaid::TBandaid(const TBandaid &a) : 
 TObj(a)
{
}

TBandaid & TBandaid::operator=(const TBandaid &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TBandaid::~TBandaid()
{
}

void TBandaid::assignFourValues(int, int, int, int)
{
}

void TBandaid::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TBandaid::statObjInfo() const
{
  sstring a("");
  return a;
}

void TBandaid::scrapMe(TBeing *ch)
{
  ch->remLimbFlags(eq_pos, PART_BANDAGED);
}

void TBandaid::findBandage(int *count)
{
  (*count)++;
}

void TBandaid::destroyBandage(int *count)
{
  (*count)++;
  delete this;
}

