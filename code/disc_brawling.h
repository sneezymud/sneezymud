//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_brawling.h,v $
// Revision 5.3  2004/12/09 05:41:16  peel
// added close quarters fighting skill
//
// Revision 5.2  2004/12/06 18:39:47  peel
// renamed smythe to blacksmithing
// renamed hand to hand to dueling
// renamed physical to soldiering
// added weapon retention and brawl avoidance skill stubs
//
// Revision 5.1.1.2  2000/10/23 05:55:41  jesus
// added spin skill
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


#ifndef __DISC_BRAWLING_H
#define __DISC_BRAWLING_H

// This is the Brawling discipline.

class CDBrawling : public CDiscipline
{
  public:
    CSkill skGrapple;
    CSkill skStomp;
    CSkill skBrawlAvoidance;
    CSkill skBodyslam;
    CSkill skSpin;
    CSkill skCloseQuartersFighting;

    CDBrawling();
    CDBrawling(const CDBrawling &a);
    CDBrawling & operator=(const CDBrawling &a);
    virtual ~CDBrawling();
    virtual CDBrawling * cloneMe() { return new CDBrawling(*this); }

  private:
};

#endif








