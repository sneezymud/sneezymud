#pragma once

#include "discipline.h"
#include "skills.h"

class CDAlchemy : public CDiscipline {
  public:
    CSkill skDivination;
    CSkill skShatter;
    CSkill skScribe;
    CSkill skSpontaneousGeneration;
    CSkill skEtherGate;
    CSkill skStaveCharge;

    virtual CDAlchemy* cloneMe() { return new CDAlchemy(*this); }

  private:
};

int identify(TBeing*, TObj*);
int castIdentify(TBeing*, TObj*);
int identify(TBeing*, TObj*, int, short);
void identify(TBeing*, TObj*, TMagicItem*);
void identify(TBeing*, TBeing*, TMagicItem*);
int identify(TBeing*, TBeing*);
int castIdentify(TBeing*, TBeing*);
int identify(TBeing*, TBeing*, int, short);

int divinationBeing(TBeing*, TBeing*);
int castDivinationBeing(TBeing*, TBeing*);
int divinationBeing(TBeing*, const TObj*, int, short);
void divinationBeing(TBeing*, TBeing*, TMagicItem*);
int divinationObj(TBeing*, TObj*);
int castDivinationObj(TBeing*, const TObj*);
int divinationObj(TBeing*, TBeing*, int, short);
void divinationObj(TBeing*, TObj*, TMagicItem*);

int eyesOfFertuman(TBeing*, const char*);
int castEyesOfFertuman(TBeing*, const char*);
//int eyesOfFertuman(TBeing*, TBeing*, int, short);
int eyesOfFertuman(TBeing*, const char*, int, short);


int powerstone(TBeing*, TObj*);
int castPowerstone(TBeing*, TObj*);
int powerstone(TBeing*, TObj*, int, short);

int shatter(TBeing*, TBeing*);
int castShatter(TBeing*, TBeing*);
int shatter(TBeing*, TBeing*, int, short);

//    void farlook(TBeing *, TBeing *);
int farlook(TBeing*, TBeing*);
int castFarlook(TBeing*, TBeing*);
int farlook(TBeing*, TBeing*, int, short);

int illuminate(TBeing*, TObj*);
void illuminate(TBeing*, TMagicItem*, TObj*);
int illuminate(TBeing*, TObj*, int, short);
int castIlluminate(TBeing*, TObj*);

int detectMagic(TBeing*, TBeing*);
int castDetectMagic(TBeing*, TBeing*);
void detectMagic(TBeing*, TBeing*, TMagicItem*);
int detectMagic(TBeing*, TBeing*, int, short);

int dispelMagic(TBeing*, TObj*);
int castDispelMagic(TBeing*, TObj*);
void dispelMagic(TBeing*, TObj*, TMagicItem*);
int dispelMagic(TBeing*, TObj*, int, short);
int dispelMagic(TBeing*, TBeing*);
int castDispelMagic(TBeing*, TBeing*);
int dispelMagic(TBeing*, TBeing*, TMagicItem*);
int dispelMagic(TBeing*, TBeing*, int, short);

int enhanceWeapon(TBeing*, TObj*);
int castEnhanceWeapon(TBeing*, TObj*);
int enhanceWeapon(TBeing*, TMagicItem*, TObj*);
int enhanceWeapon(TBeing*, TObj*, int, short);

extern bool alchemy_create_deny(int);
const int MATERIALIZE_PRICE = 100;
int castMaterialize(TBeing*, const char*);
int materialize(TBeing*, const char*);
int materialize(TBeing*, TObj**, int, const char*, short);

const int SPONT_PRICE = 1000;
void spontaneousGeneration(TBeing*, const char*);
int spontaneousGeneration(TBeing*, TObj**, const char*, int, short);
int castSpontaneousGeneration(TBeing*, const char*);

int copy(TBeing*, TObj*);
int castCopy(TBeing*, TObj*);
void copy(TBeing*, TMagicItem*, TObj*);
int copy(TBeing*, TObj*, int, short);

int etherealGate(TBeing*, const char*);
int castEtherealGate(TBeing*, const char*);
int etherealGate(TBeing*, const char*, int, short);

int galvanize(TBeing*, TObj*);
int castGalvanize(TBeing*, TObj*);
int galvanize(TBeing*, TObj*, int, short);
