#include "account.h"
#include "database.h"

// static defs
unsigned int AccountStats::account_number=0;
unsigned int AccountStats::player_count=0;
unsigned int AccountStats::active_player7=0;
unsigned int AccountStats::active_account7=0;
unsigned int AccountStats::active_player30=0;
unsigned int AccountStats::active_account30=0;
unsigned int AccountStats::player_num=0;
unsigned int AccountStats::max_player_since_reboot=0;

const unsigned int TAccount::BOSS     = (1<<0);
const unsigned int TAccount::IMMORTAL = (1<<1);
const unsigned int TAccount::BANISHED = (1<<2);
const unsigned int TAccount::EMAIL    = (1<<3);
const unsigned int TAccount::MSP      = (1<<4);
const unsigned int TAccount::ALLOW_DOUBLECLASS      = (1<<5);
const unsigned int TAccount::ALLOW_TRIPLECLASS      = (1<<6);



TAccount::TAccount()
  : birth(time(0))
{}

bool TAccount::read(const sstring &aname)
{
  TDatabase db(DB_SNEEZY);
  db.query("select multiplay_limit, account_id, email, passwd, name, birth, term, time_adjust, flags, last_logon from account where name=lower('%s')", aname.c_str());
  if (!db.fetchRow())
    return false;

  multiplay_limit = convertTo<int>(db["multiplay_limit"]);
  account_id = convertTo<int>(db["account_id"]);
  email=db["email"];
  passwd=db["passwd"];
  name=db["name"];
  birth=convertTo<int>(db["birth"]);
  term=(termTypeT)convertTo<int>(db["term"]);
  time_adjust=convertTo<int>(db["time_adjust"]);
  flags=convertTo<int>(db["flags"]);
  last_logon=convertTo<int>(db["last_logon"]);

  login = time(0);
  status = FALSE;

  return true;
}

bool TAccount::write(const sstring &aname)
{
  TDatabase db(DB_SNEEZY);
  bool res;

  db.query("select 1 from account where name=lower('%s')", aname.c_str());

  if(!db.fetchRow()){
    res=db.query("insert into account (multiplay_limit, email, passwd, name, birth, term, time_adjust, flags, last_logon) values (%i, '%s', '%s', lower('%s'), %i, %i, %i, %i, %i)",
	     multiplay_limit, email.c_str(), passwd.c_str(), name.c_str(), birth, term,
	     time_adjust, flags, last_logon);

    if (account_id == 0) {
      db.query("select account_id from account where lower(name) = lower('%s')", aname.c_str());
      assert(db.fetchRow());
      account_id = convertTo<int>(db["account_id"]);
    }

  } else {
    res=db.query("update account set multiplay_limit=%i, email='%s', passwd='%s', birth=%i, term=%i, time_adjust=%i, flags=%i, last_logon=%i where name=lower('%s')",
	     multiplay_limit, email.c_str(), passwd.c_str(), birth, term,
	     time_adjust, flags, last_logon, name.c_str());
  }
  assert(account_id);
  return res;
}

