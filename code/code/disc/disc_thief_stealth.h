#pragma once

#include "discipline.h"
#include "skills.h"

class CDStealth : public CDiscipline {
  public:
    CSkill skConcealment;
    CSkill skDisguise;

    virtual CDStealth* cloneMe() { return new CDStealth(*this); }

  private:
};

int conceal(TBeing*, TBeing*);
int sneak(TBeing*, spellNumT);
int hide(TBeing*, spellNumT);
int subterfuge(TBeing*, TBeing*);
int subterfugeFail(TBeing*, TBeing*);
int subterfugeSuccess(TBeing*, TBeing*);
int subterfugeHit(TBeing*, TBeing*);
int subterfugeMiss(TBeing*, TBeing*);
int subterfugePlayer(TBeing*, TBeing*);
int spy(TBeing*);
int disguise(TBeing*, char*);