#pragma once

#include "discipline.h"
#include "skills.h"

class CDRitualism : public CDiscipline {
  public:
    CSkill skRitualism;

    virtual CDRitualism* cloneMe() { return new CDRitualism(*this); }

    bool isAutomatic() { return true; }

  private:
};
