//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_slash.h,v $
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


#ifndef __DISC_SLASH_H
#define __DISC_SLASH_H

// This is the SLASH discipline.

class CDSlash : public CDiscipline
{
public:
    CSkill skSlashSpec;

    CDSlash();
    CDSlash(const CDSlash &a);
    CDSlash & operator=(const CDSlash &a);
    virtual ~CDSlash();
    virtual CDSlash * cloneMe() { return new CDSlash(*this); }

    bool isFast(){ return true; }

private:
};

#endif

