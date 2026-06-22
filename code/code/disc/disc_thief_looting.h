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
// Salvage trap components on a successful disarm. Pass the TTrap for
// mines/grenades (enables type variants + casing recovery); pass nullptr for
// flag-based traps (doors, containers).
bool reclaimTrapComps(TBeing*, sstring, TTrap*);

int detectTrapObj(TBeing*, const TThing*);
int detectTrapDoor(TBeing*, int);