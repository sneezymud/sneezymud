
#ifndef __DISC_SHAMAN_FROG_H
#define __DISC_SHAMAN_FROG_H


class CDShamanFrog : public CDiscipline
{
public:
    CSkill skTurnSkill;

    CDShamanFrog() 
      : CDiscipline(), 
      skTurnSkill() {
    }
    CDShamanFrog(const CDShamanFrog &a) 
      : CDiscipline(a), 
      skTurnSkill(a.skTurnSkill) {
    }
    CDShamanFrog & operator=(const CDShamanFrog &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTurnSkill = a.skTurnSkill;
      return *this;
    }
    virtual ~CDShamanFrog() {}
    virtual CDShamanFrog * cloneMe() { return new CDShamanFrog(*this); }

private:
};


#endif

