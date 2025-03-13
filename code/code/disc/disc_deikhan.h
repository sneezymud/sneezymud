#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhan : public CDiscipline {
  public:
    CSkill skHealLightDeikhan;
    CSkill skHarmLightDeikhan;
    CSkill skChivalry;
    CSkill skArmorDeikhan;
    CSkill skBlessDeikhan;
    CSkill skBashDeikhan;
    CSkill skRescueDeikhan;
    CSkill skExpelDeikhan;
    CSkill skClotDeikhan;
    CSkill skSterilizeDeikhan;
    CSkill skRemoveCurseDeikhan;
    CSkill skCurseDeikhan;
    CSkill skInfectDeikhan;
    CSkill skCureDiseaseDeikhan;
    CSkill skCreateFoodDeikhan;
    CSkill skCreateWaterDeikhan;
    CSkill skHealSeriousDeikhan;
    CSkill skCurePoisonDeikhan;
    CSkill skCharge;
    CSkill skHarmSeriousDeikhan;
    CSkill skPoisonDeikhan;
    CSkill skDisarmDeikhan;
    CSkill skLayHands;
    CSkill skHealCriticalDeikhan;
    CSkill skHarmCriticalDeikhan;
    CSkill skRepairDeikhan;
    CSkill skAuraMight;
    CSkill skAuraRegeneration;

    virtual CDDeikhan* cloneMe() { return new CDDeikhan(*this); }

    bool isBasic() { return true; }

  private:
};

int procAuraOfMight(TBeing* caster);
int auraOfMightOnHit(TBeing* caster, TBeing* vict);
int procAuraOfRegeneration(TBeing* caster);
int regenAuraPulse(TBeing* caster);
int procAuraGuardian(TBeing* caster);
int procAuraOfVengeance(TBeing* caster);
int procAuraOfAbsolution(TBeing* caster);
void doAbsolutionAuraAffects(TBeing* caster, TBeing* target, bool& done_warmth);
void doWarmth(TBeing* caster);