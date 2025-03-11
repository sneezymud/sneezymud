#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanGuardian : public CDiscipline {
  public:
    CSkill skSynostodweomer;
    CSkill skDivineGrace;
    CSkill skDivineRescue;
    CSkill skGuardiansLight;
    CSkill skAuraGuardian;

    virtual CDDeikhanGuardian* cloneMe() {
      return new CDDeikhanGuardian(*this);
    }
};
