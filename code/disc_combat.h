//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_combat.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_COMBAT_H
#define __DISC_COMBAT_H   1

// This contains the general combat skills.  

class CDCombat : public CDiscipline
{
  public:
    CSkill skBarehand;
    CSkill skArmorUse;
    CSkill skSlash;
    CSkill skBow;
    CSkill skPierce;
    CSkill skBlunt;
    CSkill skSharpen;
    CSkill skDull;

    CDCombat();
    CDCombat(const CDCombat &a);
    CDCombat & operator=(const CDCombat &a);
    virtual ~CDCombat();
    virtual CDCombat * cloneMe() { return new CDCombat(*this); }

private:
};

    void sharpen(TBeing *, TThing *);
    void dull(TBeing *, TThing *);

#endif

