//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// statue.cc

#include "stdsneezy.h"
#include "obj_statue.h"

TStatue::TStatue() :
  TObj()
{
}

TStatue::TStatue(const TStatue &a) :
  TObj(a)
{
}

TStatue & TStatue::operator=(const TStatue &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TStatue::~TStatue()
{
}

void TStatue::assignFourValues(int, int, int, int)
{
}

void TStatue::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TStatue::statObjInfo() const
{
  sstring a("");
  return a;
}

void TStatue::lowCheck()
{
  // we intentionally do not recurse to TObj::lowCheck
  // we don't care about light check, etc.
}

void TStatue::writeAffects(int i, FILE *fp) const
{
  if (affected[i].location != APPLY_NONE) {
    fprintf(fp, "A\n%d %ld %ld\n",
            mapApplyToFile(affected[i].location),
            applyTypeShouldBeSpellnum(affected[i].location) ? mapSpellnumToFile(spellNumT(affected[i].modifier)) : affected[i].modifier,
            affected[i].modifier2);
  }
}

int TStatue::addApply(TBeing *, applyTypeT)
{
  return FALSE;
}
