#pragma once

#include "discipline.h"
#include "skills.h"

class CDBash : public CDiscipline {
  public:
    CSkill skBluntSpec;

    virtual CDBash* cloneMe() { return new CDBash(*this); }

    bool isFast() { return true; }

  private:
};
