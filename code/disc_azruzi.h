//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_azruzi.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_AZRUZI_H
#define __DISC_AZRUZI_H

// This is the AZRUZI discipline.

class CDAzruzi : public CDiscipline
{
public:
    CSkill skVampiricTouch;
    CSkill skLifeLeech;

    CDAzruzi()
      : CDiscipline(),
        skVampiricTouch(), skLifeLeech() {
    }
    CDAzruzi(const CDAzruzi &a)
      : CDiscipline(a),
        skVampiricTouch(a.skVampiricTouch), skLifeLeech(a.skLifeLeech) {
    }
    CDAzruzi & operator=(const CDAzruzi &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      return *this;
    }
    virtual ~CDAzruzi() {}
    virtual CDAzruzi * cloneMe() { return new CDAzruzi(*this); }

private:
};

    void vampiricTouch(TBeing *, TBeing *);
    void lifeLeech(TBeing *);

#endif
