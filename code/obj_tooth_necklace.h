//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TOOTH_NECKLACE_H
#define __OBJ_TOOTH_NECKLACE_H

#include "obj_expandable_container.h"

class TToothNecklace : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TOOTH_NECKLACE; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void describeObjectSpecifics(const TBeing *) const;

    void updateDesc();

    TToothNecklace();
    TToothNecklace(const TToothNecklace &a);
    TToothNecklace & operator=(const TToothNecklace &a);
    virtual ~TToothNecklace();
};



#endif
