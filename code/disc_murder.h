//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_murder.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_MURDER_H
#define __DISC_MURDER_H

// This is the MURDER discipline.

class CDMurder : public CDiscipline
{
public:
    CSkill skGarrotte;

    CDMurder()
      : CDiscipline(),
      skGarrotte() {
    }
    CDMurder(const CDMurder &a)
      : CDiscipline(a),
      skGarrotte(a.skGarrotte) {
    }
    CDMurder & operator=(const CDMurder &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skGarrotte = a.skGarrotte;
      return *this;
    }
    virtual ~CDMurder() {}
    virtual CDMurder * cloneMe() { return new CDMurder(*this); }
private:
};

    int backstab(TBeing *, TBeing *);
    int poisonWeapon(TBeing *, TThing *);
    int garrotte(TBeing *, TBeing *);
    int cudgel(TBeing *, TBeing *);

#endif

