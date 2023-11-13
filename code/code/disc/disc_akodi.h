//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_akodi.h,v $
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

#pragma once

// This is the AKODI discipline.

#include "discipline.h"
#include "skills.h"

class CDAkodi : public CDiscipline {
  public:
    CSkill skMartialSpec;

    CDAkodi() : CDiscipline(), skMartialSpec() {}
    CDAkodi(const CDAkodi& a) :
      CDiscipline(a),
      skMartialSpec(a.skMartialSpec) {}
    CDAkodi& operator=(const CDAkodi& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skMartialSpec = a.skMartialSpec;
      return *this;
    }
    virtual ~CDAkodi() {}
    virtual CDAkodi* cloneMe() { return new CDAkodi(*this); }

  private:
};
