//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_COMMODITY_H
#define __OBJ_COMMODITY_H

#include "obj_mergeable.h"

class TCommodity : public TMergeable {
  private:
    bool disableMerge;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_RAW_MATERIAL; }

    virtual bool willMerge(TMergeable *);
    virtual void doMerge(TMergeable *);
    static float demandCurvePrice(int, float, int);

    virtual void updateDesc();
    int getSizeIndex() const;
    virtual int suggestedPrice() const;
    virtual void setWeight(const float);
    virtual void setMaterial(ubyte num);
    virtual bool splitMe(TBeing *, const sstring &);

    virtual void lowCheck();
    virtual void logMe(const TBeing *, const char *) const {}
    virtual int buyMe(TBeing *, TMonster *, int, int);
    virtual int sellMe(TBeing *, TMonster *, int, int);
    virtual bool sellMeCheck(TBeing *, TMonster *, int) const;
    virtual int sellCommod(TBeing *, TMonster *, int, TThing *);
    virtual void valueMe(TBeing *, TMonster *, int, int);
    virtual const sstring shopList(const TBeing *, const sstring &, int, int, int, int, int, unsigned long int) const;
    virtual float shopPriceFloat(int, int, float, const TBeing *) const;
    virtual int shopPrice(int, int, float, const TBeing *) const;
    virtual int sellPrice(int, int, float, const TBeing *);

    float pricePerUnit() const;
    int numUnits() const;

    TCommodity();
    TCommodity(const TCommodity &a);
    TCommodity & operator=(const TCommodity &a);
    virtual ~TCommodity();
};


#endif
