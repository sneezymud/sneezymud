//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_pierce.h,v $
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

private:
};

#endif

