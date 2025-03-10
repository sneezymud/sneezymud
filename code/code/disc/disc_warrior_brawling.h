//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// This is the Brawling discipline.

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

    CDBrawling();
    CDBrawling(const CDBrawling& a);
    CDBrawling& operator=(const CDBrawling& a);
    virtual ~CDBrawling();
    virtual CDBrawling* cloneMe() { return new CDBrawling(*this); }
    int procAdvancedBerserk(TBeing*);

  private:
};
