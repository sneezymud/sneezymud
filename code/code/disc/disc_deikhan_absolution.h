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
    CSkill skAuraAbsolution;

    CDDeikhanAbsolution()
      : CDiscipline(),
      skSalveDeikhan(),
      skLayHands(),
      skHeroesFeastDeikhan(),
      skRefreshDeikhan(),
      skAuraAbsolution() {
    }
    CDDeikhanAbsolution(const CDDeikhanAbsolution &a)
      : CDiscipline(a),
      skSalveDeikhan(a.skSalveDeikhan),
      skLayHands(a.skLayHands),
      skHeroesFeastDeikhan(a.skHeroesFeastDeikhan),
      skRefreshDeikhan(a.skRefreshDeikhan),
      skAuraAbsolution(a.skAuraAbsolution) {
    }
    CDDeikhanAbsolution & operator=(const CDDeikhanAbsolution &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSalveDeikhan = a.skSalveDeikhan;
      skLayHands = a.skLayHands;
      skHeroesFeastDeikhan = a.skHeroesFeastDeikhan;
      skRefreshDeikhan = a.skRefreshDeikhan;
      skAuraAbsolution = a.skAuraAbsolution;
      return *this;
    }
    virtual ~CDDeikhanAbsolution() {}
    virtual CDDeikhanAbsolution * cloneMe() { return new CDDeikhanAbsolution(*this); }

private:
};