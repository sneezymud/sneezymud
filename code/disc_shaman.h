//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SHAMAN_H
#define __DISC_SHAMAN_H

// This is the SHAMAN discipline.

class CDShaman : public CDiscipline
{
public:
    CSkill skRootControl;
    CSkill skCacaodemon;
    CSkill skVoodoo;
    CSkill skControlUndead;
    CSkill skLivingVines;
    CSkill skResurrection;
    CSkill skDancingBones;
    CDShaman()
      : CDiscipline(),
      skRootControl(),
      skCacaodemon(),
      skVoodoo(),
      skControlUndead(),
      skLivingVines(),
      skResurrection(),
      skDancingBones() { 
    }
    CDShaman(const CDShaman &a)
      : CDiscipline(a),
      skRootControl(a.skRootControl),
      skCacaodemon(a.skCacaodemon),
      skVoodoo(a.skVoodoo),
      skControlUndead(a.skControlUndead),
      skLivingVines(a.skLivingVines),
      skResurrection(a.skResurrection),
      skDancingBones(a.skDancingBones) {
    }
    CDShaman & operator=(const CDShaman &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skRootControl = a.skRootControl;
      skCacaodemon = a.skCacaodemon;
      skVoodoo = a.skVoodoo;
      skControlUndead = a.skControlUndead;
      skLivingVines = a.skLivingVines;
      skResurrection = a.skResurrection;
      skDancingBones = a.skDancingBones;
      return *this;
    }
    virtual ~CDShaman() {}
    virtual CDShaman * cloneMe() { return new CDShaman(*this); }
private:
};

    int createGolem(TBeing *);
    int createGolem(TBeing *, int, int, int, byte);

    void controlUndead(TBeing *, TBeing *);
    void controlUndead(TBeing *, TBeing *, TMagicItem *);
    int controlUndead(TBeing *, TBeing *, int, byte);

    int voodoo(TBeing *, TObj *);
    int voodoo(TBeing *, TObj *, TMagicItem *);
    int voodoo(TBeing *, TObj *, int, byte);

    int dancingBones(TBeing *, TObj *);
    int dancingBones(TBeing *, TObj *, TMagicItem *);
    int dancingBones(TBeing *, TObj *, int, byte);

    void cacaodemon(TBeing *, const char *);
    int cacaodemon(TBeing *, const char *, int, byte);

    int resurrection(TBeing *, TObj *);
    int resurrection(TBeing *, TObj *, TMagicItem *);
    int resurrection(TBeing *, TObj *, int, byte);

#endif

