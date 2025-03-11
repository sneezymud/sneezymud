#pragma once

#include "discipline.h"
#include "skills.h"

class CDRanger : public CDiscipline {
  public:
    CSkill skBeastSoother;
    CSkill skBefriendBeast;
    CSkill skBeastSummon;
    CSkill skBarkskin;
    CSkill skRepairRanger;

    virtual CDRanger* cloneMe() { return new CDRanger(*this); }

    bool isBasic() { return true; }

  private:
};
