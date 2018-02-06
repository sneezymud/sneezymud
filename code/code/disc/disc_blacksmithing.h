//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_blacksmithing.h,v $
// Revision 5.1  2004/12/06 18:39:47  peel
// renamed smythe to blacksmithing
// renamed hand to hand to dueling
// renamed physical to soldiering
// added weapon retention and brawl avoidance skill stubs
//
// Revision 5.2  2002/07/04 18:34:11  dash
// added new repair skills
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


#ifndef __DISC_BLACKSMITHING_H
#define __DISC_BLACKSMITHING_H

// This is the blacksmith discipline.

#include "discipline.h"
#include "skills.h"


class CDBlacksmithing : public CDiscipline
{
public:
  CSkill skBlacksmithingAdvanced;
    CDBlacksmithing();
    CDBlacksmithing(const CDBlacksmithing &a);
    CDBlacksmithing & operator=(const CDBlacksmithing &a);
    virtual ~CDBlacksmithing();
    virtual CDBlacksmithing * cloneMe() { return new CDBlacksmithing(*this); }

private:
};

#endif

