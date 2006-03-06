#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>

#include <sys/types.h>
#include <md5.h>


void TSession::logout()
{
  TDatabase db(DB_SNEEZY);
  db.query("delete from cgisession where session_id='%s'", 
	   session_id.c_str());
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

  cgi=&c;
  session_id=getSessionCookie();

  if(!session_id.empty()){
    account_id=validateSessionID();
  }
}


int TSession::validateSessionID()
{
  TDatabase db(DB_SNEEZY);

  db.query("select account_id from cgisession where session_id='%s'", 
	   session_id.c_str());

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

void TSession::createSession(int account_id)
{
  session_id=generateSessionID();
  TDatabase db(DB_SNEEZY);

  db.query("delete from cgisession where account_id=%i", account_id);
  db.query("insert into cgisession values ('%s', %i)", 
	   session_id.c_str(), account_id);
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

