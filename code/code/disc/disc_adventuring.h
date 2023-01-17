//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_adventuring.h,v $
// Revision 5.5  2004/08/25 00:10:21  peel
// added advanced adventuring discipline
//
// Revision 5.4  2002/11/12 01:33:55  peel
// added isAutomatic() for automatic discs like adventuring, faith etc
//
// Revision 5.3  2002/07/04 18:34:11  dash
// added new repair skills
//
// Revision 5.2  2001/07/15 19:49:32  peel
// added defense disc with advanced defense skill
// adjusted the theoretical hp formula in score
//
// Revision 5.1.1.3  2001/04/18 01:57:03  peel
// added fishing skill
//
// Revision 5.1.1.2  2001/04/15 03:53:21  peel
// added SKILL_ALCOHOLISM
//
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

// This contains the general knowledge areas that _every_ pc knows.  All of
// these are "learn by mistake".  None can be practiced.
// On the same token, I don know know if any of these set any character flags.
// They don't seem to directly have any functions.

#include "discipline.h"
#include "skills.h"

class CDAdventuring : public CDiscipline {
  public:
    CSkill skFishing;
    CSkill skLogging;
    CSkill skAlcoholism;
    CSkill skRide;
    CSkill skSwim;
    CSkill skClimb;
    CSkill skSign;
    CSkill skKnowPeople;
    CSkill skKnowGiant;
    CSkill skKnowVeggie;
    CSkill skKnowAnimal;
    CSkill skKnowReptile;
    CSkill skKnowUndead;
    CSkill skKnowOther;
    CSkill skKnowDemon;
    CSkill skReadMagic;
    CSkill skBandage;
    CSkill skFastHeal;
    CSkill skEvaluate;
    CSkill skTactics;
    CSkill skDissect;
    CSkill skOffense;
    CSkill skDefense;
    CSkill skGenWeapons;
    CSkill skWhittle;
    CSkill skMend;
    CSkill skButcher;
    CSkill skGutterCant;
    CSkill skGnollJargon;
    CSkill skTroglodytePidgin;

    CDAdventuring() :
      CDiscipline(),
      skFishing(),
      skLogging(),
      skAlcoholism(),
      skRide(),
      skSwim(),
      skClimb(),
      skSign(),
      skKnowPeople(),
      skKnowGiant(),
      skKnowVeggie(),
      skKnowAnimal(),
      skKnowReptile(),
      skKnowUndead(),
      skKnowOther(),
      skKnowDemon(),
      skReadMagic(),
      skBandage(),
      skFastHeal(),
      skEvaluate(),
      skTactics(),
      skDissect(),
      skOffense(),
      skDefense(),
      skGenWeapons(),
      skWhittle(),
      skMend(),
      skButcher(),
      skGutterCant(),
      skGnollJargon(),
      skTroglodytePidgin() {}
    CDAdventuring(const CDAdventuring& a) :
      CDiscipline(a),
      skFishing(a.skFishing),
      skLogging(a.skLogging),
      skAlcoholism(a.skAlcoholism),
      skRide(a.skRide),
      skSwim(a.skSwim),
      skClimb(a.skClimb),
      skSign(a.skSign),
      skKnowPeople(a.skKnowPeople),
      skKnowGiant(a.skKnowGiant),
      skKnowVeggie(a.skKnowVeggie),
      skKnowAnimal(a.skKnowAnimal),
      skKnowReptile(a.skKnowReptile),
      skKnowUndead(a.skKnowUndead),
      skKnowOther(a.skKnowOther),
      skKnowDemon(a.skKnowDemon),
      skReadMagic(a.skReadMagic),
      skBandage(a.skBandage),
      skFastHeal(a.skFastHeal),
      skEvaluate(a.skEvaluate),
      skTactics(a.skTactics),
      skDissect(a.skDissect),
      skOffense(a.skOffense),
      skDefense(a.skDefense),
      skGenWeapons(a.skGenWeapons),
      skWhittle(a.skWhittle),
      skMend(a.skMend),
      skButcher(a.skButcher),
      skGutterCant(a.skGutterCant),
      skGnollJargon(a.skGnollJargon),
      skTroglodytePidgin(a.skTroglodytePidgin) {}
    CDAdventuring& operator=(const CDAdventuring& a) {
      if (this == &a)
        return *this;
      CDiscipline::operator=(a);
      skFishing = a.skFishing;
      skLogging = a.skLogging;
      skAlcoholism = a.skAlcoholism;
      skRide = a.skRide;
      skSwim = a.skSwim;
      skClimb = a.skClimb;
      skSign = a.skSign;
      skKnowPeople = a.skKnowPeople;
      skKnowGiant = a.skKnowGiant;
      skKnowVeggie = a.skKnowVeggie;
      skKnowAnimal = a.skKnowAnimal;
      skKnowReptile = a.skKnowReptile;
      skKnowUndead = a.skKnowUndead;
      skKnowOther = a.skKnowOther;
      skKnowDemon = a.skKnowDemon;
      skReadMagic = a.skReadMagic;
      skBandage = a.skBandage;
      skFastHeal = a.skFastHeal;
      skEvaluate = a.skEvaluate;
      skTactics = a.skTactics;
      skDissect = a.skDissect;
      skOffense = a.skOffense;
      skDefense = a.skDefense;
      skGenWeapons = a.skGenWeapons;
      skWhittle = a.skWhittle;
      skMend = a.skMend;
      skButcher = a.skButcher;
      skGutterCant = a.skGutterCant;
      skGnollJargon = a.skGnollJargon;
      skTroglodytePidgin = a.skTroglodytePidgin;

      return *this;
    }
    virtual ~CDAdventuring(){};

    virtual CDAdventuring* cloneMe() { return new CDAdventuring(*this); }

    bool isAutomatic() { return true; }

  private:
};

int dissect(TBeing*, TObj*);
