
#ifndef __DISC_SHAMAN_H
#define __DISC_SHAMAN_H

// This is the SHAMAN discipline.

class CDShaman : public CDiscipline
{
public:
    CSkill skSacrifice;
    CSkill skShieldOfMists;
    CSkill skEnthrallSpectre;
    CSkill skEnthrallGhast;
    CSkill skVoodoo;
    CSkill skCheval;
    CSkill skChrism;
    CSkill skVampiricTouch;
    CSkill skLifeLeech;
    CSkill skDancingBones;
    CDShaman()
      : CDiscipline(),
      skSacrifice(),
      skShieldOfMists(),
      skEnthrallSpectre(),
      skEnthrallGhast(),
      skVoodoo(),
      skCheval(),
      skChrism(),
      skVampiricTouch(),
      skLifeLeech(),
      skDancingBones() { 
    }
    CDShaman(const CDShaman &a)
      : CDiscipline(a),
      skSacrifice(a.skSacrifice),
      skShieldOfMists(a.skShieldOfMists),
      skEnthrallSpectre(a.skEnthrallSpectre),
      skEnthrallGhast(a.skEnthrallGhast),
      skVoodoo(a.skVoodoo),
      skCheval(a.skCheval),
      skChrism(a.skChrism),
      skVampiricTouch(a.skVampiricTouch),
      skLifeLeech(a.skLifeLeech),
      skDancingBones(a.skDancingBones) {
    }
    CDShaman & operator=(const CDShaman &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSacrifice = a.skSacrifice;
      skShieldOfMists = a.skShieldOfMists;
      skEnthrallSpectre = a.skEnthrallSpectre;
      skEnthrallGhast = a.skEnthrallGhast;
      skVoodoo = a.skVoodoo;
      skCheval = a.skCheval;
      skChrism = a.skChrism;
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      skDancingBones = a.skDancingBones;
      return *this;
    }
    virtual ~CDShaman() {}
    virtual CDShaman * cloneMe() { return new CDShaman(*this); }
private:
};

    int voodoo(TBeing *, TObj *, int, byte);
    void voodoo(TBeing *, TObj *, TMagicItem *);
    int voodoo(TBeing *, TObj *);
    int castVoodoo(TBeing *, TObj *);

    int dancingBones(TBeing *, TObj *, int, byte);
    void dancingBones(TBeing *, TObj *, TMagicItem *);
    int dancingBones(TBeing *, TObj *);
    int castDancingBones(TBeing *, TObj *);

    int enthrallSpectre(TBeing * caster, int level, byte bKnown);
    int enthrallSpectre(TBeing * caster);
    int castEnthrallSpectre(TBeing * caster);

    int enthrallGhast(TBeing * caster, int level, byte bKnown);
    int enthrallGhast(TBeing * caster);
    int castEnthrallGhast(TBeing * caster);

    int vampiricTouch(TBeing *, TBeing *);
    int castVampiricTouch(TBeing *, TBeing *);
    int vampiricTouch(TBeing *, TBeing *, int, byte, int);
    int vampiricTouch(TBeing *, TBeing *, TMagicItem *);

    int lifeLeech(TBeing *, TBeing *);
    int castLifeLeech(TBeing *, TBeing *);
    int lifeLeech(TBeing *, TBeing *, int, byte, int);
    int lifeLeech(TBeing *, TBeing *, TMagicItem *);

    int shieldOfMists(TBeing *, TBeing *);
    int castShieldOfMists(TBeing *, TBeing *);
    void shieldOfMists(TBeing *, TBeing *, TMagicItem *);
    int shieldOfMists(TBeing *, TBeing *, int, byte);

    void sacrifice(TBeing *, TBaseCorpse *);

    int cheval(TBeing *, TBeing *);
    int castCheval(TBeing *, TBeing *);
    void cheval(TBeing *, TBeing *, TMagicItem *);
    int cheval(TBeing *, TBeing *, int, byte);
 
    extern       bool shaman_create_deny(int);
    const int CHRISM_PRICE = 250;
    int castChrism(TBeing *, const char *);
    int chrism(TBeing *, const char *);
    int chrism(TBeing *, TObj **, int, const char *, byte);
 
#endif

