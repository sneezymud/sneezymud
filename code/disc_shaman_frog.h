#ifndef __DISC_SHAMAN_FROG_H
#define __DISC_SHAMAN_FROG_H

const int LAST_TRANSFORM_LIMB = 6;
extern struct TransformLimbType TransformLimbList[LAST_TRANSFORM_LIMB];


class CDShamanFrog : public CDiscipline
{
public:
    CSkill skStormySkies;
    CSkill skDeathWave;
    CSkill skAquaticBlast;
    CSkill skTransformLimb;
    CSkill skCreepingDoom;
    CSkill skShapeShift;

    CDShamanFrog() 
      : CDiscipline(),
      skStormySkies(),
      skDeathWave(),
      skAquaticBlast(),
      skTransformLimb(),
      skCreepingDoom(),
      skShapeShift() {
    }
    CDShamanFrog(const CDShamanFrog &a) 
      : CDiscipline(a),
      skStormySkies(a.skStormySkies),
      skDeathWave(a.skDeathWave),
      skAquaticBlast(a.skAquaticBlast),
      skTransformLimb(a.skTransformLimb),
      skCreepingDoom(a.skCreepingDoom),
      skShapeShift(a.skShapeShift) {
    }
    CDShamanFrog & operator=(const CDShamanFrog &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skStormySkies = a.skStormySkies;
      skDeathWave = a.skDeathWave;
      skAquaticBlast = a.skAquaticBlast;
      skShapeShift = a.skShapeShift;
      skTransformLimb = a.skTransformLimb;
      skCreepingDoom = a.skCreepingDoom;
      return *this;
    }
    virtual ~CDShamanFrog() {}
    virtual CDShamanFrog * cloneMe() { return new CDShamanFrog(*this); }
private:
};
    int stormySkies(TBeing *, TBeing *);
    int castStormySkies(TBeing *, TBeing *);
    int stormySkies(TBeing *, TBeing *, TMagicItem *);
    int stormySkies(TBeing *, TBeing *, int, byte);

    int aquaticBlast(TBeing *, TBeing *);
    int castAquaticBlast(TBeing *, TBeing *);
    int aquaticBlast(TBeing *, TBeing *, TMagicItem *);
    int aquaticBlast(TBeing *, TBeing *, int, byte, int);

    int shapeShift(TBeing *caster, int level, byte bKnown);
    int shapeShift(TBeing *caster, const char * buffer);
    int castShapeShift(TBeing *caster);

    int deathWave(TBeing *, TBeing *);
    int castDeathWave(TBeing *, TBeing *);
    int deathWave(TBeing *, TBeing *, TMagicItem *);
    int deathWave(TBeing *, TBeing *, int, byte, int);


    int transformLimb(TBeing *, const char *);
    int castTransformLimb(TBeing *);
    int transformLimb(TBeing *, const char *, int, byte);


#endif


