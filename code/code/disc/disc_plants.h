//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_PLANTS_H
#define __DISC_PLANTS_H

// This is the RANGER plants discipline.

#include "discipline.h"
#include "skills.h"

class CDPlants : public CDiscipline
{
public:
    CSkill skApplyHerbs;

    CDPlants()
      : CDiscipline(),
      skApplyHerbs() {
    }
    CDPlants(const CDPlants &a)
      : CDiscipline(a),
      skApplyHerbs(a.skApplyHerbs){
    }
    CDPlants & operator= (const CDPlants &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skApplyHerbs = a.skApplyHerbs;

      return *this;
    }
    virtual ~CDPlants() {}
    virtual CDPlants * cloneMe() { return new CDPlants(*this); }

private:
};

#endif

