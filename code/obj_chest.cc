//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// chest.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_chest.h"
#include "obj_open_container.h"

TChest::TChest() :
  TOpenContainer()
{
}

TChest::TChest(const TChest &a) :
  TOpenContainer(a)
{
}

TChest & TChest::operator=(const TChest &a)
{
  if (this == &a) return *this;
  TOpenContainer::operator=(a);
  return *this;
}

TChest::~TChest()
{
}

void TChest::assignFourValues(int x1, int x2, int x3, int x4)
{
  TOpenContainer::assignFourValues(x1, x2, x3, x4);
}

void TChest::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TOpenContainer::getFourValues(x1, x2, x3, x4);
}

string TChest::statObjInfo() const
{
  return TOpenContainer::statObjInfo();
}

bool TChest::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];

    sprintf(buf,"%s Does this look like a locksmithery to you?", fname(ch->name).c_str());
    repair->doTell(buf);
  }
  return TRUE;
}

void TChest::lowCheck()
{
#if 0
  // this is retarded
  if (canWear(ITEM_TAKE)) {
    vlogf(LOG_LOW, "Chest (%s:%d) set takeable.  Removing take flag.",
           getName(), objVnum());                             
    remObjStat(ITEM_TAKE);
  }                                                           
#endif
  TOpenContainer::lowCheck();                                     
}
