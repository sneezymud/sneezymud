#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanAlchemy : public CDiscipline {
  public:
    CSkill skBrew;

    virtual CDShamanAlchemy* cloneMe() { return new CDShamanAlchemy(*this); }

  private:
};

extern bool shaman_create_deny(int);
const int CHRISM_PRICE = 250;
int castChrism(TBeing*, const char*);
int chrism(TBeing*, const char*);
int chrism(TBeing*, TObj**, int, const char*, short);
