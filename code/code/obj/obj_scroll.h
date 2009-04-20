//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SCROLL_H
#define __OBJ_SCROLL_H

#include "obj_magic_item.h"

class TScroll : public TMagicItem {
  private:
    spellNumT spells[3];
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SCROLL; }

    virtual void descMagicSpells(TBeing *) const;
    virtual int copyMe(TBeing *, short);
    virtual int changeItemVal2Check(TBeing *, int);
    virtual int changeItemVal3Check(TBeing *, int);
    virtual int changeItemVal4Check(TBeing *, int);
    virtual int reciteMe(TBeing *, const char *);
    virtual void divinateMe(TBeing *) const;
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int suggestedPrice() const;

    spellNumT getSpell(int num) const;
    void setSpell(int num, spellNumT xx);

    TScroll();
    TScroll(const TScroll &a);
    TScroll & operator=(const TScroll &a);
    virtual ~TScroll();
};



#endif
