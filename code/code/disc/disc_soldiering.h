//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// This is the WARRIOR SOLDIERING discipline.

#include "discipline.h"
#include "skills.h"

class CDSoldiering : public CDiscipline {
  public:
    CSkill skDoorbash;
    CSkill skDualWieldWarrior;
    CSkill skPowerMove;
    CSkill skDeathstroke;
    CSkill sk2hSpecWarrior;
    CSkill skFortify;

    CDSoldiering();
    CDSoldiering(const CDSoldiering& a);
    CDSoldiering& operator=(const CDSoldiering& a);
    virtual ~CDSoldiering();
    virtual CDSoldiering* cloneMe() { return new CDSoldiering(*this); }

  private:
};
