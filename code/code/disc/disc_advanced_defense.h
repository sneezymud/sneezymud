#pragma once

#include "discipline.h"
#include "skills.h"

class CDDefense : public CDiscipline {
  public:
    CSkill skAdvancedDefense;
    CSkill skFocusedAvoidance;
    CSkill skToughness;

    virtual CDDefense* cloneMe() { return new CDDefense(*this); }

    bool isFast() { return true; }

  private:
};
void doToughness(TBeing* ch);
