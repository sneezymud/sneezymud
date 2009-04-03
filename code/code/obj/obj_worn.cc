//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// worn.cc
//

#include "stdsneezy.h"
#include "obj_base_clothing.h"
#include "obj_worn.h"

TWorn::TWorn() :
  TBaseClothing()
{
}

TWorn::TWorn(const TWorn &a) :
  TBaseClothing(a)
{
}

TWorn & TWorn::operator=(const TWorn &a)
{
  if (this == &a) return *this;
  TBaseClothing::operator=(a);
  return *this;
}

TWorn::~TWorn()
{
}

void TWorn::assignFourValues(int , int , int , int )
{
}

void TWorn::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TWorn::statObjInfo() const
{
  sstring a("");
  return a;
}

void TWorn::lowCheck()
{
  TBaseClothing::lowCheck();
}
