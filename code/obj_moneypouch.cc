//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_moneypouch.h"

int TMoneypouch::getMoney() const
{
  TMoney *money;
  int total=0;

  for(TThing *t=getStuff();t;t=t->nextThing){
    if((money=dynamic_cast<TMoney *>(t)))
      total=money->getMoney();
  }

  return total;
}

TMoneypouch::TMoneypouch() :
  TExpandableContainer()
{
}

TMoneypouch::TMoneypouch(const TMoneypouch &a) :
  TExpandableContainer(a)
{
}

TMoneypouch & TMoneypouch::operator=(const TMoneypouch &a)
{
  if (this == &a) return *this;
  TExpandableContainer::operator=(a);
  return *this;
}

TMoneypouch::~TMoneypouch()
{
}

void TMoneypouch::assignFourValues(int x1, int x2, int x3, int x4)
{
  TExpandableContainer::assignFourValues(x1, x2, x3, x4);
}

void TMoneypouch::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  TExpandableContainer::getFourValues(x1, x2, x3, x4);
}

sstring TMoneypouch::statObjInfo() const
{
  return TExpandableContainer::statObjInfo();
}

bool TMoneypouch::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair moneypouches.");
  }
  return TRUE;
}
