#include "stdsneezy.h"
#include "database.h"
#include "session.cgi.h"

#include <vector>
#include <map>
#include <list>
#include "sstring.h"
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>

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
sstring mudColorToHTML(sstring, bool spacer=true);

void sendMoblist(int);
void sendShowMob(int, int, bool);
void saveMob(Cgicc, int);
void makeNewMob(Cgicc, int, bool);
void delMob(Cgicc, int);

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
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "mobeditor.cgi");
    return 0;
  }

  if(!session.hasWizPower(POWER_BUILDER)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Mobile Load Logs")) << endl;
    cout << body() << endl;
    cout << "You don't have permission to use this.";
    cout << body() << endl;
    return 0;
  }

    

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    sendMoblist(session.getAccountID());
    return 0;
  } else if(**state_form == "delmob"){
    delMob(cgi, session.getAccountID());
    sendMoblist(session.getAccountID());
    return 0;    
  } else if(**state_form == "newmob"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mobeditor") << endl;
    cout << head() << body() << endl;

    makeNewMob(cgi, session.getAccountID(), session.hasWizPower(POWER_LOAD));
    sendShowMob(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;    
  } else if(**state_form == "showmob"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mobeditor") << endl;
    cout << head() << body() << endl;
    
    sendShowMob(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "savemob"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mobeditor") << endl;
    cout << head() << body() << endl;
    
    saveMob(cgi, session.getAccountID());
    sendShowMob(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "logout"){
    session.logout();
    cout << HTTPRedirectHeader("mobeditor.cgi").setCookie(session.getCookie());
    cout << endl;
    return 0;
  }
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mobeditor") << endl;
  cout << head() << body() << endl;
  cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
  cout << **state_form << endl;
  cout << body() << endl;
  cout << html() << endl;
  
  return 0;
}

void delMob(Cgicc cgi, int account_id)
{
#if 0
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from obj where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from objextra where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from objaffect where vnum=%s and owner='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());
#endif
}

void makeNewMob(Cgicc cgi, int account_id, bool power_load)
{
#if 0
  TDatabase db(DB_IMMORTAL);
  TDatabase db_sneezy(DB_SNEEZY);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db_sneezy.query("select name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material from obj where vnum=%s", (**(cgi.getElement("template"))).c_str());
  db_sneezy.fetchRow();

  db.query("insert into obj (owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material) values ('%s', %s, '%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   db_sneezy["name"].c_str(), 
	   db_sneezy["short_desc"].c_str(), 
	   db_sneezy["long_desc"].c_str(), 
	   db_sneezy["action_desc"].c_str(), 
	   db_sneezy["type"].c_str(), 
	   db_sneezy["action_flag"].c_str(), 
	   db_sneezy["wear_flag"].c_str(), 
	   db_sneezy["val0"].c_str(), 
	   db_sneezy["val1"].c_str(), 
	   db_sneezy["val2"].c_str(), 
	   db_sneezy["val3"].c_str(), 
	   db_sneezy["weight"].c_str(), 
	   db_sneezy["price"].c_str(), 
	   db_sneezy["can_be_seen"].c_str(), 
	   db_sneezy["spec_proc"].c_str(), 
	   db_sneezy["max_exist"].c_str(), 
	   db_sneezy["max_struct"].c_str(), 
	   db_sneezy["cur_struct"].c_str(), 
	   db_sneezy["decay"].c_str(), 
	   db_sneezy["volume"].c_str(), 
	   db_sneezy["material"].c_str());


  db_sneezy.query("select vnum, name, description from objextra where vnum=%s", (**(cgi.getElement("template"))).c_str());

  while(db_sneezy.fetchRow()){
    db.query("insert into objextra (vnum, owner, name, description) values (%s, '%s', '%s', '%s')", 
	     (**(cgi.getElement("vnum"))).c_str(),
	     (**(cgi.getElement("owner"))).c_str(),
	     db_sneezy["name"].c_str(),
	     db_sneezy["description"].c_str());
  }


  db_sneezy.query("select vnum, type, mod1, mod2 from objaffect where vnum=%s", (**(cgi.getElement("template"))).c_str());

  while(db_sneezy.fetchRow()){
    db.query("insert into objaffect (vnum, owner, type, mod1, mod2) values (%s, '%s', %s, %s, %s)", 
	     (**(cgi.getElement("vnum"))).c_str(),
	     (**(cgi.getElement("owner"))).c_str(),
	     db_sneezy["type"].c_str(),
	     db_sneezy["mod1"].c_str(),
	     db_sneezy["mod2"].c_str());
  }

#endif
}


void saveMob(Cgicc cgi, int account_id)
{
#if 0
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  // calculate action_flag value
  int action_flag=0;
  for(int i=0;i<MAX_OBJ_STAT;++i){
    if(cgi.getElement(extra_bits[i]) != cgi.getElements().end()){
      action_flag|=(1<<i);
    }
  }

  // calculate wear_flag value
  int wear_flag=0;
  for(unsigned int i=0;i<MAX_ITEM_WEARS;++i){
    if(cgi.getElement(wear_bits[i]) != cgi.getElements().end()){
      wear_flag|=(1<<i);
    }
  }



  db.query("delete from obj where owner='%s' and vnum=%s",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str());
  
  db.query("insert into obj (owner, vnum, name, short_desc, long_desc, action_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, max_struct, cur_struct, decay, volume, material) values ('%s', %s, '%s', '%s', '%s', '%s', %s, %i, %i, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("short_desc"))).c_str(),
	   (**(cgi.getElement("long_desc"))).c_str(),
	   (**(cgi.getElement("action_desc"))).c_str(),
	   (**(cgi.getElement("type"))).c_str(),
	   action_flag,
	   wear_flag,
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
#endif
}



void sendShowMob(int account_id, int vnum, bool wizard)
{
  TDatabase db_sneezy(DB_SNEEZY);
  db_sneezy.query("select p.name as name from wizpower w, account a, player p where p.id=w.player_id and p.account_id=a.account_id and a.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(POWER_BUILDER));
  db_sneezy.fetchRow();
  sstring buildername=db_sneezy["name"].cap();



  cout << "<form method=post action=mobeditor.cgi>" << endl;
  cout << "<table width=100%><tr>";
  cout << "<td align=left><button name=state value=logout type=submit>logout</button></td>";
  cout << "<td align=left><button name=state value=main type=submit>object list</button></td>";
  cout << "<td align=left><button name=state value=showextra type=submit>edit extras</button></td>";
  cout << "<td align=left><button name=state value=showaffect type=submit>edit affects</button></td>";
  cout << "<td width=100% align=right><button name=state value=delobj type=submit>delete</button></td>";
  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"mobeditor.cgi\" method=post name=saveobj>" << endl;
  cout << "<input type=hidden name=state value=saveobj>" << endl;

  cout << "<input type=hidden name=owner value='" << buildername << "'>";


  cout << "<table border=1>";

  cout << "<textarea name=description cols=90 rows=25>" << endl;

  ifstream fp((fmt("/mud/prod/lib/immortals/%s/mobs/%i") %
	      buildername % vnum).c_str());
  sstring buf;

  while(!fp.eof()){
    getline(fp, buf);
    cout << buf << endl;
  }

  cout << "</textarea>" << endl;

  cout << "</table></table>";
  cout << "<input type=submit value='save changes'>";
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
  
}

void sendMoblist(int account_id){
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Mobeditor") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=mobeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>";

  TDatabase db_sneezy(DB_SNEEZY);
  db_sneezy.query("select p.name as name from wizpower w, account a, player p where p.id=w.player_id and p.account_id=a.account_id and a.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(POWER_BUILDER));
  db_sneezy.fetchRow();
  sstring buildername=db_sneezy["name"].cap();


  cout << "<form method=post action=mobeditor.cgi>" << endl;
  cout << "<button name=state value=newmob type=submit>new mob</button>";
  cout << "vnum <input type=text name=vnum>";
  cout << "template <input type=text name=template value=1>";
  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "</form>";
  cout << endl;

  cout << "<form action=\"mobeditor.cgi\" method=post name=pickmob>" << endl;
  cout << "<input type=hidden name=vnum>" << endl;
  cout << "<input type=hidden name=state>" << endl;
  cout << "<table border=1>";
  cout << "<tr><td>vnum</td><td>name</td><td>short_desc</td></tr>" << endl;

  DIR *dir=opendir((fmt("/mud/prod/lib/immortals/%s/mobs/") % buildername).c_str());
  struct dirent *entry;
  ifstream fp;
  sstring name, short_desc;

  while((entry = readdir(dir))){
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
       (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
      continue;
    
    fp.open((fmt("/mud/prod/lib/immortals/%s/mobs/%s") % buildername % entry->d_name).c_str(), ios::in);

    getline(fp, name); // vnum, don't need
    getline(fp, name);
    getline(fp, short_desc);
    fp.close();

    cout << "<tr><td>" << "<a href=javascript:pickmob('" << entry->d_name;
    cout << "','showmob')>" << entry->d_name << "</a>" << endl;
    cout << "</td><td>" << name << "</td>"<< endl;
    cout << "<td bgcolor=black>" << mudColorToHTML(short_desc, false);
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

  cout << "function pickmob(vnum, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickmob.state.value = state;" << endl;
  cout << "document.pickmob.vnum.value = vnum;" << endl;
  cout << "document.pickmob.submit();" << endl;
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
  replaceString(str, " ", "&nbsp;");
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

  return fmt("<span style=\"color:white\"><font face=\"courier\">%s%s</font></span>") % spacing_strip % str;
}

