//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"
#include "obj_base_clothing.h"
#include "sstring.h"

class TSaddle : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SADDLE; }

    virtual void lowCheck();

    TSaddle();
    TSaddle(const TSaddle& a);
    TSaddle& operator=(const TSaddle& a);
    virtual ~TSaddle();
};
