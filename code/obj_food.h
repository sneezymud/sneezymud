//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_FOOD_H
#define __OBJ_FOOD_H

#include "obj.h"

class TFood : public TObj {
  private:
    unsigned int foodFlags;
    int foodFill;
  public:
    virtual int chiMe(TBeing *);
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int objectDecay();
    virtual void describeCondition(const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void lowCheck();
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_FOOD; }
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual int suggestedPrice() const;

    virtual bool poisonObject();
    virtual void nukeFood();
    virtual void findSomeFood(TFood **, TBaseContainer **, TBaseContainer *);
    virtual void eatMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);

    int getFoodFill() const;
    void setFoodFill(int r);
    unsigned int getFoodFlags() const;
    void setFoodFlags(unsigned int r);
    bool isFoodFlag(unsigned int r) const;
    void addFoodFlags(unsigned int r);
    void remFoodFlags(unsigned int r);

    TFood();
    TFood(const TFood &a);
    TFood & operator=(const TFood &a);
    virtual ~TFood();

};



#endif
