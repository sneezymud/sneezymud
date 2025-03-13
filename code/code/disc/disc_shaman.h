#pragma once

#include "discipline.h"
#include "skills.h"

class CDShaman : public CDiscipline {
  public:
    CSkill skSacrifice;
    CSkill skShieldOfMists;
    CSkill skEnthrallSpectre;
    CSkill skEnthrallGhast;
    CSkill skEnthrallGhoul;
    CSkill skVoodoo;
    CSkill skFlatulence;
    CSkill skStupidity;
    CSkill skSoulTwist;
    CSkill skDistort;
    CSkill skSquish;
    CSkill skIntimidate;
    CSkill skCheval;
    CSkill skChaseSpirits;
    CSkill skChrism;
    CSkill skRombler;
    CSkill skVampiricTouch;
    CSkill skLifeLeech;
    CSkill skSenseLifeShaman;
    CSkill skDetectShadow;
    CSkill skDjallasProtection;
    CSkill skLegbasGuidance;
    CSkill skDancingBones;
    CSkill skRepairShaman;
    CSkill skEmbalm;

    virtual CDShaman* cloneMe() { return new CDShaman(*this); }

    bool isBasic() { return true; }

  private:
};

void sacrifice(TBeing*, TBaseCorpse*);

int castRombler(TBeing*);
int rombler(TBeing*, const char*);
int rombler(TBeing*, const char*, int, short);

int embalm(TBeing*, TObj*);
int castEmbalm(TBeing*, TObj*);
int embalm(TBeing*, TObj*, int, short);