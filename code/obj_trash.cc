//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_trash.cc,v $
// Revision 5.3  2003/03/13 22:40:54  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.2  2002/01/09 23:27:04  peel
// More splitting up of obj2.h
// renamed food.cc to obj_food.cc
// renamed organic.cc to obj_organic.cc
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


// trash.cc

#include "stdsneezy.h"
#include "obj_trash.h"

TTrash::TTrash() :
  TObj()
{
}

TTrash::TTrash(const TTrash &a) :
  TObj(a)
{
}

TTrash & TTrash::operator=(const TTrash &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TTrash::~TTrash()
{
}

void TTrash::assignFourValues(int, int, int, int)
{
}

void TTrash::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TTrash::statObjInfo() const
{
  sstring a("");
  return a;
}

bool TTrash::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];

    sprintf(buf, "%s I'm not the trash man. Take %s to the dump!",
      fname(ch->name).c_str(), getName());

    repair->doTell(buf);
  }
  return TRUE;
}

