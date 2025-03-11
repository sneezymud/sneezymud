#pragma once

#include "discipline.h"
#include "skills.h"

class CDEarth : public CDiscipline {
  public:
    CSkill skMeteorSwarm;
    CSkill skLavaStream;
    CSkill skStoneSkin;
    CSkill skTrailSeek;
    CSkill skProtectionFromEarth;

    virtual CDEarth* cloneMe() { return new CDEarth(*this); }

  private:
};

int slingShot(TBeing*, TBeing*);
int castSlingShot(TBeing*, TBeing*);
int slingShot(TBeing*, TBeing*, int, short, int);

int graniteFists(TBeing*, TBeing*);
int castGraniteFists(TBeing*, TBeing*);
int graniteFists(TBeing*, TBeing*, int, short, int);

int pebbleSpray(TBeing*);
int castPebbleSpray(TBeing*);
int pebbleSpray(TBeing*, int, short, int);

int sandBlast(TBeing*);
int castSandBlast(TBeing*);
int sandBlast(TBeing*, int, short, int);

int lavaStream(TBeing*);
int castLavaStream(TBeing*);
int lavaStream(TBeing*, int, short, int);

int meteorSwarm(TBeing*, TBeing*);
int castMeteorSwarm(TBeing*, TBeing*);
int meteorSwarm(TBeing*, TBeing*, TMagicItem*);
int meteorSwarm(TBeing*, TBeing*, int, short, int);

int castStoneSkin(TBeing*, TBeing*);
int stoneSkin(TBeing*, TBeing*);
void stoneSkin(TBeing*, TBeing*, TMagicItem*);
int stoneSkin(TBeing*, TBeing*, int, short);

void stoneSkin(TObj*, TBeing*, TBeing*);

int trailSeek(TBeing*, TBeing*);
int castTrailSeek(TBeing*, TBeing*);
void trailSeek(TBeing*, TBeing*, TMagicItem*);
int trailSeek(TBeing*, TBeing*, int, short);

int conjureElemEarth(TBeing*);
int castConjureElemEarth(TBeing*);
int conjureElemEarth(TBeing*, int, short);

int protectionFromEarth(TBeing*);
void protectionFromEarth(TBeing*, TMagicItem*);
int protectionFromEarth(TBeing*, int, short);
