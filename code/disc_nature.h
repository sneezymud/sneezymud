#ifndef __DISC_NATURE_H
#define __DISC_NATURE_H

// This is the NATURE discipline.
// Most of these were moved to armadillo for shaman
// reserving disc nature for ranger use

class CDNature : public CDiscipline
{
public:
    CSkill skRootControl;
    CSkill skLivingVines;
    CSkill skTreeWalk;



    CDNature() 
      : CDiscipline(),
      skRootControl(),
      skLivingVines(),
      skTreeWalk() {
    }
    CDNature(const CDNature &a) 
      : CDiscipline(a),
      skRootControl(a.skRootControl),
      skLivingVines(a.skLivingVines),
      skTreeWalk(a.skTreeWalk) {
    }
    CDNature & operator=(const CDNature &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skRootControl = a.skRootControl;
      skLivingVines = a.skLivingVines;
      skTreeWalk = a.skTreeWalk;
      return *this;
    }
    virtual ~CDNature() {}
    virtual CDNature * cloneMe() { return new CDNature(*this); }
private:
};

    void livingVines(TBeing *, TBeing *);
    void livingVines(TBeing *, TBeing *, TMagicItem *);
    int livingVines(TBeing *, TBeing *, int, byte);
    
    int treeWalk(TBeing *, const char *, int, byte);
    int treeWalk(TBeing *, const char *);

    int barkskin(TBeing *, TBeing *);
    int castBarkskin(TBeing *, TBeing *);
    int barkskin(TBeing *, TBeing *, TMagicItem *);
    int barkskin(TBeing *, TBeing *, int, byte);

    int rootControl(TBeing *, TBeing *, int, int, byte);
    int rootControl(TBeing *, TBeing *, TMagicItem *);
    int rootControl(TBeing *, TBeing *);



#endif






