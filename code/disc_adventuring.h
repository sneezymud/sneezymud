//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_adventuring.h,v $
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


#ifndef __DISC_ADVENTURING_H
#define __DISC_ADVENTURING_H   1

// This contains the general knowledge areas that _every_ pc knows.  All of
// these are "learn by mistake".  None can be practiced.
// On the same token, I don know know if any of these set any character flags.
// They don't seem to directly have any functions.

class CDAdventuring : public CDiscipline
{
  public:
    CSkill skFishing;
    CSkill skAlcoholism;
    CSkill skRide;
    CSkill skSwim;
    CSkill skClimb;
    CSkill skSign;         // Do we want this?
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
//    CSkill skSlash;
//    CSkill skBow;
//    CSkill skPierce;
//    CSkill skBlunt;
//    CSkill skSharpen;
    CSkill skEvaluate;
    CSkill skTactics;
//    CSkill skDull;
    CSkill skDissect;
    CSkill skOffense;
    CSkill skDefense;
    CSkill skGenWeapons;
    CSkill skWhittle;

    CDAdventuring()
      : CDiscipline(),
        skFishing(), skAlcoholism(), skRide(), skSwim(), skClimb(),
        skSign(), skKnowPeople(), skKnowGiant(),
        skKnowVeggie(), skKnowAnimal(), skKnowReptile(),
        skKnowUndead(), skKnowOther(), skKnowDemon(),
        skReadMagic(), skBandage(), skFastHeal(),
//        skSlash(), skBow(), skPierce(),
//        skBlunt(), skSharpen(),
        skEvaluate(), skTactics(),
//        skDull(),
        skDissect(), skOffense(), skDefense(), 
        skGenWeapons(), skWhittle() {
    }
    CDAdventuring(const CDAdventuring &a)
      : CDiscipline(a),
        skFishing(a.skFishing), skAlcoholism(a.skAlcoholism), skRide(a.skRide), skSwim(a.skSwim), skClimb(a.skClimb),
        skSign(a.skSign), skKnowPeople(a.skKnowPeople), skKnowGiant(a.skKnowGiant),
        skKnowVeggie(a.skKnowVeggie), skKnowAnimal(a.skKnowAnimal), 
        skKnowReptile(a.skKnowReptile),
        skKnowUndead(a.skKnowUndead), skKnowOther(a.skKnowOther), 
        skKnowDemon(a.skKnowDemon),
        skReadMagic(a.skReadMagic), skBandage(a.skBandage), skFastHeal(a.skFastHeal),
//        skSlash(a.skSlash), skBow(a.skBow), skPierce(a.skPierce),
//        skBlunt(a.skBlunt), skSharpen(a.skSharpen),
        skEvaluate(a.skEvaluate), skTactics(a.skTactics),
//        skDull(a.skDull),
        skDissect(a.skDissect), skOffense(a.skOffense),
        skDefense(a.skDefense), skGenWeapons(a.skGenWeapons),
        skWhittle(a.skWhittle)  {
    }
    CDAdventuring & operator=(const CDAdventuring &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skFishing = a.skFishing;
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
//      skSlash = a.skSlash;
//      skBow = a.skBow;
//      skPierce = a.skPierce;
//      skBlunt = a.skBlunt;
//      skSharpen = a.skSharpen;
      skEvaluate = a.skEvaluate;
      skTactics = a.skTactics;
//      skDull = a.skDull;
      skDissect = a.skDissect;
      skOffense = a.skOffense;
      skDefense = a.skDefense;
      skGenWeapons = a.skGenWeapons;
      skWhittle = a.skWhittle;
      return *this;
    }
    virtual ~CDAdventuring() {};

    virtual CDAdventuring * cloneMe() {
      return new CDAdventuring(*this);
    }

private:
};

//    void sharpen(TBeing *, TThing *);
//    void dull(TBeing *, TThing *);
    int dissect(TBeing *, TObj *);

#endif

