//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_barehand.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_BAREHAND_H
#define __DISC_BAREHAND_H

// This is the BAREHAND discipline.

class CDBarehand : public CDiscipline
{
public:
    CSkill skBarehandSpec;

    CDBarehand()
      : CDiscipline(),
      skBarehandSpec() {
    }
    CDBarehand(const CDBarehand &a)
      : CDiscipline(a),
      skBarehandSpec(a.skBarehandSpec) {
    }
    CDBarehand & operator=(const CDBarehand &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skBarehandSpec = a.skBarehandSpec;
      return *this;
    }
    virtual ~CDBarehand() {}
    virtual CDBarehand * cloneMe() { return new CDBarehand(*this); }

private:
};

#endif

