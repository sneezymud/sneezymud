//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_treasure.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// treasure.cc

#include "stdsneezy.h"

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

string TTreasure::statObjInfo() const
{
  string a("");
  return a;
}

