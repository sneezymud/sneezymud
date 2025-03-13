#pragma once

#include "discipline.h"
#include "skills.h"

class CDMeditationMonk : public CDiscipline {
  public:
    CSkill skWohlin;  // Advanced Yoginsa (meditation)
    CSkill skVoplat;  // Advanced Kubo (makes kubo damage magical)
    CSkill skBlindfighting;

    virtual CDMeditationMonk* cloneMe() { return new CDMeditationMonk(*this); }

  private:
};

int task_yoginsa(TBeing*, cmdTypeT, const char*, int, TRoom*, TObj*);
int chiMe(TBeing*);
int chi(TBeing*, TBeing*);
int chi(TBeing*, TObj*);