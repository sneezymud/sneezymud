//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DRUG_H
#define __DRUG_H

#include "obj.h"
#include "gametime.h"

enum drugTypeT {
     DRUG_NONE,
     DRUG_PIPEWEED,
     DRUG_OPIUM,
     DRUG_POT,
     DRUG_FROGSLIME,
     MAX_DRUG
};
const drugTypeT MIN_DRUG = drugTypeT(1);

class TDrugInfo
{
  public:
    const char *name;
    int potency; // max units of drug in body that takes affect
    int duration; 
    TDrugInfo(const char *n, int p, int d);
    TDrugInfo(const TDrugInfo &);
    TDrugInfo & operator=(const TDrugInfo &);
    ~TDrugInfo();

    TDrugInfo();  // not supposed to be called, but needed for vector
};
extern std::vector<TDrugInfo>drugTypes;

class drugData {
 public:
    struct time_info_data first_use;
    struct time_info_data last_use;
    unsigned int total_consumed;
    unsigned int current_consumed;

    drugData();
    ~drugData();
    drugData(const drugData &t);
    drugData & operator =(const drugData &t);

};

class TDrug : public TObj {
  private:
    int curFuel;
    int maxFuel;
    drugTypeT drugType;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual sstring statObjInfo() const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual void refuelMeDrug(TBeing *, TDrugContainer *);
    virtual itemTypeT itemType() const { return ITEM_DRUG; }
    virtual int getVolume() const;
    virtual float getTotalWeight(bool) const;

    void addToMaxFuel(int n);
    void setMaxFuel(int n);
    int getMaxFuel() const;
    void addToCurFuel(int n);
    void setCurFuel(int n);
    int getCurFuel() const;
    void setDrugType(drugTypeT n);
    drugTypeT getDrugType() const;

    TDrug();
    TDrug(const TDrug &a);
    TDrug & operator=(const TDrug &a);
    virtual ~TDrug();
};



void applyDrugAffects(TBeing *, drugTypeT, bool);
void applyAddictionAffects(TBeing *, drugTypeT, int);

#endif










