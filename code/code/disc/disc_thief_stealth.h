#pragma once

#include "discipline.h"
#include "skills.h"

class CDStealth : public CDiscipline {
  public:
    CSkill skConcealment;
    CSkill skDisguise;

    virtual CDStealth* cloneMe() { return new CDStealth(*this); }

  private:
};

int conceal(TBeing*, TBeing*);