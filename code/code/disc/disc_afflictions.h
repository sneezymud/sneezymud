//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_afflictions.h,v $
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

#pragma once

// This is the AFFLICT discipline.

#include "discipline.h"
#include "skills.h"

class CDAfflict : public CDiscipline {
  public:
    CSkill skHarmFull;       // MOD  NC 40th level individual
    CSkill skHarmCritSpray;  // NEW  NC 25th level area affect
    CSkill skHarmSpray;      // MOD  NC 35th level area affect
    CSkill skHarmFullSpray;  // NEW  NC 45th level area affect
    CSkill skBleed;
    CSkill skParalyze;
    CSkill skBoneBreaker;
    CSkill skWitherLimb;

    CDAfflict() :
      CDiscipline(),
      skHarmFull(),
      skHarmCritSpray(),
      skHarmSpray(),
      skHarmFullSpray(),
      skBleed(),
      skParalyze(),
      skBoneBreaker(),
      skWitherLimb() {}
    CDAfflict(const CDAfflict& a) :
      CDiscipline(a),
      skHarmFull(a.skHarmFull),
      skHarmCritSpray(a.skHarmCritSpray),
      skHarmSpray(a.skHarmSpray),
      skHarmFullSpray(a.skHarmFullSpray),
      skBleed(a.skBleed),
      skParalyze(a.skParalyze),
      skBoneBreaker(a.skBoneBreaker),
      skWitherLimb(a.skWitherLimb) {}
    CDAfflict& operator=(const CDAfflict& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skHarmFull = a.skHarmFull;
      skHarmCritSpray = a.skHarmCritSpray;
      skHarmSpray = a.skHarmSpray;
      skHarmFullSpray = a.skHarmFullSpray;
      skBleed = a.skBleed;
      skParalyze = a.skParalyze;
      skBoneBreaker = a.skBoneBreaker;
      skWitherLimb = a.skWitherLimb;
      return *this;
    }
    virtual ~CDAfflict() {}
    virtual CDAfflict* cloneMe() { return new CDAfflict(*this); }

  private:
};

int harm(TBeing*, TBeing*);
int harm(TBeing*, TBeing*, TMagicItem*, spellNumT);
int harm(TBeing*, TBeing*, int, short, spellNumT, int);

void poison(TBeing*, TObj*);
int poison(TBeing*, TObj*, int, short, spellNumT);
void poison(TBeing*, TBeing*);
int poison(TBeing*, TBeing*, TMagicItem*, spellNumT);
int poison(TBeing*, TObj*, TMagicItem*, spellNumT);
int poison(TBeing*, TBeing*, int, short, spellNumT);

void blindness(TBeing*, TBeing*);
void blindness(TBeing*, TBeing*, TMagicItem*);
int blindness(TBeing*, TBeing*, int, short);

int harmLight(TBeing*, TBeing*);
int harmLight(TBeing*, TBeing*, TMagicItem*, spellNumT);
int harmLight(TBeing*, TBeing*, int, short, spellNumT, int);

int harmCritical(TBeing*, TBeing*);
int harmCritical(TBeing*, TBeing*, TMagicItem*, spellNumT);
int harmCritical(TBeing*, TBeing*, int, short, spellNumT, int);

int harmSerious(TBeing*, TBeing*);
int harmSerious(TBeing*, TBeing*, TMagicItem*, spellNumT);
int harmSerious(TBeing*, TBeing*, int, short, spellNumT, int);

void paralyze(TBeing*, TBeing*);
void paralyze(TBeing*, TBeing*, TMagicItem*);
int paralyze(TBeing*, TBeing*, int, short);

int boneBreaker(TBeing*, TBeing*);
int boneBreaker(TBeing*, TBeing*, TMagicItem*);
int boneBreaker(TBeing*, TBeing*, int, short, int);

int bleed(TBeing*, TBeing*);
int bleed(TBeing*, TBeing*, TMagicItem*);
int bleed(TBeing*, TBeing*, int, short);

int witherLimb(TBeing*, TBeing*);
int witherLimb(TBeing*, TBeing*, TMagicItem*);
int witherLimb(TBeing*, TBeing*, int, short, int);

int paralyzeLimb(TBeing*, TBeing*);
int paralyzeLimb(TBeing*, TBeing*, TMagicItem*);
int paralyzeLimb(TBeing*, TBeing*, int, short, int);

int numb(TBeing*, TBeing*);
int numb(TBeing*, TBeing*, TMagicItem*, spellNumT);
int numb(TBeing*, TBeing*, int, short, spellNumT, int);

void infect(TBeing*, TBeing*);
void infect(TBeing*, TBeing*, TMagicItem*, spellNumT);
int infect(TBeing*, TBeing*, int, short, spellNumT);

void disease(TBeing*, TBeing*);
void disease(TBeing*, TBeing*, TMagicItem*);
int disease(TBeing*, TBeing*, int, short);