//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_WAGON_H
#define __OBJ_WAGON_H

#include "obj_open_container.h"

// The actual item
class TWagon : public TOpenContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WAGON; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TWagon();
    TWagon(const TWagon &a);
    TWagon & operator=(const TWagon &a);
    virtual ~TWagon();
};


#endif
