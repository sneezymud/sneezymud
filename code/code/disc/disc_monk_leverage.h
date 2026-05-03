//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "discipline.h"
#include "skills.h"

class CDLeverage : public CDiscipline {
  public:
    CSkill skShoulderThrow;
    CSkill skHurl;
    CSkill skChainAttack;
    CSkill skDefenestrate;
    CSkill skBoneBreak;

    virtual CDLeverage* cloneMe() { return new CDLeverage(*this); }

  private:
};

int shoulderThrow(TBeing*, TBeing*);
int hurl(TBeing*, TBeing*, char*);
int defenestrate(TBeing*, TBeing*, sstring);
int bonebreak(TBeing*, TBeing*);
