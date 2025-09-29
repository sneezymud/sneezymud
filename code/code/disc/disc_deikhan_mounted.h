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
    CSkill skSaddlePosture;
    CSkill skVaulting;

    virtual CDMounted* cloneMe() { return new CDMounted(*this); }

  private:
};

extern void startChargeTask(TBeing*, const char*);
