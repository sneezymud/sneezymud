//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_bag.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// bag.cc
//

#include "stdsneezy.h"

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

string TBag::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TBag::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];
    sprintf(buf, "%s I can't repair bags.", fname(ch->name).c_str());
    repair->doTell(buf);
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
      sprintf(buf, "junk moneypouch");
      ch->addCommandToQue(buf);
    }
  }
  return FALSE;
}

