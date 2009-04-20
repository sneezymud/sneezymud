//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan.h,v $
// Revision 5.3  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
//
// Revision 5.2  2002/07/04 18:34:11  dash
// added new repair skills
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_DEIKHAN_H
#define __DISC_DEIKHAN_H

// This is the deikhan discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhan : public CDiscipline
{
public:
// Level 1
// Level 2
    CSkill skKickDeikhan;
// Level 4
    CSkill skHealLightDeikhan;
// Level 4
    CSkill skHarmLightDeikhan;

// Level 6
    CSkill skChivalry;
//  Level 8
    CSkill skArmorDeikhan;
// Level 10
    CSkill skBlessDeikhan;
    CSkill skBashDeikhan;
    CSkill skRescueDeikhan;
//Level 11
    CSkill skExpelDeikhan;

// Level 12
    CSkill skClotDeikhan;
    CSkill skRainBrimstoneDeikhan;
// Level 14
    CSkill skSterilizeDeikhan;
    CSkill skRemoveCurseDeikhan;
// level 15
    CSkill skCurseDeikhan;
// Level 17
    CSkill skSmite;
// Level 18
    CSkill skInfectDeikhan;
    CSkill skCureDiseaseDeikhan;
// Level 19
    CSkill skCreateFoodDeikhan;
    CSkill skCreateWaterDeikhan;
// Level 20
    CSkill skHealSeriousDeikhan;
    CSkill skCurePoisonDeikhan;
    CSkill skCharge;
// Level 21
    CSkill skHarmSeriousDeikhan;
// Level 22
    CSkill skPoisonDeikhan;
// Level 25
    CSkill skSwitchDeikhan;
    CSkill skDisarmDeikhan;
    CSkill skLayHands;
// Level 28
    CSkill skHealCriticalDeikhan;
// Level 30
    CSkill skHarmCriticalDeikhan;

    CSkill skRepairDeikhan;
    
    CDDeikhan()
      : CDiscipline(),
      skKickDeikhan(),
      skHealLightDeikhan(),
      skHarmLightDeikhan(),
      skChivalry(),
      skArmorDeikhan(),
      skBlessDeikhan(),
      skBashDeikhan(),
      skRescueDeikhan(),
      skExpelDeikhan(),
      skClotDeikhan(),
      skRainBrimstoneDeikhan(),
      skSterilizeDeikhan(),
      skRemoveCurseDeikhan(),
      skCurseDeikhan(),
      skSmite(),
      skInfectDeikhan(),
      skCureDiseaseDeikhan(),
      skCreateFoodDeikhan(),
      skCreateWaterDeikhan(),
      skHealSeriousDeikhan(),
      skCurePoisonDeikhan(),
      skCharge(),
      skHarmSeriousDeikhan(),
      skPoisonDeikhan(),
      skSwitchDeikhan(),
      skDisarmDeikhan(),
      skLayHands(),
      skHealCriticalDeikhan(),
      skHarmCriticalDeikhan(),
      skRepairDeikhan() {
    }
    CDDeikhan(const CDDeikhan &a)
      : CDiscipline(a),
        skKickDeikhan(a.skKickDeikhan),
        skHealLightDeikhan(a.skHealLightDeikhan),
        skHarmLightDeikhan(a.skHarmLightDeikhan),
        skChivalry(a.skChivalry),
        skArmorDeikhan(a.skArmorDeikhan),
        skBlessDeikhan(a.skBlessDeikhan),
        skBashDeikhan(a.skBashDeikhan),
        skRescueDeikhan(a.skRescueDeikhan),
        skExpelDeikhan(a.skExpelDeikhan),
        skClotDeikhan(a.skClotDeikhan),
        skRainBrimstoneDeikhan(a.skRainBrimstoneDeikhan),
        skSterilizeDeikhan(a.skSterilizeDeikhan),
        skRemoveCurseDeikhan(a.skRemoveCurseDeikhan),
        skCurseDeikhan(a.skCurseDeikhan),
        skSmite(a.skSmite),
        skInfectDeikhan(a.skInfectDeikhan),
        skCureDiseaseDeikhan(a.skCureDiseaseDeikhan),
        skCreateFoodDeikhan(a.skCreateFoodDeikhan),
        skCreateWaterDeikhan(a.skCreateWaterDeikhan),
        skHealSeriousDeikhan(a.skHealSeriousDeikhan),
        skCurePoisonDeikhan(a.skCurePoisonDeikhan),
        skCharge(a.skCharge),
        skHarmSeriousDeikhan(a.skHarmSeriousDeikhan),
        skPoisonDeikhan(a.skPoisonDeikhan),
        skSwitchDeikhan(a.skSwitchDeikhan),
        skDisarmDeikhan(a.skDisarmDeikhan),
        skLayHands(a.skLayHands),
        skHealCriticalDeikhan(a.skHealCriticalDeikhan),
        skHarmCriticalDeikhan(a.skHarmCriticalDeikhan),
      skRepairDeikhan(a.skRepairDeikhan) {
    }
    CDDeikhan & operator=(const CDDeikhan &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skKickDeikhan = a.skKickDeikhan;
      skHealLightDeikhan = a.skHealLightDeikhan;
      skHarmLightDeikhan = a.skHarmLightDeikhan;
      skChivalry = a.skChivalry;
      skArmorDeikhan = a.skArmorDeikhan;
      skBlessDeikhan = a.skBlessDeikhan;
      skBashDeikhan = a.skBashDeikhan;
      skRescueDeikhan = a.skRescueDeikhan;
      skExpelDeikhan = a.skExpelDeikhan;
      skClotDeikhan = a.skClotDeikhan;
      skRainBrimstoneDeikhan = a.skRainBrimstoneDeikhan;
      skSterilizeDeikhan = a.skSterilizeDeikhan;
      skRemoveCurseDeikhan = a.skRemoveCurseDeikhan;
      skCurseDeikhan = a.skCurseDeikhan;
      skSmite = a.skSmite;
      skInfectDeikhan = a.skInfectDeikhan;
      skCureDiseaseDeikhan = a.skCureDiseaseDeikhan;
      skCreateFoodDeikhan = a.skCreateFoodDeikhan;
      skCreateWaterDeikhan = a.skCreateWaterDeikhan;
      skHealSeriousDeikhan = a.skHealSeriousDeikhan;
      skCurePoisonDeikhan = a.skCurePoisonDeikhan;
      skCharge = a.skCharge;
      skHarmSeriousDeikhan = a.skHarmSeriousDeikhan;
      skPoisonDeikhan = a.skPoisonDeikhan;
      skSwitchDeikhan = a.skSwitchDeikhan;
      skDisarmDeikhan = a.skDisarmDeikhan;
      skLayHands = a.skLayHands;
      skHealCriticalDeikhan = a.skHealCriticalDeikhan;
      skHarmCriticalDeikhan = a.skHarmCriticalDeikhan;
      skRepairDeikhan = a.skRepairDeikhan;
      return *this;
    }
    virtual ~CDDeikhan() {}
    virtual CDDeikhan * cloneMe() { return new CDDeikhan(*this); }

    bool isBasic(){ return true; }

private:
};

    int synostodweomer(TBeing * caster, TBeing * victim);
    int synostodweomer(TBeing * caster, TBeing * victim, int, short);

    int smite(TBeing *, TBeing *);

#endif

