//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mindbody.h,v $
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


#ifndef __DISC_MINDBODY_H
#define __DISC_MINDBODY_H 

class CDMindBody : public CDiscipline
{
public:
  CSkill skFeignDeath;
  CSkill skBlur;

    CDMindBody();
    CDMindBody(const CDMindBody &a);
    CDMindBody & operator=(const CDMindBody &a);
    virtual ~CDMindBody();
    virtual CDMindBody * cloneMe();

private:
};

#endif
