//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_MONEYPOUCH_H
#define __OBJ_MONEYPOUCH_H

#include "obj_expandable_container.h"
#include "obj_money.h"

class TMoneypouch : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_MONEYPOUCH; }
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    int getMoney(currencyTypeT c=MAX_CURRENCY) const;

    TMoneypouch();
    TMoneypouch(const TMoneypouch &a);
    TMoneypouch & operator=(const TMoneypouch &a);
    virtual ~TMoneypouch();
};



#endif
