//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
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
    CSkill skTaunt;

    CDBrawling();
    CDBrawling(const CDBrawling &a);
    CDBrawling & operator=(const CDBrawling &a);
    virtual ~CDBrawling();
    virtual CDBrawling * cloneMe() { return new CDBrawling(*this); }

  private:
};

#endif








