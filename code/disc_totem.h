#ifndef __DISC_TOTEM_H
#define __DISC_TOTEM_H

class CDTotem : public CDiscipline
{
public:
    CSkill skTurnSkill;

    CDTotem() 
      : CDiscipline(), 
      skTurnSkill() {
    }
    CDTotem(const CDTotem &a) 
      : CDiscipline(a), 
      skTurnSkill(a.skTurnSkill) {
    }
    CDTotem & operator=(const CDTotem &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skTurnSkill = a.skTurnSkill;
      return *this;
    }
    virtual ~CDTotem() {}
    virtual CDTotem * cloneMe() { return new CDTotem(*this); }

private:
};


#endif


