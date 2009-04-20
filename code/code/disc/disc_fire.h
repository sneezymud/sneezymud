//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_fire.h,v $
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


#ifndef __DISC_FIRE_H
#define __DISC_FIRE_H

// This is the FIRE discipline.

#include "discipline.h"
#include "skills.h"

class CDFire : public CDiscipline
{
public:
    CSkill skInferno;             // NEW  12th level area affect
    CSkill skHellFire;            // NEW  36th level area affect
    CSkill skFlamingFlesh;

    CDFire()
      : CDiscipline(),
      skInferno(),
      skHellFire(),
      skFlamingFlesh() {
    }
    CDFire(const CDFire &a)
      : CDiscipline(a),
      skInferno(a.skInferno),
      skHellFire(a.skHellFire),
      skFlamingFlesh(a.skFlamingFlesh) {
    }
    CDFire & operator=(const CDFire &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skInferno = a.skInferno;
      skHellFire = a.skHellFire;
      skFlamingFlesh = a.skFlamingFlesh;
      return *this;
    }
    virtual ~CDFire() {}
    virtual CDFire * cloneMe() { return new CDFire(*this); }
private:
};

    int castHandsOfFlame(TBeing *, TBeing *);
    int handsOfFlame(TBeing *, TBeing *);
    int handsOfFlame(TBeing *, TBeing *, TMagicItem *);
    int handsOfFlame(TBeing *, TBeing *, int, short, int *, int);

    void faerieFire(TBeing *, TBeing *);
    void faerieFire(TBeing *, TBeing *, TMagicItem *);
    int castFaerieFire(TBeing *, TBeing *);
    int faerieFire(TBeing *, TBeing *, int, short);

    int castFlamingSword(TBeing *, TBeing *);
    int flamingSword(TBeing *, TBeing *);
    int flamingSword(TBeing *, TBeing *, TMagicItem *);
    int flamingSword(TBeing *, TBeing *, int, short, int);

    int castInferno(TBeing *, TBeing *);
    int inferno(TBeing *, TBeing *);
    int inferno(TBeing *, TBeing *, int, short, int);
    int inferno(TBeing *, TBeing *, TMagicItem *);

    int castHellfire(TBeing *);
    int hellfire(TBeing *);
    int hellfire(TBeing *, TMagicItem *);
    int hellfire(TBeing *, int, short, int);

    int castFireball(TBeing *); 
    int fireball(TBeing *);
    int fireball(TBeing *, TMagicItem *);
    int fireball(TBeing *, int, short, int);

    int flamingFlesh(TBeing *, TBeing *);
    void flamingFlesh(TBeing *, TBeing *, TMagicItem *);
    int castFlamingFlesh(TBeing *, TBeing *);
    int flamingFlesh(TBeing *, TBeing *, int, short);

    int castConjureElemFire(TBeing *);
    int conjureElemFire(TBeing *);
    int conjureElemFire(TBeing *, int, short);

    int castFlare(TBeing *);
    int flare(TBeing *);
    int flare(TBeing *, TMagicItem *);
    int flare(TBeing *, int, short);

    void infravision(TBeing *, TBeing *);
    void infravision(TBeing *, TBeing *, TMagicItem *);
    int infravision(TBeing *, TBeing *, int, short);
    int castInfravision(TBeing *, TBeing *);

    int protectionFromFire(TBeing *, TBeing *);
    int castProtectionFromFire(TBeing *, TBeing *);
    void protectionFromFire(TBeing *, TBeing *, TMagicItem *);
    int protectionFromFire(TBeing *, TBeing *, int, short);

#endif

