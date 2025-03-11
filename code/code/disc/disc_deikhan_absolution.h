#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanAbsolution : public CDiscipline {
  public:
    CSkill skSalveDeikhan;
    CSkill skLayHands;
    CSkill skHeroesFeastDeikhan;
    CSkill skRefreshDeikhan;
    CSkill skAuraAbsolution;

    virtual CDDeikhanAbsolution* cloneMe() {
      return new CDDeikhanAbsolution(*this);
    }

  private:
};