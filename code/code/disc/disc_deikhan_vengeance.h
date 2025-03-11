#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanVengeance : public CDiscipline {
  public:
    CSkill skHarmDeikhan;
    CSkill skNumbDeikhan;
    CSkill skSmite;
    CSkill skHolyLight;  // not coded
    CSkill skAuraVengeance;

    virtual CDDeikhanVengeance* cloneMe() {
      return new CDDeikhanVengeance(*this);
    }
};
