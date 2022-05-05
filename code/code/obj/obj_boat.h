//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"


class TBoat : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOAT; }

    virtual int  putSomethingInto(TBeing *, TThing *);
    virtual int  getObjFrom(TBeing *, const char *, const char *);
    virtual int  getLight() const;
    virtual void usingBoat(int *n);

    TBoat();
    TBoat(const TBoat &a);
    TBoat & operator=(const TBoat &a);
    virtual ~TBoat();
};


