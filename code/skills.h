//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
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

extern bool bPassMageChecks(TBeing * , spellNumT, TThing *);
extern bool bPassShamanChecks(TBeing * , spellNumT, TThing *);
extern bool bPassClericChecks(TBeing *, spellNumT);
extern byte defaultProficiency(byte, byte, byte);
extern bool canDoVerbal(TBeing *);
extern bool checkRoom(const TBeing *);
extern critSuccT critSuccess(TBeing *, spellNumT);
extern critFailT critFail(TBeing *, spellNumT);
extern int checkMana(TBeing *, int);
extern int checkLifeforce(TBeing *, int);
#if FACTIONS_IN_USE
extern bool checkPerc(const TBeing *, double);
#endif
extern void checkFactionHurt(TBeing *, TBeing *);
extern void checkFactionHelp(TBeing *, TBeing *);

#endif
