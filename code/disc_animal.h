//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_animal.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_ANIMAL_H
#define __DISC_ANIMAL_H

// This is the ANIMAL discipline.

class CDAnimal : public CDiscipline
{
public:
    CSkill skBeastCharm;

    CDAnimal()
      : CDiscipline(),
        skBeastCharm() {
    }
    CDAnimal(const CDAnimal &a)
      : CDiscipline(a),
        skBeastCharm(a.skBeastCharm) {
    }
    CDAnimal & operator=(const CDAnimal &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skBeastCharm = a.skBeastCharm;
      return *this;
    }
    virtual ~CDAnimal() {}
    virtual CDAnimal * cloneMe() { return new CDAnimal(*this); }
private:
};

    int beastSoother(TBeing *, TBeing *);
    int beastSoother(TBeing *, TBeing *, int, byte);
 
    int transfix(TBeing *, TBeing *);
    int transfix(TBeing *, TBeing *, int, byte);
 
    int beastSummon(TBeing *, const char *);
    int beastSummon(TBeing *, const char *, int, byte);

    void shapeShift(TBeing *, const char *);
    int shapeShift(TBeing *, const char *, int, byte);

#endif
