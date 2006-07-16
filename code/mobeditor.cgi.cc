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


  form_iterator vnum=cgi.getElement("vnum");
  if(vnum != cgi.getElements().end()){
    sstring vnum_buf=**vnum;
    if(vnum_buf.find(".") != string::npos ||
       vnum_buf.find("/") != string::npos ||
       vnum_buf.find("\\") != string::npos){
      cout << HTTPHTMLHeader() << endl;
      cout << html() << head() << title("Mobeditor") << endl;
      cout << head() << body() << endl;
      
      cout << "You have been reported to the authorities." << endl;
      
      cout << body() << endl;
      cout << html() << endl;
      return 0;
    }
  }
    

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    sendMoblist(session.getAccountID());
    return 0;
  } else if(**state_form == "delmob"){
    delMob(cgi, session.getAccountID());
    sendMoblist(session.getAccountID());
    return 0;    
  } else if(**state_form == "newmob"){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mobeditor") << endl;
    cout << head() << body() << endl;

    makeNewMob(cgi, session.getAccountID(), session.hasWizPower(POWER_LOAD));
    sendShowMob(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;    
  } else if(**state_form == "showmob"){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Mobeditor") << endl;
    cout << head() << body() << endl;
    
    sendShowMob(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "savemob"){
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
  TDatabase db_sneezy(DB_SNEEZY);
  db_sneezy.query("select p.name as name from wizpower w, account a, player p where p.id=w.player_id and p.account_id=a.account_id and a.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(POWER_BUILDER));
  db_sneezy.fetchRow();
  sstring buildername=db_sneezy["name"].cap();

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  sstring path=(fmt("/mud/prod/lib/immortals/%s/mobs/%s") % buildername % (**(cgi.getElement("vnum"))));

  unlink(path.c_str());

}

void makeNewMob(Cgicc cgi, int account_id, bool power_load)
{
  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  ofstream fp((fmt("/mud/prod/lib/immortals/%s/mobs/%s") % (**(cgi.getElement("owner"))) % (**(cgi.getElement("vnum")))).c_str());

  if(!fp.is_open()){
    cout << "problems" << endl;
    return;
  }

  fp << "#" << (**(cgi.getElement("vnum"))) << endl;
  fp << "~" << endl;
  fp << "~" << endl;
  fp << "~" << endl;
  fp << "1 0 3 50.0 A 1.1" << endl;
  fp << "4 1 1 28 5 1d5+2" << endl;
  fp << "4 0 1 150 70" << endl;
  fp << "3 14 -25 -6 14 12 -25 13 -25 -24 24 25" << endl;
  fp << "9 9 1 0" << endl;
  fp << "0 0 0 0 0 0 0 0 0 0 0 0 0 0" << endl;
  fp << "0 0 0 0 0 0 0 0 0 0 0 0 0 0" << endl;
  fp << "77 0 0 50" << endl;



  fp.close();

  system((fmt("chmod a+rwx /mud/prod/lib/immortals/%s/mobs/%s") % (**(cgi.getElement("owner"))) % (**(cgi.getElement("vnum")))).c_str());

  cout << "Saved.<br>";
}


void saveMob(Cgicc cgi, int account_id)
{
  TDatabase db_sneezy(DB_SNEEZY);
  db_sneezy.query("select p.name as name from wizpower w, account a, player p where p.id=w.player_id and p.account_id=a.account_id and a.account_id=%i and w.wizpower=%i", account_id, mapWizPowerToFile(POWER_BUILDER));
  db_sneezy.fetchRow();
  sstring buildername=db_sneezy["name"].cap();


  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  ofstream fp((fmt("/mud/prod/lib/immortals/%s/mobs/%s") % buildername % (**(cgi.getElement("vnum")))).c_str());

  fp << (**(cgi.getElement("description"))) << endl;
  fp.close();

  cout << "Saved.<br>";

}


class mobSorter {
public:
  bool operator() (const sstring &, const sstring &) const;
};

bool mobSorter::operator()  (const sstring &x, const sstring &y) const
{
  return convertTo<int>(x) < convertTo<int>(y);
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
  cout << "<td align=left><button name=state value=main type=submit>mobile list</button></td>";
  cout << "<td width=100% align=right><button name=state value=delmob type=submit>delete</button></td>";
  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";
  cout << "<p></form>" << endl;

  cout << "<form action=\"mobeditor.cgi\" method=post name=savemob>" << endl;
  cout << "<input type=hidden name=state value=savemob>" << endl;

  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";


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
  vector <sstring> filelist;

  while((entry = readdir(dir))){
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
       (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
      continue;

    filelist.push_back(entry->d_name);
  }

  sort(filelist.begin(), filelist.end(), mobSorter());

  for(unsigned int i=0;i<filelist.size();++i){
    fp.open((fmt("/mud/prod/lib/immortals/%s/mobs/%s") % buildername % filelist[i]).c_str(), ios::in);

    getline(fp, name); // vnum, don't need
    getline(fp, name);
    getline(fp, short_desc);
    fp.close();

    cout << "<tr><td>" << "<a href=javascript:pickmob('" << filelist[i];
    cout << "','showmob')>" << filelist[i] << "</a>" << endl;
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

