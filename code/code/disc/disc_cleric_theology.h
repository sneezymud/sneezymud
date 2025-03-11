#pragma once

#include "discipline.h"
#include "skills.h"

class CDTheology : public CDiscipline {
  public:
    CSkill skPenance;
    CSkill skAttune;

    virtual CDTheology* cloneMe() { return new CDTheology(*this); }

    bool isBasic() { return true; }

  private:
};

void attune(TBeing*, TThing*);
