//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_POISON_H
#define __OBJ_POISON_H

#include "obj_base_cup.h"


class TPoison : public TBaseCup {
  public:
    virtual itemTypeT itemType() const { return ITEM_POISON; }
    virtual int shopPrice(int, int, float, const TBeing *) const;
    virtual int sellPrice(int, int, float, const TBeing *);
    virtual int objectSell(TBeing *, TMonster *);


    TPoison();
    TPoison(const TPoison &a);
    TPoison & operator=(const TPoison &a);
    virtual ~TPoison();
};


#endif
