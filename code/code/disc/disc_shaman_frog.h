#pragma once

const int LAST_TRANSFORM_LIMB = 6;
extern struct TransformLimbType TransformLimbList[LAST_TRANSFORM_LIMB];

#include "discipline.h"
#include "skills.h"

class CDShamanFrog : public CDiscipline {
  public:
    CSkill skStormySkies;
    CSkill skDeathWave;
    CSkill skAquaticBlast;
    CSkill skTransformLimb;
    CSkill skCreepingDoom;
    CSkill skShapeShift;

    virtual CDShamanFrog* cloneMe() { return new CDShamanFrog(*this); }

  private:
};
int stormySkies(TBeing*, TBeing*);
int castStormySkies(TBeing*, TBeing*);
int stormySkies(TBeing*, TBeing*, TMagicItem*);
int stormySkies(TBeing*, TBeing*, int, short);

int aquaticBlast(TBeing*, TBeing*);
int castAquaticBlast(TBeing*, TBeing*);
int aquaticBlast(TBeing*, TBeing*, TMagicItem*);
int aquaticBlast(TBeing*, TBeing*, int, short, int);

int shapeShift(TBeing* caster, int level, short bKnown);
int shapeShift(TBeing* caster, const char* buffer);
int castShapeShift(TBeing* caster);

int deathWave(TBeing*, TBeing*);
int castDeathWave(TBeing*, TBeing*);
int deathWave(TBeing*, TBeing*, TMagicItem*);
int deathWave(TBeing*, TBeing*, int, short, int);

int transformLimb(TBeing*, const char*);
int castTransformLimb(TBeing*);
int transformLimb(TBeing*, const char*, int, short);

int distort(TBeing*, TBeing*);
int castDistort(TBeing*, TBeing*);
int distort(TBeing*, TBeing*, TMagicItem*);
int distort(TBeing*, TBeing*, int, short, int);
