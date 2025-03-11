#pragma once

#include "discipline.h"
#include "skills.h"

class CDPlants : public CDiscipline {
  public:
    CSkill skApplyHerbs;

    virtual CDPlants* cloneMe() { return new CDPlants(*this); }

  private:
};
