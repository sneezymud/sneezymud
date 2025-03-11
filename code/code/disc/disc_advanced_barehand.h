#pragma once

#include "discipline.h"
#include "skills.h"

class CDBarehand : public CDiscipline {
  public:
    CSkill skBarehandSpec;

    virtual CDBarehand* cloneMe() { return new CDBarehand(*this); }

    bool isFast() { return true; }

  private:
};
