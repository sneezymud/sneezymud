//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "discipline.h"
#include "skills.h"

class CDPsionics : public CDiscipline
{
public:
  CSkill skTelepathy;
  CSkill skTeleSight;
  CSkill skTeleVision;
  CSkill skMindFocus;
  CSkill skPsiBlast;
  CSkill skMindThrust;
  CSkill skPsyCrush;
  CSkill skKineticWave;
  CSkill skMindPreservation;
  CSkill skTelekinesis;
  CSkill skPsiDrain;
  CSkill skDimensionalFold;

    CDPsionics();
    CDPsionics(const CDPsionics &a);
    CDPsionics & operator=(const CDPsionics &a);
    virtual ~CDPsionics();
    virtual CDPsionics * cloneMe() { return new CDPsionics(*this); }

    bool isFast(){ return true; }

private:
};
