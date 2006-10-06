//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// harness.cc
//

#include "stdsneezy.h"
#include "obj_base_clothing.h"
#include "obj_harness.h"

THarness::THarness() :
  TBaseClothing()
{
}

THarness::THarness(const THarness &a) :
  TBaseClothing(a)
{
}

THarness & THarness::operator=(const THarness &a)
{
  if (this == &a) return *this;
  TBaseClothing::operator=(a);
  return *this;
}

THarness::~THarness()
{
}

void THarness::assignFourValues(int , int , int , int )
{
}

void THarness::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring THarness::statObjInfo() const
{
  sstring a("");
  return a;
}

void THarness::lowCheck()
{
  TBaseClothing::lowCheck();
}
