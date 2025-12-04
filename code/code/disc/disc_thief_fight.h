#pragma once

#include "discipline.h"
#include "skills.h"

class CDThiefFight : public CDiscipline {
  public:
    CSkill skDodgeThief;
    CSkill skDualWieldThief;
    CSkill skGarrotte;

    virtual CDThiefFight* cloneMe() { return new CDThiefFight(*this); }

  private:
};