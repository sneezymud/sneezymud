//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_dissect.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __CMD_DISSECT_H
#define __CMD_DISSECT_H

class dissectInfo
{
  public:
    unsigned int loadItem;
    unsigned int amount;
    unsigned int count;
    string message_to_self;
    string message_to_others;

    dissectInfo *tNext;

    dissectInfo() :
      loadItem(0),
      amount(0),
      count(0),
      message_to_self(),
      message_to_others(),
      tNext(NULL)
    {
    }
};

extern map<unsigned short int, dissectInfo>dissect_array;

#endif
