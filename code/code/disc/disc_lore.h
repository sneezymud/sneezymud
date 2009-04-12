//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_lore.h,v $
// Revision 5.3  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
//
// Revision 5.2  2002/04/25 22:14:16  peel
// added mana skill
//
// mana for mages based on skill
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


#ifndef __DISC_LORE_H
#define __DISC_LORE_H

// This is the mage/ranger/shaman lore discipline.

#include "discipline.h"
#include "skills.h"

class CDLore : public CDiscipline
{
public:
    CSkill skMeditate;
    CSkill skMana;

    CDLore()
      : CDiscipline(),
      skMeditate(),
      skMana() {
    }
    CDLore(const CDLore &a)
      : CDiscipline(a),
      skMeditate(a.skMeditate),
      skMana(a.skMana) {
    }
    CDLore & operator=(const CDLore &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skMeditate = a.skMeditate;
      skMana = a.skMana;
      return *this;
    }
    virtual ~CDLore() {}
    virtual CDLore * cloneMe() { return new CDLore(*this); }

    bool isBasic(){ return true; }

private:
};

#endif

