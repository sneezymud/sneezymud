#ifndef __DISC_ANIMAL_H
#define __DISC_ANIMAL_H

// This is the ANIMAL discipline.

#include "discipline.h"
#include "skills.h"

class CDAnimal : public CDiscipline
{
public:
    CSkill skBeastCharm;
    CSkill skFeralWrath;
    CSkill skSkySpirit;

    CDAnimal()
      : CDiscipline(),
        skBeastCharm(),
        skFeralWrath(),
        skSkySpirit() {
    }
    CDAnimal(const CDAnimal &a)
      : CDiscipline(a),
        skBeastCharm(a.skBeastCharm),
        skFeralWrath(a.skFeralWrath),
        skSkySpirit(a.skSkySpirit) {
    }
    CDAnimal & operator=(const CDAnimal &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skBeastCharm = a.skBeastCharm;
      skFeralWrath = a.skFeralWrath;
      skSkySpirit = a.skSkySpirit;
      return *this;
    }
    virtual ~CDAnimal() {}
    virtual CDAnimal * cloneMe() { return new CDAnimal(*this); }
private:
};

    int beastSoother(TBeing *, TBeing *);
    int beastSoother(TBeing *, TBeing *, TMagicItem *);
    int beastSoother(TBeing *, TBeing *, int, short);
 
    int beastSummon(TBeing *, const char *);
    int beastSummon(TBeing *, const char *, int, short);


#endif
