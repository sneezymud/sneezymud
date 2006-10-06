//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_CLOTHING_H
#define __OBJ_BASE_CLOTHING_H

#include "obj.h"


class TBaseClothing : public virtual TObj
{
  private:
  public:
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual sstring statObjInfo() const = 0;

    virtual int editAverageMe(TBeing *, const char *);
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void objMenu(const TBeing *) const;
    virtual int rentCost() const;
    virtual void lowCheck();
    virtual void descMaxStruct(const TBeing *, int) const;
    virtual bool sellMeCheck(TBeing *, TMonster *, int) const;
    virtual void evaluateMe(TBeing *) const;
    virtual int scavengeMe(TBeing *, TObj **);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual int suggestedPrice() const;
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;
    virtual bool isPluralItem() const;
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);
    virtual int putMeInto(TBeing *, TOpenContainer *);

    int armorPriceStruct(armorLevT, double *) const;
    void setDefArmorLevel(float);
    double armorLevel(armorLevT) const;
    virtual double objLevel() const;
    void armorPercs(double *, double *) const;

    bool isSaddle() const;
    bool isHarness() const;
    bool isShield() const;
    bool isBarding() const;

protected:
    TBaseClothing();
public:
    TBaseClothing(const TBaseClothing &a);
    TBaseClothing & operator=(const TBaseClothing &a);
    virtual ~TBaseClothing();
};

#endif
