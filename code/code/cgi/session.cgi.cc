#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPPlainHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>


#include <sys/types.h>
#include <unistd.h>
#include <openssl/md5.h>

using namespace cgicc;

bool TSession::hasWizPower(wizPowerT wp)
{
  TDatabase db(DB_SNEEZY);
  
  db.query("select 1 from wizpower w, player p where w.player_id=p.id and p.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(wp));

  if(db.fetchRow())
    return true;
  return false;
}


void TSession::sendLoginPage(sstring url)
{
  cgicc::CgiEnvironment env = cgi->getEnvironment();
/* killing this for now. hopefully i can get IE to cooperate with the various tools
  if((env.getUserAgent().find("MSIE")) != string::npos){
    cout << HTTPRedirectHeader("http://www.mozilla.com/en-US/firefox/switch.html");
    cout << endl;
    cout << html() << head() << title("Broken Browser") << endl;
    cout << head() << body() << endl;
    cout << "Microsoft's Internet Explorer does not conform to web standards.  Please use a proper browser.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }
*/
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("SneezyMUD Web Login") << endl;
  cout << head() << body() << endl;

  cout << "<form action=\"" << url << "\" method=post>" << endl;
  cout << "<table>" << endl;
  cout << "<tr><td colspan=2>" << endl;
  cout << "<img src=\"/sneezy_forum_logo.gif\">" << endl;
  cout << "<tr><td>Account</td>" << endl;
  cout << "<td><input type=text name=account></td></tr>" << endl;
  cout << "<tr><td>Password</td>" << endl;
  cout << "<td><input type=password name=passwd></td></tr>" << endl;
  cout << "<tr><td><input type=checkbox name=autologin></td>" << endl;
  cout << "<td>Log me on permanently.</td></tr>" << endl;
  cout << "<tr><td colspan=2><input type=submit name=Login value=Login></td></tr>" <<endl;
  cout << "</table>" << endl;
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}

void TSession::doLogin(Cgicc cgi, sstring url)
{
  form_iterator state_form=cgi.getElement("Login");
  
  if(state_form == cgi.getElements().end()){
    sendLoginPage(url);
  } else {
    sendLoginCheck(cgi, url);
  }
}

// I tried using the cgi that TSession stores, 
// but it is unstable for some reason
void TSession::sendLoginCheck(Cgicc cgi, sstring url)
{
  // validate, create session cookie + db entry, redirect to main
  sstring name=**(cgi.getElement("account"));
  sstring passwd=**(cgi.getElement("passwd"));
  form_iterator autologin=cgi.getElement("autologin");

  if(!checkPasswd(name, passwd)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Login") << endl;
    cout << head() << body() << endl;
    cout << "Password incorrect or account not found.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  if(autologin == cgi.getElements().end()){
    createSession();
  } else {
    // log them in for a year or so
    createSession(60*60*24*365);
  }

  cout << HTTPRedirectHeader(url).setCookie(getCookie());
  cout << endl;
  cout << html() << head() << title("Login") << endl;
  cout << head() << body() << endl;
  cout << "Good job, you logged in.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;
}





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
  sstring crypted=crypt(passwd.c_str(), db["passwd"].c_str());
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

  db.query("delete from cgisession where account_id=%i and name='%s'", 
	   account_id, cookiename.c_str());
  db.query("insert into cgisession values ('%s', %i, %i, %i, '%s')", 
	   session_id.c_str(), account_id, duration, 
	   time(NULL), cookiename.c_str());
}


sstring TSession::generateSessionID()
{
  unsigned char data[16];
  int length=16;
  int seed[4];

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

  sstring md5=(char *)MD5(data, length, NULL);
  sstring ret="";
  char buf[16];

  for(unsigned int i=0;i<md5.length();++i){
    sprintf(buf, "%x", (unsigned char)md5[i]);
    ret+=buf;
  }

  return ret;
}


// this doesn't really belong here
sstring escape_html(sstring content)
{
    unsigned int loc;
    for(loc=content.find("&",0);loc!=sstring::npos;loc=content.find("&",loc+1))
      content.replace(loc, 1, "&amp;");
    for(loc=content.find("<",0);loc!=sstring::npos;loc=content.find("<",loc+1))
      content.replace(loc, 1, "&lt;");
    for(loc=content.find(">",0);loc!=sstring::npos;loc=content.find(">",loc+1))
      content.replace(loc, 1, "&gt;");
    return content;
}
