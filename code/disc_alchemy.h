//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_alchemy.h,v $
// Revision 5.2  2004/09/12 23:53:06  maror
// divination scroll will now work
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


#ifndef __DISC_ALCHEMY_H
#define __DISC_ALCHEMY_H

// This is the ALCHEMY discipline.

class CDAlchemy : public CDiscipline
{
public:
    CSkill skDivination;
    CSkill skShatter;
    CSkill skScribe;
    CSkill skSpontaneousGeneration;
    CSkill skEtherGate;
    CSkill skStaveCharge;

    CDAlchemy() :
      CDiscipline(),
      skDivination(),
      skShatter(),
      skScribe(),
      skSpontaneousGeneration(),
      skEtherGate(),
      skStaveCharge()
    {
    }
    CDAlchemy(const CDAlchemy &a) :
      CDiscipline(a),
      skDivination(a.skDivination),
      skShatter(a.skShatter),
      skScribe(a.skScribe), 
      skSpontaneousGeneration(a.skSpontaneousGeneration),
      skEtherGate(a.skEtherGate),
      skStaveCharge()
    {
    }
    CDAlchemy & operator=(const CDAlchemy &a)
    {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDivination = a.skDivination;
      skShatter = a.skShatter;
      skScribe = a.skScribe;
      skSpontaneousGeneration = a.skSpontaneousGeneration;
      skEtherGate = a.skEtherGate;
      skStaveCharge = a.skStaveCharge;
      return *this;
    }
    virtual ~CDAlchemy() {}
    virtual CDAlchemy * cloneMe() { return new CDAlchemy(*this); }

private:
};

    int identify(TBeing *, TObj *);
    int castIdentify(TBeing *, TObj *);
    int identify(TBeing *, TObj *, int, byte);
    void identify(TBeing *, TObj *, TMagicItem *);
    void identify(TBeing *, TBeing *, TMagicItem *);
    int identify(TBeing *, TBeing *);
    int castIdentify(TBeing *, TBeing *);
    int identify(TBeing *, TBeing *, int, byte);
 
    int divinationBeing(TBeing *, TBeing *);
    int castDivinationBeing(TBeing *, TBeing *);
    int divinationBeing(TBeing *, const TObj *, int, byte);
    void divinationBeing(TBeing *, TBeing *, TMagicItem *);
    int divinationObj(TBeing *, TObj *);
    int castDivinationObj(TBeing *, const TObj *);
    int divinationObj(TBeing *, TBeing *, int, byte);
    void divinationObj(TBeing *, TObj *, TMagicItem *);
  
 
    int eyesOfFertuman(TBeing *, const char *);
    int castEyesOfFertuman(TBeing *, const char *);
    int eyesOfFertuman(TBeing *, const char *, int, byte);
 
    int powerstone(TBeing *, TObj *);
    int castPowerstone(TBeing *, TObj *);
    int powerstone(TBeing *, TObj *, int, byte);
 
    int shatter(TBeing *, TBeing *);
    int castShatter(TBeing *, TBeing *);
    int shatter(TBeing *, TBeing *, int, byte);

//    void farlook(TBeing *, TBeing *); 
    int farlook(TBeing *, TBeing *);
    int castFarlook(TBeing *, TBeing *);
    int farlook(TBeing *, TBeing *, int, byte);
 
    int illuminate(TBeing *, TObj *);
    void illuminate(TBeing *, TMagicItem *, TObj *);
    int illuminate(TBeing *, TObj *, int, byte);
    int castIlluminate(TBeing *, TObj *);
 
    int detectMagic(TBeing *, TBeing *);
    int castDetectMagic(TBeing *, TBeing *);
    void detectMagic(TBeing *, TBeing *, TMagicItem *);
    int detectMagic(TBeing *, TBeing *, int, byte);
 
    int dispelMagic(TBeing *, TObj *);
    int castDispelMagic(TBeing *, TObj *);
    void dispelMagic(TBeing *, TObj *, TMagicItem *);
    int dispelMagic(TBeing *, TObj *, int, byte);
    int dispelMagic(TBeing *, TBeing *);
    int castDispelMagic(TBeing *, TBeing *);
    int dispelMagic(TBeing *, TBeing *, TMagicItem *);
    int dispelMagic(TBeing *, TBeing *, int, byte);
 
    int enhanceWeapon(TBeing *, TObj *);
    int castEnhanceWeapon(TBeing *, TObj *);
    int enhanceWeapon(TBeing *, TMagicItem *, TObj *);
    int enhanceWeapon(TBeing *, TObj *, int, byte);
 
    extern       bool alchemy_create_deny(int);
    const int MATERIALIZE_PRICE = 100;
    int castMaterialize(TBeing *, const char *);
    int materialize(TBeing *, const char *);
    int materialize(TBeing *, TObj **, int, const char *, byte);
 
    const int SPONT_PRICE = 1000;
    void spontaneousGeneration(TBeing *, const char *);
    int spontaneousGeneration(TBeing *, TObj **, const char *, int, byte);
    int castSpontaneousGeneration(TBeing *, const char *);
 
    int copy(TBeing *, TObj *);
    int castCopy(TBeing *, TObj *);
    void copy(TBeing *, TMagicItem *, TObj *);
    int copy(TBeing *, TObj *, int, byte);
 
    int ethrealGate(TBeing *, TObj *);
    int castEthrealGate(TBeing *, TObj *);
    int ethrealGate(TBeing *, TObj *, int, byte);

    int galvanize(TBeing *, TObj *);
    int castGalvanize(TBeing *, TObj *);
    int galvanize(TBeing *, TObj *, int, byte);

#endif
