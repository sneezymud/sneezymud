//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_wizardry.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_WIZARDRY_H
#define __DISC_WIZARDRY_H

// This is the WIZARDRY discipline.

class CDWizardry : public CDiscipline
{
public:
    CSkill skWizardry;

    CDWizardry() :
      CDiscipline(),
      skWizardry()
    {
    }
    CDWizardry(const CDWizardry &a) :
      CDiscipline(a),
      skWizardry(a.skWizardry)
    {
    }
    CDWizardry & operator=(const CDWizardry &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skWizardry = a.skWizardry;
      return *this;
    }
    virtual ~CDWizardry() {}
    virtual CDWizardry * cloneMe() { return new CDWizardry(*this); }

private:
};

#endif

