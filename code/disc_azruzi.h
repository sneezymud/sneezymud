//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_azruzi.h,v $
// Revision 5.2  2001/05/06 14:39:15  jesus
// rewrote vampiric touch spell fo shaman
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


#ifndef __DISC_AZRUZI_H
#define __DISC_AZRUZI_H

// This is the AZRUZI discipline.

class CDAzruzi : public CDiscipline
{
public:
    CSkill skLifeLeech;

    CDAzruzi()
      : CDiscipline(),
        skLifeLeech() {
    }
    CDAzruzi(const CDAzruzi &a)
      : CDiscipline(a),
        skLifeLeech(a.skLifeLeech) {
    }
    CDAzruzi & operator=(const CDAzruzi &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skLifeLeech = a.skLifeLeech;
      return *this;
    }
    virtual ~CDAzruzi() {}
    virtual CDAzruzi * cloneMe() { return new CDAzruzi(*this); }

private:
};

    void lifeLeech(TBeing *);

#endif
