//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_water.h,v $
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


#ifndef __DISC_WATER_H
#define __DISC_WATER_H

// This is the WATER discipline.

class CDWater : public CDiscipline
{
public:
    CSkill skWateryGrave;               //      40th level individual
    CSkill skTsunami;            // NEW  36th level area affect
    CSkill skBreathOfSarahage;
    CSkill skPlasmaMirror;
    CSkill skGarmulsTail;

    CDWater();
    CDWater(const CDWater &a);
    CDWater & operator=(const CDWater &a);
    virtual ~CDWater();
    virtual CDWater * cloneMe();
private:
};

    int faerieFog(TBeing *);
    int castFaerieFog(TBeing *);
    int faerieFog(TBeing *, int, byte);

    int gusher(TBeing *, TBeing *);
    int castGusher(TBeing *, TBeing *);
    int gusher(TBeing *, TBeing *, TMagicItem *);
    int gusher(TBeing *, TBeing *, int, byte, int);

    int icyGrip(TBeing *, TBeing *);
    int castIcyGrip(TBeing *, TBeing *);
    int icyGrip(TBeing *, TBeing *, TMagicItem *);
    int icyGrip(TBeing *, TBeing *, int, byte, int);

    int wateryGrave(TBeing *, TBeing *);
    int castWateryGrave(TBeing *, TBeing *);
    int wateryGrave(TBeing *, TBeing *, int, byte, int);

    int arcticBlast(TBeing *);
    int castArcticBlast(TBeing *);
    int arcticBlast(TBeing *, TMagicItem *);
    int arcticBlast(TBeing *, int, byte, int);

    int iceStorm(TBeing *);
    int castIceStorm(TBeing *);
    int iceStorm(TBeing *, TMagicItem *);
    int iceStorm(TBeing *, int, byte, int);

    int tsunami(TBeing *);
    int castTsunami(TBeing *);
    int tsunami(TBeing *, int, byte, int);

    int conjureElemWater(TBeing *);
    int castConjureElemWater(TBeing *);
    int conjureElemWater(TBeing *, int, byte);

    int gillsOfFlesh(TBeing *, TBeing *);
    int castGillsOfFlesh(TBeing *, TBeing *);
    void gillsOfFlesh(TBeing *, TBeing *, TMagicItem *);
    int gillsOfFlesh(TBeing *, TBeing *, int, byte);

    int breathOfSarahage(TBeing *);
    int castBreathOfSarahage(TBeing *);
    int breathOfSarahage(TBeing *, int, byte);

    int protectionFromWater(TBeing *, TBeing *);
    int castProtectionFromWater(TBeing *, TBeing *);
    void protectionFromWater(TBeing *, TBeing *, TMagicItem *);
    int protectionFromWater(TBeing *, TBeing *, int, byte);

    // this spell is intentionally NOT an object spell - too powerful
    int plasmaMirror(TBeing *);
    int castPlasmaMirror(TBeing *);
    int plasmaMirror(TBeing *, int, byte);

    int garmulsTail(TBeing *, TBeing *);
    int castGarmulsTail(TBeing *, TBeing *);
    void garmulsTail(TBeing *, TBeing *, TMagicItem *);
    int garmulsTail(TBeing *, TBeing *, int, byte);

#endif
