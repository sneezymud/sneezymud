#pragma once

#include "discipline.h"
#include "skills.h"

class CDSpirit : public CDiscipline {
  public:
    CSkill skSilence;
    CSkill skCloudOfConcealment;
    CSkill skTrueSight;
    CSkill skPolymorph;
    CSkill skFumble;
    CSkill skKnot;

    virtual CDSpirit* cloneMe() { return new CDSpirit(*this); }

  private:
};

int knot(TBeing*, TBeing*);
int castKnot(TBeing*, TBeing*);
int knot(TBeing*, TBeing*, int, short);

int silence(TBeing*, TBeing*);
int castSilence(TBeing*, TBeing*);
int silence(TBeing*, TBeing*, int, short);

int slumber(TBeing*, TBeing*);
int castSlumber(TBeing*, TBeing*);
int slumber(TBeing*, TBeing*, TMagicItem*);
int slumber(TBeing*, TBeing*, int, short);

int ensorcer(TBeing*, TBeing*);
int castEnsorcer(TBeing*, TBeing*);
void ensorcer(TBeing*, TBeing*, TMagicItem*);
int ensorcer(TBeing*, TBeing*, int, short);

int cloudOfConcealment(TBeing*);
int castCloudOfConcealment(TBeing*);
int cloudOfConcealment(TBeing*, int, short);

int dispelInvisible(TBeing*, TBeing*);
int castDispelInvisible(TBeing*, TBeing*);
void dispelInvisible(TBeing*, TBeing*, TMagicItem*);
int dispelInvisible(TBeing*, TBeing*, int, short);
void dispelInvisible(TBeing*, TObj*);
void dispelInvisible(TBeing*, TObj*, TMagicItem*);
int dispelInvisible(TBeing*, TObj*, int, short);

int polymorph(TBeing*, const char*);
int castPolymorph(TBeing*);
int polymorph(TBeing*, int, short);

int castStealth(TBeing*, TBeing*);
int stealth(TBeing*, TBeing*);
void stealth(TBeing*, TBeing*, TMagicItem*);
int stealth(TBeing*, TBeing*, int, short);

int accelerate(TBeing*, TBeing*);
int castAccelerate(TBeing*, TBeing*);
void accelerate(TBeing*, TBeing*, TMagicItem*);
int accelerate(TBeing*, TBeing*, int, short);

int haste(TBeing*, TBeing*);
int castHaste(TBeing*, TBeing*);
void haste(TBeing*, TBeing*, TMagicItem*);
int haste(TBeing*, TBeing*, int, short);

int calm(TBeing*, TBeing*);
int castCalm(TBeing*, TBeing*);
int calm(TBeing*, TBeing*, int, short);

int invisibility(TBeing*, TBeing*);
int castInvisibility(TBeing*, TBeing*);
void invisibility(TBeing*, TBeing*, TMagicItem*);
int invisibility(TBeing*, TBeing*, int, short);
int invisibility(TBeing*, TObj*);
int castInvisibility(TBeing*, TObj*);
void invisibility(TBeing*, TObj*, TMagicItem*);
int invisibility(TBeing*, TObj*, int, short);

int senseLife(TBeing*, TBeing*);
int castSenseLife(TBeing*, TBeing*);
void senseLife(TBeing*, TBeing*, TMagicItem*);
int senseLife(TBeing*, TBeing*, int, short);

int detectInvisibility(TBeing*, TBeing*);
int castDetectInvisibility(TBeing*, TBeing*);
void detectInvisibility(TBeing*, TBeing*, TMagicItem*);
int detectInvisibility(TBeing*, TBeing*, int, short);

int trueSight(TBeing*, TBeing*);
int castTrueSight(TBeing*, TBeing*);
void trueSight(TBeing*, TBeing*, TMagicItem*);
int trueSight(TBeing*, TBeing*, int, short);

int castTelepathy(TBeing*);
int telepathy(TBeing*, const char*);
int telepathy(TBeing*, const char*, int, short);

int fear(TBeing*, TBeing*);
int castFear(TBeing*, TBeing*);
int fear(TBeing*, TBeing*, int, short);

int fumble(TBeing*, TBeing*);
int castFumble(TBeing*, TBeing*);
int fumble(TBeing*, TBeing*, int, short);
