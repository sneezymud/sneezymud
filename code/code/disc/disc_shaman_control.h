#pragma once

#include "discipline.h"
#include "skills.h"

class CDShamanControl : public CDiscipline {
  public:
    CSkill skEnthrallDemon;
    CSkill skCreateWoodGolem;
    CSkill skCreateRockGolem;
    CSkill skCreateIronGolem;
    CSkill skCreateDiamondGolem;
    CSkill skResurrection;

    virtual CDShamanControl* cloneMe() { return new CDShamanControl(*this); }

  private:
};
int enthrallDemon(TBeing* caster, int level, short bKnown);
int enthrallDemon(TBeing* caster);
int castEnthrallDemon(TBeing* caster);

int createWoodGolem(TBeing* caster, int level, short bKnown);
int createWoodGolem(TBeing* caster);
int castCreateWoodGolem(TBeing* caster);

int createRockGolem(TBeing* caster, int level, short bKnown);
int createRockGolem(TBeing* caster);
int castCreateRockGolem(TBeing* caster);

int createIronGolem(TBeing* caster, int level, short bKnown);
int createIronGolem(TBeing* caster);
int castCreateIronGolem(TBeing* caster);

int createDiamondGolem(TBeing* caster, int level, short bKnown);
int createDiamondGolem(TBeing* caster);
int castCreateDiamondGolem(TBeing* caster);

int resurrection(TBeing*, TObj*, int, short);
void resurrection(TBeing*, TObj*, TMagicItem*);
int resurrection(TBeing*, TObj*);
int castResurrection(TBeing*, TObj*);

int voodoo(TBeing*, TObj*, int, short);
void voodoo(TBeing*, TObj*, TMagicItem*);
int voodoo(TBeing*, TObj*);
int castVoodoo(TBeing*, TObj*);

int dancingBones(TBeing*, TObj*, int, short);
void dancingBones(TBeing*, TObj*, TMagicItem*);
int dancingBones(TBeing*, TObj*);
int castDancingBones(TBeing*, TObj*);

int enthrallSpectre(TBeing* caster, int level, short bKnown);
int enthrallSpectre(TBeing* caster);
int castEnthrallSpectre(TBeing* caster);

int enthrallGhast(TBeing* caster, int level, short bKnown);
int enthrallGhast(TBeing* caster);
int castEnthrallGhast(TBeing* caster);

int enthrallGhoul(TBeing* caster, int level, short bKnown);
int enthrallGhoul(TBeing* caster);
int castEnthrallGhoul(TBeing* caster);