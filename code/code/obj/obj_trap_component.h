//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj_mergeable.h"

class TTrapComponent : public TMergeable {
  private:
    int charges;
    int trap_comp_type;  // which trap types this component can be used for

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual itemTypeT itemType() const { return ITEM_TRAP_COMPONENT; }
    virtual sstring statObjInfo() const;
    virtual sstring getNameForShow(bool, bool, const TBeing*) const;
    virtual void purchaseMe(TBeing*, TMonster*, int, int);
    virtual void sellMeMoney(TBeing*, TMonster*, int, int);
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);
    virtual void lowCheck();
    virtual void evaluateMe(TBeing*) const;
    virtual void changeObjValue4(TBeing*);
    virtual void boottimeInit();
    virtual void update(int);
    virtual void describeObjectSpecifics(const TBeing*) const;
    virtual void decayMe();
    virtual bool sellMeCheck(TBeing*, TMonster*, int, int) const;
    virtual int trapComponentSell(TBeing*, TMonster*, int, TThing*);
    virtual int trapComponentNumSell(TBeing*, TMonster*, int, TThing*, int);
    virtual int trapComponentValue(TBeing*, TMonster*, int, TThing*);
    virtual int trapComponentNumValue(TBeing*, TMonster*, int, TThing*, int);
    virtual bool splitMe(TBeing*, const sstring&);
    virtual int suggestedPrice() const;
    virtual void objMenu(const TBeing*) const;
    double priceMultiplier() const;
    virtual int sellMe(TBeing* ch, TMonster* tKeeper, int tShop, int num = 1);
    virtual int buyMe(TBeing*, TMonster*, int, int);
    virtual void valueMe(TBeing* ch, TMonster* keeper, int shop_nr, int num = 1);
    virtual int shopPrice(int, int, float, const TBeing*) const;
    virtual int sellPrice(int, int, float, const TBeing*);

    virtual bool willMerge(TMergeable*);
    virtual void doMerge(TMergeable*);

    int getTrapComponentCharges() const;
    void setTrapComponentCharges(int n);
    void addToTrapComponentCharges(int n);
    int getTrapComponentType() const;
    void setTrapComponentType(int n);
    void addTrapComponentType(int n);
    void remTrapComponentType(int n);
    bool isTrapComponentType(int n) const;
    int pricePerUnit() const;

    virtual TThingKind getKind() const;
    TTrapComponent();
    TTrapComponent(const TTrapComponent& a);
    TTrapComponent& operator=(const TTrapComponent& a);
    virtual ~TTrapComponent();
    virtual TThing& operator--();
};
