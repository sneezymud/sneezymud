//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_draining.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_DRAINING_H
#define __DISC_DRAINING_H

// This is the SHAMAN DRAINING discipline.

class CDDraining : public CDiscipline
{
public:
    CSkill skVampiricTouch;
    CSkill skLifeLeech;

    CDDraining()
      : CDiscipline(),
      skVampiricTouch(),
      skLifeLeech() {
    }
    CDDraining(const CDDraining &a)
      : CDiscipline(a),
      skVampiricTouch(a.skVampiricTouch),
      skLifeLeech(a.skLifeLeech) {
    }
    CDDraining & operator=(const CDDraining &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      return *this;
    }
    virtual ~CDDraining() {}
    virtual CDDraining * cloneMe() { return new CDDraining(*this); }
private:
};

    void vampiricTouch(TBeing *, TBeing *);
    void lifeLeech(TBeing *);

#endif
