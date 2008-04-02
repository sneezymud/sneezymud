//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_ADV_ADVENTURING_H
#define __DISC_ADV_ADVENTURING_H   1


class CDAdvAdventuring : public CDiscipline
{
  public:
    CSkill skHiking;
    CSkill skForage;
    CSkill skSeekWater;
    CSkill skSkin;
    CSkill skDivination;
    CSkill skEncamp;
    CSkill skFishlore;


    CDAdvAdventuring()
      : CDiscipline(),
      skHiking(),
      skForage(),
      skSeekWater(),
      skSkin(),
      skDivination(),
      skEncamp(),
      skFishlore()
      {
      }
    CDAdvAdventuring(const CDAdvAdventuring &a)
      : CDiscipline(a),
      skHiking(a.skHiking),
      skForage(a.skForage),
      skSeekWater(a.skSeekWater),
      skSkin(a.skSkin),
      skDivination(a.skDivination),
      skEncamp(a.skEncamp),
      skFishlore(a.skFishlore)
      {
      }
    CDAdvAdventuring & operator=(const CDAdvAdventuring &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHiking = a.skHiking;
      skForage = a.skForage;
      skSeekWater = a.skSeekWater;
      skSkin = a.skSkin;
      skDivination = a.skDivination;
      skEncamp = a.skEncamp;
      skFishlore = a.skFishlore;
      return *this;
    }
    virtual ~CDAdvAdventuring() {};

    virtual CDAdvAdventuring * cloneMe() {
      return new CDAdvAdventuring(*this);
    }

    bool isFast(){ return true; }

private:
};

    void forage(TBeing *);
    int forage(TBeing *, byte);
    int forage_insect(TBeing *caster);

    void divine(TBeing *, TThing *);
    int divine(TBeing *, int, byte, TThing *);

    int encamp(TBeing *);


#endif

