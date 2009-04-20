
#ifndef __DISC_SHAMAN_H
#define __DISC_SHAMAN_H

// This is the SHAMAN discipline.

#include "discipline.h"
#include "skills.h"

class CDShaman : public CDiscipline
{
public:
    CSkill skSacrifice;
    CSkill skShieldOfMists;
    CSkill skEnthrallSpectre;
    CSkill skEnthrallGhast;
    CSkill skEnthrallGhoul;
    CSkill skVoodoo;
    CSkill skFlatulence;
    CSkill skStupidity;
    CSkill skSoulTwist;
    CSkill skDistort;
    CSkill skSquish;
    CSkill skIntimidate;
    CSkill skCheval;
    CSkill skChaseSpirits;
    CSkill skChrism;
    CSkill skRombler;
    CSkill skVampiricTouch;
    CSkill skLifeLeech;
    CSkill skSenseLifeShaman;
    CSkill skDetectShadow;
    CSkill skDjallasProtection;
    CSkill skLegbasGuidance;
    CSkill skDancingBones;
    CSkill skRepairShaman;
    CSkill skEmbalm;

    CDShaman()
      : CDiscipline(),
      skSacrifice(),
      skShieldOfMists(),
      skEnthrallSpectre(),
      skEnthrallGhast(),
      skEnthrallGhoul(),
      skVoodoo(),
      skFlatulence(),
      skStupidity(),
      skSoulTwist(),
      skDistort(),
      skSquish(),
      skIntimidate(),
      skCheval(),
      skChaseSpirits(),
      skChrism(),
      skRombler(),
      skVampiricTouch(),
      skLifeLeech(),
      skSenseLifeShaman(),
      skDetectShadow(),
      skDjallasProtection(),
      skLegbasGuidance(),
      skDancingBones(),
      skRepairShaman(),
      skEmbalm() { 
    }
    CDShaman(const CDShaman &a)
      : CDiscipline(a),
      skSacrifice(a.skSacrifice),
      skShieldOfMists(a.skShieldOfMists),
      skEnthrallSpectre(a.skEnthrallSpectre),
      skEnthrallGhast(a.skEnthrallGhast),
      skEnthrallGhoul(a.skEnthrallGhoul),
      skVoodoo(a.skVoodoo),
      skFlatulence(a.skFlatulence),
      skStupidity(a.skStupidity),
      skSoulTwist(a.skSoulTwist),
      skDistort(a.skDistort),
      skSquish(a.skSquish),
      skIntimidate(a.skIntimidate),
      skCheval(a.skCheval),
      skChaseSpirits(a.skChaseSpirits),
      skChrism(a.skChrism),
      skRombler(a.skRombler),
      skVampiricTouch(a.skVampiricTouch),
      skLifeLeech(a.skLifeLeech),
      skSenseLifeShaman(a.skSenseLifeShaman),
      skDetectShadow(a.skDetectShadow),
      skDjallasProtection(a.skDjallasProtection),
      skLegbasGuidance(a.skLegbasGuidance),
      skDancingBones(a.skDancingBones),
      skRepairShaman(a.skRepairShaman),
      skEmbalm(a.skEmbalm){
    }
    CDShaman & operator=(const CDShaman &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSacrifice = a.skSacrifice;
      skShieldOfMists = a.skShieldOfMists;
      skEnthrallSpectre = a.skEnthrallSpectre;
      skEnthrallGhast = a.skEnthrallGhast;
      skEnthrallGhoul = a.skEnthrallGhoul;
      skVoodoo = a.skVoodoo;
      skFlatulence = a.skFlatulence;
      skStupidity = a.skStupidity;
      skSoulTwist = a.skSoulTwist;
      skDistort = a.skDistort;
      skSquish = a.skSquish;
      skIntimidate = a.skIntimidate;
      skCheval = a.skCheval;
      skChaseSpirits = a.skChaseSpirits;
      skChrism = a.skChrism;
      skRombler = a.skRombler;
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      skSenseLifeShaman = a.skSenseLifeShaman;
      skDetectShadow = a.skDetectShadow;
      skDjallasProtection = a.skDjallasProtection;
      skLegbasGuidance = a.skLegbasGuidance;
      skDancingBones = a.skDancingBones;
      skRepairShaman = a.skRepairShaman;
      skEmbalm = a.skEmbalm;
      return *this;
    }
    virtual ~CDShaman() {}
    virtual CDShaman * cloneMe() { return new CDShaman(*this); }

    bool isBasic(){ return true; }

private:
};

    int voodoo(TBeing *, TObj *, int, short);
    void voodoo(TBeing *, TObj *, TMagicItem *);
    int voodoo(TBeing *, TObj *);
    int castVoodoo(TBeing *, TObj *);

    int dancingBones(TBeing *, TObj *, int, short);
    void dancingBones(TBeing *, TObj *, TMagicItem *);
    int dancingBones(TBeing *, TObj *);
    int castDancingBones(TBeing *, TObj *);

    int enthrallSpectre(TBeing * caster, int level, short bKnown);
    int enthrallSpectre(TBeing * caster);
    int castEnthrallSpectre(TBeing * caster);

    int enthrallGhast(TBeing * caster, int level, short bKnown);
    int enthrallGhast(TBeing * caster);
    int castEnthrallGhast(TBeing * caster);

    int enthrallGhoul(TBeing * caster, int level, short bKnown);
    int enthrallGhoul(TBeing * caster);
    int castEnthrallGhoul(TBeing * caster);

    int chaseSpirits(TBeing *, TObj *);
    int castChaseSpirits(TBeing *, TObj *);  
    void chaseSpirits(TBeing *, TObj *, TMagicItem *);  
    int chaseSpirits(TBeing *, TObj *, int, short);  
    int chaseSpirits(TBeing *, TBeing *);  
    int castChaseSpirits(TBeing *, TBeing *);
    int chaseSpirits(TBeing *, TBeing *, TMagicItem *);
    int chaseSpirits(TBeing *, TBeing *, int, short);

    int vampiricTouch(TBeing *, TBeing *);
    int castVampiricTouch(TBeing *, TBeing *);
    int vampiricTouch(TBeing *, TBeing *, int, short, int);
    int vampiricTouch(TBeing *, TBeing *, TMagicItem *);

    int lifeLeech(TBeing *, TBeing *);
    int castLifeLeech(TBeing *, TBeing *);
    int lifeLeech(TBeing *, TBeing *, int, short, int);
    int lifeLeech(TBeing *, TBeing *, TMagicItem *);

    int shieldOfMists(TBeing *, TBeing *);
    int castShieldOfMists(TBeing *, TBeing *);
    void shieldOfMists(TBeing *, TBeing *, TMagicItem *);
    int shieldOfMists(TBeing *, TBeing *, int, short);

    void sacrifice(TBeing *, TBaseCorpse *);

    int cheval(TBeing *, TBeing *);
    int castCheval(TBeing *, TBeing *);
    void cheval(TBeing *, TBeing *, TMagicItem *);
    int cheval(TBeing *, TBeing *, int, short);
 
    extern       bool shaman_create_deny(int);
    const int CHRISM_PRICE = 250;
    int castChrism(TBeing *, const char *);
    int chrism(TBeing *, const char *);
    int chrism(TBeing *, TObj **, int, const char *, short);

    int castRombler(TBeing *);
    int rombler(TBeing *, const char *);
    int rombler(TBeing *, const char *, int, short);

    int intimidate(TBeing *, TBeing *);
    int castIntimidate(TBeing *, TBeing *);
    int intimidate(TBeing *, TBeing *, int, short);

    int senseLifeShaman(TBeing *, TBeing *);
    int castSenseLifeShaman(TBeing *, TBeing *);
    void senseLifeShaman(TBeing *, TBeing *, TMagicItem *);
    int senseLifeShaman(TBeing *, TBeing *, int, short);

    int detectShadow(TBeing *, TBeing *);
    int castDetectShadow(TBeing *, TBeing *);
    void detectShadow(TBeing *, TBeing *, TMagicItem *);
    int detectShadow(TBeing *, TBeing *, int, short);

    int djallasProtection(TBeing *, TBeing *);
    int castDjallasProtection(TBeing *, TBeing *);
    void djallasProtection(TBeing *, TBeing *, TMagicItem *);
    int djallasProtection(TBeing *, TBeing *, int, short);

    int legbasGuidance(TBeing *, TBeing *);
    int castLegbasGuidance(TBeing *, TBeing *);
    void legbasGuidance(TBeing *, TBeing *, TMagicItem *);
    int legbasGuidance(TBeing *, TBeing *, int, short);

    int squish(TBeing *, TBeing *);
    int castSquish(TBeing *, TBeing *);
    int squish(TBeing *, TBeing *, int, short, int);

    int distort(TBeing *, TBeing *);
    int castDistort(TBeing *, TBeing *);
    int distort(TBeing *, TBeing *, TMagicItem *);
    int distort(TBeing *, TBeing *, int, short, int);

    int soulTwist(TBeing *, TBeing *);
    int castSoulTwist(TBeing *, TBeing *);
    int soulTwist(TBeing *, TBeing *, TMagicItem *);
    int soulTwist(TBeing *, TBeing *, int, short, int);

    void stupidity(TBeing *, TBeing *);
    void stupidity(TBeing *, TBeing *, TMagicItem *);
    int castStupidity(TBeing *, TBeing *);
    int stupidity(TBeing *, TBeing *, int, short);

    int flatulence(TBeing *);
    int castFlatulence(TBeing *);
    int flatulence(TBeing *, int, short, int);

    int embalm(TBeing *, TObj *);
    int castEmbalm(TBeing *, TObj *);
    int embalm(TBeing *, TObj *, int, short);

#endif

