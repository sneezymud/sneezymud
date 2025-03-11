#pragma once

#include "discipline.h"
#include "skills.h"

class CDPoisons : public CDiscipline {
  public:
    CSkill skPoisonWeapons;

    virtual CDPoisons* cloneMe() { return new CDPoisons(*this); }

  private:
};