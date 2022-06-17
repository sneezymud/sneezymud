//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"

class TTrash : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRASH; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TTrash();
    TTrash(const TTrash &a);
    TTrash & operator=(const TTrash &a);
    virtual ~TTrash();
};


