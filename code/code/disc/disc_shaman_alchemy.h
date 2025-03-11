#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanAlchemy : public CDiscipline {
  public:
    CSkill skBrew;

    virtual CDShamanAlchemy* cloneMe() { return new CDShamanAlchemy(*this); }

  private:
};
