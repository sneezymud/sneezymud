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
    CSkill skEncamp;
    CSkill skFishlore;
    CSkill skTrollish;
    CSkill skBullywug;
    CSkill skAvian;
    CSkill skKalysian;
    CSkill skCommon;

    CDAdvAdventuring() :
      CDiscipline(),
      skHiking(),
      skForage(),
      skSeekWater(),
      skSkin(),
      skDivination(),
      skEncamp(),
      skFishlore(),
      skTrollish(),
      skBullywug(),
      skAvian(),
      skKalysian(),
      skCommon() {}

    CDAdvAdventuring(const CDAdvAdventuring& a) :
      CDiscipline(a),
      skHiking(a.skHiking),
      skForage(a.skForage),
      skSeekWater(a.skSeekWater),
      skSkin(a.skSkin),
      skDivination(a.skDivination),
      skEncamp(a.skEncamp),
      skFishlore(a.skFishlore),
      skTrollish(a.skTrollish),
      skBullywug(a.skBullywug),
      skAvian(a.skAvian),
      skKalysian(a.skKalysian),
      skCommon(a.skCommon) {}

    CDAdvAdventuring& operator=(const CDAdvAdventuring& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skHiking = a.skHiking;
      skForage = a.skForage;
      skSeekWater = a.skSeekWater;
      skSkin = a.skSkin;
      skDivination = a.skDivination;
      skEncamp = a.skEncamp;
      skFishlore = a.skFishlore;
      skTrollish = a.skTrollish;
      skBullywug = a.skBullywug;
      skAvian = a.skAvian;
      skKalysian = a.skKalysian;
      skCommon = a.skCommon;
      return *this;
    }
    virtual ~CDAdvAdventuring(){};

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
