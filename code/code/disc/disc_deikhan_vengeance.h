#pragma once

#include "discipline.h"
#include "skills.h"

class CDDeikhanVengeance : public CDiscipline
{
public:
    CSkill skHarmDeikhan;
    CSkill skNumbDeikhan;
    CSkill skSmite;
    CSkill skHolyLight;    // not coded

    CDDeikhanVengeance()
      : CDiscipline(),
      skHarmDeikhan(),
      skNumbDeikhan(),
      skSmite(),
      skHolyLight() {
    }
    CDDeikhanVengeance(const CDDeikhanVengeance &a)
      : CDiscipline(a),
      skHarmDeikhan(a.skHarmDeikhan),
      skNumbDeikhan(a.skNumbDeikhan),
      skSmite(a.skSmite),
      skHolyLight(a.skHolyLight) {
    }
    CDDeikhanVengeance & operator=(const CDDeikhanVengeance &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHarmDeikhan = a.skHarmDeikhan;
      skNumbDeikhan = a.skNumbDeikhan;
      skSmite = a.skSmite;
      skHolyLight = a.skHolyLight;
      return *this;
    }
    virtual ~CDDeikhanVengeance() {}
    virtual CDDeikhanVengeance * cloneMe() { return new CDDeikhanVengeance(*this); }

};