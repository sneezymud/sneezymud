//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_plants.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_PLANTS_H
#define __DISC_PLANTS_H

// This is the RANGER plants discipline.

class CDPlants : public CDiscipline
{
public:
    CSkill skConcealment;
    CSkill skApplyHerbs;

    CDPlants()
      : CDiscipline(),
      skConcealment(),
      skApplyHerbs() {
    }
    CDPlants(const CDPlants &a)
      : CDiscipline(a),
      skConcealment(a.skConcealment),
      skApplyHerbs(a.skApplyHerbs) {
    }
    CDPlants & operator= (const CDPlants &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skConcealment = a.skConcealment;
      skApplyHerbs = a.skApplyHerbs;
      return *this;
    }
    virtual ~CDPlants() {}
    virtual CDPlants * cloneMe() { return new CDPlants(*this); }

private:
};

#endif

