//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_PLANTS_H
#define __DISC_PLANTS_H

// This is the RANGER plants discipline.

class CDPlants : public CDiscipline
{
public:
    CSkill skApplyHerbs;
    CSkill skEarthmaw;

    CDPlants()
      : CDiscipline(),
      skApplyHerbs(),
      skEarthmaw() {
    }
    CDPlants(const CDPlants &a)
      : CDiscipline(a),
      skApplyHerbs(a.skApplyHerbs),
      skEarthmaw(a.skEarthmaw) {
    }
    CDPlants & operator= (const CDPlants &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skApplyHerbs = a.skApplyHerbs;
      skEarthmaw = a.skEarthmaw;
      return *this;
    }
    virtual ~CDPlants() {}
    virtual CDPlants * cloneMe() { return new CDPlants(*this); }

private:
};

#endif

