//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_survival.h,v $
// Revision 5.2  2003/01/16 23:35:07  peel
// Added poison arrow skill for rangers
// Added template for plant skill for thieves
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


#ifndef __DISC_SURVIVAL_H
#define __DISC_SURVIVAL_H

// This is the survival discipline.

class CDSurvival : public CDiscipline
{
public:
    CSkill skDivination;
    CSkill skEncamp;
    CSkill skPoisonArrows;
  
    CDSurvival() :
      CDiscipline(),
      skDivination(),
      skEncamp(),
      skPoisonArrows(){
    }
    CDSurvival(const CDSurvival &a) :
      CDiscipline(a),
      skDivination(a.skDivination),
      skEncamp(a.skEncamp),
      skPoisonArrows(a.skPoisonArrows){
    }
    CDSurvival & operator=(const CDSurvival &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDivination = a.skDivination;
      skEncamp = a.skEncamp;
      skPoisonArrows = a.skPoisonArrows;
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

