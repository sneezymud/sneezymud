//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman_alchemy.h,v $
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


#ifndef __DISC_SHAMAN_ALCHEMY_H
#define __DISC_SHAMAN_ALCHEMY_H

// This is the SHAMAN ALCHEMY discipline.

class CDShamanAlchemy : public CDiscipline
{
public:
    CSkill skBrew;

    CDShamanAlchemy()
      : CDiscipline(),
      skBrew() {
    }
    CDShamanAlchemy(const CDShamanAlchemy &a)
      : CDiscipline(a),
      skBrew(a.skBrew) {
    }
    CDShamanAlchemy & operator=(const CDShamanAlchemy &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a),
      skBrew = a.skBrew;
      return *this;
    }
    virtual ~CDShamanAlchemy() {}
    virtual CDShamanAlchemy * cloneMe() { return new CDShamanAlchemy(*this); }
private:
};


#endif
