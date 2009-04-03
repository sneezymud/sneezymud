//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: dirsort.h,v $
// Revision 5.2  2003/03/13 22:40:53  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
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


#ifndef __DIRSORT_H
#define __DIRSORT_H

class dirlistSort {
  public:
    bool operator() (const sstring &, const sstring &) const;
};

#endif
