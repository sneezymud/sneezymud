//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
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
  if (!silent)
    repair->doTell(fname(ch->name), fmt("I'm not the trash man. Take %s to the dump!") % getName());

  return TRUE;
}

