#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanMartial : public CDiscipline {
  public:
    CSkill skSwitchDeikhan;
    CSkill skRetreatDeikhan;
    CSkill skShoveDeikhan;
    CSkill sk2hSpecDeikhan;
    CSkill skShockCavalry;
    CSkill skOrient;

    virtual CDDeikhanMartial* cloneMe() { return new CDDeikhanMartial(*this); }

  private:
};