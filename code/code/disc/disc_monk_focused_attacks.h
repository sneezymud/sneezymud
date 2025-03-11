#pragma once

#include "discipline.h"
#include "skills.h"

class CDFAttacks : public CDiscipline {
  public:
    CSkill skQuiveringPalm;
    CSkill skCriticalHitting;

    virtual CDFAttacks* cloneMe() { return new CDFAttacks(*this); }

  private:
};
