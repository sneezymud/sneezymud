//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_leverage.h,v $
// Revision 1.1  1999/09/12 17:24:04  peel
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

    CDLeverage()
      : CDiscipline(),
      skShoulderThrow(),
      skHurl(){
    }
    CDLeverage(const CDLeverage &a)
      : CDiscipline(a),
      skShoulderThrow(a.skShoulderThrow),
      skHurl(a.skHurl){
    }
    CDLeverage & operator=(const CDLeverage &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skShoulderThrow = a.skShoulderThrow;
      skHurl = a.skHurl;
      return *this;
    }
    virtual ~CDLeverage() {}
    virtual CDLeverage * cloneMe() { return new CDLeverage(*this); }

private:
};

  int shoulderThrow(TBeing *, TBeing *);
  int hurl(TBeing *, TBeing *, char *);

#endif








