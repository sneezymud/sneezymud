#pragma once

#include "discipline.h"
#include "skills.h"

class CDMindBody : public CDiscipline {
  public:
    CSkill skFeignDeath;
    CSkill skBlur;

    virtual CDMindBody* cloneMe() { return new CDMindBody(*this); }

  private:
};
