//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_survival.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SURVIVAL_H
#define __DISC_SURVIVAL_H

// This is the survival discipline.

class CDSurvival : public CDiscipline
{
public:
    CSkill skDivination;
    CSkill skEncamp;

    CDSurvival() :
      CDiscipline(),
      skDivination(),
      skEncamp() {
    }
    CDSurvival(const CDSurvival &a) :
      CDiscipline(a),
      skDivination(a.skDivination),
      skEncamp(a.skEncamp) {
    }
    CDSurvival & operator=(const CDSurvival &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDivination = a.skDivination;
      skEncamp = a.skEncamp;
      return *this;
    }
    virtual ~CDSurvival() {}
    virtual CDSurvival * cloneMe() { return new CDSurvival(*this); }
private:
};

    void forage(TBeing *);
    int forage(TBeing *, byte);

    void divine(TBeing *, TThing *);
    int divine(TBeing *, int, byte, TThing *);

    int encamp(TBeing *);

#endif

