#pragma once

#include "discipline.h"
#include "skills.h"

class CDPsionics : public CDiscipline {
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

    virtual CDPsionics* cloneMe() { return new CDPsionics(*this); }

    bool isFast() { return true; }

  private:
};
