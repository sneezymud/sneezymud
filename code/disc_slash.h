//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_slash.h,v $
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

private:
};

#endif

