//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_bag.cc,v $
// Revision 5.5  2003/03/13 22:40:53  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.4  2002/08/09 16:13:00  peel
// changed junk to drop in auto pouch code
//
// Revision 5.3  2002/01/16 05:40:29  peel
// added plants
//
// Revision 5.2  2002/01/08 21:05:12  peel
// removed the TBaseContainer hierarchy from obj2.h
// added header files for those objects
// inserted appropriate includes
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


// bag.cc
//

#include "stdsneezy.h"
#include "obj_bag.h"

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
      sprintf(buf, "drop moneypouch");
      ch->addCommandToQue(buf);
    }
  }
  return FALSE;
}
