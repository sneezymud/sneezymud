#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanArmadillo : public CDiscipline {
  public:
    CSkill skAqualung;
    CSkill skThornflesh;
    CSkill skCelerite;
    CSkill skEarthmaw;
    CSkill skShadowWalk;

    virtual CDShamanArmadillo* cloneMe() { return new CDShamanArmadillo(*this);}

  private:
};

int thornflesh(TBeing* caster);
int castThornflesh(TBeing* caster);
int thornflesh(TBeing* caster, int level, short bKnown);

int aqualung(TBeing* caster, TBeing* victim, int level, short bKnown);
void aqualung(TBeing* caster, TBeing* victim, TMagicItem* obj);
int aqualung(TBeing* caster, TBeing* victim);
int castAqualung(TBeing* caster, TBeing* victim);

int shadowWalk(TBeing*, TBeing*);
int castShadowWalk(TBeing*, TBeing*);
void shadowWalk(TBeing*, TBeing*, TMagicItem*);
int shadowWalk(TBeing*, TBeing*, int, short);

int celerite(TBeing*, TBeing*);
int castCelerite(TBeing*, TBeing*);
void celerite(TBeing*, TBeing*, TMagicItem*);
int celerite(TBeing*, TBeing*, int, short);

int shieldOfMists(TBeing*, TBeing*);
int castShieldOfMists(TBeing*, TBeing*);
void shieldOfMists(TBeing*, TBeing*, TMagicItem*);
int shieldOfMists(TBeing*, TBeing*, int, short);

int senseLifeShaman(TBeing*, TBeing*);
int castSenseLifeShaman(TBeing*, TBeing*);
void senseLifeShaman(TBeing*, TBeing*, TMagicItem*);
int senseLifeShaman(TBeing*, TBeing*, int, short);

int detectShadow(TBeing*, TBeing*);
int castDetectShadow(TBeing*, TBeing*);
void detectShadow(TBeing*, TBeing*, TMagicItem*);
int detectShadow(TBeing*, TBeing*, int, short);

int djallasProtection(TBeing*, TBeing*);
int castDjallasProtection(TBeing*, TBeing*);
void djallasProtection(TBeing*, TBeing*, TMagicItem*);
int djallasProtection(TBeing*, TBeing*, int, short);

int legbasGuidance(TBeing*, TBeing*);
int castLegbasGuidance(TBeing*, TBeing*);
void legbasGuidance(TBeing*, TBeing*, TMagicItem*);
int legbasGuidance(TBeing*, TBeing*, int, short);

int cheval(TBeing*, TBeing*);
int castCheval(TBeing*, TBeing*);
void cheval(TBeing*, TBeing*, TMagicItem*);
int cheval(TBeing*, TBeing*, int, short);

int chaseSpirits(TBeing*, TObj*);
int castChaseSpirits(TBeing*, TObj*);
void chaseSpirits(TBeing*, TObj*, TMagicItem*);
int chaseSpirits(TBeing*, TObj*, int, short);
int chaseSpirits(TBeing*, TBeing*);
int castChaseSpirits(TBeing*, TBeing*);
int chaseSpirits(TBeing*, TBeing*, TMagicItem*);
int chaseSpirits(TBeing*, TBeing*, int, short);
