//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_RANGER_H
#define __DISC_RANGER_H

// This is the RANGER discipline.

class CDRanger : public CDiscipline
{
public:
// Level 1

// Level 3
    CSkill skKickRanger;

// Level 7

// Level 8
    CSkill skBeastSoother;

//Level 10

// Level 12
    CSkill skBashRanger;

// level 14
    CSkill skRescueRanger;

//Level 15
    CSkill skBefriendBeast;
    CSkill skTransfix;

// Level 17

// Level 18
    CSkill skSwitchRanger;

// Level 20
    CSkill skDualWield;

// Level 23

// Level 26
    CSkill skBeastSummon;

// Level 30
    CSkill skBarkskin;

    CSkill skRepairRanger;

    CDRanger()
      : CDiscipline(),
      skKickRanger(),
      skBeastSoother(),
      skBashRanger(),
      skRescueRanger(),
      skBefriendBeast(),
      skTransfix(),
      skSwitchRanger(),
      skDualWield(),
      skBeastSummon(),
      skBarkskin(),
      skRepairRanger() {
    }
    CDRanger(const CDRanger &a)
      : CDiscipline(a),
      skKickRanger(a.skKickRanger),
      skBeastSoother(a.skBeastSoother),
      skBashRanger(a.skBashRanger),
      skRescueRanger(a.skRescueRanger),
      skBefriendBeast(a.skBefriendBeast),
      skTransfix(a.skTransfix),
      skSwitchRanger(a.skSwitchRanger),
      skDualWield(a.skDualWield),
      skBeastSummon(a.skBeastSummon),
      skBarkskin(a.skBarkskin), 
      skRepairRanger(a.skRepairRanger) {
    }
    CDRanger & operator=(const CDRanger &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skKickRanger = a.skKickRanger;
      skBeastSoother = a.skBeastSoother;
      skBashRanger = a.skBashRanger;
      skRescueRanger = a.skRescueRanger;
      skBefriendBeast = a.skBefriendBeast;
      skTransfix = a.skTransfix;
      skSwitchRanger = a.skSwitchRanger;
      skDualWield = a.skDualWield;
      skBeastSummon = a.skBeastSummon;
      skBarkskin = a.skBarkskin;
      skRepairRanger = a.skRepairRanger;
      return *this;
    }
    virtual ~CDRanger() {}
    virtual CDRanger * cloneMe() { return new CDRanger(*this); }

    bool isBasic(){ return true; }

private:
};

#endif

