//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_martial.h,v $
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


#ifndef __DISC_DEIKHAN_MARTIAL_H
#define __DISC_DEIKHAN_MARTIAL_H

// This is the DEIKHAN FIGHTING skills discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanMartial : public CDiscipline
{
  public:
    CSkill skSwitchDeikhan;
    CSkill skRetreatDeikhan;
    CSkill skShoveDeikhan;
    CSkill sk2hSpecDeikhan;

    CDDeikhanMartial()
      : CDiscipline(),
      skSwitchDeikhan(),
      skRetreatDeikhan(),
      skShoveDeikhan(),
      sk2hSpecDeikhan()
    {
    }
    CDDeikhanMartial(const CDDeikhanMartial &a)
      : CDiscipline(a),
      skSwitchDeikhan(a.skSwitchDeikhan),
      skRetreatDeikhan(a.skRetreatDeikhan),
      skShoveDeikhan(a.skShoveDeikhan),
      sk2hSpecDeikhan(a.sk2hSpecDeikhan)
    {
    }
    CDDeikhanMartial & operator=(const CDDeikhanMartial &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSwitchDeikhan = a.skSwitchDeikhan;
      skRetreatDeikhan = a.skRetreatDeikhan;
      skShoveDeikhan = a.skShoveDeikhan;
      sk2hSpecDeikhan = a.sk2hSpecDeikhan;
      return *this;
    }
    virtual ~CDDeikhanMartial() {}
    virtual CDDeikhanMartial * cloneMe() { return new CDDeikhanMartial(*this); }

private:
};


#endif

