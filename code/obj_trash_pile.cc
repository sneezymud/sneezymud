#include "stdsneezy.h"
#include "obj_trash_pile.h"

TTrashPile::TTrashPile() :
  TExpandableContainer()
{
}

TTrashPile::TTrashPile(const TTrashPile &a) :
  TExpandableContainer(a)
{
}

TTrashPile & TTrashPile::operator=(const TTrashPile &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TTrashPile::~TTrashPile()
{
}

void TTrashPile::assignFourValues(int, int, int, int)
{
}

void TTrashPile::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TTrashPile::statObjInfo() const
{
  sstring a("");
  return a;
}

bool TTrashPile::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent)
    repair->doTell(fname(ch->name), fmt("I'm not the trash man. Take %s to the dump!") % getName());

  return TRUE;
}

