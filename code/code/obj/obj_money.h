//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_MONEY_H
#define __OBJ_MONEY_H

#include <map>

#include "obj.h"
#include "obj_mergeable.h"


enum currencyTypeT {
  MIN_CURRENCY=0,
  CURRENCY_GRIMHAVEN=0,
  CURRENCY_LOGRUS,
  CURRENCY_BRIGHTMOON,
  CURRENCY_AMBER,
  MAX_CURRENCY
};


class TMoney : public TMergeable {
  private:
    int money;
    currencyTypeT type;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_MONEY; }

    virtual bool willMerge(TMergeable *);
    virtual void doMerge(TMergeable *);

    virtual int scavengeMe(TBeing *, TObj **);
    virtual int getMe(TBeing *, TThing *);
    virtual int moneyMeMoney(TBeing *, TThing *);
    virtual void logMe(const TBeing *, const char *) const {}
    virtual int rentCost() const;
    virtual void moneyMove(TBeing *);
    virtual bool canCarryMe(const TBeing *, silentTypeT) const;
    virtual bool isPluralItem() const;
    virtual void onObjLoad();
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;
    
    currencyTypeT getCurrency() const;
    void setCurrency(currencyTypeT);
    sstring getCurrencyName() const;

    int getMoney() const;
    void setMoney(int n);

    TMoney();
    TMoney(const TMoney &a);
    TMoney & operator=(const TMoney &a);
    virtual ~TMoney();
};


class currencyEntry {
  sstring name;         // eg, "talen"
  sstring affiliation;  // eg, "Grimhaven"

 public:  
  sstring getName(){return name;}
  sstring getAffiliation(){return affiliation;}
  float getExchangeRate(currencyTypeT);


  currencyEntry(sstring, sstring);
  currencyEntry & operator=(const currencyEntry &a);
  ~currencyEntry();
  
 private:
  currencyEntry();  // deny usage in this format
};


class currencyInfoT {
  std::map<currencyTypeT, currencyEntry *>currencies;

 public:
  currencyEntry *operator[] (const currencyTypeT);

  currencyInfoT();
  ~currencyInfoT();  
};

extern currencyTypeT & operator++(currencyTypeT &c, int);


#endif
