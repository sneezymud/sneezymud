#pragma once

#include "discipline.h"
#include "skills.h"

class CDBlacksmithing : public CDiscipline {
  public:
    CSkill skBlacksmithingAdvanced;

    virtual CDBlacksmithing* cloneMe() { return new CDBlacksmithing(*this); }
    
  private:
};
