//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SPELLBAG_H
#define __OBJ_SPELLBAG_H

#include "obj_expandable_container.h"

// The actual item
class TSpellBag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SPELLBAG; }

    virtual void findSomeComponent(TComponent **, TComponent **, TComponent **, spellNumT, int);
    virtual bool allowsCast() { return true; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int componentSell(TBeing *, TMonster *, int, TThing *);
    virtual int componentValue(TBeing *, TMonster *, int, TThing *);
    virtual bool lowCheckSlots(silentTypeT);
    virtual void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);

    TSpellBag();
    TSpellBag(const TSpellBag &a);
    TSpellBag & operator=(const TSpellBag &a);
    virtual ~TSpellBag();
    virtual TThing & operator+= (TThing & t);
};



#endif
