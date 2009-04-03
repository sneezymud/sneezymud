//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_FLAME_H
#define __OBJ_FLAME_H

#include "obj_base_light.h"

const unsigned int TFFLAME_INVHEAT  = (1 << 0); //  1
const unsigned int TFFLAME_INVLIGHT = (1 << 1); //  2
const unsigned int TFFLAME_MAGHEAT  = (1 << 2); //  4
const unsigned int TFFLAME_MAGLIGHT = (1 << 3); //  8
const unsigned int TFFLAME_IMMORTAL = (1 << 4); // 16

class TFFlame : public TBaseLight {
  private:
    int magBV;
  public:
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_FLAME; }

    virtual int    chiMe(TBeing *);
    virtual void   addFlameToMe(TBeing *, const char *, TThing *, bool);
    virtual void   peeOnMe(const TBeing *);
    virtual void   updateFlameInfo();
    virtual void   addFlameMessages();
    virtual void   decayMe();
    virtual int    objectDecay();
    virtual int    getMe(TBeing *, TThing *);
    virtual void   lightMe(TBeing *, silentTypeT);
    virtual void   extinguishMe(TBeing *);
    virtual int    pourWaterOnMe(TBeing *, TObj *);
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void   refuelMeLight(TBeing *, TThing *);
    virtual void   describeObjectSpecifics(const TBeing *) const;
    virtual void   assignFourValues(int, int, int, int);
    virtual bool   isLit() const;
    virtual void   getFourValues(int *, int *, int *, int *) const;
    virtual void   putLightOut();

    int  igniteMessage(TBeing *) const;
    void setMagBV(int);
    int  getMagBV() const;
    bool hasMagBV(int) const;
    void addMagBV(int);
    void remMagBV(int);

    TFFlame();
    TFFlame(const TFFlame &a);
    TFFlame & operator=(const TFFlame &a);
    virtual ~TFFlame();
};


#endif
