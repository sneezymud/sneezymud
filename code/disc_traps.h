//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_traps.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_TRAPS_H
#define __DISC_TRAPS_H

// This is the THIEF TRAPS discipline.

class CDTraps : public CDiscipline
{
public:
    CSkill skSetTraps;   

    CDTraps()
      : CDiscipline(),
      skSetTraps() {
    }
    CDTraps(const CDTraps &a)
      : CDiscipline(a),
      skSetTraps(a.skSetTraps) {
    }
    CDTraps & operator=(const CDTraps &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skSetTraps = a.skSetTraps;
      return *this;
    }
    virtual ~CDTraps() {}
    virtual CDTraps * cloneMe() { return new CDTraps(*this); }
private:
};


#endif
