//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_physical.h,v $
// Revision 5.1.1.3  2000/11/11 10:21:01  jesus
// added powermove skill
//
// Revision 5.1.1.2  2000/10/26 05:43:34  jesus
// dual wield for warriors
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


#ifndef __DISC_PHYSICAL_H
#define __DISC_PHYSICAL_H

// This is the WARRIOR PHYSICAL discipline.

class CDPhysical : public CDiscipline
{
public:
    CSkill skDoorbash;
    CSkill skDualWieldWarrior;
    CSkill skPowerMove;
    CSkill skDeathstroke;

    CDPhysical();
    CDPhysical(const CDPhysical &a);
    CDPhysical & operator=(const CDPhysical &a);
    virtual ~CDPhysical();
    virtual CDPhysical * cloneMe() { return new CDPhysical(*this); }

private:
};


#endif

