#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanMartial : public CDiscipline
{
  public:
    CSkill skSwitchDeikhan;
    CSkill skRetreatDeikhan;
    CSkill skShoveDeikhan;
    CSkill sk2hSpecDeikhan;
    CSkill skShockCavalry;
    CSkill skOrient;

    CDDeikhanMartial()
      : CDiscipline(),
      skSwitchDeikhan(),
      skRetreatDeikhan(),
      skShoveDeikhan(),
      sk2hSpecDeikhan(),
      skShockCavalry(),
      skOrient()
    {
    }
    CDDeikhanMartial(const CDDeikhanMartial &a)
      : CDiscipline(a),
      skSwitchDeikhan(a.skSwitchDeikhan),
      skRetreatDeikhan(a.skRetreatDeikhan),
      skShoveDeikhan(a.skShoveDeikhan),
      sk2hSpecDeikhan(a.sk2hSpecDeikhan),
      skShockCavalry(a.skShockCavalry),
      skOrient(a.skOrient)
    {
    }
    CDDeikhanMartial & operator=(const CDDeikhanMartial &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSwitchDeikhan = a.skSwitchDeikhan;
      skRetreatDeikhan = a.skRetreatDeikhan;
      skShoveDeikhan = a.skShoveDeikhan;
      sk2hSpecDeikhan = a.sk2hSpecDeikhan;
      skShockCavalry= a.skShockCavalry;
      skOrient = a.skOrient;
      return *this;
    }
    virtual ~CDDeikhanMartial() {}
    virtual CDDeikhanMartial * cloneMe() { return new CDDeikhanMartial(*this); }

private:
};