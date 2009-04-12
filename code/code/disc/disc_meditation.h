//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_meditation.h,v $
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


#ifndef __DISC_MEDITATION_MONK_H
#define __DISC_MEDITATION_MONK_H

#include "discipline.h"
#include "skills.h"

class CDMeditationMonk: public CDiscipline
{
public:
    CSkill skWohlin;          // Advanced Yoginsa (meditation)
    CSkill skVoplat;          // Advanced Kubo (makes kubo damage magical)
    CSkill skBlindfighting; 

    CDMeditationMonk() :
      CDiscipline(),
      skWohlin(),
      skVoplat(),
      skBlindfighting()
    {
    }
    CDMeditationMonk(const CDMeditationMonk & a) :
      CDiscipline(a),
      skWohlin(a.skWohlin),
      skVoplat(a.skVoplat),
      skBlindfighting(a.skBlindfighting)
    {
    }
    CDMeditationMonk & operator=(const CDMeditationMonk & a)
    {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skWohlin = a.skWohlin;
      skVoplat = a.skVoplat;
      skBlindfighting = a.skBlindfighting;
      return *this;
    }
    virtual ~CDMeditationMonk() {}
    virtual CDMeditationMonk * cloneMe() { return new CDMeditationMonk(*this); }

private:
};


#endif





















