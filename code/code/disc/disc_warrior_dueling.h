#pragma once

#include "discipline.h"
#include "skills.h"

class CDDueling : public CDiscipline {
  public:
    CSkill skShove;
    CSkill skRetreat;
    CSkill skParryWarrior;
    CSkill skTranceOfBlades;
    CSkill skWeaponRetention;
    CSkill skRiposte;

    virtual CDDueling* cloneMe() { return new CDDueling(*this); }

  private:
};

int shove(TBeing*, TBeing*, char*, spellNumT);
