//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_cleric.h,v $
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


#ifndef __DISC_CLERIC_H
#define __DISC_CLERIC_H

// This is the CLERIC BASIC discipline.

class CDCleric : public CDiscipline
{
public:
//Level 1
    CSkill skHealLight;       // MOD   1st level individual
    CSkill skHarmLight;       // MOD   1st level individual

//Level 4
    CSkill skCreateFood;
    CSkill skCreateWater;

//Level 5
   CSkill skArmor;

//Level 6 
    CSkill skBless;

//Level 8 
    CSkill skClot;
    CSkill skRainBrimstone;

//Level 9 
    CSkill skHealSerious;    // MOD  10th level individual
    CSkill skHarmSerious;    // MOD  10th level individual

//Level 10
    CSkill skSterilize;
    CSkill skExpel;
    CSkill skCureDisease;

//Level 11

//Level 12
    CSkill skCurse;
    CSkill skRemoveCurse;
    CSkill skCurePoison;


//Level 15
    CSkill skHealCritical;   // MOD  20th level individual
    CSkill skSalve;
    CSkill skPoison;

//Level18 
    CSkill skHarmCritical;   // MOD  20th level individual
    CSkill skInfect;
    CSkill skRefresh;

//Level 20
    CSkill skNumb;
    CSkill skDisease;
    CSkill skFlamestrike;
    CSkill skPlagueOfLocusts;

// Level 21
    CSkill skCureBlindness;

//Level 22
    CSkill skSummon;

//Level 25
    CSkill skHeal;           // MOD  30th level individual
    CSkill skParalyzeLimb;
    CSkill skWordOfRecall;

//Level 28
    CSkill skHarm;           // MOD  30th level individual

//Level 30
    CSkill skKnitBone;
    CSkill skBlindness;

    CSkill skRepairCleric;

    CDCleric();
    CDCleric(const CDCleric &a);
    CDCleric & operator=(const CDCleric &a);
    virtual ~CDCleric();
    virtual CDCleric * cloneMe();

    bool isBasic(){ return true; }

private:
};


#endif

