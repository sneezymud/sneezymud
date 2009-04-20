//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SORCERY_H
#define __DISC_SORCERY_H

// This is the SORCERY discipline.

#include "discipline.h"
#include "skills.h"

class CDSorcery : public CDiscipline
{
public:
    CSkill skEnergyDrain;
    CSkill skAtomize;
    CSkill skBlastOfFury;
//    CSkill skChainLightning;
    CSkill skAnimate;
    CSkill skBind;

    CDSorcery() :
      CDiscipline(),
      skEnergyDrain(),
      skAtomize(),
      skBlastOfFury(),
//      skChainLightning(),
      skAnimate(),
      skBind() {
    }
    CDSorcery(const CDSorcery &a) :
      CDiscipline(a),
      skEnergyDrain(a.skEnergyDrain),
      skAtomize(a.skAtomize),
      skBlastOfFury(a.skBlastOfFury),
//      skChainLightning(a.skChainLightning),
      skAnimate(a.skAnimate),
      skBind(a.skBind) {
    }
    CDSorcery & operator=(const CDSorcery &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skEnergyDrain = a.skEnergyDrain;
      skAtomize = a.skAtomize;
      skBlastOfFury = a.skBlastOfFury;
//      skChainLightning = a.skChainLightning;
      skAnimate = a.skAnimate;
      skBind = a.skBind;
      return *this;
    }
    virtual ~CDSorcery() {}
    virtual CDSorcery * cloneMe() { return new CDSorcery(*this); }
private:
};

    int mysticDarts(TBeing *, TBeing *);
    int castMysticDarts(TBeing *, TBeing *);
    int mysticDarts(TBeing *, TBeing *, TMagicItem *);
    int mysticDarts(TBeing *, TBeing *, int, short, int);

    int stunningArrow(TBeing *, TBeing *);
    int castStunningArrow(TBeing *, TBeing *);
    int stunningArrow(TBeing *, TBeing *, TMagicItem *);
    int stunningArrow(TBeing *, TBeing *, int, short, int);

    int blastOfFury(TBeing *, TBeing *);
    int castBlastOfFury(TBeing *, TBeing *);
    int blastOfFury(TBeing *, TBeing *, TMagicItem *);
    int blastOfFury(TBeing *, TBeing *, int, short, int);

    int colorSpray(TBeing *);
    int castColorSpray(TBeing *);
    int colorSpray(TBeing *, TMagicItem *);
    int colorSpray(TBeing *, int, short, int);

    int energyDrain(TBeing *, TBeing *);
    int castEnergyDrain(TBeing *, TBeing *);
    int energyDrain(TBeing *, TBeing *, int, short, int);
    int energyDrain(TBeing *, TBeing *, TMagicItem *);

    int acidBlast(TBeing *);
    int castAcidBlast(TBeing *);
    int acidBlast(TBeing *, int, short, int);

    int atomize(TBeing *, TBeing *);
    int castAtomize(TBeing *, TBeing *);
    int atomize(TBeing *, TBeing *, int, short, int);
    int atomize(TBeing *, TBeing *, TMagicItem *);

    int castAnimate(TBeing *);
    int animate(TBeing *);
    int animate(TBeing *, int, short);

    int sorcerersGlobe(TBeing *, TBeing *);
    int castSorcerersGlobe(TBeing *, TBeing *);
    void sorcerersGlobe(TBeing *, TBeing *, TMagicItem *);
    int sorcerersGlobe(TBeing *, TBeing *, int, short);

    int bind(TBeing *, TBeing *);
    int castBind(TBeing *, TBeing *);
    int bind(TBeing *, TBeing *, int, short);
    void bind(TBeing *, TBeing *, TMagicItem *);

    int teleport(TBeing *, TBeing *);
    int castTeleport(TBeing *, TBeing *);
    int teleport(TBeing *, TBeing *, TMagicItem *);
    int teleport(TBeing *, TBeing *, int, short);

    int protectionFromElements(TBeing *, TBeing *);
    int castProtectionFromElements(TBeing *, TBeing *);
    void protectionFromElements(TBeing *, TBeing *, TMagicItem *);
    int protectionFromElements(TBeing *, TBeing *, int, short);

#if 0
    int chainLightning(TBeing *, TBeing *);
    int chainLightning(TBeing *, TBeing *, int, short);
#endif

#endif

