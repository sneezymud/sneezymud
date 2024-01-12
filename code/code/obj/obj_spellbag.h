//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_expandable_container.h"
#include "spells.h"
#include "sstring.h"

class TBeing;
class TComponent;
class TMonster;
class TThing;

// The actual item
class TSpellBag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SPELLBAG; }

    virtual void findSomeComponent(TComponent**, TComponent**, TComponent**,
      spellNumT, int);
    virtual bool allowsCast() { return true; }
    virtual void putMoneyInto(TBeing*, int);
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);
    virtual int componentSell(TBeing*, TMonster*, int, TThing*);
    virtual int componentValue(TBeing*, TMonster*, int, TThing*);
    virtual bool lowCheckSlots(silentTypeT);
    virtual void getObjFromMeText(TBeing*, TThing*, getTypeT, bool);

    TSpellBag();
    TSpellBag(const TSpellBag& a);
    TSpellBag& operator=(const TSpellBag& a);
    virtual ~TSpellBag();
    virtual TThing& operator+=(TThing& t);
};
