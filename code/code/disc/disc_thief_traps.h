#pragma once

#include "discipline.h"
#include "skills.h"

class CDTraps : public CDiscipline {
  public:
    CSkill skSetTrapsCont;
    CSkill skSetTrapsDoor;
    CSkill skSetTrapsMine;
    CSkill skSetTrapsGren;
    CSkill skSetTrapsArrow;

    virtual CDTraps* cloneMe() { return new CDTraps(*this); }

  private:
};