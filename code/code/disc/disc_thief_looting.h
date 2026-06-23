#pragma once

#include "discipline.h"
#include "skills.h"

class CDLooting : public CDiscipline {
  public:
    CSkill skCounterSteal;
    CSkill skPlant;

    virtual CDLooting* cloneMe() { return new CDLooting(*this); }

  private:
};

int detectSecret(TBeing*);

int disarmTrapObj(TBeing*, TObj*);
int disarmTrapDoor(TBeing*, dirTypeT);

class TTrap;
// Salvage trap components on a successful disarm. `targ` selects the recipe for
// the target the trap was set against (door/cont/mine/grenade). Pass the TTrap
// for mines/grenades (yields the casing); pass nullptr for flag-based traps
// (doors, containers, portals).
bool reclaimTrapComps(TBeing*, sstring, trap_targ_t, TTrap*);

int detectTrapObj(TBeing*, const TThing*);
int detectTrapDoor(TBeing*, int);