//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ACCOUNT_H
#define __ACCOUNT_H

class TAccountStats
{
  public:
    unsigned int account_number;
    unsigned int player_count;
    unsigned int active_player7;
    unsigned int active_account7;
    unsigned int active_player30;
    unsigned int active_account30;
    unsigned int player_num;
    unsigned int max_player_since_reboot;

    TAccountStats() :
      account_number(0),
      player_count(0),
      active_player7(0),
      active_account7(0),
      active_player30(0),
      active_account30(0),
      player_num(0),
      max_player_since_reboot(0)
    {}
};
extern TAccountStats accStat;

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
    time_t last_logon;
};
#endif
