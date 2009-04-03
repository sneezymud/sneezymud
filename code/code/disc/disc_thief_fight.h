//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_thief_fight.h,v $
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


#ifndef __DISC_THIEF_FIGHT_H
#define __DISC_THIEF_FIGHT_H

// This is the THIEF FIGHTHING skills discipline.

class CDThiefFight : public CDiscipline
{
public:
    CSkill skDodgeThief;
    CSkill skDualWieldThief;

    CDThiefFight()
      : CDiscipline(),
      skDodgeThief(),
      skDualWieldThief() {
    }
    CDThiefFight(const CDThiefFight &a)
      : CDiscipline(a),
      skDodgeThief(a.skDodgeThief),
      skDualWieldThief(a.skDualWieldThief) {
    }
    CDThiefFight & operator=(const CDThiefFight &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDodgeThief = a.skDodgeThief;
      skDualWieldThief = a.skDualWieldThief;
      return *this;
    }
    virtual ~CDThiefFight() {}
    virtual CDThiefFight * cloneMe() { return new CDThiefFight(*this); }
private:
};

#endif

