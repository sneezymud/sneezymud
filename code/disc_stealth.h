//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_stealth.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_STEALTH_H
#define __DISC_STEALTH_H

// This is the THIEF Stealth discipline.

class CDStealth : public CDiscipline
{
public:
    CSkill skDisguise;

    CDStealth()
      : CDiscipline(),
      skDisguise() {
    }
    CDStealth(const CDStealth &a)
      : CDiscipline(a),
      skDisguise(a.skDisguise) {
    }
    CDStealth & operator=(const CDStealth &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skDisguise = a.skDisguise;
      return *this;
    }
    virtual ~CDStealth() {}
    virtual CDStealth * cloneMe() { return new CDStealth(*this); }
private:
};


#endif
