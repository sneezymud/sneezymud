//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_theology.h,v $
// Revision 5.2  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
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


#ifndef __DISC_THEOLOGY_H
#define __DISC_THEOLOGY_H

// This is the THEOLOGY discipline.

#include "discipline.h"
#include "skills.h"

class CDTheology : public CDiscipline
{
public:
    CSkill skPenance;
    CSkill skAttune;

    CDTheology()
      : CDiscipline(),
      skPenance(), 
      skAttune() {
    }
    CDTheology(const CDTheology &a)
      : CDiscipline(a),
      skPenance(a.skPenance),
      skAttune(a.skAttune) {
    }
    CDTheology & operator=(const CDTheology &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skPenance = a.skPenance;
      skAttune = a.skAttune;
      return *this;
    }
    virtual ~CDTheology() {}
    virtual CDTheology * cloneMe() { return new CDTheology(*this); }

    bool isBasic(){ return true; }

private:
};
 void attune(TBeing *, TThing *);
#endif

