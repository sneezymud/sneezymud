//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_theology.h,v $
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

private:
};
 void attune(TBeing *, TThing *);
#endif

