//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman_healing.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SHAMAN_HEALING_H
#define __DISC_SHAMAN_HEALING_H

// This is the SHAMAN CURES discipline.

class CDShamanHealing : public CDiscipline
{
public:
    CDShamanHealing()
      : CDiscipline() {
    }
    CDShamanHealing(const CDShamanHealing &a)
      : CDiscipline(a) {
    }
    CDShamanHealing & operator=(const CDShamanHealing &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      return *this;
    }
    virtual ~CDShamanHealing() {}
    virtual CDShamanHealing * cloneMe() { return new CDShamanHealing(*this); }

private:
};


#endif
