//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_survival.h,v $
// Revision 5.3  2004/08/25 00:10:21  peel
// added advanced adventuring discipline
//
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
    CSkill skPoisonArrows;
  
    CDSurvival() :
      CDiscipline(),
      skPoisonArrows(){
    }
    CDSurvival(const CDSurvival &a) :
      CDiscipline(a),
      skPoisonArrows(a.skPoisonArrows){
    }
    CDSurvival & operator=(const CDSurvival &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skPoisonArrows = a.skPoisonArrows;
      return *this;
    }
    virtual ~CDSurvival() {}
    virtual CDSurvival * cloneMe() { return new CDSurvival(*this); }
private:
};

#endif

