//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "sstring.h"

class TBeing;
class TMonster;
class TOpenContainer;

class TKey : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_KEY; }
    virtual int putMeInto(TBeing*, TOpenContainer*);

    virtual void lowCheck();
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);
    virtual int stealModifier();

    TKey();
    TKey(const TKey& a);
    TKey& operator=(const TKey& a);
    virtual ~TKey();
};
