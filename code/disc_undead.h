//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_undead.h,v $
// Revision 5.1.1.2  2001/01/28 22:56:57  jesus
// no longer needed...
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


#ifndef __DISC_UNDEAD_H
#define __DISC_UNDEAD_H


// This is the SHAMAN UNDEAD discipline.

class CDUndead : public CDiscipline
{
public:
    CSkill skCreateGolem;

    CDUndead()
      : CDiscipline(),
      skCreateGolem() {
    }
    CDUndead(const CDUndead &a)
      : CDiscipline(a),
      skCreateGolem(a.skCreateGolem) {
    }
    CDUndead & operator=(const CDUndead &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skCreateGolem = a.skCreateGolem;
      return *this;
    }
    virtual ~CDUndead() {}
    virtual CDUndead * cloneMe() { return new CDUndead(*this); } 
private:
};


#endif
