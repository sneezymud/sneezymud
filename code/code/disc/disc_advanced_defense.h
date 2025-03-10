//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "discipline.h"
#include "skills.h"

class CDDefense : public CDiscipline {
  public:
    CSkill skAdvancedDefense;
    CSkill skFocusedAvoidance;
    CSkill skToughness;

    CDDefense();
    CDDefense(const CDDefense& a);
    CDDefense& operator=(const CDDefense& a);
    virtual ~CDDefense();
    virtual CDDefense* cloneMe() { return new CDDefense(*this); }

    bool isFast() { return true; }

  private:
};
void doToughness(TBeing* ch);
