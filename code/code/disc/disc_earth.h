//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_earth.h,v $
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


#ifndef __DISC_EARTH_H
#define __DISC_EARTH_H

// This is the EARTH discipline.

#include "discipline.h"
#include "skills.h"

class CDEarth : public CDiscipline
{
public:
    CSkill skMeteorSwarm;     //      40th level individual
    CSkill skLavaStream;      // NEW  36th level area affect
    CSkill skStoneSkin;
    CSkill skTrailSeek;

    CDEarth() :
      CDiscipline(),
      skMeteorSwarm(),
      skLavaStream(),
      skStoneSkin(),
      skTrailSeek()
    {
    }
    CDEarth(const CDEarth &a) :
      CDiscipline(a),
      skMeteorSwarm(a.skMeteorSwarm),
      skLavaStream(a.skLavaStream),
      skStoneSkin(a.skStoneSkin),
      skTrailSeek(a.skTrailSeek)
    {
    }
    CDEarth & operator=(const CDEarth &a)
    {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skMeteorSwarm = a.skMeteorSwarm;
      skLavaStream = a.skLavaStream;
      skStoneSkin = a.skStoneSkin;
      skTrailSeek = a.skTrailSeek;
      return *this;
    }
    virtual ~CDEarth() {}
    virtual CDEarth * cloneMe() { return new CDEarth(*this); }
private:
};

    int slingShot(TBeing *, TBeing *);
    int castSlingShot(TBeing *, TBeing *);
    int slingShot(TBeing *, TBeing *, int, short, int);

    int graniteFists(TBeing *, TBeing *);
    int castGraniteFists(TBeing *, TBeing *);
    int graniteFists(TBeing *, TBeing *, int, short, int);

    int pebbleSpray(TBeing *);
    int castPebbleSpray(TBeing *);
    int pebbleSpray(TBeing *, int, short, int);

    int sandBlast(TBeing *);
    int castSandBlast(TBeing *);
    int sandBlast(TBeing *, int, short, int);

    int lavaStream(TBeing *);
    int castLavaStream(TBeing *);
    int lavaStream(TBeing *, int, short, int);

    int meteorSwarm(TBeing *, TBeing *);
    int castMeteorSwarm(TBeing *, TBeing *);
    int meteorSwarm(TBeing *, TBeing *, TMagicItem *);
    int meteorSwarm(TBeing *, TBeing *, int, short, int);

    int castStoneSkin(TBeing *, TBeing *);
    int stoneSkin(TBeing *, TBeing *);
    void stoneSkin(TBeing *, TBeing *, TMagicItem *);
    int stoneSkin(TBeing *, TBeing *, int, short);

    void stoneSkin(TObj *, TBeing *, TBeing *);

    int trailSeek(TBeing *, TBeing *);
    int castTrailSeek(TBeing *, TBeing *);
    void trailSeek(TBeing *, TBeing *, TMagicItem *);
    int trailSeek(TBeing *, TBeing *, int, short);

    int conjureElemEarth(TBeing *);
    int castConjureElemEarth(TBeing *);
    int conjureElemEarth(TBeing *, int, short);

    int protectionFromEarth(TBeing *, TBeing *);
    int castProtectionFromEarth(TBeing *, TBeing *);
    void protectionFromEarth(TBeing *, TBeing *, TMagicItem *);
    int protectionFromEarth(TBeing *, TBeing *, int, short);

#endif

