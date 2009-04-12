//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_RANGER_H
#define __DISC_RANGER_H

// This is the RANGER discipline.

#include "discipline.h"
#include "skills.h"

class CDRanger : public CDiscipline
{
public:
// Level 1

// Level 3

// Level 7

// Level 8
    CSkill skBeastSoother;

//Level 10

// Level 12

// level 14

//Level 15
    CSkill skBefriendBeast;

// Level 17

// Level 18

// Level 20

// Level 23

// Level 26
    CSkill skBeastSummon;

// Level 30
    CSkill skBarkskin;

    CSkill skRepairRanger;

    CDRanger()
      : CDiscipline(),
      skBeastSoother(),
      skBefriendBeast(),
      skBeastSummon(),
      skBarkskin(),
      skRepairRanger() {
    }
    CDRanger(const CDRanger &a)
      : CDiscipline(a),
      skBeastSoother(a.skBeastSoother),
      skBefriendBeast(a.skBefriendBeast),
      skBeastSummon(a.skBeastSummon),
      skBarkskin(a.skBarkskin), 
      skRepairRanger(a.skRepairRanger) {
    }
    CDRanger & operator=(const CDRanger &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skBeastSoother = a.skBeastSoother;
      skBefriendBeast = a.skBefriendBeast;
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

