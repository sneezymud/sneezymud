//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_cures.h,v $
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


#ifndef __DISC_DEIKHAN_CURES_H
#define __DISC_DEIKHAN_CURES_H

// This is the DEIKHAN CURES discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanCures : public CDiscipline
{
public:
    CSkill skSalveDeikhan;
    CSkill skLayHands;

    CDDeikhanCures()
      : CDiscipline(),
      skSalveDeikhan(),
      skLayHands() {
    }
    CDDeikhanCures(const CDDeikhanCures &a)
      : CDiscipline(a),
      skSalveDeikhan(a.skSalveDeikhan),
      skLayHands(a.skLayHands) {
    }
    CDDeikhanCures & operator=(const CDDeikhanCures &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSalveDeikhan = a.skSalveDeikhan;
      skLayHands = a.skLayHands;
      return *this;
    }
    virtual ~CDDeikhanCures() {}
    virtual CDDeikhanCures * cloneMe() { return new CDDeikhanCures(*this); }

private:
};


#endif

