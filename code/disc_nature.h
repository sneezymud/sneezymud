//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_nature.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_NATURE_H
#define __DISC_NATURE_H

// This is the NATURE discipline.

class CDNature : public CDiscipline
{
public:
    CSkill skSticksToSnakes;
    CSkill skStormySkies;
    CSkill skTreeWalk;
    CSkill skShapeShift;

    CDNature() 
      : CDiscipline(),
      skSticksToSnakes(),
      skStormySkies(),
      skTreeWalk(),
      skShapeShift() {
    }
    CDNature(const CDNature &a) 
      : CDiscipline(a),
      skSticksToSnakes(a.skSticksToSnakes),
      skStormySkies(a.skStormySkies),
      skTreeWalk(a.skTreeWalk),
      skShapeShift(a.skShapeShift) {
    }
    CDNature & operator=(const CDNature &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSticksToSnakes = a.skSticksToSnakes;
      skStormySkies = a.skStormySkies;
      skTreeWalk = a.skTreeWalk;
      skShapeShift = a.skShapeShift;
      return *this;
    }
    virtual ~CDNature() {}
    virtual CDNature * cloneMe() { return new CDNature(*this); }
private:
};

    int barkskin(TBeing *, TBeing *);
    int castBarkskin(TBeing *, TBeing *);
    int barkskin(TBeing *, TBeing *, TMagicItem *);
    int barkskin(TBeing *, TBeing *, int, byte);

    int transformLimb(TBeing *, const char *);
    int castTransformLimb(TBeing *);
    int transformLimb(TBeing *, const char *, int, byte);

    int sticksToSnakes(TBeing *, TBeing *);
    int sticksToSnakes(TBeing *, TBeing *, TMagicItem *);
    int sticksToSnakes(TBeing *, TBeing *, int, byte);

    void livingVines(TBeing *, TBeing *);
    void livingVines(TBeing *, TBeing *, TMagicItem *);
    int livingVines(TBeing *, TBeing *, int, byte);
    
    int rootControl(TBeing *, TBeing *, int, int, byte);
    int rootControl(TBeing *, TBeing *);

    int stormySkies(TBeing *, TBeing *, int, byte);
    int stormySkies(TBeing *, TBeing *, TMagicItem *);
    int stormySkies(TBeing *, TBeing *);

    int treeWalk(TBeing *, const char *, int, byte);
    int treeWalk(TBeing *, const char *);

const int LAST_TRANSFORM_LIMB = 6;
extern struct TransformLimbType TransformLimbList[LAST_TRANSFORM_LIMB];

#endif

