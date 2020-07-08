#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanGuardian : public CDiscipline
{
public:
    CSkill skSynostodweomer;
    CSkill skDivineGrace;
    CSkill skDivineRescue;

    CDDeikhanGuardian()
      : CDiscipline(),
      skSynostodweomer(),
      skDivineGrace(),
      skDivineRescue() {
    }
    CDDeikhanGuardian(const CDDeikhanGuardian &a)
      : CDiscipline(a),
      skSynostodweomer(a.skSynostodweomer),
      skDivineGrace(a.skDivineGrace),
      skDivineRescue(a.skDivineRescue) {
    }
    CDDeikhanGuardian & operator=(const CDDeikhanGuardian &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSynostodweomer = a.skSynostodweomer;
      skDivineGrace = a.skDivineGrace;
      skDivineRescue = a.skDivineRescue;
      return *this;
    }
    virtual ~CDDeikhanGuardian() {}
    virtual CDDeikhanGuardian * cloneMe() { return new CDDeikhanGuardian(*this); }

};