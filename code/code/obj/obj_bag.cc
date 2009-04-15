//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// bag.cc
//

#include "monster.h"
#include "obj_bag.h"
#include "connect.h"

TBag::TBag() :
  TExpandableContainer()
{
}

TBag::TBag(const TBag &a) :
  TExpandableContainer(a)
{
}

TBag & TBag::operator=(const TBag &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TBag::~TBag()
{
}

void TBag::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TBag::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TBag::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TBag::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair bags.");
  }
  return TRUE;
}

int TBag::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff for recusivity
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  if (isname("moneypouch", name)) {
    if (ch->desc && IS_SET(ch->desc->autobits, AUTO_POUCH)) {
      char buf[256];
      sprintf(buf, "open moneypouch");
      ch->addCommandToQue(buf);
      sprintf(buf, "get all moneypouch");
      ch->addCommandToQue(buf);
      sprintf(buf, "drop moneypouch");
      ch->addCommandToQue(buf);
    }
  }
  return FALSE;
}
