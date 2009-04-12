//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// wagon.cc
//
//////////////////////////////////////////////////////////////////////////


#include "monster.h"
#include "obj_wagon.h"
#include "obj_open_container.h"

TWagon::TWagon() :
  TOpenContainer()
{
}

TWagon::TWagon(const TWagon &a) :
  TOpenContainer(a)
{
}

TWagon & TWagon::operator=(const TWagon &a)
{
  if (this == &a) return *this;
  TOpenContainer::operator=(a);
  return *this;
}

TWagon::~TWagon()
{
}

void TWagon::assignFourValues(int x1, int x2, int x3, int x4)
{
  TOpenContainer::assignFourValues(x1, x2, x3, x4);
}

void TWagon::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TOpenContainer::getFourValues(x1, x2, x3, x4);
}

sstring TWagon::statObjInfo() const
{
  return TOpenContainer::statObjInfo();
}

bool TWagon::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "Does this look like a mechanics shop to you?");
  }
  return TRUE;
}

