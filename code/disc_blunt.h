//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_blunt.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_BLUNT_H
#define __DISC_BLUNT_H

// This is the BLUNT discipline.

class CDBash : public CDiscipline
{
public:
    CSkill skBluntSpec;

    CDBash();
    CDBash(const CDBash &a);
    CDBash & operator=(const CDBash &a);
    virtual ~CDBash();
    virtual CDBash * cloneMe() { return new CDBash(*this); }

private:
};

#endif

