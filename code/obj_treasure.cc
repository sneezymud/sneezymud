//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// treasure.cc

#include "stdsneezy.h"
#include "obj_treasure.h"

TTreasure::TTreasure() :
  TObj()
{
}

TTreasure::TTreasure(const TTreasure &a) :
  TObj(a)
{
}

TTreasure & TTreasure::operator=(const TTreasure &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TTreasure::~TTreasure()
{
}

void TTreasure::assignFourValues(int, int, int, int)
{
}

void TTreasure::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TTreasure::statObjInfo() const
{
  sstring a("");
  return a;
}

