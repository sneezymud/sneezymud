#ifndef __DISC_MAGE_THIEF_H
#define __DISC_MAGE_THIEF_H

class CDMageThief : public CDiscipline
{
public:
  CSkill skBackstabMT;

    CDMageThief();
    CDMageThief(const CDMageThief &);
    CDMageThief & operator=(const CDMageThief &a);
    virtual ~CDMageThief();
    virtual CDMageThief * cloneMe() { return new CDMageThief(*this); }
};


#endif
