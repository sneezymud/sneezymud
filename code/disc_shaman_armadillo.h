#ifndef __DISC_SHAMAN_ARMADILLO_H
#define __DISC_SHAMAN_ARMADILLO_H

// This is the ARMADILLO discipline for shaman.

class CDShamanArmadillo : public CDiscipline
{
public:
    CSkill skAqualung;
    CSkill skThornflesh;
    CSkill skShadowWalk;

    CDShamanArmadillo() 
      : CDiscipline(),
      skAqualung(),
      skThornflesh(),
      skShadowWalk() {
    }
    CDShamanArmadillo(const CDShamanArmadillo &a) 
      : CDiscipline(a),
      skAqualung(a.skAqualung),
      skThornflesh(a.skThornflesh),
      skShadowWalk(a.skShadowWalk) {
    }
    CDShamanArmadillo & operator=(const CDShamanArmadillo &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skAqualung = a.skAqualung;
      skThornflesh = a.skThornflesh;
      skShadowWalk = a.skShadowWalk;
      return *this;
    }
    virtual ~CDShamanArmadillo() {}
    virtual CDShamanArmadillo * cloneMe() { return new CDShamanArmadillo(*this); }
private:
};

int thornflesh(TBeing *caster);
int castThornflesh(TBeing *caster);
int thornflesh(TBeing *caster, int level, byte bKnown);

int aqualung(TBeing * caster, TBeing * victim, int level, byte bKnown);
void aqualung(TBeing * caster, TBeing * victim, TMagicItem * obj);
int aqualung(TBeing * caster, TBeing * victim);
int castAqualung(TBeing * caster, TBeing * victim);

int shadowWalk(TBeing *, TBeing *);
int castShadowWalk(TBeing *, TBeing *);
void shadowWalk(TBeing *, TBeing *, TMagicItem *);
int shadowWalk(TBeing *, TBeing *, int, byte);

#endif


