//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_spirit.h,v $
// Revision 5.2  2002/06/22 22:19:46  peel
// added knot spell and knot room proc
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


#ifndef __DISC_SPIRIT_H
#define __DISC_SPIRIT_H

// This is the SPIRIT discipline.

#include "discipline.h"
#include "skills.h"

class CDSpirit : public CDiscipline
{
public:
    CSkill skSilence;
    CSkill skCloudOfConcealment;
    CSkill skTrueSight;
    CSkill skPolymorph;
    CSkill skFumble;
    CSkill skKnot;

    CDSpirit()
      : CDiscipline(),
      skSilence(),
      skCloudOfConcealment(),
      skTrueSight(),
      skPolymorph(),
      skFumble(),
      skKnot(){
    }
    CDSpirit(const CDSpirit &a)
      : CDiscipline(a),
      skSilence(a.skSilence),
      skCloudOfConcealment(a.skCloudOfConcealment),
      skTrueSight(a.skTrueSight),
      skPolymorph(a.skPolymorph),
      skFumble(a.skFumble),
      skKnot(a.skKnot){
    }
    CDSpirit & operator=(const CDSpirit &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSilence = a.skSilence;
      skCloudOfConcealment = a.skCloudOfConcealment;
      skTrueSight = a.skTrueSight;
      skPolymorph = a.skPolymorph;
      skFumble = a.skFumble;
      skKnot = a.skKnot;
      return *this;
    }
    virtual ~CDSpirit() {}
    virtual CDSpirit * cloneMe() { return new CDSpirit(*this); }
private:
};

    int knot(TBeing *, TBeing *);
    int castKnot(TBeing *, TBeing *);
    int knot(TBeing *, TBeing *, int, sh_int);

    int silence(TBeing *, TBeing *);
    int castSilence(TBeing *, TBeing *);
    int silence(TBeing *, TBeing *, int, sh_int);

    int slumber(TBeing *, TBeing *);
    int castSlumber(TBeing *, TBeing *);
    int slumber(TBeing *, TBeing *, TMagicItem *);
    int slumber(TBeing *, TBeing *, int, sh_int);

    int ensorcer(TBeing *, TBeing *);
    int castEnsorcer(TBeing *, TBeing *);
    void ensorcer(TBeing *, TBeing *, TMagicItem *);
    int ensorcer(TBeing *, TBeing *, int, sh_int);

    int cloudOfConcealment(TBeing *);
    int castCloudOfConcealment(TBeing *);
    int cloudOfConcealment(TBeing *, int, sh_int);

    int dispelInvisible(TBeing *, TBeing *);
    int castDispelInvisible(TBeing *, TBeing *);
    void dispelInvisible(TBeing *, TBeing *, TMagicItem *);
    int dispelInvisible(TBeing *, TBeing *, int, sh_int);
    void dispelInvisible(TBeing *, TObj *);
    void dispelInvisible(TBeing *, TObj *, TMagicItem *);
    int dispelInvisible(TBeing *, TObj *, int, sh_int);

    int polymorph(TBeing *, const char *);
    int castPolymorph(TBeing *);
    int polymorph(TBeing *, int, sh_int);

    int castStealth(TBeing *, TBeing *);
    int stealth(TBeing *, TBeing *);
    void stealth(TBeing *, TBeing *, TMagicItem *);
    int stealth(TBeing *, TBeing *, int, sh_int);

    int accelerate(TBeing *, TBeing *);
    int castAccelerate(TBeing *, TBeing *);
    void accelerate(TBeing *, TBeing *, TMagicItem *);
    int accelerate(TBeing *, TBeing *, int, sh_int);

    int haste(TBeing *, TBeing *);
    int castHaste(TBeing *, TBeing *);
    void haste(TBeing *, TBeing *, TMagicItem *);
    int haste(TBeing *, TBeing *, int, sh_int);

    int calm(TBeing *, TBeing *);
    int castCalm(TBeing *, TBeing *);
    int calm(TBeing *, TBeing *, int, sh_int);

    int invisibility(TBeing *, TBeing *);
    int castInvisibility(TBeing *, TBeing *);
    void invisibility(TBeing *, TBeing *, TMagicItem *);
    int invisibility(TBeing *, TBeing *, int, sh_int);
    int invisibility(TBeing *, TObj*);
    int castInvisibility(TBeing *, TObj *);
    void invisibility(TBeing *, TObj*, TMagicItem *);
    int invisibility(TBeing *, TObj*, int, sh_int);

    int senseLife(TBeing *, TBeing *);
    int castSenseLife(TBeing *, TBeing *);
    void senseLife(TBeing *, TBeing *, TMagicItem *);
    int senseLife(TBeing *, TBeing *, int, sh_int);

    int detectInvisibility(TBeing *, TBeing *);
    int castDetectInvisibility(TBeing *, TBeing *);
    void detectInvisibility(TBeing *, TBeing *, TMagicItem *);
    int detectInvisibility(TBeing *, TBeing *, int, sh_int);

    int trueSight(TBeing *, TBeing *);
    int castTrueSight(TBeing *, TBeing *);
    void trueSight(TBeing *, TBeing *, TMagicItem *);
    int trueSight(TBeing *, TBeing *, int, sh_int);

    int castTelepathy(TBeing *);
    int telepathy(TBeing *, const char *);
    int telepathy(TBeing *, const char *, int, sh_int);

    int fear(TBeing *, TBeing *);
    int castFear(TBeing *, TBeing *);
    int fear(TBeing *, TBeing *, int, sh_int);

    int fumble(TBeing *, TBeing *);
    int castFumble(TBeing *, TBeing *);
    int fumble(TBeing *, TBeing *, int, sh_int);

#endif

