#pragma once

#include "discipline.h"
#include "skills.h"

class CDFaith : public CDiscipline {
  public:
    CSkill skDevotion;

    virtual CDFaith* cloneMe() { return new CDFaith(*this); }

    bool isAutomatic() { return true; }

  private:
};
