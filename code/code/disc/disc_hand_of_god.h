//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_hand_of_god.h,v $
// Revision 5.1.1.2  2001/04/26 20:42:28  peel
// portal changes
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


#pragma once

// This is the HAND_OF_GOD discipline.

#include "discipline.h"
#include "skills.h"

class CDHand : public CDiscipline
{
public:
    CSkill skHeroesFeast;
    CSkill skAstralWalk;
    CSkill skPortal;

    CDHand()
      : CDiscipline(),
      skHeroesFeast(),
      skAstralWalk(),
      skPortal() {
    }
    CDHand(const CDHand &a)
      : CDiscipline(a),
      skHeroesFeast(a.skHeroesFeast),
      skAstralWalk(a.skAstralWalk),
      skPortal(a.skPortal) {
    }
    CDHand & operator=(const CDHand &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHeroesFeast = a.skHeroesFeast;
      skAstralWalk = a.skAstralWalk;
      skPortal = a.skPortal;
      return *this;
    }
    virtual ~CDHand() {}
    virtual CDHand * cloneMe() { return new CDHand(*this); }
private:
};

    int astralWalk(TBeing *, TBeing *);
    int astralWalk(TBeing *, TBeing *, int, short);

    void createFood(TBeing *);
    int createFood(TBeing *, int, short, spellNumT);

    void createWater(TBeing *, TObj *);
    int castCreateWater(TBeing *, TObj *);
    int createWater(TBeing *, TObj *, int, short, spellNumT);

    void wordOfRecall(TBeing *, TBeing *);
    void wordOfRecall(TBeing *, TBeing *, TMagicItem *);
    int wordOfRecall(TBeing *, TBeing *, int, short);

    int summon(TBeing *, TBeing *);
    int summon(TBeing *, TBeing *, int, short);

    void heroesFeast(TBeing *);
    int heroesFeast(TBeing *, int, short, spellNumT);

    void portal(TBeing *, const char *);
    int portal(TBeing *, const char *, int, short);


