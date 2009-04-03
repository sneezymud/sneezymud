//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_CUP_H
#define __OBJ_BASE_CUP_H

#include "obj.h"




class TBaseCup : public virtual TObj {
  private:
    int maxDrinks;
    int curDrinks;
    liqTypeT liquidType;
    unsigned int drinkFlags;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void describeContains(const TBeing *) const;
    virtual void lowCheck();
    virtual bool waterSource();
    virtual sstring statObjInfo() const;
    void updateDesc();

    virtual int chiMe(TBeing *);
    virtual bool poisonObject();
    virtual int freezeObject(TBeing *, int);
    virtual int thawObject(TBeing *, int);
    virtual void nukeFood();
    virtual int drinkMe(TBeing *);
    virtual int quaffMe(TBeing *);
    virtual void sipMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual void pourMeOut(TBeing *);
    virtual void pourMeIntoDrink2(TBeing *, TBaseCup *);
    virtual void pourMeIntoDrink1(TBeing *, TObj *);
    virtual void spill(const TBeing *);
    virtual void fillMe(const TBeing *, liqTypeT);
    virtual void weightCorrection();
    virtual void evaporate(TBeing *, silentTypeT);
    virtual void weightChangeObject(float);
    virtual void setEmpty();
    virtual void lookObj(TBeing *, int) const;
    virtual void examineObj(TBeing *) const;
    virtual void peeMe(const TBeing *, liqTypeT);
    virtual int getReducedVolume(const TThing *) const;
    virtual int poisonMePoison(TBeing *, TBaseWeapon *);
    sstring showModifier(showModeT, const TBeing *) const;

    void genericEmpty();
    unsigned int getDrinkConFlags() const;
    void setDrinkConFlags(unsigned int r);
    bool isDrinkConFlag(unsigned int r) const;
    void addDrinkConFlags(unsigned int r);
    void remDrinkConFlags(unsigned int r);
    int getMaxDrinkUnits() const;
    void setMaxDrinkUnits(int n);
    void addToMaxDrinkUnits(int n);
    int getDrinkUnits() const;
    virtual void setDrinkUnits(int n);
    virtual void addToDrinkUnits(int n);
    liqTypeT getDrinkType() const;
    virtual void setDrinkType(liqTypeT n);
    int getLiqDrunk() const;
    int getLiqHunger() const;
    int getLiqThirst() const;

    TBaseCup();
    TBaseCup(const TBaseCup &a);
    TBaseCup & operator=(const TBaseCup &a);
    virtual ~TBaseCup();
};



#endif
