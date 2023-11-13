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

#pragma once

// this class serves two related purposes
// first, it provides an operator that can be sent to the sort() algorithm
// second, it stores the data that will be needed by that sorting routine.
class skillSorter {
  public:
    const TBeing* ch{nullptr};
    spellNumT theSkill{TYPE_UNDEFINED};

    skillSorter() = default;
    skillSorter(const TBeing* c2, const spellNumT ts) : ch(c2), theSkill(ts){};

    bool operator()(const skillSorter&, const skillSorter&) const;
};
