//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_GEMSTONE_H
#define __OBJ_GEMSTONE_H

#include "obj.h"


class TGemstone : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_GEMSTONE; }

    TGemstone();
    TGemstone(const TGemstone &a);
    TGemstone & operator=(const TGemstone &a);
    virtual ~TGemstone();
};


#endif
