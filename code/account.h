//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: account.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ACCOUNT_H
#define __ACCOUNT_H

extern int account_number;
extern int player_count;

class TAccount
{
  public:
    byte status;
    char email[80];
    char passwd[11];
    char name[10];
    long birth;
    long login;
    termTypeT term;
    Descriptor *desc;
    byte time_adjust;
    unsigned int flags;

    TAccount();
    TAccount(const TAccount &a);
    TAccount & operator=(const TAccount &a);
    ~TAccount();
};

// list of account flags
const unsigned int ACCOUNT_BOSS     = (1<<0);
const unsigned int ACCOUNT_IMMORTAL = (1<<1);
const unsigned int ACCOUNT_BANISHED = (1<<2);
const unsigned int ACCOUNT_EMAIL    = (1<<3);
const unsigned int ACCOUNT_MSP      = (1<<4);
const unsigned int ACCOUNT_ALLOW_DOUBLECLASS      = (1<<5);
const unsigned int ACCOUNT_ALLOW_TRIPLECLASS      = (1<<6);


// This file is saved as "account".  Do not alter structure size
class accountFile {
  public:
    char email[80];
    char passwd[11];
    char name[10];
    long birth;
    byte term;
    byte time_adjust;
    unsigned int flags;
};
#endif
