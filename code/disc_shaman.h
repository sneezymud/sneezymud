
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
    CSkill skVampiricTouch;
    CSkill skLifeLeech;
    CSkill skResurrection;
    CSkill skDancingBones;
    CDShaman()
      : CDiscipline(),
      skSacrifice(),
      skShieldOfMists(),
      skEnthrallSpectre(),
      skEnthrallGhast(),
      skVoodoo(),
      skVampiricTouch(),
      skLifeLeech(),
      skResurrection(),
      skDancingBones() { 
    }
    CDShaman(const CDShaman &a)
      : CDiscipline(a),
      skSacrifice(a.skSacrifice),
      skShieldOfMists(a.skShieldOfMists),
      skEnthrallSpectre(a.skEnthrallSpectre),
      skEnthrallGhast(a.skEnthrallGhast),
      skVoodoo(a.skVoodoo),
      skVampiricTouch(a.skVampiricTouch),
      skLifeLeech(a.skLifeLeech),
      skResurrection(a.skResurrection),
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
      skVampiricTouch = a.skVampiricTouch;
      skLifeLeech = a.skLifeLeech;
      skResurrection = a.skResurrection;
      skDancingBones = a.skDancingBones;
      return *this;
    }
    virtual ~CDShaman() {}
    virtual CDShaman * cloneMe() { return new CDShaman(*this); }
private:
};

    int voodoo(TBeing *, TObj *);
    int voodoo(TBeing *, TObj *, TMagicItem *);
    int voodoo(TBeing *, TObj *, int, byte);

    int dancingBones(TBeing *, TObj *);
    int dancingBones(TBeing *, TObj *, TMagicItem *);
    int dancingBones(TBeing *, TObj *, int, byte);

    int enthrallSpectre(TBeing * caster, int level, byte bKnown);
    int enthrallSpectre(TBeing * caster);
    int castEnthrallSpectre(TBeing * caster);

    int enthrallGhast(TBeing * caster, int level, byte bKnown);
    int enthrallGhast(TBeing * caster);
    int castEnthrallGhast(TBeing * caster);

    int resurrection(TBeing *, TObj *);
    int resurrection(TBeing *, TObj *, TMagicItem *);
    int resurrection(TBeing *, TObj *, int, byte);

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
#endif

