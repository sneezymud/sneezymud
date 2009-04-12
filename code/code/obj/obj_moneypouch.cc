//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "monster.h"
#include "obj_moneypouch.h"
#include "obj_money.h"

int TMoneypouch::getMoney(currencyTypeT c) const
{
  TMoney *money;
  int currency[MAX_CURRENCY];
  int total=0;

  for(int i=0;i<MAX_CURRENCY;++i)
    currency[i]=0;

  for(StuffIter it=stuff.begin();it!=stuff.end();++it){
    if((money=dynamic_cast<TMoney *>(*it)))
      currency[money->getCurrency()]+=money->getMoney();
  }
  
  // MAX_CURRENCY specified, so return total amounted converted to talens
  if(c==MAX_CURRENCY){
    for(int i=0;i<MAX_CURRENCY;++i)
      total += (int)((float)currency[i] * 
	 currencyInfo[(currencyTypeT)i]->getExchangeRate(CURRENCY_GRIMHAVEN));
  } else {
    total=currency[c];
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
  sstring buf;
  int total=0;

  for(currencyTypeT c=MIN_CURRENCY;c<MAX_CURRENCY;c++){
    if(getMoney(c) > 0){
      buf += format("%ss inside: %i\n\r") % currencyInfo[c]->getName().cap() % 
	getMoney(c);
      total += (int)((float)getMoney(c) * currencyInfo[c]->getExchangeRate(CURRENCY_GRIMHAVEN));
    }
  }
  
  buf += format("Total (in talens): %i\n\r") % total;

  buf = buf + TExpandableContainer::statObjInfo();

  return buf;
}

bool TMoneypouch::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->name), "I can't repair moneypouches.");
  }
  return TRUE;
}
