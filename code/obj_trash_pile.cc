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

void TTrashPile::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TTrashPile::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
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

void TTrashPile::updateDesc()
{
  const char *pilename [] =
  {
    "<o>a tiny pile of trash<1>", 
    "<o>a small pile of trash<1>", 
    "<o>a pile of trash<1>", 
    "<o>a fair sized pile of trash<1>", 
    "<o>a big pile of trash<1>", 
    "<o>a large pile of trash<1>", 
    "<o>a huge pile of trash<1>",
    "<o>a massive pile of trash<1>",
    "<o>a tremendously huge pile of trash<1>",
    "<o>a veritable sea of trash<1>"
  };
  
  const char *piledesc [] =
  {
    "<o>A tiny pile of trash has gathered here.<1>",
    "<o>A small pile of trash is here.<1>",
    "<o>A pile of trash is here.<1>",
    "<o>A fair sized pile of trash is here.<1>",
    "<o>A big pile of trash is here.<1>",
    "<o>A large pile of trash is here.<1>",
    "<o>A huge pile of trash is here.<1>",
    "<o>A massive pile of trash is here.<1>",
    "<o>A tremendously huge pile of trash dominates the area.<1>",
    "<o>A veritable sea of trash covers the area.<1>"
  };


  int vol=getTotalVolume();
  int step=46656/9;
  int index=0;

  if(vol < step*1)
    index=0;
  else if(vol < step*2)
    index=1;
  else if(vol < step*3)
    index=2;
  else if(vol < step*4)
    index=3;
  else if(vol < step*5)
    index=4;
  else if(vol < step*6)
    index=5;
  else if(vol < step*7)
    index=6;
  else if(vol < step*8)
    index=7;
  else if(vol < step*9)
    index=8;
  else
    index=9;


  if (isObjStat(ITEM_STRUNG)) {
    delete [] shortDescr;
    delete [] descr;

    extraDescription *exd;
    while ((exd = ex_description)) {
      ex_description = exd->next;
      delete exd;
    }
    ex_description = NULL;
    delete [] action_description;
    action_description = NULL;
  } else {
    addObjStat(ITEM_STRUNG);
    name=mud_str_dup(obj_index[getItemIndex()].name);
    ex_description = NULL;
    action_description = NULL;
  }

  shortDescr = mud_str_dup(pilename[index]);
  setDescr(mud_str_dup(piledesc[index]));
}


