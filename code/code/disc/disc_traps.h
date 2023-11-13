//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// This is the THIEF TRAPS discipline.

#include "discipline.h"
#include "skills.h"

class CDTraps : public CDiscipline {
  public:
    CSkill skSetTrapsCont;
    CSkill skSetTrapsDoor;
    CSkill skSetTrapsMine;
    CSkill skSetTrapsGren;
    CSkill skSetTrapsArrow;

    CDTraps() :
      CDiscipline(),
      skSetTrapsCont(),
      skSetTrapsDoor(),
      skSetTrapsMine(),
      skSetTrapsGren(),
      skSetTrapsArrow() {}
    CDTraps(const CDTraps& a) :
      CDiscipline(a),
      skSetTrapsCont(a.skSetTrapsCont),
      skSetTrapsDoor(a.skSetTrapsDoor),
      skSetTrapsMine(a.skSetTrapsMine),
      skSetTrapsGren(a.skSetTrapsGren),
      skSetTrapsArrow(a.skSetTrapsArrow) {}
    CDTraps& operator=(const CDTraps& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skSetTrapsCont = a.skSetTrapsCont;
      skSetTrapsDoor = a.skSetTrapsDoor;
      skSetTrapsMine = a.skSetTrapsMine;
      skSetTrapsGren = a.skSetTrapsGren;
      skSetTrapsArrow = a.skSetTrapsArrow;
      return *this;
    }
    virtual ~CDTraps() {}
    virtual CDTraps* cloneMe() { return new CDTraps(*this); }

  private:
};
