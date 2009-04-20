

#ifndef __DISC_SHAMAN_SKUNK_H
#define __DISC_SHAMAN_SKUNK_H


#include "discipline.h"
#include "skills.h"

class CDShamanSkunk : public CDiscipline
{
public:
    CSkill skTurnSkill;
    CSkill skDeathMist;
    CSkill skBloodBoil;
    CSkill skCardiacStress;
    CSkill skCleanse;
    CSkill skLichTouch;

    CDShamanSkunk()
      : CDiscipline(),
      skTurnSkill(),
      skDeathMist(),
      skBloodBoil(),
      skCardiacStress(),
      skCleanse(),
      skLichTouch() {
    }
    CDShamanSkunk(const CDShamanSkunk &a)
      : CDiscipline(a),
      skTurnSkill(a.skTurnSkill),
      skDeathMist(a.skDeathMist),
      skBloodBoil(a.skBloodBoil),
      skCardiacStress(a.skCardiacStress),
      skCleanse(a.skCleanse),
      skLichTouch(a.skLichTouch) {
    }
    CDShamanSkunk & operator=(const CDShamanSkunk &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTurnSkill = a.skTurnSkill;
      skDeathMist = a.skDeathMist;
      skBloodBoil = a.skBloodBoil;
      skCardiacStress = a.skCardiacStress;
      skCleanse = a.skCleanse;
      skLichTouch = a.skLichTouch;
      return *this;
    }
    virtual ~CDShamanSkunk() {}
    virtual CDShamanSkunk * cloneMe() { return new CDShamanSkunk(*this); }

private:
};
    int deathMist(TBeing * caster, int level, short bKnown, int adv_learn);
    int deathMist(TBeing * caster);
    int castDeathMist(TBeing * caster);

    int lichTouch(TBeing *, TBeing *);
    int castLichTouch(TBeing *, TBeing *);
    int lichTouch(TBeing *, TBeing *, int, short, int);
    int lichTouch(TBeing *, TBeing *, TMagicItem *);

    int cleanse(TBeing *, TBeing *);
    int castCleanse(TBeing *, TBeing *);
    int cleanse(TBeing *, TBeing *, int, short, spellNumT);
    int cleanse(TBeing *, TBeing *, TMagicItem *);

    int cardiacStress(TBeing *, TBeing *);
    int castCardiacStress(TBeing *, TBeing *);
    int cardiacStress(TBeing *, TBeing *, TMagicItem *);
    int cardiacStress(TBeing *, TBeing *, int, short, int);

    int bloodBoil(TBeing *, TBeing *);
    int castBloodBoil(TBeing *, TBeing *);
    int bloodBoil(TBeing *, TBeing *, TMagicItem *);
    int bloodBoil(TBeing *, TBeing *, int, short, int);

#endif



