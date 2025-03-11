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

#pragma once

// This is the RANGED discipline.

#include "discipline.h"
#include "skills.h"

class CDRanged : public CDiscipline {
  public:
    CSkill skRangedSpec;
    CSkill skFastLoad;

    virtual CDRanged* cloneMe() { return new CDRanged(*this); }

  private:
};
