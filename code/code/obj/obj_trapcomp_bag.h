//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj_expandable_container.h"

class TTrapComponent;

// A container that holds only trap components (ITEM_TRAP_COMPONENT). Its
// contents count when building traps: TBeing::findTrapComp descends into any
// TTrapCompBag carried in inventory. Modeled on TSpellBag.
class TTrapCompBag : public TExpandableContainer {
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRAPCOMP_BAG; }

    // Reject anything that isn't a trap component.
    virtual int putSomethingInto(TBeing*, TThing*);
    virtual bool lowCheckSlots(silentTypeT);

    // "evaluate <bag>": report how many traps of each type the held
    // components could build (skill-gated by SKILL_EVALUATE).
    virtual void evaluateMe(TBeing*) const;

    // Find a held component matching name (get_number-style, e.g. "2.spring").
    TThing* findComponent(const TBeing*, const sstring& name);

    TTrapCompBag();
    TTrapCompBag(const TTrapCompBag& a);
    TTrapCompBag& operator=(const TTrapCompBag& a);
    virtual ~TTrapCompBag();
};
