//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "discipline.h"
#include "skills.h"

class CDIronBody : public CDiscipline {
  public:
    CSkill skIronFist;
    CSkill skIronFlesh;
    CSkill skIronSkin;
    CSkill skIronBones;
    CSkill skIronMuscles;
    CSkill skIronLegs;
    CSkill skIronWill;

    virtual CDIronBody* cloneMe() { return new CDIronBody(*this); }

  private:
};
