//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "discipline.h"
#include "skills.h"

class CDAdvAdventuring : public CDiscipline {
  public:
    CSkill skHiking;
    CSkill skForage;
    CSkill skSeekWater;
    CSkill skSkin;
    CSkill skDivination;
    CSkill skFishlore;
    CSkill skTrollish;
    CSkill skBullywug;
    CSkill skAvian;
    CSkill skKalysian;
    CSkill skCommon;

    virtual CDAdvAdventuring* cloneMe() { return new CDAdvAdventuring(*this); }

    bool isFast() { return true; }

  private:
};

void forage(TBeing*);
int forage(TBeing*, short);
int forage_insect(TBeing* caster);

void divine(TBeing*, TThing*);
int divine(TBeing*, int, short, TThing*);

int encamp(TBeing*);
