//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_POTION_H
#define __OBJ_POTION_H

#include "obj_magic_item.h"


class TPotion : public TMagicItem {
  private:
    spellNumT spells[3];
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_POTION; }

    virtual void descMagicSpells(TBeing *) const;
    virtual int changeItemVal2Check(TBeing *, int);
    virtual int changeItemVal3Check(TBeing *, int);
    virtual int changeItemVal4Check(TBeing *, int);
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void divinateMe(TBeing *) const;
    virtual int quaffMe(TBeing *);
    virtual int drinkMe(TBeing *);
    virtual bool sellMeCheck(TBeing *, TMonster *) const;
    virtual int suggestedPrice() const;

    spellNumT getSpell(int num) const;
    void setSpell(int num, spellNumT xx);

    TPotion();
    TPotion(const TPotion &a);
    TPotion & operator=(const TPotion &a);
    virtual ~TPotion();
};



#endif
