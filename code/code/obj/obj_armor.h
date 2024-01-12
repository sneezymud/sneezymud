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

class TBeing;

class TArmor : public virtual TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARMOR; }

    virtual void lowCheck();
    virtual int galvanizeMe(TBeing*, short);

    TArmor();
    TArmor(const TArmor& a);
    TArmor& operator=(const TArmor& a);
    virtual ~TArmor();
};
