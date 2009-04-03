//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// gemstone.cc

#include "stdsneezy.h"
#include "obj_gemstone.h"

TGemstone::TGemstone() :
  TObj()
{
}

TGemstone::TGemstone(const TGemstone &a) :
  TObj(a)
{
}

TGemstone & TGemstone::operator=(const TGemstone &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TGemstone::~TGemstone()
{
}

void TGemstone::assignFourValues(int, int, int, int)
{
}

void TGemstone::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TGemstone::statObjInfo() const
{
  sstring a("");
  return a;
}

