//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: gemstone.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// gemstone.cc

#include "stdsneezy.h"

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

string TGemstone::statObjInfo() const
{
  string a("");
  return a;
}

