//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// suitcase.cc
// Peel

#include "stdsneezy.h"
#include "obj_suitcase.h"

TSuitcase::TSuitcase() :
  TExpandableContainer()
{
}

TSuitcase::TSuitcase(const TSuitcase &a) :
  TExpandableContainer(a)
{
}

TSuitcase & TSuitcase::operator=(const TSuitcase &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TSuitcase::~TSuitcase()
{
}

void TSuitcase::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TSuitcase::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TSuitcase::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TSuitcase::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair suitcases.");
  }
  return TRUE;
}
