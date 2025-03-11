#pragma once

#include "discipline.h"
#include "skills.h"

class CDWater : public CDiscipline {
  public:
    CSkill skWateryGrave;  //      40th level individual
    CSkill skTsunami;      // NEW  36th level area affect
    CSkill skBreathOfSarahage;
    CSkill skPlasmaMirror;
    CSkill skGarmulsTail;
    CSkill skProtectionFromWater;

    virtual CDWater* cloneMe() { return new CDWater(*this); }

  private:
};

int faerieFog(TBeing*);
int castFaerieFog(TBeing*);
int faerieFog(TBeing*, int, short);

int gusher(TBeing*, TBeing*);
int castGusher(TBeing*, TBeing*);
int gusher(TBeing*, TBeing*, TMagicItem*);
int gusher(TBeing*, TBeing*, int, short, int);

int icyGrip(TBeing*, TBeing*);
int castIcyGrip(TBeing*, TBeing*);
int icyGrip(TBeing*, TBeing*, TMagicItem*);
int icyGrip(TBeing*, TBeing*, int, short, int);

int wateryGrave(TBeing*, TBeing*);
int castWateryGrave(TBeing*, TBeing*);
int wateryGrave(TBeing*, TBeing*, int, short, int);

int arcticBlast(TBeing*);
int castArcticBlast(TBeing*);
int arcticBlast(TBeing*, TMagicItem*);
int arcticBlast(TBeing*, int, short, int);

int iceStorm(TBeing*);
int castIceStorm(TBeing*);
int iceStorm(TBeing*, TMagicItem*);
int iceStorm(TBeing*, int, short, int);

int tsunami(TBeing*);
int castTsunami(TBeing*);
int tsunami(TBeing*, int, short, int);

int conjureElemWater(TBeing*);
int castConjureElemWater(TBeing*);
int conjureElemWater(TBeing*, int, short);

int gillsOfFlesh(TBeing*, TBeing*);
int castGillsOfFlesh(TBeing*, TBeing*);
void gillsOfFlesh(TBeing*, TBeing*, TMagicItem*);
int gillsOfFlesh(TBeing*, TBeing*, int, short);

int breathOfSarahage(TBeing*);
int castBreathOfSarahage(TBeing*);
int breathOfSarahage(TBeing*, int, short);

int protectionFromWater(TBeing*);
void protectionFromWater(TBeing*, TMagicItem*);
int protectionFromWater(TBeing*, int, short);

// this spell is intentionally NOT an object spell - too powerful
int plasmaMirror(TBeing*);
int castPlasmaMirror(TBeing*);
int plasmaMirror(TBeing*, int, short);

int garmulsTail(TBeing*, TBeing*);
int castGarmulsTail(TBeing*, TBeing*);
void garmulsTail(TBeing*, TBeing*, TMagicItem*);
int garmulsTail(TBeing*, TBeing*, int, short);
