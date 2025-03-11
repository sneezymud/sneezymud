#pragma once

#include "discipline.h"
#include "skills.h"

class CDNature : public CDiscipline {
  public:
    CSkill skTreeWalk;

    virtual CDNature* cloneMe() { return new CDNature(*this); }

  private:
};

int treeWalk(TBeing*, const char*, int, short);
int treeWalk(TBeing*, const char*);

int barkskin(TBeing*, TBeing*);
int castBarkskin(TBeing*, TBeing*);
int barkskin(TBeing*, TBeing*, TMagicItem*);
int barkskin(TBeing*, TBeing*, int, short);
