//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman_healing.h,v $
// Revision 5.4  2003/09/05 17:06:41  peel
// added two new potions
// healing grasp and cleanse
//
// Revision 5.3  2002/11/21 02:48:56  jesus
// added enliven spell and updated the fireball weapon proc a little
//
// Revision 5.2  2002/02/20 23:02:19  jesus
// healing grasp spell for shaman
// in the new healing disc for shaman
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


#ifndef __DISC_SHAMAN_HEALING_H
#define __DISC_SHAMAN_HEALING_H

// This is the SHAMAN CURES discipline.

class CDShamanHealing : public CDiscipline
{
public:

    CSkill skHealingGrasp;
    CSkill skEnliven;

    CDShamanHealing()
      : CDiscipline(),
      skHealingGrasp(),
      skEnliven() {
    }
    CDShamanHealing(const CDShamanHealing &a)
      : CDiscipline(a),
      skHealingGrasp(a.skHealingGrasp),
      skEnliven(a.skEnliven) {
    }
    CDShamanHealing & operator=(const CDShamanHealing &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHealingGrasp = a.skHealingGrasp;
      skEnliven = a.skEnliven;
      return *this;
    }
    virtual ~CDShamanHealing() {}
    virtual CDShamanHealing * cloneMe() { return new CDShamanHealing(*this); }

private:
};
   void healingGrasp(TBeing *, TBeing *);
   int castHealingGrasp(TBeing *, TBeing *);
   void healingGrasp(TBeing *, TBeing *, TMagicItem *, spellNumT);
   int healingGrasp(TBeing *, TBeing *, int, byte, spellNumT, int=0);

   int enliven(TBeing *, TBeing *, int, byte);
   void enliven(TBeing *, TBeing *, TMagicItem *);
   int enliven(TBeing *, TBeing *);
   int castEnliven(TBeing *, TBeing *);
#endif
