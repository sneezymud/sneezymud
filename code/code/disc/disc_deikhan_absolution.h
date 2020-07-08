//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_cures.h,v $
#pragma once


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

};
