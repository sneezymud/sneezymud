#pragma once

#include "discipline.h"
#include "skills.h"

class CDAir : public CDiscipline {
  public:
    CSkill skProtectionFromAir;
    CSkill skImmobilize;     // NEW  30th level individual
    CSkill skSuffocate;      //      40th level individual
    CSkill skLightningBolt;   //      20th level single target
    CSkill skChainLightning;  //      80th level chaining
    CSkill skFly;
    CSkill skPierceResist;

    virtual CDAir* cloneMe() { return new CDAir(*this); }

  private:
};

int gust(TBeing*, TBeing*);
int gust(TBeing*, TBeing*, int, short, int);
int castGust(TBeing*, TBeing*);
int gust(TBeing*, TBeing*, TMagicItem*);

int immobilize(TBeing*, TBeing*);
int immobilize(TBeing*, TBeing*, int, short);
int castImmobilize(TBeing*, TBeing*);

int suffocate(TBeing*, TBeing*);
void suffocate(TBeing*, TBeing*, TMagicItem*);
int suffocate(TBeing*, TBeing*, int, short);
int castSuffocate(TBeing*, TBeing*);

int dustStorm(TBeing*);
int dustStorm(TBeing*, int, short, int);
int castDustStorm(TBeing*);

int lightningBolt(TBeing* caster, TBeing* victim);
int lightningBolt(TBeing* caster, TBeing* victim, int level, short bKnown, int advLearn);
int castLightningBolt(TBeing* caster, TBeing* victim);
int lightningBolt(TBeing* caster, TBeing* victim, TMagicItem* obj);

int chainLightning(TBeing* caster, TBeing* victim);
int chainLightning(TBeing* caster, TBeing* victim, int level, short bKnown, int advLearn);
int castChainLightning(TBeing* caster, TBeing* victim);
int chainLightning(TBeing* caster, TBeing* victim, TMagicItem* obj);

void tornado(TBeing*);
void tornado(TBeing*, TMagicItem*);
int tornado(TBeing*, int, short, int);
int castTornado(TBeing*);

void featheryDescent(TBeing*, TBeing*, int, int);
void featheryDescent(TBeing*, TBeing*, TMagicItem*);
int featheryDescent(TBeing*, TBeing*, int, short);
int castFeatheryDescent(TBeing*, TBeing*);
int featheryDescent(TBeing*, TBeing*);

int castFly(TBeing*, TBeing*);
int fly(TBeing*, TBeing*);
void fly(TBeing*, TBeing*, TMagicItem*);
int fly(TBeing*, TBeing*, int, affectedData*, short);

int castConjureElemAir(TBeing*);
int conjureElemAir(TBeing*, int, short);
int conjureElemAir(TBeing*);

void levitate(TBeing*, TBeing*);
int levitate(TBeing*, TBeing*, int, short);
int castLevitate(TBeing*, TBeing*);

int falconWings(TBeing*, TBeing*);
void falconWings(TBeing*, TBeing*, TMagicItem*);
int falconWings(TBeing*, TBeing*, int, short);
int castFalconWings(TBeing*, TBeing*);

int protectionFromAir(TBeing*);
void protectionFromAir(TBeing*, TMagicItem*);
int protectionFromAir(TBeing*, int, short);

void weightCorrectDuration(const TBeing*, affectedData*);
