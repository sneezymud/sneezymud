//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mage.h,v $
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


#ifndef __DISC_MAGE_H
#define __DISC_MAGE_H

// This is the MAGE BASIC discipline.

class CDMage : public CDiscipline
{
public:
//Level 1
    CSkill skGust;            // NEW   1st level individual
    CSkill skSlingShot;       // NEW   1st level individual
    CSkill skGusher;            // NEW   1st level individual

//Level 3
    CSkill skHandsOfFlame;            // NEW   1st level individual
    CSkill skMysticDarts;       //      1st level individual

//Level 5
    CSkill skFlare;
    CSkill skSorcerersGlobe;

//Level 8 
    CSkill skFaerieFire;          // NEW  10th level individual
    CSkill skIlluminate;
    CSkill skDetectMagic;

//Level 10
    CSkill skStunningArrow;     //      10th level individual
    CSkill skMaterialize;

//Level 12
    CSkill skProtectionFromEarth;
    CSkill skProtectionFromAir;
    CSkill skProtectionFromFire;
    CSkill skProtectionFromWater;
    CSkill skProtectionFromElements;


//Level 14
    CSkill skPebbleSpray;       // NEW  12th level area affect
    CSkill skArcticBlast;             // NEW  12th level area affect
    CSkill skColorSpray;        //      12th level area affect

//Level 15

    CSkill skInfravision;
    CSkill skIdentify;
    CSkill skPowerstone;
    CSkill skFlamingSword;

//Level 16
    CSkill skFaerieFog;          // NEW  10th level individual
    CSkill skTeleport;
    CSkill skSenseLife;
    CSkill skCalm;

//Level 17
    CSkill skAccelerate;

//Level 18

    CSkill skDustStorm;
    CSkill skLevitate;
    CSkill skFeatheryDescent;
    CSkill skStealth;
//Level 20
    CSkill skGraniteFists;
    CSkill skIcyGrip;
    CSkill skGillsOfFlesh;
#if 0
    CSkill skFindFamiliar;
#endif
    CSkill skTelepathy;
// Level 21
    CSkill skFear;
    CSkill skSlumber;

//Level 22
    CSkill skConjureElemEarth;
    CSkill skConjureElemAir;
    CSkill skConjureElemFire;
    CSkill skConjureElemWater;
    CSkill skDispelMagic;

//Level 24
    CSkill skEnhanceWeapon;
    CSkill skGalvanize;
    CSkill skDetectInvisible;

//Level 25
    CSkill skDispelInvisible;
    CSkill skTornado;            // NEW  25th level area affect
    CSkill skSandBlast;       // NEW  25th level area affect
    CSkill skIceStorm;            // NEW  25th level area affect
    CSkill skAcidBlast;         // MOD  25th level area affect

//Level 27
    CSkill skFireball;            // NEW  25th level area affect
    CSkill skFarlook;

//Level 28
    CSkill skFalconWings;
    CSkill skInvisibility;
//Level 29
    CSkill skEnsorcer;

//Level 30
    CSkill skEyesOfFertuman;
    CSkill skCopy;
    CSkill skHaste;

    CSkill skRepairMage;

    CDMage();
    CDMage(const CDMage &);
    CDMage & operator=(const CDMage &a);
    virtual ~CDMage();
    virtual CDMage * cloneMe() { return new CDMage(*this); }

    bool isBasic(){ return true; }

};


#endif
