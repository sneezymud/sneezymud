//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_COOKWARE_H
#define __OBJ_COOKWARE_H

#include "obj_open_container.h"

// The actual item
class TCookware : public TOpenContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_COOKWARE; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void pourMeOut(TBeing *);
    virtual void pourMeIntoDrink2(TBeing *, TBaseCup *);
    virtual void pourMeIntoDrink1(TBeing *, TObj *);

    TCookware();
    TCookware(const TCookware &a);
    TCookware & operator=(const TCookware &a);
    virtual ~TCookware();
};


#endif
