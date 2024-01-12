//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"
#include "sstring.h"

class TBeing;

class TCasinoChip : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CASINO_CHIP; }
    virtual int shopPrice(int, int, float, const TBeing*) const;
    virtual int sellPrice(int, int, float, const TBeing*);

    TCasinoChip();
    TCasinoChip(const TCasinoChip& a);
    TCasinoChip& operator=(const TCasinoChip& a);
    virtual ~TCasinoChip();
};
