//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman_fight.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SHAMAN_FIGHT_H
#define __DISC_SHAMAN_FIGHT_H

// This is the SHAMAN FIGHTING skills discipline.

class CDShamanFight : public CDiscipline
{
public:
    CSkill skTurnSkill;

    CDShamanFight() 
      : CDiscipline(), 
      skTurnSkill() {
    }
    CDShamanFight(const CDShamanFight &a) 
      : CDiscipline(a), 
      skTurnSkill(a.skTurnSkill) {
    }
    CDShamanFight & operator=(const CDShamanFight &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTurnSkill = a.skTurnSkill;
      return *this;
    }
    virtual ~CDShamanFight() {}
    virtual CDShamanFight * cloneMe() { return new CDShamanFight(*this); }

private:
};


#endif

