//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_wrath.h,v $
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


#ifndef __DISC_WRATH_H
#define __DISC_WRATH_H

// This is the WRATH discipline.

class CDWrath : public CDiscipline
{
public:
    CSkill skPillarOfSalt;
    CSkill skEarthquake;
    CSkill skCallLightning;
    CSkill skSpontaneousCombust;
    CSkill skHailStorm;
    CSkill skHolocaust;

    CDWrath()
      : CDiscipline(),
      skPillarOfSalt(),
      skEarthquake(),
      skCallLightning(),
      skSpontaneousCombust(),
      skHailStorm(),
      skHolocaust() {
    }
    CDWrath(const CDWrath &a)
      : CDiscipline(a),
      skPillarOfSalt(a.skPillarOfSalt),
      skEarthquake(a.skEarthquake),
      skCallLightning(a.skCallLightning),
      skSpontaneousCombust(a.skSpontaneousCombust),
      skHailStorm(a.skHailStorm),
      skHolocaust(a.skHolocaust) {
    }
    CDWrath & operator=(const CDWrath &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skPillarOfSalt = a.skPillarOfSalt;
      skEarthquake = a.skEarthquake;
      skCallLightning = a.skCallLightning;
      skSpontaneousCombust = a.skSpontaneousCombust;
      skHailStorm = a.skHailStorm;
      skHolocaust = a.skHolocaust;
      return *this;
    }
    virtual ~CDWrath() {}
    virtual CDWrath * cloneMe() { return new CDWrath(*this); }
private:
};

    void curse(TBeing *, TObj *);
    void curse(TBeing *, TObj *, TMagicItem *, spellNumT);
    int curse(TBeing *, TObj *, int, byte, spellNumT);

    void curse(TBeing *, TBeing *);
    void curse(TBeing *, TBeing *, TMagicItem *, spellNumT);
    int curse(TBeing *, TBeing *, int, byte, spellNumT);

    int earthquake(TBeing *);
    int earthquake(TBeing *, TMagicItem * obj, spellNumT);
    int earthquake(TBeing *, int, byte, spellNumT, int);

    int callLightning(TBeing *, TBeing *);
    int callLightning(TBeing *, TBeing *, TMagicItem * obj, spellNumT);
    int callLightning(TBeing *, TBeing *, int, byte, spellNumT, int);

    int spontaneousCombust(TBeing *, TBeing *);
    int spontaneousCombust(TBeing *, TBeing *, TMagicItem *);
    int spontaneousCombust(TBeing *, TBeing *, int, byte, spellNumT);

    int flamestrike(TBeing *, TBeing *);
    int flamestrike(TBeing *, TBeing *, TMagicItem * obj);
    int flamestrike(TBeing *, TBeing *, int, byte, spellNumT);

    int rainBrimstone(TBeing *, TBeing *);
    int rainBrimstone(TBeing *, TBeing *, TMagicItem * obj, spellNumT);
    int rainBrimstone(TBeing *, TBeing *, int, byte, spellNumT, int);

    int pillarOfSalt(TBeing *, TBeing *);
    int pillarOfSalt(TBeing *, TBeing *, TMagicItem * obj);
    int pillarOfSalt(TBeing *, TBeing *, int, byte, spellNumT);

    int plagueOfLocusts(TBeing *, TBeing *);
    int plagueOfLocusts(TBeing *, TBeing *, TMagicItem * obj);
    int plagueOfLocusts(TBeing *, TBeing *, int, byte);

    int hailStorm(TBeing *);
    int hailStorm(TBeing *, int, byte);

#endif

