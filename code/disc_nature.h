#ifndef __DISC_NATURE_H
#define __DISC_NATURE_H

// This is the NATURE discipline.
// Most of these were moved to armadillo for shaman
// reserving disc nature for ranger use

class CDNature : public CDiscipline
{
public:
    CSkill skTreeWalk;



    CDNature() 
      : CDiscipline(),
      skTreeWalk() {
    }
    CDNature(const CDNature &a) 
      : CDiscipline(a),
      skTreeWalk(a.skTreeWalk) {
    }
    CDNature & operator=(const CDNature &a)  {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTreeWalk = a.skTreeWalk;
      return *this;
    }
    virtual ~CDNature() {}
    virtual CDNature * cloneMe() { return new CDNature(*this); }
private:
};

    int treeWalk(TBeing *, const char *, int, byte);
    int treeWalk(TBeing *, const char *);

    int barkskin(TBeing *, TBeing *);
    int castBarkskin(TBeing *, TBeing *);
    int barkskin(TBeing *, TBeing *, TMagicItem *);
    int barkskin(TBeing *, TBeing *, int, byte);



#endif






