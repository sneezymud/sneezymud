//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_ranged.h,v $
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


#ifndef __DISC_RANGED_H
#define __DISC_RANGED_H

// This is the RANGED discipline.

class CDRanged : public CDiscipline
{
public:
    CSkill skRangedSpec;
    CSkill skFastLoad;

    CDRanged() :
      CDiscipline(),
      skRangedSpec(),
      skFastLoad() {
    }
    CDRanged(const CDRanged &a) :
      CDiscipline(a),
      skRangedSpec(a.skRangedSpec),
      skFastLoad(a.skFastLoad) {
    }
    CDRanged & operator=(const CDRanged &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skRangedSpec = a.skRangedSpec;
      skFastLoad = a.skFastLoad;
      return *this;
    }
    virtual ~CDRanged() {}
    virtual CDRanged * cloneMe() { return new CDRanged(*this); }

private:
};

#endif

