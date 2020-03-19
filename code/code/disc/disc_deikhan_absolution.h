//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_cures.h,v $
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


#ifndef __DISC_DEIKHAN_ABSOLUTION_H
#define __DISC_DEIKHAN_ABSOLUTION_H

// This is the DEIKHAN CURES discipline.

#include "discipline.h"
#include "skills.h"

class CDDeikhanAbsolution : public CDiscipline
{
public:
    CSkill skSalveDeikhan;
    CSkill skLayHands;
    CSkill skHeroesFeastDeikhan;
    CSkill skRefreshDeikhan;

    CDDeikhanAbsolution()
      : CDiscipline(),
      skSalveDeikhan(),
      skLayHands(),
      skHeroesFeastDeikhan(),
      skRefreshDeikhan() {
    }
    CDDeikhanAbsolution(const CDDeikhanAbsolution &a)
      : CDiscipline(a),
      skSalveDeikhan(a.skSalveDeikhan),
      skLayHands(a.skLayHands),
      skHeroesFeastDeikhan(a.skHeroesFeastDeikhan),
      skRefreshDeikhan(a.skRefreshDeikhan) {
    }
    CDDeikhanAbsolution & operator=(const CDDeikhanAbsolution &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSalveDeikhan = a.skSalveDeikhan;
      skLayHands = a.skLayHands;
      skHeroesFeastDeikhan = a.skHeroesFeastDeikhan;
      skRefreshDeikhan = a.skRefreshDeikhan;
      return *this;
    }
    virtual ~CDDeikhanAbsolution() {}
    virtual CDDeikhanAbsolution * cloneMe() { return new CDDeikhanAbsolution(*this); }

private:
};


#endif

