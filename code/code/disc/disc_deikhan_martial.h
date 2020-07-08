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

};