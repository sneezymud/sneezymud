#pragma once

#include "discipline.h"
#include "skills.h"

class CDLore : public CDiscipline {
  public:
    CSkill skMeditate;
    CSkill skMana;

    virtual CDLore* cloneMe() { return new CDLore(*this); }

    bool isBasic() { return true; }

  private:
};
