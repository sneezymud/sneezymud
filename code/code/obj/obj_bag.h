//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BAG_H
#define __OBJ_BAG_H

#include "obj_expandable_container.h"

// The actual item
class TBag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BAG; }

    virtual int getMe(TBeing *, TThing *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TBag();
    TBag(const TBag &a);
    TBag & operator=(const TBag &a);
    virtual ~TBag();
};

#endif
