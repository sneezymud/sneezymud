//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_keyring.cc,v $
// Revision 5.3  2002/01/10 00:45:49  peel
// more splitting up of obj2.h
//
// Revision 5.2  2002/01/08 21:05:12  peel
// removed the TBaseContainer hierarchy from obj2.h
// added header files for those objects
// inserted appropriate includes
//
// Revision 5.1  2001/07/13 05:32:20  peel
// renamed a bunch of source files
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// keyring.cc
// Peel

#include "stdsneezy.h"
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

string TKeyring::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TKeyring::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];
    sprintf(buf, "%s I can't repair keyrings.", fname(ch->name).c_str());
    repair->doTell(buf);
  }
  return TRUE;
}
