//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: skillsort.h,v $
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


#ifndef _SKILL_SORT_H
#define _SKILL_SORT_H

// this class serves two related purposes
// first, it provides an operator that can be sent to the sort() algorithm
// second, it stores the data that will be needed by that sorting routine.
class skillSorter {
  public:
    TBeing *ch;
    spellNumT theSkill;
    skillSorter(TBeing *c2, spellNumT ts) :
      ch(c2), theSkill(ts) {}

    skillSorter() : ch(NULL), theSkill(TYPE_UNDEFINED) {}

    bool operator() (const skillSorter &, const skillSorter &) const;
};

#endif
