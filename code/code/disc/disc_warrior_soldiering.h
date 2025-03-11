#pragma once

#include "discipline.h"
#include "skills.h"

class CDSoldiering : public CDiscipline {
  public:
    CSkill skDoorbash;
    CSkill skDualWieldWarrior;
    CSkill skPowerMove;
    CSkill skDeathstroke;
    CSkill sk2hSpecWarrior;
    CSkill skFortify;

    virtual CDSoldiering* cloneMe() { return new CDSoldiering(*this); }

  private:
};
