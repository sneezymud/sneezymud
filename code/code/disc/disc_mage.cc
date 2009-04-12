//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_mage.h"

CDMage::CDMage() :
  CDiscipline(),
  skGust(),
  skSlingShot(),
  skGusher(),
  skHandsOfFlame(),
  skMysticDarts(),
  skFlare(),
  skSorcerersGlobe(),
  skFaerieFire(),
  skIlluminate(),
  skDetectMagic(),
  skStunningArrow(),
  skMaterialize(),
  skProtectionFromEarth(),
  skProtectionFromAir(),
  skProtectionFromFire(),
  skProtectionFromWater(),
  skProtectionFromElements(),
  skPebbleSpray(),
  skArcticBlast(),
  skColorSpray(),
  skInfravision(),
  skIdentify(),
  skPowerstone(),
  skFlamingSword(),
  skFaerieFog(),
  skTeleport(),
  skSenseLife(),
  skCalm(),
  skAccelerate(),
  skDustStorm(),
  skLevitate(),
  skFeatheryDescent(),
  skStealth(),
  skGraniteFists(),
  skIcyGrip(),
  skGillsOfFlesh(),
//  skFindFamiliar(),
  skTelepathy(),
  skFear(),
  skSlumber(),
  skConjureElemEarth(),
  skConjureElemAir(),
  skConjureElemFire(),
  skConjureElemWater(),
  skDispelMagic(),
  skEnhanceWeapon(),
  skGalvanize(),
  skDetectInvisible(),
  skDispelInvisible(),
  skTornado(),
  skSandBlast(),
  skIceStorm(),
  skAcidBlast(),
  skFireball(),
  skFarlook(),
  skFalconWings(),
  skInvisibility(),
  skEnsorcer(),
  skEyesOfFertuman(),
  skCopy(),
  skHaste()
{
}

CDMage::CDMage(const CDMage &a) :
      CDiscipline(a),
      skGust(a.skGust),
      skSlingShot(a.skSlingShot),
      skGusher(a.skGusher),
      skHandsOfFlame(a.skHandsOfFlame),
      skMysticDarts(a.skMysticDarts),
      skFlare(a.skFlare),
      skSorcerersGlobe(a.skSorcerersGlobe),
      skFaerieFire(a.skFaerieFire),
      skIlluminate(a.skIlluminate),
      skDetectMagic(a.skDetectMagic),
      skStunningArrow(a.skStunningArrow),
      skMaterialize(a.skMaterialize),
      skProtectionFromEarth(a.skProtectionFromEarth),
      skProtectionFromAir(a.skProtectionFromAir),
      skProtectionFromFire(a.skProtectionFromFire),
      skProtectionFromWater(a.skProtectionFromWater),
      skProtectionFromElements(a.skProtectionFromElements),
      skPebbleSpray(a.skPebbleSpray),
      skArcticBlast(a.skArcticBlast),
      skColorSpray(a.skColorSpray),
      skInfravision(a.skInfravision),
      skIdentify(a.skIdentify),
      skPowerstone(a.skPowerstone),
      skFlamingSword(a.skFlamingSword),
      skFaerieFog(a.skFaerieFog),
      skTeleport(a.skTeleport),
      skSenseLife(a.skSenseLife),
      skCalm(a.skCalm),
      skAccelerate(a.skAccelerate),
      skDustStorm(a.skDustStorm),
      skLevitate(a.skLevitate),
      skFeatheryDescent(a.skFeatheryDescent),
      skStealth(a.skStealth),
      skGraniteFists(a.skGraniteFists),
      skIcyGrip(a.skIcyGrip),
      skGillsOfFlesh(a.skGillsOfFlesh),
//      skFindFamiliar(a.skFindFamiliar),
      skTelepathy(a.skTelepathy),
      skFear(a.skFear),
      skSlumber(a.skSlumber),
      skConjureElemEarth(a.skConjureElemEarth),
      skConjureElemAir(a.skConjureElemAir),
      skConjureElemFire(a.skConjureElemFire),
      skConjureElemWater(a.skConjureElemWater),
      skDispelMagic(a.skDispelMagic),
      skEnhanceWeapon(a.skEnhanceWeapon),
      skGalvanize(a.skGalvanize),
      skDetectInvisible(a.skDetectInvisible),
      skDispelInvisible(a.skDispelInvisible),
      skTornado(a.skTornado),
      skSandBlast(a.skSandBlast),
      skIceStorm(a.skIceStorm),
      skAcidBlast(a.skAcidBlast),
      skFireball(a.skFireball),
      skFarlook(a.skFarlook),
      skFalconWings(a.skFalconWings),
      skInvisibility(a.skInvisibility),
      skEnsorcer(a.skEnsorcer),
      skEyesOfFertuman(a.skEyesOfFertuman),
      skCopy(a.skCopy),
      skHaste(a.skHaste)
{
}

CDMage & CDMage::operator=(const CDMage &a)
{
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skGust = a.skGust;
      skSlingShot = a.skSlingShot;
      skGusher = a.skGusher;
      skHandsOfFlame = a.skHandsOfFlame;
      skMysticDarts = a.skMysticDarts;
      skFlare = a.skFlare;
      skSorcerersGlobe = a.skSorcerersGlobe;
      skFaerieFire = a.skFaerieFire;
      skIlluminate = a.skIlluminate;
      skDetectMagic = a.skDetectMagic;
      skStunningArrow = a.skStunningArrow;
      skMaterialize = a.skMaterialize;
      skProtectionFromEarth = a.skProtectionFromEarth;
      skProtectionFromAir = a.skProtectionFromAir;
      skProtectionFromFire = a.skProtectionFromFire;
      skProtectionFromWater = a.skProtectionFromWater;
      skProtectionFromElements = a.skProtectionFromElements;
      skPebbleSpray = a.skPebbleSpray;
      skArcticBlast = a.skArcticBlast;
      skColorSpray = a.skColorSpray;
      skInfravision = a.skInfravision;
      skIdentify = a.skIdentify;
      skPowerstone = a.skPowerstone;
      skFlamingSword = a.skFlamingSword;
      skFaerieFog = a.skFaerieFog;
      skTeleport = a.skTeleport;
      skSenseLife = a.skSenseLife;
      skCalm = a.skCalm;
      skAccelerate = a.skAccelerate;
      skDustStorm = a.skDustStorm;
      skLevitate = a.skLevitate;
      skFeatheryDescent = a.skFeatheryDescent;
      skStealth = a.skStealth;
      skGraniteFists = a.skGraniteFists;
      skIcyGrip = a.skIcyGrip;
      skGillsOfFlesh = a.skGillsOfFlesh;
//      skFindFamiliar = a.skFindFamiliar;
      skTelepathy = a.skTelepathy;
      skFear = a.skFear;
      skSlumber = a.skSlumber;
      skConjureElemEarth = a.skConjureElemEarth;
      skConjureElemAir = a.skConjureElemAir;
      skConjureElemFire = a.skConjureElemFire;
      skConjureElemWater = a.skConjureElemWater;
      skDispelMagic = a.skDispelMagic;
      skEnhanceWeapon = a.skEnhanceWeapon;
      skGalvanize = a.skGalvanize;
      skDetectInvisible = a.skDetectInvisible;
      skDispelInvisible = a.skDispelInvisible;
      skTornado = a.skTornado;
      skSandBlast = a.skSandBlast;
      skIceStorm = a.skIceStorm;
      skAcidBlast = a.skAcidBlast;
      skFireball = a.skFireball;
      skFarlook = a.skFarlook;
      skFalconWings = a.skFalconWings;
      skInvisibility = a.skInvisibility;
      skEnsorcer = a.skEnsorcer;
      skEyesOfFertuman = a.skEyesOfFertuman;
      skCopy = a.skCopy;
      skHaste = a.skHaste;
      return *this;
}

CDMage::~CDMage()
{
}

