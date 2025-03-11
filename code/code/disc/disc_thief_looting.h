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

int detectTrapObj(TBeing*, const TThing*);
int detectTrapDoor(TBeing*, int);