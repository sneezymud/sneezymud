//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_QUIVER_H
#define __OBJ_QUIVER_H

#include "obj_expandable_container.h"

class TQuiver : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_QUIVER; }
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void putMoneyInto(TBeing *, int);
    virtual void closeMe(TBeing *);
    virtual int  openMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);

    TQuiver();
    TQuiver(const TQuiver &a);
    TQuiver & operator=(const TQuiver &a);
    virtual ~TQuiver();
};


#endif
