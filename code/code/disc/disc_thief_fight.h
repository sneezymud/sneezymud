#pragma once

#include "discipline.h"
#include "skills.h"

class CDThiefFight : public CDiscipline {
  public:
    CSkill skDodgeThief;
    CSkill skDualWieldThief;

    virtual CDThiefFight* cloneMe() { return new CDThiefFight(*this); }

  private:
};