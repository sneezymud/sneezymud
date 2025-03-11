#pragma once

#include "discipline.h"
#include "skills.h"

class CDCombat : public CDiscipline {
  public:
    CSkill skBarehand;
    CSkill skArmorUse;
    CSkill skSlash;
    CSkill skBow;
    CSkill skPierce;
    CSkill skBlunt;
    CSkill skSharpen;
    CSkill skDull;

    virtual CDCombat* cloneMe() { return new CDCombat(*this); }

    bool isBasic() { return true; }

  private:
};

void sharpen(TBeing*, TThing*);
void dull(TBeing*, TThing*);
