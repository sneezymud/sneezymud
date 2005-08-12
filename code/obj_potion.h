//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_POTION_H
#define __OBJ_POTION_H

#include "obj_base_cup.h"


class TPotion : public TBaseCup {
  public:
    virtual itemTypeT itemType() const { return ITEM_POTION; }
    virtual int shopPrice(int, int, float, const TBeing *) const;
    virtual int sellPrice(int, int, float, const TBeing *);
    virtual int getValue() const; 
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool isSimilar(const TThing *) const;
    virtual bool potIsEmpty() const;

    TPotion();
    TPotion(const TPotion &a);
    TPotion & operator=(const TPotion &a);
    virtual ~TPotion();
};


liqTypeT spell_to_liq(spellNumT which);

#endif
