//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_smythe.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SMYTHE_H
#define __DISC_SMYTHE_H

// This is the smythe discipline.

class CDSmythe : public CDiscipline
{
public:
    CDSmythe();
    CDSmythe(const CDSmythe &a);
    CDSmythe & operator=(const CDSmythe &a);
    virtual ~CDSmythe();
    virtual CDSmythe * cloneMe() { return new CDSmythe(*this); }

private:
};

#endif

