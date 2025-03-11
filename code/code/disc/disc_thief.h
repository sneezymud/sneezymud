#pragma once

#include "discipline.h"
#include "skills.h"

class CDThief : public CDiscipline {
  public:
    CSkill skSwindle;
    CSkill skSneak;
    CSkill skStabbing;
    CSkill skRetreatThief;
    CSkill skKickThief;
    CSkill skPickLock;
    CSkill skBackstab;
    CSkill skSearch;
    CSkill skSpy;
    CSkill skSwitchThief;
    CSkill skSteal;
    CSkill skDetectTraps;
    CSkill skSubterfuge;
    CSkill skDisarmTraps;
    CSkill skCudgel;
    CSkill skHide;
    CSkill skDisarmThief;
    CSkill skTrack;
    CSkill skRepairThief;

    virtual CDThief* cloneMe() { return new CDThief(*this); }

    bool isBasic() { return true; }

  private:
};

int sneak(TBeing*, spellNumT);
int subterfuge(TBeing*, TBeing*);
int pickLocks(TBeing*, const char*, const char*, const char*);
int spy(TBeing*);
int hide(TBeing*, spellNumT);
int disguise(TBeing*, char*);