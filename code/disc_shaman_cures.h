//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman_cures.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DISC_SHAMAN_CURES_H
#define __DISC_SHAMAN_CURES_H

// This is the SHAMAN CURES discipline.

class CDShamanCures : public CDiscipline
{
public:
    CDShamanCures() : CDiscipline("cures") {
    }
    virtual ~CDShamanCures() {}

private:
};


#endif
