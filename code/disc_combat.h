//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_combat.h,v $
// Revision 5.2  2002/11/12 00:14:33  peel
// added isBasic() and isFast() to CDiscipline
// added isBasic() return true to each discipline that is a basic disc
// added isFast() return true for fast discs, weapon specs etc
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

    bool isBasic(){ return true; }

private:
};

    void sharpen(TBeing *, TThing *);
    void dull(TBeing *, TThing *);

#endif

