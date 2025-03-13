#pragma once

#include "discipline.h"
#include "skills.h"

class CDBrawling : public CDiscipline {
  public:
    CSkill skGrapple;
    CSkill skStomp;
    CSkill skBrawlAvoidance;
    CSkill skBodyslam;
    CSkill skHeadbutt;
    CSkill skKneestrike;
    CSkill skSpin;
    CSkill skCloseQuartersFighting;
    CSkill skTaunt;
    CSkill skTrip;
    CSkill skAdvBerserk;

    virtual CDBrawling* cloneMe() { return new CDBrawling(*this); }
    int procAdvancedBerserk(TBeing*);

  private:
};

int berserk(TBeing*);