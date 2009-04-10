//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_POOL_H
#define __OBJ_POOL_H

#include "obj_base_cup.h"
#include "obj_mergeable.h"

class TPool : public TBaseCup, public TMergeable {
  public:    
    void fillMeAmount(int, liqTypeT);
    void updateDesc();

    virtual void setDrinkUnits(int);    
    virtual void addToDrinkUnits(int);    
    virtual void decayMe();
    virtual void setDrinkType(liqTypeT);
    virtual void weightChangeObject(float);
    virtual void peeMe(const TBeing *, liqTypeT);
    virtual int freezeObject(TBeing *, int);
    virtual int thawObject(TBeing *, int);
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_POOL; }
    void overFlow();

    virtual bool willMerge(TMergeable *);
    virtual void doMerge(TMergeable *);

    void initPool(int, liqTypeT);

    int getDrinkIndex() const;

    TPool();
    TPool(const TPool &a);
    TPool & operator=(const TPool &a);
    virtual ~TPool();
};



#endif
