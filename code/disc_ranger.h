//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_ranger.h,v $
// Revision 5.2  2001/12/15 04:26:52  jesus
// added in butcher skill under adventuring disc
// will add more descs for races as time goes on 30 of about 125 races supported
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


#ifndef __DISC_RANGER_H
#define __DISC_RANGER_H

// This is the RANGER discipline.

class CDRanger : public CDiscipline
{
public:
// Level 1
    CSkill skHiking;
    CSkill skButcher;

// Level 3
    CSkill skKickRanger;

// Level 7
    CSkill skForage;
    CSkill skSeekWater;

// Level 8
    CSkill skTransformLimb;
    CSkill skBeastSoother;

//Level 10
    CSkill skTrack;

// Level 12
    CSkill skBashRanger;

// level 14
    CSkill skRescueRanger;

//Level 15
    CSkill skBefriendBeast;
    CSkill skTransfix;

// Level 17
    CSkill skSkin;

// Level 18
    CSkill skSwitchRanger;

// Level 20
    CSkill skDualWield;

// Level 23

// Level 26
    CSkill skBeastSummon;

// Level 30
    CSkill skBarkskin;

    CDRanger()
      : CDiscipline(),
      skHiking(),
      skButcher(),
      skKickRanger(),
      skForage(),
      skSeekWater(),
      skTransformLimb(),
      skBeastSoother(),
      skTrack(),
      skBashRanger(),
      skRescueRanger(),
      skBefriendBeast(),
      skTransfix(),
      skSkin(),
      skSwitchRanger(),
      skDualWield(),
      skBeastSummon(),
      skBarkskin() {
    }
    CDRanger(const CDRanger &a)
      : CDiscipline(a),
      skHiking(a.skHiking),
      skButcher(a.skButcher),
      skKickRanger(a.skKickRanger),
      skForage(a.skForage),
      skSeekWater(a.skSeekWater),
      skTransformLimb(a.skTransformLimb),
      skBeastSoother(a.skBeastSoother),
      skTrack(a.skTrack),
      skBashRanger(a.skBashRanger),
      skRescueRanger(a.skRescueRanger),
      skBefriendBeast(a.skBefriendBeast),
      skTransfix(a.skTransfix),
      skSkin(a.skSkin),
      skSwitchRanger(a.skSwitchRanger),
      skDualWield(a.skDualWield),
      skBeastSummon(a.skBeastSummon),
      skBarkskin(a.skBarkskin) {
    }
    CDRanger & operator=(const CDRanger &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHiking = a.skHiking;
      skButcher = a.skButcher;
      skKickRanger = a.skKickRanger;
      skForage = a.skForage;
      skSeekWater = a.skSeekWater;
      skTransformLimb = a.skTransformLimb;
      skBeastSoother = a.skBeastSoother;
      skTrack = a.skTrack;
      skBashRanger = a.skBashRanger;
      skRescueRanger = a.skRescueRanger;
      skBefriendBeast = a.skBefriendBeast;
      skTransfix = a.skTransfix;
      skSkin = a.skSkin;
      skSwitchRanger = a.skSwitchRanger;
      skDualWield = a.skDualWield;
      skBeastSummon = a.skBeastSummon;
      skBarkskin = a.skBarkskin;
      return *this;
    }
    virtual ~CDRanger() {}
    virtual CDRanger * cloneMe() { return new CDRanger(*this); }
private:
};

    int conceal(TBeing *, TBeing *);

#endif

