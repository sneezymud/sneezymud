//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_CASINO_CHIP_H
#define __OBJ_CASINO_CHIP_H

#include "obj.h"

class TCasinoChip : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CASINO_CHIP; }

    TCasinoChip();
    TCasinoChip(const TCasinoChip &a);
    TCasinoChip & operator=(const TCasinoChip &a);
    virtual ~TCasinoChip();
};


#endif
