//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_STAFF_H
#define __OBJ_STAFF_H

#include "obj_magic_item.h"

class TStaff : public TMagicItem {
  private:
    int maxCharges;
    int curCharges;
    spellNumT spell;
    
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_STAFF; }

    virtual int changeItemVal4Check(TBeing *, int);
    virtual void descMagicSpells(TBeing *) const;
    virtual void divinateMe(TBeing *) const;
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;
    virtual int useMe(TBeing *, const char *);
    virtual int objectSell(TBeing *, TMonster *);
    virtual int foodItemUsed(TBeing *ch, const char *arg);
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int rentCost() const;
    virtual int suggestedPrice() const;

    virtual int taskChargeMe(TBeing *, spellNumT, int &);
    virtual void taskChargeMeUpdate(TBeing *, spellNumT);

    void setMaxCharges(int n);
    int getMaxCharges() const;
    void addToMaxCharges(int n);
    void setCurCharges(int n);
    int getCurCharges() const;
    void addToCurCharges(int n);
    void setSpell(spellNumT n);
    spellNumT getSpell() const;

    TStaff();
    TStaff(const TStaff &a);
    TStaff & operator=(const TStaff &a);
    virtual ~TStaff();
};


#endif
