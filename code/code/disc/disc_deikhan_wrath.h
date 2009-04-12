//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_wrath.h,v $
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


#ifndef __DISC_DEIKHAN_WRATH_H
#define __DISC_DEIKHAN_WRATH_H

// This is the DEIKHAN WRATH discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanWrath : public CDiscipline
{
public:
    CSkill skHarmDeikhan;
    CSkill skNumbDeikhan;
    CSkill skEarthquakeDeikhan;
    CSkill skCallLightningDeikhan;
    CSkill skHolyLight;    // not coded

    CDDeikhanWrath()
      : CDiscipline(),
      skHarmDeikhan(),
      skNumbDeikhan(),
      skEarthquakeDeikhan(),
      skCallLightningDeikhan(),
      skHolyLight() {
    }
    CDDeikhanWrath(const CDDeikhanWrath &a)
      : CDiscipline(a),
      skHarmDeikhan(a.skHarmDeikhan),
      skNumbDeikhan(a.skNumbDeikhan),
      skEarthquakeDeikhan(a.skEarthquakeDeikhan),
      skCallLightningDeikhan(a.skCallLightningDeikhan),
      skHolyLight(a.skHolyLight) {
    }
    CDDeikhanWrath & operator=(const CDDeikhanWrath &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHarmDeikhan = a.skHarmDeikhan;
      skNumbDeikhan = a.skNumbDeikhan;
      skEarthquakeDeikhan = a.skEarthquakeDeikhan;
      skCallLightningDeikhan = a.skCallLightningDeikhan;
      skHolyLight = a.skHolyLight;
      return *this;
    }
    virtual ~CDDeikhanWrath() {}
    virtual CDDeikhanWrath * cloneMe() { return new CDDeikhanWrath(*this); }

private:
};

#endif

