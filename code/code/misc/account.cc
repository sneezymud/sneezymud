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



TAccount::TAccount() :
  status(FALSE),
  birth(time(0)),
  login(0),
  term(TERM_NONE),
  desc(NULL),
  time_adjust(0),
  flags(0),
  account_id(0)
{
  name = "";
  passwd = "";
  email = "";
}

TAccount::TAccount(const TAccount &a) :
  status(a.status),
  birth(a.birth),
  login(a.login),
  term(a.term),
  desc(a.desc),
  time_adjust(a.time_adjust),
  flags(a.flags),
  account_id(a.account_id)
{
  name=a.name;
  passwd=a.passwd;
  email=a.email;
}

TAccount & TAccount::operator=(const TAccount &a)
{
  if (this == &a) return *this;
  name=a.name;
  passwd=a.passwd;
  email=a.email;
  birth = a.birth;
  login = a.login;
  desc = a.desc;
  term = a.term;
  status = a.status;
  time_adjust = a.time_adjust;
  flags = a.flags;
  account_id = a.account_id;
  return *this;
}

TAccount::~TAccount()
{
}

bool TAccount::read(const sstring &aname)
{
  TDatabase db(DB_SNEEZY);
  bool res;

  res=db.query("select account_id, email, passwd, name, birth, term, time_adjust, flags, last_logon from account where name=lower('%s')", aname.c_str());

  db.fetchRow();

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

  return res;
}

bool TAccount::write(const sstring &aname)
{
  TDatabase db(DB_SNEEZY);
  bool res;

  db.query("select 1 from account where name=lower('%s')", aname.c_str());

  if(!db.fetchRow()){
    res=db.query("insert into account (email, passwd, name, birth, term, time_adjust, flags, last_logon) values ('%s', '%s', lower('%s'), %i, %i, %i, %i, %i)",
	     email.c_str(), passwd.c_str(), name.c_str(), birth, term,
	     time_adjust, flags, last_logon);
  } else {
    res=db.query("update account set email='%s', passwd='%s', birth=%i, term=%i, time_adjust=%i, flags=%i, last_logon=%i where name=lower('%s')",
	     email.c_str(), passwd.c_str(), birth, term,
	     time_adjust, flags, last_logon, name.c_str());
  }
  return res;
}

