//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_draining.h,v $
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


#ifndef __DISC_DRAINING_H
#define __DISC_DRAINING_H

// This is the SHAMAN DRAINING discipline.

class CDDraining : public CDiscipline
{
public:
    CSkill skLifeLeech;

    CDDraining()
      : CDiscipline(),
      skLifeLeech() {
    }
    CDDraining(const CDDraining &a)
      : CDiscipline(a),
      skLifeLeech(a.skLifeLeech) {
    }
    CDDraining & operator=(const CDDraining &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skLifeLeech = a.skLifeLeech;
      return *this;
    }
    virtual ~CDDraining() {}
    virtual CDDraining * cloneMe() { return new CDDraining(*this); }
private:
};

    void lifeLeech(TBeing *);

#endif
