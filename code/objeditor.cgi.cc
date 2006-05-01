#include "stdsneezy.h"
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
#include <md5.h>

using namespace cgicc;

void sendJavaScript();

void sendLogin();
void sendLoginCheck(Cgicc cgi, TSession);

void sendObjlist(int);
void sendShowObj(int, int);
void saveObj(Cgicc, int);

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
    names=fmt("'%s'") % db["name"];

  while(db.fetchRow()){
    names+=fmt(", '%s'") % db["name"];
  }

  return names;
}


int main(int argc, char **argv)
{
  // trick the DB code into use prod database
  gamePort=PROD_GAMEPORT;

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "objeditor");

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
      sendObjlist(session.getAccountID());
      return 0;
    } else if(**state_form == "showobj"){
      form_iterator vnum=cgi.getElement("vnum");
      cout << HTTPHTMLHeader() << endl;
      cout << html() << head() << title("Objeditor") << endl;
      cout << head() << body() << endl;

      sendShowObj(session.getAccountID(), convertTo<int>(**vnum));
      return 0;
    } else if(**state_form == "saveobj"){
      form_iterator vnum=cgi.getElement("vnum");
      cout << HTTPHTMLHeader() << endl;
      cout << html() << head() << title("Objeditor") << endl;
      cout << head() << body() << endl;

      saveObj(cgi, session.getAccountID());
      sendShowObj(session.getAccountID(), convertTo<int>(**vnum));
      return 0;
    } else if(**state_form == "logout"){
      session.logout();
      cout << HTTPRedirectHeader("objeditor.cgi").setCookie(session.getCookie());
      cout << endl;
      return 0;
    }

    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Objeditor") << endl;
    cout << head() << body() << endl;
    cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
    cout << **state_form << endl;
    cout << body() << endl;
    cout << html() << endl;

    return 0;
  }  

  // shouldn't get here
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Objeditor") << endl;
  cout << head() << body() << endl;
  cout << "This is bad.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;

}

void saveObj(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }


  db.query("delete from obj where owner='%s' and vnum=%s",
	   (**(cgi.getElement("owner"))).c_str(), 
	   (**(cgi.getElement("vnum"))).c_str());

  
  db.query("insert into obj (owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material) values ('%s', %s, '%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("short_desc"))).c_str(),
	   (**(cgi.getElement("long_desc"))).c_str(),
	   (**(cgi.getElement("action_desc"))).c_str(),
	   (**(cgi.getElement("type"))).c_str(),
	   (**(cgi.getElement("action_flag"))).c_str(),
	   (**(cgi.getElement("wear_flag"))).c_str(),
	   (**(cgi.getElement("val0"))).c_str(),
	   (**(cgi.getElement("val1"))).c_str(),
	   (**(cgi.getElement("val2"))).c_str(),
	   (**(cgi.getElement("val3"))).c_str(),
	   (**(cgi.getElement("weight"))).c_str(),
	   (**(cgi.getElement("price"))).c_str(),
	   (**(cgi.getElement("can_be_seen"))).c_str(),
	   (**(cgi.getElement("spec_proc"))).c_str(),
	   (**(cgi.getElement("max_exist"))).c_str(),
	   (**(cgi.getElement("max_struct"))).c_str(),
	   (**(cgi.getElement("cur_struct"))).c_str(),
	   (**(cgi.getElement("decay"))).c_str(),
	   (**(cgi.getElement("volume"))).c_str(),
	   (**(cgi.getElement("material"))).c_str());

  cout << "Saved.<br>";
}


void sendShowObj(int account_id, int vnum)
{
  TDatabase db(DB_IMMORTAL);

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<button name=state value=main type=submit>go back</button>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"objeditor.cgi\" method=post name=saveobj>" << endl;
  cout << "<input type=hidden name=state value=saveobj>" << endl;

  db.query("select owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material from obj where vnum=%i and owner in (%r)", vnum, getPlayerNames(account_id).c_str());
  db.fetchRow();

  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";


  cout << "<table border=1>";


  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % db["name"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "short_desc" % "short_desc" % db["short_desc"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "long_desc" % "long_desc" % db["long_desc"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "action_desc" % "action_desc" % db["action_desc"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "type" % "type" % db["type"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "action_flag" % "action_flag" % db["action_flag"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "wear_flag" % "wear_flag" % db["wear_flag"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "val0" % "val0" % db["val0"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "val1" % "val1" % db["val1"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "val2" % "val2" % db["val2"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "val3" % "val3" % db["val3"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "weight" % "weight" % db["weight"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "price" % "price" % db["price"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "can_be_seen" % "can_be_seen" % db["can_be_seen"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "spec_proc" % "spec_proc" % db["spec_proc"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "max_exist" % "max_exist" % db["max_exist"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "max_struct" % "max_struct" % db["max_struct"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "cur_struct" % "cur_struct" % db["cur_struct"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "decay" % "decay" % db["decay"];

  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "volume" % "volume" % db["volume"];
  
  cout << fmt("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "material" % "material" % db["material"];
  
  

  cout << "</table>";

  cout << "<input type=submit value='save changes'>";
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
  
  
}

void sendObjlist(int account_id){
  TDatabase db(DB_IMMORTAL);

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Objeditor") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=objeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"objeditor.cgi\" method=post name=pickobj>" << endl;
  cout << "<input type=hidden name=vnum>" << endl;
  cout << "<input type=hidden name=state>" << endl;

  cout << "<table border=1>";
  cout << "<tr><td>vnum</td><td>name</td></tr>";

  db.query("select vnum, name, short_desc from obj where lower(owner) in (%r)", 
	   getPlayerNames(account_id).c_str());

  while(db.fetchRow()){
    cout << "<tr><td>" << "<a href=javascript:pickobj('" << db["vnum"];
    cout << "','showobj')>" << db["vnum"] << "</a>" << endl;
    cout << "</td><td>" << db["name"] << "</td></tr>" << endl;
  }

  cout << "</table></form>" << endl;

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
    cout << html() << head() << title("Objeditor") << endl;
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

  
  cout << HTTPRedirectHeader("objeditor.cgi").setCookie(session.getCookie());
  cout << endl;
  cout << html() << head() << title("Objeditor") << endl;
  cout << head() << body() << endl;
  cout << "Good job, you logged in.<p><hr><p>" << endl;
  cout << body() << endl;
  cout << html() << endl;
}


void sendLogin()
{
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Objeditor") << endl;
  cout << head() << body() << endl;

  cout << "<form action=\"objeditor.cgi\" method=post>" << endl;
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


void sendJavaScript()
{
  cout << "<script language=\"JavaScript\" type=\"text/javascript\">" << endl;
  cout << "<!--" << endl;

  cout << "function pickobj(vnum, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickobj.state.value = state;" << endl;
  cout << "document.pickobj.vnum.value = vnum;" << endl;
  cout << "document.pickobj.submit();" << endl;
  cout << "}" << endl;

  cout << "-->" << endl;
  cout << "</script>" << endl;


}
