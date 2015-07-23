#ifndef __OBJ_APPLIED_SUB_H
#define __OBJ_APPLIED_SUB_H

#include "obj.h"

// stub object relating to ranger herbalism

class TASubstance : public TObj {
  public:
    virtual void assignFourValues(int, int, int, int) { }
    virtual void getFourValues(int *a, int *b, int *c, int *d) const { *a = *b = *c = *d = 0; }
    virtual sstring statObjInfo() const { return sstring(""); }

    virtual itemTypeT itemType() const {return ITEM_APPLIED_SUB; }

    TASubstance() { }
    TASubstance(const TASubstance &a) { }
    TASubstance & operator=(const TASubstance &a) {
      if (this == &a) return *this;
      TObj::operator=(a);
      return *this;
    }
    virtual ~TASubstance() { }
};

#endif
