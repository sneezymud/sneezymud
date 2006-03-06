#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

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


void sendJavaScript();

void sendLogin();
void sendLoginCheck(Cgicc cgi, TSession);
void sendPickPlayer(int);
void sendMessageList(Cgicc cgi, int);

void deleteMessage(int, int);


int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  toggleInfo.loadToggles();

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "mudmail");

  if(!session.isValid()){
    if(state_form == cgi.getElements().end() ||
       **state_form != "logincheck"){
      sendLogin();
      return 0;
    } else {
      sendLoginCheck(cgi, session);
      return 0;
    }
  } else {
    if(state_form == cgi.getElements().end() || **state_form == "main"){
      sendPickPlayer(session.getAccountID());
      return 0;
    } else if(**state_form == "listmessages"){
      sendMessageList(cgi, session.getAccountID());
      return 0;
    } else if(**state_form == "logout"){
      session.logout();
      cout << HTTPRedirectHeader("mudmail.cgi").setCookie(session.getCookie());
      cout << endl;
      return 0;
    } else if(**state_form == "messageaction"){
      form_iterator delete_form=cgi.getElement("delete");
      
      if(delete_form != cgi.getElements().end()){
	deleteMessage(convertTo<int>(**delete_form), session.getAccountID());
	sendMessageList(cgi, session.getAccountID());
      }
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

void deleteMessage(int mail_id, int account_id)
{
  TDatabase db(DB_SNEEZY);

  // make sure they aren't trying to delete someone elses messages
  db.query("delete from mail where mail.mailid=%i and mail.mailto=player.name and player.account_id=%i", mail_id, account_id);
}

void sendMessageList(Cgicc cgi, int account_id)
{
  int player_id=convertTo<int>(**(cgi.getElement("player")));
  TDatabase db(DB_SNEEZY);
  sstring content;

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;

  db.query("select m.mailid, m.mailfrom, m.timesent, m.content from mail m, player p where m.mailto=p.name and p.id=%i order by m.timesent desc", player_id);

  cout << "<a href=mudmail.cgi>go back</a><br>" << endl;
  cout << "<hr>" << endl;

  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<input type=hidden name=player value=" << player_id << endl;
  cout << "<input type=hidden name=state value=messageaction>" << endl;
  
  while(db.fetchRow()){
    cout << "<table width=50%>";
    cout << "<tr bgcolor=#DDDDFF><td>From:</td><td>" << db["mailfrom"];
    cout << "</td></tr>" << endl;
    cout << "<tr bgcolor=#DDDDFF><td>Date:</td><td>" << db["timesent"];
    cout << "</td></tr>" << endl;
    cout << "<tr bgcolor=#DDDDFF><td colspan=2><pre>" << endl;
    
    content=db["content"];
    unsigned int loc;
    for(loc=content.find("&");loc!=sstring::npos;loc=content.find("&"))
      content.replace(loc, 1, "&amp;");
    for(loc=content.find("<");loc!=sstring::npos;loc=content.find("<"))
      content.replace(loc, 1, "&lt;");
    for(loc=content.find(">");loc!=sstring::npos;loc=content.find(">"))
      content.replace(loc, 1, "&gt;");

    cout << content;


    cout << "</pre></td></tr>" << endl;
    cout << "</table>";
    cout << "<button name=delete value=" << db["mailid"] << " type=submit>";
    cout << "delete</button><hr>";
  }

  cout << "</table>" << endl;
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}

void sendLoginCheck(Cgicc cgi, TSession session)
{
  // validate, create session cookie + db entry, redirect to main
  sstring name=**(cgi.getElement("account"));
  sstring passwd=**(cgi.getElement("passwd"));
  form_iterator autologin=cgi.getElement("autologin");

  if(!session.checkPasswd(name, passwd)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "Password incorrect or account not found.<p><hr><p>" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return;
  }

  if(autologin == cgi.getElements().end()){
    session.createSession();
  } else {
    // log them in for a year or so
    session.createSession(60*60*24*365);
  }

  
  cout << HTTPRedirectHeader("mudmail.cgi").setCookie(session.getCookie());
  cout << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;
  cout << "Good job, you logged in.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;
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
  cout << "<tr><td><input type=checkbox name=autologin></td>" << endl;
  cout << "<td>Log me on automatically each visit.</td></tr>" << endl;
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
  cout << "<a href=\"mudmail.cgi?state=logout\">logout</a><p>" << endl;

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
