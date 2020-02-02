//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_DEIKHAN_GUARDIAN_H
#define __DISC_DEIKHAN_GUARDIAN_H

// This is the DEIKHAN guardian discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanGuardian : public CDiscipline
{
public:
    CSkill skHeroesFeastDeikhan;
    CSkill skRefreshDeikhan;
    CSkill skSynostodweomer;
    CSkill skDivineGrace;

    CDDeikhanGuardian()
      : CDiscipline(),
      skHeroesFeastDeikhan(),
      skRefreshDeikhan(),
      skSynostodweomer(),
      skDivineGrace() {
    }
    CDDeikhanGuardian(const CDDeikhanGuardian &a)
      : CDiscipline(a),
      skHeroesFeastDeikhan(a.skHeroesFeastDeikhan),
      skRefreshDeikhan(a.skRefreshDeikhan),
      skSynostodweomer(a.skSynostodweomer),
      skDivineGrace(a.skDivineGrace) {
    }
    CDDeikhanGuardian & operator=(const CDDeikhanGuardian &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHeroesFeastDeikhan = a.skHeroesFeastDeikhan;
      skRefreshDeikhan = a.skRefreshDeikhan;
      skSynostodweomer = a.skSynostodweomer;
      skDivineGrace = a.skDivineGrace;
      return *this;
    }
    virtual ~CDDeikhanGuardian() {}
    virtual CDDeikhanGuardian * cloneMe() { return new CDDeikhanGuardian(*this); }

private:
};

#endif

