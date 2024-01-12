//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "liquids.h"
#include "obj.h"
#include "obj_base_cup.h"
#include "spells.h"

class TBeing;
class TMonster;
class TThing;

class TPotion : public TBaseCup {
  public:
    virtual itemTypeT itemType() const { return ITEM_POTION; }
    virtual int shopPrice(int, int, float, const TBeing*) const;
    virtual int sellPrice(int, int, float, const TBeing*);
    virtual int getValue() const;
    virtual int objectSell(TBeing*, TMonster*);
    virtual bool isSimilar(const TThing*) const;
    virtual bool potIsEmpty() const;

    TPotion();
    TPotion(const TPotion& a);
    TPotion& operator=(const TPotion& a);
    virtual ~TPotion();
};

liqTypeT spell_to_liq(spellNumT which);
