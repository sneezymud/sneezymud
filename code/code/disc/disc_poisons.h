//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_poisons.h,v $
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


#ifndef __DISC_POISONS_H
#define __DISC_POISONS_H

// This is the THIEF POISONS discipline.

class CDPoisons : public CDiscipline
{
public:
    CSkill skPoisonWeapons;

    CDPoisons()
      : CDiscipline(),
      skPoisonWeapons() {
    }
    CDPoisons(const CDPoisons &a)
      : CDiscipline(a),
      skPoisonWeapons(a.skPoisonWeapons) {
    }
    CDPoisons & operator=(const CDPoisons &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skPoisonWeapons = a.skPoisonWeapons;
      return *this;
    }
    virtual ~CDPoisons() {}
    virtual CDPoisons * cloneMe() { return new CDPoisons(*this); }
private:
};


#endif
