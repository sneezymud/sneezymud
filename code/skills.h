//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: skills.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SKILLS_H
#define __SKILLS_H

const int SKILL_MIN = -99;

class CSkill {
  public:
    int value;
    time_t lastUsed;

    CSkill();
    CSkill(const CSkill &a);
    CSkill & operator=(const CSkill &a);
    ~CSkill();

    int getLearnedness() {
      return (GET_BITS(value, 7, 8));
    }
    void setLearnedness(int n) {
      SET_BITS(value, 7, 8, n);
    }
    int getNatLearnedness() {
      return (GET_BITS(value, 15, 8));
    }
    void setNatLearnedness(int n) {
      SET_BITS(value, 15, 8, n);
    }

};

// learnFromDoing prevents const on TBeing in bSuccess
extern bool bSuccess(TBeing *, int, spellNumT);
extern bool bSuccess(TBeing *, int, double, spellNumT);

extern bool bPassMageChecks(TBeing * , spellNumT, TThing *);
extern bool bPassClericChecks(TBeing *, spellNumT);
extern byte defaultProficiency(byte, byte, byte);
extern bool canDoVerbal(TBeing *);
extern bool checkRoom(const TBeing *);
extern critSuccT critSuccess(TBeing *, spellNumT);
extern critFailT critFail(TBeing *, spellNumT);
extern int checkMana(TBeing *, int);
#if FACTIONS_IN_USE
extern bool checkPerc(const TBeing *, double);
#endif
extern void checkFactionHurt(TBeing *, TBeing *);
extern void checkFactionHelp(TBeing *, TBeing *);

#endif
