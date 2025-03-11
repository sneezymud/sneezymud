#pragma once

#include "discipline.h"
#include "skills.h"

class CDAdventuring : public CDiscipline {
  public:
    CSkill skFishing;
    CSkill skLogging;
    CSkill skAlcoholism;
    CSkill skRide;
    CSkill skSwim;
    CSkill skClimb;
    CSkill skSign;
    CSkill skKnowPeople;
    CSkill skKnowGiant;
    CSkill skKnowVeggie;
    CSkill skKnowAnimal;
    CSkill skKnowReptile;
    CSkill skKnowUndead;
    CSkill skKnowOther;
    CSkill skKnowDemon;
    CSkill skReadMagic;
    CSkill skBandage;
    CSkill skFastHeal;
    CSkill skEvaluate;
    CSkill skTactics;
    CSkill skDissect;
    CSkill skOffense;
    CSkill skDefense;
    CSkill skGenWeapons;
    CSkill skWhittle;
    CSkill skMend;
    CSkill skButcher;
    CSkill skGutterCant;
    CSkill skGnollJargon;
    CSkill skTroglodytePidgin;

    virtual CDAdventuring* cloneMe() { return new CDAdventuring(*this); }

    bool isAutomatic() { return true; }

  private:
};

int dissect(TBeing*, TObj*);
