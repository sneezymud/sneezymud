#pragma once

#include "discipline.h"
#include "skills.h"

class CDCures : public CDiscipline {
  public:
    CSkill skHealFull;       // MOD  40th level individual
    CSkill skHealCritSpray;  // NEW  25th level area affect
    CSkill skHealSpray;      // MOD  35th level area affect
    CSkill skHealFullSpray;  // NEW  45th level area affect
    CSkill skRestoreLimb;

    virtual CDCures* cloneMe() { return new CDCures(*this); }

  private:
};

void healLight(TBeing*, TBeing*);
int castHealLight(TBeing*, TBeing*);
void healLight(TBeing*, TBeing*, TMagicItem*, spellNumT);
int healLight(TBeing*, TBeing*, int, short, spellNumT, int);

void healSerious(TBeing*, TBeing*);
int castHealSerious(TBeing*, TBeing*);
void healSerious(TBeing*, TBeing*, TMagicItem*, spellNumT);
int healSerious(TBeing*, TBeing*, int, short, spellNumT);

void healCritical(TBeing*, TBeing*);
int castHealCritical(TBeing*, TBeing*);
void healCritical(TBeing*, TBeing*, TMagicItem*, spellNumT);
int healCritical(TBeing*, TBeing*, int, short, spellNumT, int);

void heal(TBeing*, TBeing*);
int castHeal(TBeing*, TBeing*);
void heal(TBeing*, TBeing*, TMagicItem*, spellNumT);
int heal(TBeing*, TBeing*, int, short, spellNumT, int);

void healFull(TBeing*, TBeing*);
int castHealFull(TBeing*, TBeing*);
void healFull(TBeing*, TBeing*, TMagicItem*);
int healFull(TBeing*, TBeing*, int, short, int);

void healCritSpray(TBeing*);
int castHealCritSpray(TBeing*, TBeing*);
void healCritSpray(TBeing*, TMagicItem*);
int healCritSpray(TBeing*, int, short);

void healSpray(TBeing*);
int castHealSpray(TBeing*, TBeing*);
void healSpray(TBeing*, TMagicItem*);
int healSpray(TBeing*, int, short);

void healFullSpray(TBeing*);
int castHealFullSpray(TBeing*, TBeing*);
void healFullSpray(TBeing*, TMagicItem*);
int healFullSpray(TBeing*, int, short);

void knitBone(TBeing*, TBeing*);
void knitBone(TBeing*, TBeing*, TMagicItem*);
int knitBone(TBeing*, TBeing*, int, short);

void clot(TBeing*, TBeing*);
void clot(TBeing*, TBeing*, TMagicItem*, spellNumT);
int clot(TBeing*, TBeing*, int, short, spellNumT);

void restoreLimb(TBeing*, TBeing*);
void restoreLimb(TBeing*, TBeing*, TMagicItem*);
int restoreLimb(TBeing*, TBeing*, int, short);

void sterilize(TBeing*, TBeing*);
void sterilize(TBeing*, TBeing*, TMagicItem*, spellNumT);
int sterilize(TBeing*, TBeing*, int, short, spellNumT);

void salve(TBeing*, TBeing*);
void salve(TBeing*, TBeing*, TMagicItem*, spellNumT);
int salve(TBeing*, TBeing*, int, short, spellNumT);

void expel(TBeing*, TBeing*);
void expel(TBeing*, TBeing*, TMagicItem*, spellNumT);
int expel(TBeing*, TBeing*, int, short, spellNumT);

// this is the amount of healing a removing bruise is equivalent to
#define BRUISE_HEAL_AMOUNT 7
