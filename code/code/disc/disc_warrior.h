//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_warrior.h,v $
// Revision 5.4  2004/12/06 18:39:47  peel
// renamed smythe to blacksmithing
// renamed hand to hand to dueling
// renamed physical to soldiering
// added weapon retention and brawl avoidance skill stubs
//
// Revision 5.3  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
//
// Revision 5.2  2002/11/07 04:02:37  jesus
// Added a trip skill for warriors
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

#pragma once

// This is the WARRIOR discipline.

#include "discipline.h"
#include "skills.h"

class TBeing;
class TObj;

class CDWarrior : public CDiscipline {
  public:
    // Level 2
    CSkill skSlam;
    // Level 10
    CSkill skBash;
    CSkill skFocusAttack;

    // Level 15
    CSkill skRescue;

    // Level 20
    CSkill skBlacksmithing;
    CSkill skDisarm;
    // Level 25
    CSkill skBerserk;
    CSkill skSwitch;
    CSkill skBloodlust;
    CSkill skWhirlwind;
    CSkill skRally;

    CDWarrior() :
      CDiscipline(),
      skSlam(),
      skBash(),
      skFocusAttack(),
      skRescue(),
      skBlacksmithing(),
      skDisarm(),
      skBerserk(),
      skSwitch(),
      skBloodlust(),
      skWhirlwind(),
      skRally() {}
    CDWarrior(const CDWarrior& a) :
      CDiscipline(a),
      skSlam(a.skSlam),
      skBash(a.skBash),
      skFocusAttack(a.skFocusAttack),
      skRescue(a.skRescue),
      skBlacksmithing(a.skBlacksmithing),
      skDisarm(a.skDisarm),
      skBerserk(a.skBerserk),
      skSwitch(a.skSwitch),
      skBloodlust(a.skBloodlust),
      skWhirlwind(a.skWhirlwind),
      skRally(a.skRally) {}
    CDWarrior& operator=(const CDWarrior& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skSlam = a.skSlam;
      skBash = a.skBash;
      skFocusAttack = a.skFocusAttack;
      skRescue = a.skRescue;
      skBlacksmithing = a.skBlacksmithing;
      skDisarm = a.skDisarm;
      skBerserk = a.skBerserk;
      skSwitch = a.skSwitch;
      skBloodlust = a.skBloodlust;
      skWhirlwind = a.skWhirlwind;
      skRally = a.skRally;
      return *this;
    }
    virtual ~CDWarrior() {}
    virtual CDWarrior* cloneMe() { return new CDWarrior(*this); }

    bool isBasic() { return true; }

  private:
};

int berserk(TBeing*);
void repair(TBeing*, TObj*);
