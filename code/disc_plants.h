//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_plants.h,v $
// Revision 5.4  2004/08/24 19:00:26  peel
// moved concealment and track to thief and stealth discs, respectively
//
// Revision 5.3  2001/07/05 21:25:54  peel
// Trying to fix cvs
// what a headache
//
// Revision 5.2  2001/04/26 22:23:57  peel
// *** empty log message ***
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
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
    CSkill skApplyHerbs;
    CSkill skEarthmaw;
    CSkill skCreepingDoom;

    CDPlants()
      : CDiscipline(),
      skApplyHerbs(),
      skEarthmaw(),
      skCreepingDoom() {
    }
    CDPlants(const CDPlants &a)
      : CDiscipline(a),
      skApplyHerbs(a.skApplyHerbs),
      skEarthmaw(a.skEarthmaw),
      skCreepingDoom(a.skCreepingDoom)  {
    }
    CDPlants & operator= (const CDPlants &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skApplyHerbs = a.skApplyHerbs;
      skEarthmaw = a.skEarthmaw;
      skCreepingDoom = a.skCreepingDoom;
      return *this;
    }
    virtual ~CDPlants() {}
    virtual CDPlants * cloneMe() { return new CDPlants(*this); }

private:
};

#endif

