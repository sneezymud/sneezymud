#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanSkunk : public CDiscipline {
  public:
    CSkill skDeathMist;
    CSkill skBloodBoil;
    CSkill skCardiacStress;
    CSkill skCleanse;
    CSkill skLichTouch;

    virtual CDShamanSkunk* cloneMe() { return new CDShamanSkunk(*this); }

  private:
};
int deathMist(TBeing* caster, int level, short bKnown, int adv_learn);
int deathMist(TBeing* caster);
int castDeathMist(TBeing* caster);

int lichTouch(TBeing*, TBeing*);
int castLichTouch(TBeing*, TBeing*);
int lichTouch(TBeing*, TBeing*, int, short, int);
int lichTouch(TBeing*, TBeing*, TMagicItem*);

int cleanse(TBeing*, TBeing*);
int castCleanse(TBeing*, TBeing*);
int cleanse(TBeing*, TBeing*, int, short, spellNumT);
int cleanse(TBeing*, TBeing*, TMagicItem*);

int cardiacStress(TBeing*, TBeing*);
int castCardiacStress(TBeing*, TBeing*);
int cardiacStress(TBeing*, TBeing*, TMagicItem*);
int cardiacStress(TBeing*, TBeing*, int, short, int);

int bloodBoil(TBeing*, TBeing*);
int castBloodBoil(TBeing*, TBeing*);
int bloodBoil(TBeing*, TBeing*, TMagicItem*);
int bloodBoil(TBeing*, TBeing*, int, short, int);

int vampiricTouch(TBeing*, TBeing*);
int castVampiricTouch(TBeing*, TBeing*);
int vampiricTouch(TBeing*, TBeing*, int, short, int);
int vampiricTouch(TBeing*, TBeing*, TMagicItem*);

int lifeLeech(TBeing*, TBeing*);
int castLifeLeech(TBeing*, TBeing*);
int lifeLeech(TBeing*, TBeing*, int, short, int);
int lifeLeech(TBeing*, TBeing*, TMagicItem*);

int intimidate(TBeing*, TBeing*);
int castIntimidate(TBeing*, TBeing*);
int intimidate(TBeing*, TBeing*, int, short);

int flatulence(TBeing*);
int castFlatulence(TBeing*);
int flatulence(TBeing*, int, short, int);

int soulTwist(TBeing*, TBeing*);
int castSoulTwist(TBeing*, TBeing*);
int soulTwist(TBeing*, TBeing*, TMagicItem*);
int soulTwist(TBeing*, TBeing*, int, short, int);