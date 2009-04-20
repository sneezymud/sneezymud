//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_DRINKCON_H
#define __OBJ_DRINKCON_H

#include "obj_base_cup.h"


class TDrinkCon : public TBaseCup {
  public:
    virtual itemTypeT itemType() const { return ITEM_DRINKCON; }
    virtual void findSomeDrink(TDrinkCon **, TBaseContainer **, TBaseContainer *);
    virtual void waterCreate(const TBeing *, int);
    virtual int divineMe(TBeing *, int, short);

    TDrinkCon();
    TDrinkCon(const TDrinkCon &a);
    TDrinkCon & operator=(const TDrinkCon &a);
    virtual ~TDrinkCon();
};


#endif
