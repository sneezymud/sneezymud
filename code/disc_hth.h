//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_hth.h,v $
// Revision 5.1.1.3  2000/12/27 08:27:35  dash
// added code for 'defensive trance' warrior skill
//
// Revision 5.1.1.2  2000/09/29 15:45:27  jesus
// added warrior parry skill
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


#ifndef __DISC_HTH_H
#define __DISC_HTH_H

// This is the HTH discipline.

class CDHTH : public CDiscipline
{
public:
    CSkill skShove;
    CSkill skRetreat;
    CSkill skParryWarrior;
    CSkill skTranceOfBlades;

    CDHTH()
      : CDiscipline(),
      skShove(),
      skRetreat(),
      skParryWarrior(),
      skTranceOfBlades() {
    }      
    CDHTH(const CDHTH &a)
      : CDiscipline(a),
      skShove(a.skShove),
      skRetreat(a.skRetreat),
      skParryWarrior(a.skParryWarrior),
      skTranceOfBlades(a.skTranceOfBlades) {
    }
    CDHTH & operator=(const CDHTH &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skShove = a.skShove;
      skRetreat = a.skRetreat;
      skParryWarrior = a.skParryWarrior;
      skTranceOfBlades = a.skTranceOfBlades;
      return *this;
    } 
    //    CDHTH();
    //    CDHTH(const CDHTH &a);
    //    CDHTH & operator=(const CDHTH &a);
    virtual ~CDHTH() {}
    virtual CDHTH * cloneMe() { return new CDHTH(*this); }
private:
};

    int shove(TBeing *, TBeing *, char *, spellNumT);

#endif



