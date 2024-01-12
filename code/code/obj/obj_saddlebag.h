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
class TThing;

// The actual item
class TSaddlebag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SADDLEBAG; }

    virtual int getMe(TBeing*, TThing*);
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);

    TSaddlebag();
    TSaddlebag(const TSaddlebag& a);
    TSaddlebag& operator=(const TSaddlebag& a);
    virtual ~TSaddlebag();
};
