//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_expandable_container.h"
#include "sstring.h"

class TBeing;
class TMonster;

// The actual item
class TSuitcase : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SUITCASE; }
    virtual void putMoneyInto(TBeing*, int);
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);

    TSuitcase();
    TSuitcase(const TSuitcase& a);
    TSuitcase& operator=(const TSuitcase& a);
    virtual ~TSuitcase();
};
