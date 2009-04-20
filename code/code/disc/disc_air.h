//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_air.h,v $
// Revision 5.3  2003/01/13 17:37:21  peel
// fixed feathery descent function for potions
//
// Revision 5.2  2002/11/29 20:03:24  peel
// added prototype for weightCorrectDuration
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


#ifndef __DISC_AIR_H
#define __DISC_AIR_H

// This is the AIR discipline.

#include "discipline.h"
#include "skills.h"

class CDAir : public CDiscipline
{
public:
    CSkill skImmobilize;               // NEW  30th level individual
    CSkill skSuffocate;               //      40th level individual
    CSkill skFly;
    CSkill skAntigravity;

    CDAir();
    CDAir(const CDAir &a);
    CDAir & operator= (const CDAir &a);
    virtual ~CDAir();
    virtual CDAir * cloneMe();
private:
};

    int gust(TBeing *, TBeing *);
    int gust(TBeing*, TBeing *, int, short, int);
    int castGust(TBeing *, TBeing *);
    int gust(TBeing *, TBeing *, TMagicItem *);
 
    int immobilize(TBeing *, TBeing *);
    int immobilize(TBeing *, TBeing *, int, short);
    int castImmobilize(TBeing *, TBeing *);
 
    int suffocate(TBeing *, TBeing *);
    void suffocate(TBeing *, TBeing *, TMagicItem *);
    int suffocate(TBeing *, TBeing *, int, short );
    int castSuffocate(TBeing *, TBeing *);
 
    int dustStorm(TBeing *);
    int dustStorm(TBeing *, int, short, int);
    int castDustStorm(TBeing *);
 
    void tornado(TBeing *);
    void tornado(TBeing *, TMagicItem *);
    int tornado(TBeing *, int, short, int);
    int castTornado(TBeing *);
 
    void featheryDescent(TBeing *, TBeing *, int, int);
    void featheryDescent(TBeing *, TBeing *, TMagicItem *);
    int featheryDescent(TBeing *, TBeing *, int, affectedData *, short);
    int castFeatheryDescent(TBeing *, TBeing *);
    int featheryDescent(TBeing *, TBeing *);
 
    int castFly(TBeing *, TBeing *);
    int fly(TBeing *, TBeing *);
    void fly(TBeing *, TBeing *, TMagicItem *);
    int fly(TBeing *, TBeing *, int, affectedData *, short);
 
    int antigravity(TBeing *);
    int antigravity(TBeing *, int, affectedData *, short);
    int castAntigravity(TBeing *);
 
    int castConjureElemAir(TBeing *);
    int conjureElemAir(TBeing *, int, short);
    int conjureElemAir(TBeing *);

 
    void levitate(TBeing *, TBeing *);
    int levitate(TBeing *, TBeing *, int, short);
    int castLevitate(TBeing *, TBeing *);
 
    int falconWings(TBeing *, TBeing *);
    void falconWings(TBeing *, TBeing *, TMagicItem *);
    int falconWings(TBeing *, TBeing *, int, short);
    int castFalconWings(TBeing *, TBeing *);
 
    int castProtectionFromAir(TBeing *, TBeing *);
    int protectionFromAir(TBeing *, TBeing *);
    void protectionFromAir(TBeing *, TBeing *, TMagicItem *);
    int protectionFromAir(TBeing *, TBeing *, int, short);

    void weightCorrectDuration(const TBeing *, affectedData *);

#endif

