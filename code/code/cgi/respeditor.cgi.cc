#include "database.h"
#include "session.cgi.h"

#include <vector>
#include <map>
#include <list>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPPlainHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/HTTPRedirectHeader.h>

#include <sys/types.h>

using namespace cgicc;


void sendJavaScript();
sstring mudColorToHTML(sstring, bool spacer=true);

void sendResplist(int);
void sendShowResp(int, int, bool);
void saveResp(Cgicc, int);
void makeNewResp(Cgicc, int, bool);
void delResp(Cgicc, int);


bool checkPlayerName(int account_id, sstring name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select 1 from player where lower(name)=lower('%s') and account_id=%i", name.c_str(), account_id);

  if(db.fetchRow())
    return true;
  return false;
}

sstring getPlayerNames(int account_id)
{
  TDatabase db(DB_SNEEZY);
  sstring names;

  db.query("select lower(name) as name from player where account_id=%i",
	   account_id);

  if(db.fetchRow())
    names=format("'%s'") % db["name"];

  while(db.fetchRow()){
    names+=format(", '%s'") % db["name"];
  }

  return names;
}


int main(int argc, char **argv)
{
  // trick the DB code into use prod database
  gamePort=Config::Port::PROD;

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "respeditor.cgi");
    return 0;
  }

  if(!session.hasWizPower(POWER_BUILDER)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Response Editor")) << endl;
    cout << body() << endl;
    cout << "You don't have permission to use this.";
    cout << body() << endl;
    return 0;
  }

    

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    sendResplist(session.getAccountID());
    return 0;
  } else if(**state_form == "delresp"){
    delResp(cgi, session.getAccountID());
    sendResplist(session.getAccountID());
    return 0;    
  } else if(**state_form == "newresp"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Respeditor") << endl;
    cout << head() << body() << endl;

    makeNewResp(cgi, session.getAccountID(), session.hasWizPower(POWER_LOAD));
    sendShowResp(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;    
  } else if(**state_form == "showresp"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Respeditor") << endl;
    cout << head() << body() << endl;
    
    sendShowResp(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "saveresp"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Respeditor") << endl;
    cout << head() << body() << endl;
    
    saveResp(cgi, session.getAccountID());
    sendShowResp(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "logout"){
    session.logout();
    cout << HTTPRedirectHeader("respeditor.cgi").setCookie(session.getCookie());
    cout << endl;
    return 0;
  }
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Respeditor") << endl;
  cout << head() << body() << endl;
  cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
  cout << **state_form << endl;
  cout << body() << endl;
  cout << html() << endl;
  
  return 0;
}

void delResp(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from mobresponses where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());
}


void makeNewResp(Cgicc cgi, int account_id, bool power_load)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db_sneezy(DB_SNEEZY);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  

  db.query("insert into mobresponses (owner, vnum, response) values ('%s', %s, '')",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str());


}


void saveResp(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from mobresponses where owner='%s' and vnum=%s",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str());
  
  db.query("insert into mobresponses (owner, vnum, response) values ('%s', %s, '%s')",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("response"))).c_str());

  cout << "Saved.<br>";
}


void sendShowResp(int account_id, int vnum, bool wizard)
{
  TDatabase db(DB_IMMORTAL);

  assign_item_info();

  db.query("select owner, vnum, response from mobresponses where vnum=%i and owner in (%r)", vnum, getPlayerNames(account_id).c_str());
  db.fetchRow();

  cout << "<form method=post action=respeditor.cgi>" << endl;
  cout << "<table width=100%><tr>";
  cout << "<td align=left><button name=state value=logout type=submit>logout</button></td>";
  cout << "<td align=left><button name=state value=main type=submit>response list</button></td>";
  cout << "<td width=100% align=right><button name=state value=delresp type=submit>delete</button></td>";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"respeditor.cgi\" method=post name=saveresp>" << endl;
  cout << "<input type=hidden name=state value=saveresp>" << endl;

  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";


  cout << "<table border=1>";


  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];


  sstring buf=db["response"];
  while (buf.find("'") != sstring::npos)
    buf.replace(buf.find("'"), 1, "&#39;");

  cout << format("<tr><td>%s</td><td><textarea name='%s' cols=90 rows=30>%s</textarea></td></tr>\n") % "response" % "response" % buf;



  cout << "</table>";

  cout << "<input type=submit value='save changes'>";
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
  
  
}

void sendResplist(int account_id){
  TDatabase db(DB_IMMORTAL);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Respeditor") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=respeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>";

  sstring buildername;

  db.query("select owner, max(vnum)+1 as nvnum from mobresponses where lower(owner) in (%r) group by owner",
	   getPlayerNames(account_id).c_str());
  
  if(db.fetchRow())
    buildername=db["owner"];
  else {
    // no objects yet
    TDatabase db_sneezy(DB_SNEEZY);
    db_sneezy.query("select p.name as name from wizpower w, account a, player p where p.id=w.player_id and p.account_id=a.account_id and a.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(POWER_BUILDER));
    db_sneezy.fetchRow();
    buildername=db_sneezy["name"];
  }

  cout << "<form method=post action=respeditor.cgi>" << endl;
  cout << "<button name=state value=newresp type=submit>new resp</button>";
  cout << "vnum <input type=text name=vnum value=" << db["nvnum"] << ">";
  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "</form>";
  cout << endl;

  cout << "<form action=\"respeditor.cgi\" method=post name=pickresp>" << endl;
  cout << "<input type=hidden name=vnum>" << endl;
  cout << "<input type=hidden name=state>" << endl;

  cout << "<table border=1>";
  cout << "<tr><td>vnum</td></tr>";

  db.query("select vnum, response from mobresponses where lower(owner) in (%r) order by vnum asc", getPlayerNames(account_id).c_str());
  

  while(db.fetchRow()){
    cout << "<tr><td>" << "<a href=javascript:pickresp('" << db["vnum"];
    cout << "','showresp')>" << db["vnum"] << "</a>" << endl;
    cout << "</td>" << endl;
    cout << "</tr>" << endl;

  }

  cout << "</table></form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
}


void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  cout << "function pickresp(vnum, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickresp.state.value = state;" << endl;
  cout << "document.pickresp.vnum.value = vnum;" << endl;
  cout << "document.pickresp.submit();" << endl;
  cout << "}" << endl;

  cout << "-->" << endl;
  cout << "</script>" << endl;


}

// candidate for inclusion in sstring
void replaceString(sstring &str, sstring find, sstring replace)
{
  while(str.find(find)!=sstring::npos){
    str.replace(str.find(find), find.size(), replace);
  }
}

// candidate for some sort of global cgi tools library
sstring mudColorToHTML(sstring str, bool spacer)
{

  replaceString(str, "\n", "<br>");

  replaceString(str, "<f>", "");
  //  replaceString(str, " ", "&nbsp;");
  replaceString(str, "<r>", "</span><span style=\"color:red\">");
  replaceString(str, "<R>", "</span><span style=\"color:red;font-weight:bold\">");

  replaceString(str, "<b>", "</span><span style=\"color:blue\">");
  replaceString(str, "<B>", "</span><span style=\"color:blue;font-weight:bold\">");
  replaceString(str, "<g>", "</span><span style=\"color:green\">");
  replaceString(str, "<G>", "</span><span style=\"color:green;font-weight:bold\">");
  replaceString(str, "<c>", "</span><span style=\"color:cyan\">");
  replaceString(str, "<C>", "</span><span style=\"color:cyan;font-weight:bold\">");
  replaceString(str, "<p>", "</span><span style=\"color:purple\">");
  replaceString(str, "<P>", "</span><span style=\"color:purple;font-weight:bold\">");
  replaceString(str, "<o>", "</span><span style=\"color:orange\">");
  replaceString(str, "<O>", "</span><span style=\"color:orange;font-weight:bold\">");
  replaceString(str, "<y>", "</span><span style=\"color:yellow\">");
  replaceString(str, "<Y>", "</span><span style=\"color:yellow;font-weight:bold\">");
  replaceString(str, "<k>", "</span><span style=\"color:gray\">");
  replaceString(str, "<K>", "</span><span style=\"color:gray;font-weight:bold\">");
  replaceString(str, "<w>", "</span><span style=\"color:white\">");
  replaceString(str, "<W>", "</span><span style=\"color:white;font-weight:bold\">");
  replaceString(str, "<Z>", "</span><span style=\"color:white\">");
  replaceString(str, "<z>", "</span><span style=\"color:white\">");
  replaceString(str, "<1>", "</span><span style=\"color:white\">");

  // to help builders line up text
  sstring spacing_strip="01234567890123456789012345678901234567890123456789012345678901234567890123456789<br>";

  if(!spacer)
    spacing_strip="";

  return format("<span style=\"color:white\"><font face=\"courier\">%s%s</font></span>") % spacing_strip % str;
}

