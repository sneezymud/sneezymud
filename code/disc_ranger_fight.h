//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_ranger_fight.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_RANGER_FIGHT_H
#define __DISC_RANGER_FIGHT_H

// This is the RANGER FIGHTING skills discipline.

class CDRangerFight : public CDiscipline
{
public:
    CSkill skSwitchRanger;
    CSkill skRetreatRanger;

    CDRangerFight()
      : CDiscipline(),
      skSwitchRanger(),
      skRetreatRanger() {
    }
    CDRangerFight(const CDRangerFight &a)
      : CDiscipline(a),
      skSwitchRanger(a.skSwitchRanger),
      skRetreatRanger(a.skRetreatRanger) {
    }
    CDRangerFight & operator=(const CDRangerFight &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSwitchRanger = a.skSwitchRanger;
      skRetreatRanger = a.skRetreatRanger;
      return *this;
    }
    virtual ~CDRangerFight() {}
    virtual CDRangerFight * cloneMe() { return new CDRangerFight(*this); }

private:
};

#endif

