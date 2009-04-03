//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_cures.h,v $
// Revision 5.3  2002/11/29 20:03:36  peel
// fixed a few incorrect prototypes
//
// Revision 5.2  2002/11/28 22:43:17  peel
// fixed a bad a prototype
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


#ifndef __DISC_CURES_H
#define __DISC_CURES_H

// This is the CLERIC CURES discipline.

class CDCures : public CDiscipline
{
public:
    CSkill skHealFull;       // MOD  40th level individual
    CSkill skHealCritSpray;  // NEW  25th level area affect
    CSkill skHealSpray;      // MOD  35th level area affect
    CSkill skHealFullSpray;  // NEW  45th level area affect
    CSkill skRestoreLimb;

    CDCures()
      : CDiscipline(),
      skHealFull(),
      skHealCritSpray(),
      skHealSpray(),
      skHealFullSpray(),
      skRestoreLimb() {
    }
    CDCures(const CDCures &a)
      : CDiscipline(a),
      skHealFull(a.skHealFull),
      skHealCritSpray(a.skHealCritSpray),
      skHealSpray(a.skHealSpray),
      skHealFullSpray(a.skHealFullSpray),
      skRestoreLimb(a.skRestoreLimb) {
    }
    CDCures & operator=(const CDCures &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHealFull = a.skHealFull;
      skHealCritSpray = a.skHealCritSpray;
      skHealSpray = a.skHealSpray;
      skHealFullSpray = a.skHealFullSpray;
      skRestoreLimb = a.skRestoreLimb;
      return *this;
    }
    virtual ~CDCures() {}
    virtual CDCures * cloneMe() { return new CDCures(*this); }

private:
};

    void healLight(TBeing *, TBeing *);      
    int castHealLight(TBeing *, TBeing *);      
    void healLight(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int healLight(TBeing *, TBeing *, int, byte, spellNumT, int);

    void healSerious(TBeing *, TBeing *);    
    int castHealSerious(TBeing *, TBeing *);      
    void healSerious(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int healSerious(TBeing *, TBeing *, int, byte, spellNumT);

    void healCritical(TBeing *, TBeing *);   
    int castHealCritical(TBeing *, TBeing *);      
    void healCritical(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int healCritical(TBeing *, TBeing *, int, byte, spellNumT, int);

    void heal(TBeing *, TBeing *);           
    int castHeal(TBeing *, TBeing *);      
    void heal(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int heal(TBeing *, TBeing *, int, byte, spellNumT, int);

    void healFull(TBeing *, TBeing *);       
    int castHealFull(TBeing *, TBeing *);      
    void healFull(TBeing *, TBeing *, TMagicItem *);
    int healFull(TBeing *, TBeing *, int, byte, int);

    void healCritSpray(TBeing *);
    int castHealCritSpray(TBeing *, TBeing *);      
    void healCritSpray(TBeing *, TMagicItem *);
    int healCritSpray(TBeing *, int, byte);

    void healSpray(TBeing *);     
    int castHealSpray(TBeing *, TBeing *);      
    void healSpray(TBeing *, TMagicItem *);
    int healSpray(TBeing *, int, byte);

    void healFullSpray(TBeing *);  
    int castHealFullSpray(TBeing *, TBeing *);      
    void healFullSpray(TBeing *, TMagicItem *);
    int healFullSpray(TBeing *, int, byte);

    void knitBone(TBeing *, TBeing *);
    void knitBone(TBeing *, TBeing *, TMagicItem *);
    int knitBone(TBeing *, TBeing *, int, byte);

    void clot(TBeing *, TBeing *);
    void clot(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int clot(TBeing *, TBeing *, int, byte, spellNumT);

    void restoreLimb(TBeing *, TBeing *);
    void restoreLimb(TBeing *, TBeing *, TMagicItem *);
    int restoreLimb(TBeing *, TBeing *, int, byte);

    void sterilize(TBeing *, TBeing *);
    void sterilize(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int sterilize(TBeing *, TBeing *, int, byte, spellNumT);

    void salve(TBeing *, TBeing *);
    void salve(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int salve(TBeing *, TBeing *, int, byte, spellNumT);

    void expel(TBeing *, TBeing *);
    void expel(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int expel(TBeing *, TBeing *, int, byte, spellNumT);

// this is the amount of healing a removing bruise is equivalent to
#define BRUISE_HEAL_AMOUNT 7

#endif
