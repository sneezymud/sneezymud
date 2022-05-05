//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"

class TBandage : public TObj {
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BANDAGE; }

    virtual int removeMe(TBeing *, wearSlotT);
    virtual void scrapMe(TBeing *);
    virtual void findBandage(int *);
    virtual void destroyBandage(int *);

    TBandage();
    TBandage(const TBandage &a);
    TBandage & operator=(const TBandage &a);
    virtual ~TBandage();
};


