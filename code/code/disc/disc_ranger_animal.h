#pragma once

#include "discipline.h"
#include "skills.h"

class CDAnimal : public CDiscipline {
  public:
    CSkill skBeastCharm;
    CSkill skFeralWrath;
    CSkill skSkySpirit;

    virtual CDAnimal* cloneMe() { return new CDAnimal(*this); }

  private:
};

int beastSoother(TBeing*, TBeing*);
int beastSoother(TBeing*, TBeing*, TMagicItem*);
int beastSoother(TBeing*, TBeing*, int, short);

int beastSummon(TBeing*, const char*);
int beastSummon(TBeing*, const char*, int, short);
