//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_hth.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_HTH_H
#define __DISC_HTH_H

// This is the HTH discipline.

class CDHTH : public CDiscipline
{
public:
    CSkill skShove;
    CSkill skRetreat;

    CDHTH();
    CDHTH(const CDHTH &a);
    CDHTH & operator=(const CDHTH &a);
    virtual ~CDHTH();
    virtual CDHTH * cloneMe();
private:
};

    int shove(TBeing *, TBeing *, char *, spellNumT);

#endif

