//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_JEWELRY_H
#define __OBJ_JEWELRY_H

#include "obj_base_clothing.h"

class TJewelry : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_JEWELRY; }

    virtual void lowCheck();

    TJewelry();
    TJewelry(const TJewelry &a);
    TJewelry & operator=(const TJewelry &a);
    virtual ~TJewelry();
};


#endif
