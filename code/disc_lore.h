//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_lore.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_LORE_H
#define __DISC_LORE_H

// This is the mage/ranger/shaman lore discipline.

class CDLore : public CDiscipline
{
public:
    CSkill skMeditate;

    CDLore()
      : CDiscipline(),
      skMeditate() {
    }
    CDLore(const CDLore &a)
      : CDiscipline(a),
      skMeditate(a.skMeditate) {
    }
    CDLore & operator=(const CDLore &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skMeditate = a.skMeditate;
      return *this;
    }
    virtual ~CDLore() {}
    virtual CDLore * cloneMe() { return new CDLore(*this); }

private:
};

#endif

