//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_totem.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_TOTEM_H
#define __DISC_TOTEM_H

// This is the SHAMAN TOTEM discipline.

class CDTotem : public CDiscipline
{
public:
    CDTotem()
      : CDiscipline() {
    }
    CDTotem(const CDTotem &a)
      : CDiscipline(a) {
    }
    CDTotem & operator=(const CDTotem &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      return *this;
    }
    virtual ~CDTotem() {}
    virtual CDTotem * cloneMe() { return new CDTotem(*this); }

private:
};


#endif
