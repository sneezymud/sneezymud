//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_looting.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_LOOTING_H
#define __DISC_LOOTING_H

// This is the LOOTING discipline.

class CDLooting : public CDiscipline
{
public:
  CSkill skCounterSteal;

    CDLooting() 
      : CDiscipline(),
      skCounterSteal(){
    }
    CDLooting(const CDLooting &a) 
      : CDiscipline(a),
      skCounterSteal(a.skCounterSteal){
    }
    CDLooting & operator=(const CDLooting &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skCounterSteal = a.skCounterSteal;
      return *this;
    }
    virtual ~CDLooting() {}
    virtual CDLooting * cloneMe() { return new CDLooting(*this); }

// no skills yet
private:
};

    int detectSecret(TBeing *);

    int disarmTrapObj(TBeing *, TObj *);
    int disarmTrapDoor(TBeing *, dirTypeT);

    int detectTrapObj(TBeing *, const TThing *);
    int detectTrapDoor(TBeing *, int);

#endif

