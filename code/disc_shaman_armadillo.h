#ifndef __DISC_SHAMAN_ARMADILLO_H
#define __DISC_SHAMAN_ARMADILLO_H

// This is the ARMADILLO discipline for shaman.

class CDShamanArmadillo : public CDiscipline
{
public:
    CSkill skSticksToSnakes;
    CSkill skStormySkies;
    CSkill skTreeWalk;
    CSkill skShapeShift;

    CDShamanArmadillo() 
      : CDiscipline(),
      skSticksToSnakes(),
      skStormySkies(),
      skTreeWalk(),
      skShapeShift() {
    }
    CDShamanArmadillo(const CDShamanArmadillo &a) 
      : CDiscipline(a),
      skSticksToSnakes(a.skSticksToSnakes),
      skStormySkies(a.skStormySkies),
      skTreeWalk(a.skTreeWalk),
      skShapeShift(a.skShapeShift) {
    }
    CDShamanArmadillo & operator=(const CDShamanArmadillo &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSticksToSnakes = a.skSticksToSnakes;
      skStormySkies = a.skStormySkies;
      skTreeWalk = a.skTreeWalk;
      skShapeShift = a.skShapeShift;
      return *this;
    }
    virtual ~CDShamanArmadillo() {}
    virtual CDShamanArmadillo * cloneMe() { return new CDShamanArmadillo(*this); }
private:
};

    int sticksToSnakes(TBeing *, TBeing *);
    int sticksToSnakes(TBeing *, TBeing *, TMagicItem *);
    int sticksToSnakes(TBeing *, TBeing *, int, byte);

    void livingVines(TBeing *, TBeing *);
    void livingVines(TBeing *, TBeing *, TMagicItem *);
    int livingVines(TBeing *, TBeing *, int, byte);
    
    int stormySkies(TBeing *, TBeing *, int, byte);
    int stormySkies(TBeing *, TBeing *, TMagicItem *);
    int stormySkies(TBeing *, TBeing *);

    int treeWalk(TBeing *, const char *, int, byte);
    int treeWalk(TBeing *, const char *);

    int shapeShift(TBeing *caster, int level, byte bKnown);
    int shapeShift(TBeing *caster, const char * buffer);
    int castShapeShift(TBeing *caster);


#endif


