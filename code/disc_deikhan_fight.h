//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_fight.h,v $
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


#ifndef __DISC_DEIKHAN_FIGHT_H
#define __DISC_DEIKHAN_FIGHT_H

// This is the DEIKHAN FIGHTING skills discipline.

class CDDeikhanFight : public CDiscipline
{
  public:
    CSkill skSwitchDeikhan;
    CSkill skRetreatDeikhan;
    CSkill skShoveDeikhan;

    CDDeikhanFight()
      : CDiscipline(),
      skSwitchDeikhan(),
      skRetreatDeikhan(),
      skShoveDeikhan()
    {
    }
    CDDeikhanFight(const CDDeikhanFight &a)
      : CDiscipline(a),
      skSwitchDeikhan(a.skSwitchDeikhan),
      skRetreatDeikhan(a.skRetreatDeikhan),
      skShoveDeikhan(a.skShoveDeikhan)
    {
    }
    CDDeikhanFight & operator=(const CDDeikhanFight &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSwitchDeikhan = a.skSwitchDeikhan;
      skRetreatDeikhan = a.skRetreatDeikhan;
      skShoveDeikhan = a.skShoveDeikhan;
      return *this;
    }
    virtual ~CDDeikhanFight() {}
    virtual CDDeikhanFight * cloneMe() { return new CDDeikhanFight(*this); }

private:
};


#endif

