#pragma once

#include <algorithm>

#include "discipline.h"
#include "skills.h"

class CDStealth : public CDiscipline {
  public:
    CSkill skConcealment;
    CSkill skDisguise;
    CSkill skSkulk;

    virtual CDStealth* cloneMe() { return new CDStealth(*this); }

  private:
};

int conceal(TBeing*, TBeing*);
int sneak(TBeing*, spellNumT);
int subterfuge(TBeing*, TBeing*);
int subterfugeFail(TBeing*, TBeing*);
int subterfugeSuccess(TBeing*, TBeing*);
int subterfugeHit(TBeing*, TBeing*);
int subterfugeMiss(TBeing*, TBeing*);
int subterfugePlayer(TBeing*, TBeing*);
int spy(TBeing*);
int disguise(TBeing*, char*);
int skulk(TBeing*, spellNumT);

// Movement cost per pulse (build-up) and per half-tick (maintenance) for an
// active skulker. Inverse-scales with skill: 5 at low skill, 1 at mastery.
constexpr int skulkMoveCost(int learning) {
  return std::max(1, 5 - (learning / 20));
}
