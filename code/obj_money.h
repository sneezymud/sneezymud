//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_MONEY_H
#define __OBJ_MONEY_H

#include "obj.h"

class TMoney : public TObj {
  private:
    int money;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_MONEY; }

    virtual int scavengeMe(TBeing *, TObj **);
    virtual int getMe(TBeing *, TThing *);
    virtual int moneyMeMoney(TBeing *, TThing *);
    virtual void logMe(const TBeing *, const char *) const {}
    virtual int rentCost() const;
    virtual void moneyMove(TBeing *);
    virtual bool canCarryMe(const TBeing *, silentTypeT) const;
    virtual bool isPluralItem() const;
    virtual void onObjLoad();
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;

    int getMoney() const;
    void setMoney(int n);

    TMoney();
    TMoney(const TMoney &a);
    TMoney & operator=(const TMoney &a);
    virtual ~TMoney();
};




#endif
