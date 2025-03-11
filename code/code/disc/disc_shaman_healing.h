#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanHealing : public CDiscipline {
  public:
    CSkill skHealingGrasp;
    CSkill skEnliven;

    virtual CDShamanHealing* cloneMe() { return new CDShamanHealing(*this); }

  private:
};
void healingGrasp(TBeing*, TBeing*);
int castHealingGrasp(TBeing*, TBeing*);
void healingGrasp(TBeing*, TBeing*, TMagicItem*, spellNumT);
int healingGrasp(TBeing*, TBeing*, int, short, spellNumT, int = 0);

int enliven(TBeing*, TBeing*, int, short);
void enliven(TBeing*, TBeing*, TMagicItem*);
int enliven(TBeing*, TBeing*);
int castEnliven(TBeing*, TBeing*);