//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_ORGANIC_H
#define __OBJ_ORGANIC_H

#include "obj.h"

class TOrganic : public TObj {
  private:
    organicTypeT OCType;
    int TUnits;
    int OLevel;
    int TAEffect;
  public:
    virtual void   assignFourValues(int, int, int, int);
    virtual void   getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const {return ITEM_RAW_ORGANIC; }

    virtual int  chiMe(TBeing *);
    virtual int  objectSell(TBeing *, TMonster *);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool splitMe(TBeing *, const sstring &);
    virtual void lightMe(TBeing *, silentTypeT);
    virtual int  sellPrice(int, int, float, const TBeing *);
    virtual int  shopPrice(int, int, float, const TBeing *) const;
    virtual int buyMe(TBeing *, TMonster *, int, int);
    virtual void sellMe(TBeing *ch, TMonster *, int, int);
    virtual void valueMe(TBeing *, TMonster *, int, int);
    virtual const sstring shopList(const TBeing *, const sstring &, int, int, int, int, int, unsigned long int) const;
    virtual int sellHidenSkin(TBeing *, TMonster *, int, TThing *);
    void setOType(organicTypeT);
    organicTypeT getOType() const;
    void setUnits(int);
    int  getUnits() const;
    void setOLevel(int);
    int  getOLevel() const;
    void setAEffect(int);
    int  getAEffect() const;

    TOrganic();
    TOrganic(const TOrganic &a);
    TOrganic & operator=(const TOrganic &a);
    virtual ~TOrganic();
};


#endif
