
//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_LIGHT_H
#define __OBJ_LIGHT_H

#include "obj_base_light.h"

class TLight : public TBaseLight {
  protected:
    bool lit;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_LIGHT; }

    virtual int chiMe(TBeing *);
    virtual int illuminateMe(TBeing *, int, byte);
    virtual void refuelMeLight(TBeing *, TThing*);
    virtual int objectDecay();
    virtual void unequipMe(TBeing *);
    virtual void extinguishWater(TBeing *);
    virtual void extinguishWater();
    virtual void lampLightStuff(TMonster *);
    virtual void lightDecay();
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void adjustLight();
    virtual int getMe(TBeing *, TThing *);
    virtual bool isSimilar(const TThing *t) const;
    virtual void peeOnMe(const TBeing *);
    virtual void lightMe(TBeing *, silentTypeT);
    virtual void extinguishMe(TBeing *);
    virtual void putLightOut();

    void genericExtinguish(const TBeing *);
    void setLit(bool n);
    virtual bool isLit() const;

    TLight();
    TLight(const TLight &a);
    TLight & operator=(const TLight &a);
    virtual ~TLight();
};


#endif
