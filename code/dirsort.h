//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: dirsort.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DIRSORT_H
#define __DIRSORT_H

class dirlistSort {
  public:
    bool operator() (const string &, const string &) const;
};

#endif
