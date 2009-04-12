//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// saddlebag.cc
//

#include "monster.h"
#include "obj_saddlebag.h"

TSaddlebag::TSaddlebag() :
  TExpandableContainer()
{
}

TSaddlebag::TSaddlebag(const TSaddlebag &a) :
  TExpandableContainer(a)
{
}

TSaddlebag & TSaddlebag::operator=(const TSaddlebag &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TSaddlebag::~TSaddlebag()
{
}

void TSaddlebag::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TSaddlebag::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TSaddlebag::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TSaddlebag::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair saddlebags.");
  }
  return TRUE;
}

int TSaddlebag::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff for recusivity
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  return FALSE;
}
