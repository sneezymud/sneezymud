//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#pragma once

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
  
  unsigned multiplay_limit = 2;
  int status = 0;
  sstring email;
  sstring passwd;
  sstring name;
  long birth = 0;
  long login = 0;
  termTypeT term = TERM_NONE;
  Descriptor *desc = nullptr;
  int time_adjust = 0;
  unsigned int flags = 0;
  int account_id = 0;
  time_t last_logon = 0;
  
  bool read(const sstring &);
  bool write(const sstring &);
  
  TAccount();
};

