//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_LIGHT_H
#define __OBJ_BASE_LIGHT_H

#include "obj.h"

class TBaseLight : public TObj {
  protected:
    int amtLight;
    int maxBurn;
    int curBurn;
  public:
    virtual void lookObj(TBeing *, int) const {}
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;

    virtual void lightMe(TBeing *, silentTypeT) = 0;
    virtual bool monkRestrictedItem(const TBeing *) const;
    virtual bool shamanRestrictedItem(const TBeing *) const;
    virtual bool rangerRestrictedItem(const TBeing *) const;
    virtual void putLightOut();
    virtual sstring compareMeAgainst(TBeing *, TObj *);

    void addToLightAmt(int n);
    void setLightAmt(int n);
    int getLightAmt() const;
    void addToMaxBurn(int n);
    void setMaxBurn(int n);
    int getMaxBurn() const;
    void addToCurBurn(int n);
    void setCurBurn(int n);
    int getCurBurn() const;
    virtual bool isLit() const = 0;

    TBaseLight();
    TBaseLight(const TBaseLight &a);
    TBaseLight & operator=(const TBaseLight &a);
    virtual ~TBaseLight();
};


#endif
