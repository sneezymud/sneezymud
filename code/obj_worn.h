//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_WORN_H
#define __OBJ_WORN_H

#include "obj_base_clothing.h"


class TWorn : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WORN; }

    virtual void lowCheck();

    TWorn();
    TWorn(const TWorn &a);
    TWorn & operator=(const TWorn &a);
    virtual ~TWorn();
};


#endif
