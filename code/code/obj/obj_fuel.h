//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"
#include "sstring.h"

class TBeing;
class TLight;
class TMonster;

class TFuel : public TObj {
  private:
    int curFuel;
    int maxFuel;

  public:
    virtual int chiMe(TBeing*);
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing*) const;
    virtual sstring statObjInfo() const;
    virtual int objectSell(TBeing*, TMonster*);
    virtual void refuelMeFuel(TBeing*, TLight*);
    virtual itemTypeT itemType() const { return ITEM_FUEL; }
    virtual int getVolume() const;
    virtual float getTotalWeight(bool) const;

    void addToMaxFuel(int n);
    void setMaxFuel(int n);
    int getMaxFuel() const;
    void addToCurFuel(int n);
    void setCurFuel(int n);
    int getCurFuel() const;

    TFuel();
    TFuel(const TFuel& a);
    TFuel& operator=(const TFuel& a);
    virtual ~TFuel();
};
