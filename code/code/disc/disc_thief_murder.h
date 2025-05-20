#pragma once

#include "discipline.h"
#include "skills.h"

class CDMurder : public CDiscipline {
  public:
    CSkill skGarrotte;
    CSkill skThroatSlit;

    virtual CDMurder* cloneMe() { return new CDMurder(*this); }

  private:
};

int backstab(TBeing*, TBeing*);
int throatSlit(TBeing*, TBeing*);
int poisonWeapon(TBeing*, TThing*, TThing*);
int garrotte(TBeing*, TBeing*);
int cudgel(TBeing*, TBeing*);
int biteStab(TBeing*, TBeing*);
int clawStab(TBeing*, TBeing*);


bool addPoison(affectedData aff[5], liqTypeT liq, int level, int duration);
