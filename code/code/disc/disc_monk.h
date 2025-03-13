#pragma once

#include "discipline.h"
#include "skills.h"

#include "discipline.h"

class CDMonk : public CDiscipline {
  public:
    CSkill skYoginsa;  // Basic breathing and meditation.
    CSkill skGroundfighting;
    CSkill skCintai;
    CSkill skOomlat;
    CSkill skKickMonk;
    CSkill skAdvancedKicking;
    CSkill skGrappleMonk;
    CSkill skSpringleap;
    CSkill skRetreatMonk;
    CSkill skSnofalte;
    CSkill skCounterMove;
    CSkill skSwitchMonk;
    CSkill skJirin;
    CSkill skKubo;
    CSkill skDufali;
    CSkill skChop;
    CSkill skChi;
    CSkill skDisarmMonk;
    CSkill skCatfall;
    CSkill skRepairMonk;
    CSkill skCatleap;

    virtual CDMonk* cloneMe() { return new CDMonk(*this); }

    bool isBasic() { return true; }

  private:
};