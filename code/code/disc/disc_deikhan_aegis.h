//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_deikhan_aegis.h,v $
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


#ifndef __DISC_DEIKHAN_AEGIS_H
#define __DISC_DEIKHAN_AEGIS_H

// This is the DEIKHAN AEGIS discipline.

class CDDeikhanAegis : public CDiscipline
{
public:
    CSkill skHeroesFeastDeikhan;
    CSkill skRefreshDeikhan;
    CSkill skSynostodweomer;

    CDDeikhanAegis()
      : CDiscipline(),
      skHeroesFeastDeikhan(),
      skRefreshDeikhan(),
      skSynostodweomer() {
    }
    CDDeikhanAegis(const CDDeikhanAegis &a)
      : CDiscipline(a),
      skHeroesFeastDeikhan(a.skHeroesFeastDeikhan),
      skRefreshDeikhan(a.skRefreshDeikhan),
      skSynostodweomer(a.skSynostodweomer) {
    }
    CDDeikhanAegis & operator=(const CDDeikhanAegis &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skHeroesFeastDeikhan = a.skHeroesFeastDeikhan;
      skRefreshDeikhan = a.skRefreshDeikhan;
      skSynostodweomer = a.skSynostodweomer;
      return *this;
    }
    virtual ~CDDeikhanAegis() {}
    virtual CDDeikhanAegis * cloneMe() { return new CDDeikhanAegis(*this); }

private:
};

#endif

