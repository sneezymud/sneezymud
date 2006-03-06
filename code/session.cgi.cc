#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>

#include <sys/types.h>
#include <md5.h>

cgicc::HTTPCookie TSession::getCookie()
{
  cgicc::HTTPCookie cookie(cookiename, getSessionID());
  if(cookieduration>=0)
    cookie.setMaxAge(cookieduration);
  return cookie;
}

bool TSession::checkPasswd(sstring name, sstring passwd)
{
  TDatabase db(DB_SNEEZY);

  db.query("select account_id, passwd from account where name='%s'", 
	   name.c_str());

  // account not found.  I debated whether or not we should let users know
  // about this and decided it's bad to let them figure out what account
  // names exist.
  if(!db.fetchRow())
    return false;

  // get the encrypted form.
  sstring crypted=crypt(passwd.c_str(), name.c_str());
  // sneezy truncates the encrypted password for some reason
  crypted=crypted.substr(0,10);

  // bad password
  if(crypted != db["passwd"])
    return false;

  account_id=convertTo<int>(db["account_id"]);
  if(!account_id){
    account_id=-1;
    return -1;
  }
  

  return true;
}


void TSession::logout()
{
  TDatabase db(DB_SNEEZY);
  db.query("delete from cgisession where session_id='%s'", 
	   session_id.c_str());
  cookieduration=0;
}

bool TSession::isValid()
{
  if(session_id.empty() || account_id < 0){
    return false;
  }
  return true;
}

TSession::TSession(cgicc::Cgicc c, sstring cn)
{
  session_id="";
  account_id=-1;
  cookiename=cn;
  cookieduration=0;

  cgi=&c;
  session_id=getSessionCookie();

  if(!session_id.empty()){
    account_id=validateSessionID();
  }
}


int TSession::validateSessionID()
{
  TDatabase db(DB_SNEEZY);

  db.query("select account_id from cgisession where session_id='%s' and (timeset+duration) > %i", 
	   session_id.c_str(), time(NULL));

  if(!db.fetchRow())
    return -1;

  int a=convertTo<int>(db["account_id"]);

  return (a ? a : -1);
}

// gets the value of the "mudmail" cookie and returns it
sstring TSession::getSessionCookie()
{
  cgicc::CgiEnvironment env = cgi->getEnvironment();
  vector<cgicc::HTTPCookie> cookies = env.getCookieList();

  if(!cookies.size())
    return "";
  
  for(unsigned int i=0;i<cookies.size();++i){
    if(cookies[i].getName() == cookiename)
      return cookies[i].getValue();
  }
  return "";
}

void TSession::createSession()
{
  createSession(60*60);
  cookieduration=-1;
}


void TSession::createSession(int duration)
{
  cookieduration=duration;
  TDatabase db(DB_SNEEZY);

  do {
    session_id=generateSessionID();
    db.query("select 1 from cgisession where session_id='%s'",
	     session_id.c_str());
  } while(db.fetchRow());

  db.query("delete from cgisession where account_id=%i", account_id);
  db.query("insert into cgisession values ('%s', %i, %i, %i)", 
	   session_id.c_str(), account_id, duration, time(NULL));
}


sstring TSession::generateSessionID()
{
  unsigned char data[16];
  int length=16;
  int seed[4];

  srandomdev();

  seed[0]=time(NULL);
  seed[1]=random();
  seed[2]=getpid();
  seed[3]=(int)&seed;

  int c=0;
  for(int i=0;i<4;++i){
    for(int j=0;j<4;++j){
      data[c++]=(&seed[i])[j];
    }
  }

  return MD5Data(data, length, NULL);
}

