//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_chest.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// chest.cc
//

#include "stdsneezy.h"

TChest::TChest() :
  TRealContainer()
{
}

TChest::TChest(const TChest &a) :
  TRealContainer(a)
{
}

TChest & TChest::operator=(const TChest &a)
{
  if (this == &a) return *this;
  TRealContainer::operator=(a);
  return *this;
}

TChest::~TChest()
{
}

void TChest::assignFourValues(int x1, int x2, int x3, int x4)
{
  TRealContainer::assignFourValues(x1, x2, x3, x4);
}

void TChest::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TRealContainer::getFourValues(x1, x2, x3, x4);
}

string TChest::statObjInfo() const
{
  return TRealContainer::statObjInfo();
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


