#pragma once

#include "discipline.h"
#include "skills.h"

class CDSlash : public CDiscipline {
  public:
    CSkill skSlashSpec;

    virtual CDSlash* cloneMe() { return new CDSlash(*this); }

    bool isFast() { return true; }

  private:
};
