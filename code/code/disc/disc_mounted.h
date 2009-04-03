//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mounted.h,v $
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


#ifndef __DISC_MOUNTED_H
#define __DISC_MOUNTED_H

// This is the DEIKHAN MOUNT discipline.

class CDMounted : public CDiscipline
{
public:
    CSkill skCalmMount;
    CSkill skTrainMount;
    CSkill skAdvancedRiding;
    CSkill skRideDomestic;
    CSkill skRideNonDomestic;
    CSkill skRideWinged;
    CSkill skRideExotic;

    CDMounted()
      : CDiscipline(),
      skCalmMount(),
      skTrainMount(),
      skAdvancedRiding(),
      skRideDomestic(),
      skRideNonDomestic(),
      skRideWinged(),
      skRideExotic() {
    }
    CDMounted(const CDMounted &a)
      : CDiscipline(a),
      skCalmMount(a.skCalmMount),
      skTrainMount(a.skTrainMount),
      skAdvancedRiding(a.skAdvancedRiding),
      skRideDomestic(a.skRideDomestic),
      skRideNonDomestic(a.skRideNonDomestic),
      skRideWinged(a.skRideWinged),
      skRideExotic(a.skRideExotic) {
    }
    CDMounted & operator=(const CDMounted &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skCalmMount = a.skCalmMount;
      skTrainMount = a.skTrainMount;
      skAdvancedRiding = a.skAdvancedRiding;
      skRideDomestic = a.skRideDomestic;
      skRideNonDomestic = a.skRideNonDomestic;
      skRideWinged = a.skRideWinged;
      skRideExotic = a.skRideExotic;
      return *this;
    }
    virtual ~CDMounted() {}
    virtual CDMounted * cloneMe() { return new CDMounted(*this); }

private:
};

#endif

