//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mindbody.h,v $
// Revision 1.1  1999/09/12 17:24:04  peel
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
