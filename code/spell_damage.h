//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: spell_damage.h,v $
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


#ifndef __SPELL_DAMAGE_H
#define __SPELL_DAMAGE_H

const int MAX_DAM 		= (1<<0);
const int BASE_RANGE 		= (1<<1);
const int INV_RANGE 		= (1<<2);
const int DAM_MULTI 		= (1<<3);
const int CAP_LEVEL 		= (1<<4);
const int INCREMENT 		= (1<<5);
const int DAM_BEL_CAP 		= (1<<6);
const int DAM_ABV_CAP 		= (1<<7);
const int DAM_ABV_50 		= (1<<8);

/* DO I NEED TO BUILD THE ARRAY IN THE HEADER?
          spellDamArray[SPELL_<NAME>] = new spellDamInfo(SPELL_<CLASS>, DISC_<NAME>, DISC_<TYPE>, "<spellname>", TARGE T, BASE_RANGE, INV_RANGE, DAM_MULT, CAP_LEVEL, SLOPE_1, SLOPE_2);
*/

extern int getSpellDam(TBeing *, TBeing *, TThing *, TRoom *, int);

extern int modifyForSector(int, TBeing *, TThing *, TObj *, int, int, char, int);


class spellDamInfo {
  public:
    int baseRange;
    int invRange;
    int damMult;
    int capLevel;
    float slope_1;
    float slope_2;
    spellDamInfo(int baseRange, int invRange, int damMult, int capLevel, float slope_1, float slope_2);
    ~spellDamInfo();
};


#if 0
discipline, assDiscipline, spellName are all in discArray as constants
casterLevel you can get from the caster whenever you need it, dont have to
pass it or store it in another form same with most of this
most is wrong word, wisdom is on the caster, skilllearn is on the caster,
weather too
	 spellDam(int styp, int discipline, int assDiscipline, char *spellname, int casterLevel, int baseRange, int invRange, unsigned int maxDam, float rawDam, int rawDamMod, int finalDam, int damMult, int capLevel, float slope_1, float slope_2, int wis, float skillLearn, int terrain, int weather);
};
#endif
extern spellDamInfo *spellDamArray[MAX_SKILL+1];
#endif

