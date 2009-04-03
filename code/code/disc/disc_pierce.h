//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_pierce.h,v $
// Revision 5.2  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
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


#ifndef __DISC_PIERCE_H
#define __DISC_PIERCE_H

// This is the PIERCE discipline.

class CDPierce : public CDiscipline
{
public:
    CSkill skPierceSpec;

    CDPierce();
    CDPierce(const CDPierce &a);
    CDPierce & operator=(const CDPierce &a);
    virtual ~CDPierce();
    virtual CDPierce * cloneMe() { return new CDPierce(*this); }

    bool isFast(){ return true; }

private:
};

#endif

