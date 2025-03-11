#pragma once

#include "discipline.h"
#include "skills.h"

class CDMage : public CDiscipline {
  public:
    CSkill skGust;
    CSkill skSlingShot;
    CSkill skGusher;
    CSkill skHandsOfFlame;
    CSkill skMysticDarts;
    CSkill skFlare;
    CSkill skSorcerersGlobe;
    CSkill skFaerieFire;
    CSkill skIlluminate;
    CSkill skDetectMagic;
    CSkill skStunningArrow;
    CSkill skMaterialize;
    CSkill skPebbleSpray;
    CSkill skArcticBlast;
    CSkill skColorSpray;
    CSkill skInfravision;
    CSkill skIdentify;
    CSkill skPowerstone;
    CSkill skFlamingSword;
    CSkill skFaerieFog;
    CSkill skTeleport;
    CSkill skSenseLife;
    CSkill skCalm;
    CSkill skAccelerate;
    CSkill skDustStorm;
    CSkill skLevitate;
    CSkill skFeatheryDescent;
    CSkill skStealth;
    CSkill skGraniteFists;
    CSkill skIcyGrip;
    CSkill skGillsOfFlesh;
    CSkill skTelepathy;
    CSkill skFear;
    CSkill skSlumber;
    CSkill skConjureElemEarth;
    CSkill skConjureElemAir;
    CSkill skConjureElemFire;
    CSkill skConjureElemWater;
    CSkill skDispelMagic;
    CSkill skEnhanceWeapon;
    CSkill skGalvanize;
    CSkill skDetectInvisible;
    CSkill skDispelInvisible;
    CSkill skTornado;
    CSkill skSandBlast;
    CSkill skIceStorm;
    CSkill skAcidBlast;
    CSkill skFireball;
    CSkill skFarlook;
    CSkill skFalconWings;
    CSkill skInvisibility;
    CSkill skEnsorcer;
    CSkill skEyesOfFertuman;
    CSkill skCopy;
    CSkill skHaste;
    CSkill skRepairMage;

    virtual CDMage* cloneMe() { return new CDMage(*this); }

    bool isBasic() { return true; }
};
