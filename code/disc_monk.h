//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_monk.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_MONK_H
#define __DISC_MONK_H

class CDMonk : public CDiscipline
{
public:
    CSkill skYoginsa;	// Basic breathing and meditation.
    CSkill skGroundfighting;
    CSkill skCintai;
    CSkill skOomlat;
    CSkill skKickMonk;
    CSkill skAdvancedKicking;
    CSkill skGrappleMonk;
    CSkill skSpringleap;
    CSkill skRetreatMonk;
    CSkill skSnofalte;
    CSkill skCounterMove;
    CSkill skSwitchMonk;
    CSkill skJirin;
    CSkill skKubo;
    CSkill skDufali;
    CSkill skChop;
    CSkill skChi;
    CSkill skDisarmMonk;
    CSkill skCatfall;

    CDMonk()
      : CDiscipline(),
      skYoginsa(),
      skGroundfighting(),
      skCintai(),
      skOomlat(),
      skKickMonk(),
      skAdvancedKicking(),
      skGrappleMonk(),
      skSpringleap(),
      skRetreatMonk(),
      skSnofalte(),
      skCounterMove(),
      skSwitchMonk(),
      skJirin(),
      skKubo(),
      skDufali(),
      skChop(),
      skChi(),
      skDisarmMonk(),
      skCatfall(){
    }
    CDMonk(const CDMonk &a)
      : CDiscipline(a),
      skYoginsa(a.skYoginsa),
      skGroundfighting(a.skGroundfighting),
      skCintai(a.skCintai),
      skOomlat(a.skOomlat),
      skKickMonk(a.skKickMonk),
      skAdvancedKicking(a.skAdvancedKicking),
      skGrappleMonk(a.skGrappleMonk),
      skSpringleap(a.skSpringleap),
      skRetreatMonk(a.skRetreatMonk),
      skSnofalte(a.skSnofalte),
      skCounterMove(a.skCounterMove),
      skSwitchMonk(a.skSwitchMonk),
      skJirin(a.skJirin),
      skKubo(a.skKubo),
      skDufali(a.skDufali),
      skChop(a.skChop),
      skChi(a.skChi),
      skDisarmMonk(a.skDisarmMonk),
      skCatfall(a.skDisarmMonk){
    }
    CDMonk & operator=(const CDMonk &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skYoginsa = a.skYoginsa;
      skGroundfighting = a.skGroundfighting;
      skCintai = a.skCintai;
      skOomlat = a.skOomlat;
      skKickMonk = a.skKickMonk;
      skAdvancedKicking = a.skAdvancedKicking;
      skGrappleMonk = a.skGrappleMonk;
      skSpringleap = a.skSpringleap;
      skRetreatMonk = a.skRetreatMonk;
      skSnofalte = a.skSnofalte;
      skCounterMove = a.skCounterMove;
      skSwitchMonk = a.skSwitchMonk;
      skJirin = a.skJirin;
      skKubo = a.skKubo;
      skDufali = a.skDufali;
      skChop = a.skChop;
      skChi = a.skChi;
      skDisarmMonk = a.skDisarmMonk;
      skCatfall = a.skCatfall;
      return *this;
    }
    virtual ~CDMonk() {}
    virtual CDMonk * cloneMe() { return new CDMonk(*this); }

private:
};

    int task_yoginsa(TBeing *, cmdTypeT, const char *, int, TRoom *, TObj *);
    int grappleMonk(TBeing *, TBeing *, int);
    int springleap(TBeing *, TBeing *, bool);
    void chiMe(TBeing *);
    int chi(TBeing *, TBeing *);
    int chi(TBeing *, TObj *);


#endif
















