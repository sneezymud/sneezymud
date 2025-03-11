
#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanSpider : public CDiscipline {
  public:
    CSkill skSticksToSnakes;
    CSkill skControlUndead;
    CSkill skHypnosis;
    CSkill skRaze;
    CSkill skRootControl;
    CSkill skLivingVines;
    CSkill skTransfix;
    CSkill skClarity;

    virtual CDShamanSpider* cloneMe() { return new CDShamanSpider(*this); }

  private:
};

int transfix(TBeing*, TBeing*);
int transfix(TBeing*, TBeing*, int, short);

void livingVines(TBeing*, TBeing*);
void livingVines(TBeing*, TBeing*, TMagicItem*);
int livingVines(TBeing*, TBeing*, int, short);

int rootControl(TBeing*, TBeing*, int, int, short);
int rootControl(TBeing*, TBeing*, TMagicItem*);
int rootControl(TBeing*, TBeing*);

int controlUndead(TBeing*, TBeing*);
int castControlUndead(TBeing*, TBeing*);
void controlUndead(TBeing*, TBeing*, TMagicItem*);
int controlUndead(TBeing*, TBeing*, int, short);

int sticksToSnakes(TBeing*, TBeing*);
void sticksToSnakes(TBeing*, TBeing*, TMagicItem*);
int castSticksToSnakes(TBeing*, TBeing*);
int sticksToSnakes(TBeing*, TBeing*, int, short);

int clarity(TBeing*, TBeing*);
int castClarity(TBeing*, TBeing*);
void clarity(TBeing*, TBeing*, TMagicItem*);
int clarity(TBeing*, TBeing*, int, short);

int hypnosis(TBeing*, TBeing*);
int castHypnosis(TBeing*, TBeing*);
void hypnosis(TBeing*, TBeing*, TMagicItem*);
int hypnosis(TBeing*, TBeing*, int, short);

int raze(TBeing*, TBeing*);
int castRaze(TBeing*, TBeing*);
int raze(TBeing*, TBeing*, int, short, int);
int raze(TBeing*, TBeing*, TMagicItem*);