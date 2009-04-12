//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_thief.h,v $
// Revision 5.4  2004/08/24 19:48:15  peel
// move track to thief discipline
//
// Revision 5.3  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
//
// Revision 5.2  2002/07/04 18:34:11  dash
// added new repair skills
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_THIEF_H
#define __DISC_THIEF_H

// This is the BASIC THIEF discipline.

#include "discipline.h"
#include "skills.h"

class CDThief : public CDiscipline
{
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

    CDThief()
      : CDiscipline(),
      skSwindle(),
      skSneak(),
      skStabbing(),
      skRetreatThief(),
      skKickThief(),
      skPickLock(),
      skBackstab(),
      skSearch(),
      skSpy(),
      skSwitchThief(),
      skSteal(),
      skDetectTraps(),
      skSubterfuge(),
      skDisarmTraps(),
      skCudgel(),
      skHide(),
      skDisarmThief(),
      skTrack(),
      skRepairThief() {
    }
    CDThief(const CDThief &a)
      : CDiscipline(a),
      skSwindle(a.skSwindle),
      skSneak(a.skSneak),
      skStabbing(a.skStabbing),
      skRetreatThief(a.skRetreatThief),
      skKickThief(a.skKickThief),
      skPickLock(a.skPickLock),
      skBackstab(a.skBackstab),
      skSearch(a.skSearch),
      skSpy(a.skSpy),
      skSwitchThief(a.skSwitchThief),
      skSteal(a.skSteal),
      skDetectTraps(a.skDetectTraps),
      skSubterfuge(a.skSubterfuge),
      skDisarmTraps(a.skDisarmTraps),
      skCudgel(a.skCudgel),
      skHide(a.skHide),
      skDisarmThief(a.skDisarmThief),
      skTrack(a.skTrack),
      skRepairThief(a.skRepairThief) {
    }
    CDThief & operator=(const CDThief &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSwindle = a.skSwindle;
      skSneak = a.skSneak;
      skStabbing = a.skStabbing;
      skRetreatThief = a.skRetreatThief;
      skKickThief = a.skKickThief;
      skPickLock = a.skPickLock;
      skBackstab = a.skBackstab;
      skSearch = a.skSearch;
      skSpy = a.skSpy;
      skSwitchThief = a.skSwitchThief;
      skSteal = a.skSteal;
      skDetectTraps = a.skDetectTraps;
      skSubterfuge = a.skSubterfuge;
      skDisarmTraps = a.skDisarmTraps;
      skCudgel = a.skCudgel;
      skHide = a.skHide;
      skDisarmThief = a.skDisarmThief;
      skTrack = a.skTrack;
      skRepairThief = a.skRepairThief;
      return *this;
    }
    virtual ~CDThief() {}
    virtual CDThief * cloneMe() { return new CDThief(*this); }

    bool isBasic(){ return true; }

private:
};

    int sneak(TBeing *, spellNumT);
    int subterfuge(TBeing *, TBeing *);
    int pickLocks(TBeing *, const char *, const char *, const char *);
    int spy(TBeing *);
    int hide(TBeing *, spellNumT);
    int disguise(TBeing *, char *);

#endif

