#pragma once

#include "discipline.h"
#include "skills.h"

class CDMounted : public CDiscipline {
  public:
    CSkill skCalmMount;
    CSkill skTrainMount;
    CSkill skAdvancedRiding;
    CSkill skRideDomestic;
    CSkill skRideNonDomestic;
    CSkill skRideWinged;
    CSkill skRideExotic;

    virtual CDMounted* cloneMe() { return new CDMounted(*this); }

  private:
};
