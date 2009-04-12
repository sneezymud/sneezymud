//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_stealth.h,v $
// Revision 5.2  2004/08/24 19:00:26  peel
// moved concealment and track to thief and stealth discs, respectively
//
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


#ifndef __DISC_STEALTH_H
#define __DISC_STEALTH_H

// This is the THIEF Stealth discipline.

#include "discipline.h"
#include "skills.h"

class CDStealth : public CDiscipline
{
public:
    CSkill skConcealment;
    CSkill skDisguise;

    CDStealth()
      : CDiscipline(),
      skConcealment(),
      skDisguise() {
    }
    CDStealth(const CDStealth &a)
      : CDiscipline(a),
      skConcealment(a.skConcealment),
      skDisguise(a.skDisguise) {
    }
    CDStealth & operator=(const CDStealth &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skConcealment = a.skConcealment;
      skDisguise = a.skDisguise;
      return *this;
    }
    virtual ~CDStealth() {}
    virtual CDStealth * cloneMe() { return new CDStealth(*this); }
private:
};


int conceal(TBeing *, TBeing *);


#endif
