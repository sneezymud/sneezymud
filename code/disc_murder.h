//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_murder.h,v $
// Revision 5.2  2002/01/21 03:04:11  jesus
// added a throat slitting skill for thieves into the murder disc
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


#ifndef __DISC_MURDER_H
#define __DISC_MURDER_H

// This is the MURDER discipline.

class CDMurder : public CDiscipline
{
public:
    CSkill skGarrotte;
    CSkill skThroatSlit;

    CDMurder()
      : CDiscipline(),
      skGarrotte(),
      skThroatSlit() {
    }
    CDMurder(const CDMurder &a)
      : CDiscipline(a),
      skGarrotte(a.skGarrotte),
      skThroatSlit(a.skThroatSlit) {
    }
    CDMurder & operator=(const CDMurder &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skGarrotte = a.skGarrotte;
      skThroatSlit = a.skThroatSlit;
      return *this;
    }
    virtual ~CDMurder() {}
    virtual CDMurder * cloneMe() { return new CDMurder(*this); }
private:
};

    int backstab(TBeing *, TBeing *);
    int throatSlit(TBeing *, TBeing *);
    int poisonWeapon(TBeing *, TThing *);
    int garrotte(TBeing *, TBeing *);
    int cudgel(TBeing *, TBeing *);

#endif

