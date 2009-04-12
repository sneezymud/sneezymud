//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// keyring.cc
// Peel

#include "monster.h"
#include "obj_keyring.h"
#include "obj_key.h"
TKeyring::TKeyring() :
  TExpandableContainer()
{
}

TKeyring::TKeyring(const TKeyring &a) :
  TExpandableContainer(a)
{
}

TKeyring & TKeyring::operator=(const TKeyring &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TKeyring::~TKeyring()
{
}

void TKeyring::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TKeyring::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TKeyring::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TKeyring::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair keyrings.");
  }
  return TRUE;
}
