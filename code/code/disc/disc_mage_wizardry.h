#pragma once

#include "discipline.h"
#include "skills.h"

class CDWizardry : public CDiscipline {
  public:
    CSkill skWizardry;

    virtual CDWizardry* cloneMe() { return new CDWizardry(*this); }

    bool isAutomatic() { return true; }

  private:
};
