//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_faith.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_PIETY_H
#define __DISC_PIETY_H

// This is the FAITH discipline.

class CDFaith : public CDiscipline
{
public:
    CSkill skDevotion;

    CDFaith() :
      CDiscipline(),
      skDevotion()
    {
    }
    CDFaith(const CDFaith &a) :
      CDiscipline(a),
      skDevotion(a.skDevotion)
    {
    }
    CDFaith & operator=(const CDFaith &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDevotion = a.skDevotion;
      return *this;
    }
    virtual ~CDFaith() {}
    virtual CDFaith * cloneMe() { return new CDFaith(*this); }

private:
};

#endif

