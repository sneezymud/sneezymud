#pragma once

#include "discipline.h"
#include "skills.h"

class CDCleric : public CDiscipline {
  public:
    CSkill skHealLight;
    CSkill skHarmLight;
    CSkill skCreateFood;
    CSkill skCreateWater;
    CSkill skArmor;
    CSkill skBless;
    CSkill skClot;
    CSkill skRainBrimstone;
    CSkill skHealSerious;
    CSkill skHarmSerious;
    CSkill skSterilize;
    CSkill skExpel;
    CSkill skCureDisease;
    CSkill skCurse;
    CSkill skRemoveCurse;
    CSkill skCurePoison;
    CSkill skHealCritical;
    CSkill skSalve;
    CSkill skPoison;
    CSkill skHarmCritical;
    CSkill skInfect;
    CSkill skRefresh;
    CSkill skNumb;
    CSkill skDisease;
    CSkill skFlamestrike;
    CSkill skPlagueOfLocusts;
    CSkill skCureBlindness;
    CSkill skSummon;
    CSkill skHeal;
    CSkill skParalyzeLimb;
    CSkill skWordOfRecall;
    CSkill skHarm;
    CSkill skKnitBone;
    CSkill skBlindness;
    CSkill skRepairCleric;

    virtual CDCleric* cloneMe() { return new CDCleric(*this); }

    bool isBasic() { return true; }

  private:
};
