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

using namespace cgicc;


void sendJavaScript();

void sendPickPlayer(int);
void sendMessageList(Cgicc cgi, int);
void sendComposeMail(sstring, sstring, sstring);

void deleteMessage(int, int);
void sendMail(Cgicc cgi, TSession);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  toggleInfo.loadToggles();

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "mudmail.cgi");
    return 0;
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
      form_iterator reply_form=cgi.getElement("reply");
      
      if(delete_form != cgi.getElements().end()){
	deleteMessage(convertTo<int>(**delete_form), session.getAccountID());
	sendMessageList(cgi, session.getAccountID());
      } else if(reply_form != cgi.getElements().end()){
	form_iterator from=cgi.getElement("fromplayer");

	TDatabase db(DB_SNEEZY);
	db.query("select mailfrom, content from mail m, player p where m.mailid=%s and m.mailto=p.name and p.account_id=%i",
		 (**reply_form).c_str(), session.getAccountID());
	if(db.fetchRow()){
	  sstring content="> " + db["content"];
	  for(unsigned int loc=content.find("\n",0);
	      loc!=sstring::npos;
	      loc=content.find("\n",loc+1))
	    content.replace(loc, 1, "\n> ");

	  sendComposeMail(**from, db["mailfrom"], content);
	} else {
	  cout << "secret pwipe code received. full pwipe initiating in 3.. 2.. 1..";
	}
      }
      return 0;
    } else if(**state_form == "compose"){
      form_iterator player_form=cgi.getElement("playername");
      form_iterator from=cgi.getElement("fromplayer");
      sstring send_to, send_from;

      if(player_form == cgi.getElements().end())
	send_to="";
      else
	send_to=**player_form;

      if(from == cgi.getElements().end())
	send_from="";
      else
	send_from=**from;
      
      sendComposeMail(send_from, send_to, "");
      return 0;
    } else if(**state_form == "sendmail"){
      sendMail(cgi, session);
      return 0;
    }

    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
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

void sendMail(Cgicc cgi, TSession session)
{
  TDatabase db(DB_SNEEZY);
  form_iterator player_name=cgi.getElement("playername");
  form_iterator content=cgi.getElement("content");
  form_iterator from=cgi.getElement("fromname");

  db.query("select p.id from player p, account a where a.account_id=%i and p.name='%s' and p.account_id=a.account_id", session.getAccountID(), (**from).c_str());

  if(!db.fetchRow()){
    cout << "secret pwipe code received. full pwipe initiating in 3.. 2.. 1..";
    return;
  }

  store_mail((**player_name).c_str(), (**from).c_str(), (**content).c_str());


  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;
  cout << "Mail sent." << endl;
  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<input type=hidden name=state value=listmessages>" << endl;
  cout << "<input type=hidden name=player value=" << db["id"] << ">";
  cout << "<button type=submit>go back</button></form>";
  cout << body() << endl;
  cout << html() << endl;

}


void sendComposeMail(sstring from, sstring to, sstring content)
{
  TDatabase db(DB_SNEEZY);


  if(to.empty()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "You need to enter a player name to send mail to." << endl;
    cout << body() << endl;
    cout << html() << endl;    
    return;
  }

  db.query("select name from player where name like lower('%s')",
	   to.c_str());

  if(!db.fetchRow()){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mudmail") << endl;
    cout << head() << body() << endl;
    cout << "No such player to mail to!" << endl;
    cout << body() << endl;
    cout << html() << endl;    
    return;
  }

  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;
  
  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<input type=hidden name=state value=sendmail>" << endl;
  cout << "<input type=hidden name=playername value=" << db["name"] << ">";
  cout << "<input type=hidden name=fromname value=" << from << ">";
  cout << "<table width=50%>" << endl;
  cout << "<tr bgcolor=#DDDDFF><td>To:</td><td>" << db["name"];
  cout << "</td></tr>" << endl;
  cout << "<tr bgcolor=#DDDDFF><td>From:</td><td>" << from;
  cout << "</td></tr>" << endl;
  cout << "<tr bgcolor=#DDDDFF><td colspan=2>" << endl;
  cout << "<textarea name=content rows=20 cols=75>" << content <<"</textarea>";
  cout << "</td></tr>" << endl;
  cout << "</table>";
  cout << "<button name=send type=submit>";
  cout << "send</button>";

  
  cout << "</form>";
  
  cout << body() << endl;
  cout << html() << endl;
}


void deleteMessage(int mail_id, int account_id)
{
  TDatabase db(DB_SNEEZY);

  // make sure they aren't trying to delete someone elses messages
  db.query("delete mail from mail, player where mail.mailid=%i and mail.mailto=player.name and player.account_id=%i", mail_id, account_id);
}

void sendMessageList(Cgicc cgi, int account_id)
{
  int player_id=convertTo<int>(**(cgi.getElement("player")));
  TDatabase db(DB_SNEEZY);
  sstring content;

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << head() << body() << endl;


  db.query("select name from player where id=%i", player_id);
  db.fetchRow();
  sstring fromplayer=db["name"];

  db.query("select m.mailid, m.mailfrom, m.timesent, m.content from mail m, player p where m.mailto=p.name and p.id=%i order by m.timesent desc", player_id);

  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<input type=hidden name=fromplayer value=" << fromplayer << ">";
  cout << "<button type=submit>go back</button>" << endl;
  cout << "<button type=submit name=state value=compose>" << endl;
  cout << "compose mail:</button>";
  cout << "<input type=text name=playername>";
  cout << "</form>";
  cout << "<hr>" << endl;

  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<input type=hidden name=player value=" << player_id << endl;
  cout << "<input type=hidden name=fromplayer value=" << fromplayer << ">";
  cout << "<input type=hidden name=state value=messageaction>" << endl;
  
  while(db.fetchRow()){
    cout << "<table width=50%>";
    cout << "<tr bgcolor=#DDDDFF><td>From:</td><td>" << db["mailfrom"];
    cout << "</td></tr>" << endl;
    cout << "<tr bgcolor=#DDDDFF><td>Date:</td><td>" << db["timesent"];
    cout << "</td></tr>" << endl;
    cout << "<tr bgcolor=#DDDDFF><td colspan=2><pre>" << endl;
    
    content=escape_html(db["content"]);
    cout << content;


    cout << "</pre></td></tr>" << endl;
    cout << "</table>";
    cout << "<button name=delete value=" << db["mailid"] << " type=submit>";
    cout << "delete</button>";
    cout << "<button name=reply value=" << db["mailid"] << " type=submit>";
    cout << "reply</button>";
    cout << "<hr>";
  }

  cout << "</table>" << endl;
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}


void sendPickPlayer(int account_id)
{
  TDatabase db(DB_SNEEZY);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mudmail") << endl;
  cout << "<meta http-equiv=refresh content=120>" << endl;
  sendJavaScript();
  cout << head() << body() << endl;
  
  cout << "<form method=post action=mudmail.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>" << endl;

  // get a list of players in this account
  db.query("select p.id, p.name, count(m.mailid) as count from player p left outer join mail m on (p.name=m.mailto) where account_id=%i group by p.id, p.name order by p.name",
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
