//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_DEIKHAN_VENGEANCE_H
#define __DISC_DEIKHAN_VENGEANCE_H

// This is the DEIKHAN VENGEANCE discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanVengeance : public CDiscipline
{
public:
    CSkill skHarmDeikhan;
    CSkill skNumbDeikhan;
    CSkill skEarthquakeDeikhan;
    CSkill skCallLightningDeikhan;
    CSkill skHolyLight;    // not coded

    CDDeikhanVengeance()
      : CDiscipline(),
      skHarmDeikhan(),
      skNumbDeikhan(),
      skEarthquakeDeikhan(),
      skCallLightningDeikhan(),
      skHolyLight() {
    }
    CDDeikhanVengeance(const CDDeikhanVengeance &a)
      : CDiscipline(a),
      skHarmDeikhan(a.skHarmDeikhan),
      skNumbDeikhan(a.skNumbDeikhan),
      skEarthquakeDeikhan(a.skEarthquakeDeikhan),
      skCallLightningDeikhan(a.skCallLightningDeikhan),
      skHolyLight(a.skHolyLight) {
    }
    CDDeikhanVengeance & operator=(const CDDeikhanVengeance &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHarmDeikhan = a.skHarmDeikhan;
      skNumbDeikhan = a.skNumbDeikhan;
      skEarthquakeDeikhan = a.skEarthquakeDeikhan;
      skCallLightningDeikhan = a.skCallLightningDeikhan;
      skHolyLight = a.skHolyLight;
      return *this;
    }
    virtual ~CDDeikhanVengeance() {}
    virtual CDDeikhanVengeance * cloneMe() { return new CDDeikhanVengeance(*this); }

private:
};

#endif

