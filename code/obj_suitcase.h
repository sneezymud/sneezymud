//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SUITCASE_H
#define __OBJ_SUITCASE_H

#include "obj_expandable_container.h"

// The actual item
class TSuitcase : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SUITCASE; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TSuitcase();
    TSuitcase(const TSuitcase &a);
    TSuitcase & operator=(const TSuitcase &a);
    virtual ~TSuitcase();
};



#endif
