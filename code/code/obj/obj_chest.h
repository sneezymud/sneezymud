//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_open_container.h"
#include "sstring.h"

class TBeing;
class TMonster;

// The actual item
class TChest : public TOpenContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CHEST; }
    virtual void lowCheck();

    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);

    TChest();
    TChest(const TChest& a);
    TChest& operator=(const TChest& a);
    virtual ~TChest();
};
