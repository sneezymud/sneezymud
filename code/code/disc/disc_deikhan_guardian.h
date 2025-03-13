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

int synostodweomer(TBeing* caster, TBeing* victim);
int synostodweomer(TBeing* caster, TBeing* victim, int, short);
void divineRescue(TBeing* caster, TBeing* victim);
