#include "stdsneezy.h"
#include "database.h"

#include <map>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>

#include <sys/types.h>
#include <md5.h>

using namespace cgicc;

Cgicc cgi;

sstring getSessionID();
sstring generateSessionID();
int validateSessionID(sstring);

void sendJavaScript();

void sendLogin();
void sendLoginCheck();
void sendPickPlayer(int);
void sendMessageList(int);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  toggleInfo.loadToggles();
  form_iterator state_form=cgi.getElement("state");

  sstring session_id=getSessionID();
  int account_id=validateSessionID(session_id);

  // this if statement sucks
  if(session_id.empty() || account_id < 0){
    if(state_form == cgi.getElements().end() ||
       **state_form != "logincheck"){
      sendLogin();
      return 0;
    } else {
      sendLoginCheck();
      return 0;
    }
  } else {
    if(state_form == cgi.getElements().end() || **state_form == "main"){
      sendPickPlayer(account_id);
      return 0;
    } else if(**state_form == "listmessages"){
      sendMessageList(account_id);
      return 0;
    } else if(**state_form == "logout"){
      TDatabase db(DB_SNEEZY);
      db.query("delete from cgisession where session_id='%s'", 
	       session_id.c_str());
      sendLogin();
      return 0;
    }

    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "Fell through state switch.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;

    return 0;
  }  

  // shouldn't get here
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;
  cout << "This is bad.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;

}

void sendMessageList(int account_id)
{
  int player_id=convertTo<int>(**(cgi.getElement("player")));
  TDatabase db(DB_SNEEZY);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;

  db.query("select m.mailfrom, m.timesent, m.content from mail m, player p where m.mailto=p.name and p.id=%i", player_id);
  
  cout << "<table border=1>" << endl;

  while(db.fetchRow()){
    cout << "<tr><td>" << db["mailfrom"] << endl;
    cout << "</td><td>" << db["timesent"] << endl;
    cout << "</td></tr><tr><td colspan=2>" << db["content"] << endl;
    cout << "</td></tr>" << endl;
  }

  cout << "</table>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}

int validateSessionID(sstring session_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("select account_id from cgisession where session_id='%s'", 
	   session_id.c_str());

  if(!db.fetchRow())
    return -1;

  return convertTo<int>(db["account_id"]);
}

// gets the value of the "mudmail" cookie and returns it
sstring getSessionID()
{
  CgiEnvironment env = cgi.getEnvironment();
  vector<HTTPCookie> cookies = env.getCookieList();

  if(!cookies.size())
    return "";
  
  for(unsigned int i=0;i<cookies.size();++i){
    if(cookies[i].getName() == "mudmail")
      return cookies[i].getValue();
  }
  return "";
}

void sendLoginCheck()
{
  // validate, create session cookie + db entry, redirect to main
  TDatabase db(DB_SNEEZY);
  sstring name=**(cgi.getElement("account"));
  sstring passwd=**(cgi.getElement("passwd"));
  
  //// make sure the account exists, and pull out the encrypted password
  db.query("select account_id, passwd from account where name='%s'", 
	   name.c_str());
  
  if(!db.fetchRow()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "Account not found.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }
  
  //// validate password
  sstring crypted=crypt(passwd.c_str(), name.c_str());
  
  // for some retarded reason the mud truncates the crypted portion to 10 chars
  if(crypted.substr(0,10) != db["passwd"]){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "Password incorrect.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  // ok, login is good, create session db entry and cookie
  sstring session_id=generateSessionID();
  int account_id=convertTo<int>(db["account_id"]);

  db.query("delete from cgisession where account_id=%i", account_id);
  db.query("insert into cgisession values ('%s', %i)", 
	   session_id.c_str(), account_id);

  cout << HTTPRedirectHeader("mudmail.cgi").setCookie(HTTPCookie("mudmail",session_id.c_str()));
  cout << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;
  cout << "Good job, you logged in.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;
}

sstring generateSessionID()
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


void sendLogin()
{
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;

  cout << "<form action=\"mudmail.cgi\" method=post>" << endl;
  cout << "<table>" << endl;
  cout << "<tr><td>Account</td>" << endl;
  cout << "<td><input type=text name=account></td></tr>" << endl;
  cout << "<tr><td>Password</td>" << endl;
  cout << "<td><input type=password name=passwd></td></tr>" << endl;
  cout << "<tr><td colspan=2><input type=submit value=Login></td></tr>" <<endl;
  cout << "</table>" << endl;
  cout << "<input type=hidden name=state value=logincheck>" << endl;
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}



void sendPickPlayer(int account_id)
{
  TDatabase db(DB_SNEEZY);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  sendJavaScript();
  cout << head() << body() << endl;
  cout << "<a href=\"mudmail.cgi?state=logout\">Logout</a><p>" << endl;

  // get a list of players in this account
  db.query("select p.id, p.name, count(m.*) as count from player p left outer join mail m on (p.name=m.mailto) where account_id=%i group by p.id, p.name order by p.name",
	   account_id);

  if(!db.fetchRow()){
    cout << "No players were found in this account." << endl;
    return;
  }

  cout << "<form action=\"mudmail.cgi\" method=post name=pickplayer>" << endl;
  cout << "<input type=hidden name=player>" << endl;
  cout << "<input type=hidden name=state value=listmessages>" << endl;
  cout << "<table border=1>" << endl;
  cout << "<tr><td>Player</td><td># of Letters</td></tr>";

  do {
    cout << "<tr><td>" << endl;
    cout << "<a href=javascript:pickplayer('" << db["id"] << "')>";
    cout << db["name"] << "</a></td><td align=center>" << endl;
    cout << db["count"] << "</td></tr>";
  } while(db.fetchRow());

  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}

void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  // this function is for making links emulate submits in player selection
  cout << "function pickplayer(player)" << endl;
  cout << "{" << endl;
  cout << "document.pickplayer.player.value = player;" << endl;
  cout << "document.pickplayer.submit();" << endl;
  cout << "}" << endl;
  cout << "-->" << endl;
  cout << "</script>" << endl;
}

