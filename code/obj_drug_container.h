//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_DRUG_CONTAINER_H
#define __OBJ_DRUG_CONTAINER_H

#include "obj.h"


class TDrugContainer : public TObj {
  protected:
    drugTypeT drugType;
    int maxBurn;
    int curBurn;
    bool lit;
  public:
    virtual void lookObj(TBeing *, int) const {}
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_DRUG_CONTAINER; }

    virtual bool isSimilar(const TThing *t) const;
    virtual void lightDecay();
    virtual int objectDecay();
    virtual void extinguishWater(TBeing *);
    virtual void extinguishWater();
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void lightMe(TBeing *, silentTypeT);
    virtual void extinguishMe(TBeing *);
    virtual bool monkRestrictedItem(const TBeing *) const;
    virtual bool shamanRestrictedItem(const TBeing *) const;
    virtual bool rangerRestrictedItem(const TBeing *) const;
    virtual void refuelMeLight(TBeing *, TThing *);
    virtual void peeOnMe(const TBeing *);

    void putLightOut();

    void addToMaxBurn(int n);
    void setMaxBurn(int n);
    int getMaxBurn() const;
    void addToCurBurn(int n);
    void setCurBurn(int n);
    int getCurBurn() const;
    void setLit(bool n);
    bool getLit() const;
    void setDrugType(drugTypeT n);
    drugTypeT getDrugType() const;

    TDrugContainer();
    TDrugContainer(const TDrugContainer &a);
    TDrugContainer & operator=(const TDrugContainer &a);
    virtual ~TDrugContainer();
};



#endif
