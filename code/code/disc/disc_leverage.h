//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_LEVERAGE_H
#define __DISC_LEVERAGE_H 

class CDLeverage : public CDiscipline
{
public:
    CSkill skShoulderThrow;
    CSkill skHurl;
    CSkill skChainAttack;
    CSkill skDefenestrate;
    CSkill skBoneBreak;

    CDLeverage()
      : CDiscipline(),
      skShoulderThrow(),
      skHurl(),
      skChainAttack(),
      skDefenestrate(),
      skBoneBreak(){
    }
    CDLeverage(const CDLeverage &a)
      : CDiscipline(a),
      skShoulderThrow(a.skShoulderThrow),
      skHurl(a.skHurl),
      skChainAttack(a.skChainAttack),
      skDefenestrate(a.skDefenestrate),
      skBoneBreak(a.skBoneBreak){

    }
    CDLeverage & operator=(const CDLeverage &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skShoulderThrow = a.skShoulderThrow;
      skHurl = a.skHurl;
      skChainAttack = a.skChainAttack;
      skDefenestrate = a.skDefenestrate;
      skBoneBreak = a.skBoneBreak;
      return *this;
    }
    virtual ~CDLeverage() {}
    virtual CDLeverage * cloneMe() { return new CDLeverage(*this); }

private:
};

  int shoulderThrow(TBeing *, TBeing *);
  int hurl(TBeing *, TBeing *, char *);
  int defenestrate(TBeing *, TBeing *, sstring);
  int bonebreak(TBeing *, TBeing *);

#endif








