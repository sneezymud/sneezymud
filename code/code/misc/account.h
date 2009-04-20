//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#include "sstring.h"
#include "connect.h" // termTypeT


class AccountStats
{
 public:
  static unsigned int account_number;
  static unsigned int player_count;
  static unsigned int active_player7;
  static unsigned int active_account7;
  static unsigned int active_player30;
  static unsigned int active_account30;
  static unsigned int player_num;
  static unsigned int max_player_since_reboot;

 private:
  AccountStats();
};

class TAccount
{
 public:
  // list of account flags
  static const unsigned int BOSS;
  static const unsigned int IMMORTAL;
  static const unsigned int BANISHED;
  static const unsigned int EMAIL;
  static const unsigned int MSP;
  static const unsigned int ALLOW_DOUBLECLASS;
  static const unsigned int ALLOW_TRIPLECLASS;
  
  int status;
  sstring email;
  sstring passwd;
  sstring name;
  long birth;
  long login;
  termTypeT term;
  Descriptor *desc;
  int time_adjust;
  unsigned int flags;
  int account_id;
  time_t last_logon;
  
  bool read(const sstring &);
  bool write(const sstring &);
  
  TAccount();
  TAccount(const TAccount &a);
  TAccount & operator=(const TAccount &a);
  ~TAccount();
};


#endif
