#pragma once

#include "discipline.h"
#include "skills.h"

class CDOffense : public CDiscipline {
  public:
    CSkill skAdvancedOffense;
    CSkill skInevitability;

    virtual CDOffense* cloneMe() { return new CDOffense(*this); }

    bool isFast() { return true; }
};
