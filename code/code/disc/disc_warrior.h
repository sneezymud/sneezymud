#pragma once

#include "discipline.h"
#include "skills.h"

class CDWarrior : public CDiscipline {
  public:
    CSkill skSlam;
    CSkill skBash;
    CSkill skFocusAttack;
    CSkill skRescue;
    CSkill skBlacksmithing;
    CSkill skDisarm;
    CSkill skBerserk;
    CSkill skSwitch;
    CSkill skBloodlust;
    CSkill skWhirlwind;
    CSkill skRally;

    virtual CDWarrior* cloneMe() { return new CDWarrior(*this); }

    bool isBasic() { return true; }

  private:
};