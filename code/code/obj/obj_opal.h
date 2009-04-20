//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_OPAL_H
#define __OBJ_OPAL_H

#include "obj.h"


class TOpal : public TObj {
  private:
    int psSize;
    int psStrength;
    int psMana;
    int psFails;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual sstring statObjInfo() const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void powerstoneCheck(TOpal **);
    virtual int powerstoneMe(TBeing *, int, short);
    virtual itemTypeT itemType() const { return ITEM_OPAL; }
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual int suggestedPrice() const;
    virtual void lowCheck();

    int psGetStrength() const;
    void psSetStrength(int num);
    void psAddStrength(int num);
    int psGetConsecFails() const;
    void psSetConsecFails(int num);
    void psAddConsecFails(int num);
    int psGetMaxMana() const;
    int psGetCarats() const;
    void psSetCarats(int num);

    TOpal();
    TOpal(const TOpal &a);
    TOpal & operator=(const TOpal &a);
    virtual ~TOpal();
};



#endif
