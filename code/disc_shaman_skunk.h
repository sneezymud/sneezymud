

#ifndef __DISC_SHAMAN_SKUNK_H
#define __DISC_SHAMAN_SKUNK_H


class CDShamanSkunk : public CDiscipline
{
public:
    CSkill skTurnSkill;
    CSkill skDeathMist;

    CDShamanSkunk()
      : CDiscipline(),
      skTurnSkill(),
      skDeathMist() {
    }
    CDShamanSkunk(const CDShamanSkunk &a)
      : CDiscipline(a),
      skTurnSkill(a.skTurnSkill),
      skDeathMist(a.skDeathMist) {
    }
    CDShamanSkunk & operator=(const CDShamanSkunk &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTurnSkill = a.skTurnSkill;
      skDeathMist = a.skDeathMist;
      return *this;
    }
    virtual ~CDShamanSkunk() {}
    virtual CDShamanSkunk * cloneMe() { return new CDShamanSkunk(*this); }

private:
};
int deathMist(TBeing * caster, int level, byte bKnown, int adv_learn);
int deathMist(TBeing * caster);
int castDeathMist(TBeing * caster);

#endif



