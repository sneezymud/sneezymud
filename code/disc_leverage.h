//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_leverage.h,v $
// Revision 5.2  2002/07/02 07:09:57  peel
// Monk changes:
//
// removed the affect of the meditation disc on barehand damage (25% damage)
// added 5% damage to blur (+5% occurance rate)
// added 5% damage to critical hitting (double damage for all crits)
// added 5% damage for voplat
// added chain attack skill, 5% damage
// increase yoginsa meditation speed by 25% based on wohlin learning
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


#ifndef __DISC_LEVERAGE_H
#define __DISC_LEVERAGE_H 

class CDLeverage : public CDiscipline
{
public:
    CSkill skShoulderThrow;
    CSkill skHurl;
    CSkill skChainAttack;

    CDLeverage()
      : CDiscipline(),
      skShoulderThrow(),
      skHurl(),
      skChainAttack(){
    }
    CDLeverage(const CDLeverage &a)
      : CDiscipline(a),
      skShoulderThrow(a.skShoulderThrow),
      skHurl(a.skHurl),
      skChainAttack(a.skChainAttack){
    }
    CDLeverage & operator=(const CDLeverage &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skShoulderThrow = a.skShoulderThrow;
      skHurl = a.skHurl;
      skChainAttack = a.skChainAttack;
      return *this;
    }
    virtual ~CDLeverage() {}
    virtual CDLeverage * cloneMe() { return new CDLeverage(*this); }

private:
};

  int shoulderThrow(TBeing *, TBeing *);
  int hurl(TBeing *, TBeing *, char *);

#endif








