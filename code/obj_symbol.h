//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SYMBOL_H
#define __OBJ_SYMBOL_H

#include "obj.h"

class TSymbol : public TObj {
  private:
    int strength;
    int max_strength;
    factionTypeT faction;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_HOLY_SYM; }

    virtual int chiMe(TBeing *);
    virtual bool lowCheckSlots(silentTypeT);
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual int getShopPrice(int *) const;
    virtual void recalcShopData(int, int);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool allowsCast() { return true; }
    virtual void findSym(TSymbol **);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void lowCheck();
    virtual void attuneMe(TBeing *, TVial *);
    virtual void attunePulse(TBeing *);
    virtual void attunerValue(TBeing *, TMonster *);
    virtual void attunerGiven(TBeing *, TMonster *);
    virtual int suggestedPrice() const;
    virtual void objMenu(const TBeing *) const;
    double getSymbolLevel() const;
    virtual double objLevel() const;
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual string getNameForShow(bool, bool, const TBeing *) const;

    int getSymbolCurStrength() const;
    void setSymbolCurStrength(int r);
    void addToSymbolCurStrength(int r);
    int getSymbolMaxStrength() const;
    void setSymbolMaxStrength(int r);
    void addToSymbolMaxStrength(int r);
    factionTypeT getSymbolFaction() const;
    void setSymbolFaction(factionTypeT r);

    TSymbol();
    TSymbol(const TSymbol &a);
    TSymbol & operator=(const TSymbol &a);
    virtual ~TSymbol();
};


#endif
