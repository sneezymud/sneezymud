#pragma once

#include "obj.h"

// stub object relating to ranger herbalism

class TASubstance : public TObj {
  public:
    virtual void assignFourValues(int, int, int, int) { }
    virtual void getFourValues(int *a, int *b, int *c, int *d) const { *a = *b = *c = *d = 0; }
    virtual sstring statObjInfo() const { return sstring(""); }
    virtual itemTypeT itemType() const {return ITEM_APPLIED_SUB; }
};
