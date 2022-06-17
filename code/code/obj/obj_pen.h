//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"

class TPen : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual void junkMe(TBeing *) {}
    virtual itemTypeT itemType() const { return ITEM_PEN; }

    virtual void writeMePen(TBeing *, TThing *);

    TPen();
    TPen(const TPen &a);
    TPen & operator=(const TPen &a);
    virtual ~TPen();
};

