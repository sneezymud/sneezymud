#include "extern.h"
#include "database.h"
#include "session.cgi.h"
#include "spec_rooms.h"

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

void sendRoomlist(int);
void sendShowRoom(int, int, bool);
void saveRoom(Cgicc, int);
void makeNewRoom(Cgicc, int, sstring, sstring);
void delRoom(Cgicc, int);

void sendShowExtra(int, int);
void saveExtra(Cgicc, int);
void makeNewExtra(Cgicc, int);
void delExtra(Cgicc, int);

void sendShowExit(int, int);
void saveExit(Cgicc, int);
void makeNewExit(Cgicc, int);
void delExit(Cgicc, int);


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

sstring getNextVnum(int account_id)
{
  TDatabase db(DB_IMMORTAL);

  db.query("select max(vnum)+1 as nvnum from room where block=1 and lower(owner) in (%r) group by owner",
	   getPlayerNames(account_id).c_str());
  db.fetchRow();

  return db["nvnum"];
}


int main(int argc, char **argv)
{
  // trick the DB code into use prod database
  gamePort=Config::Port::PROD;

  Cgicc cgi;
  form_iterator state_form=cgi.getElement("state");
  TSession session(cgi, "SneezyMUD");

  if(!session.isValid()){
    session.doLogin(cgi, "roomeditor.cgi");
    return 0;
  }

  if(!session.hasWizPower(POWER_BUILDER)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("SneezyMUD Room Editor")) << endl;
    cout << body() << endl;
    cout << "You don't have permission to use this.";
    cout << body() << endl;
    return 0;
  }

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    sendRoomlist(session.getAccountID());
    return 0;
  } else if(**state_form == "delroom"){
    delRoom(cgi, session.getAccountID());
    sendRoomlist(session.getAccountID());
    return 0;    
  } else if(**state_form == "newroom"){
    form_iterator vnum=cgi.getElement("vnum");
    form_iterator templatevnum=cgi.getElement("template");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    makeNewRoom(cgi, session.getAccountID(), (**vnum), (**templatevnum));
    sendShowRoom(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;    
  } else if(**state_form == "showroom"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;
    
    sendShowRoom(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "delextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    delExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "newextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    makeNewExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "showextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;
    
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "saveextra"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    saveExtra(cgi, session.getAccountID());
    sendShowExtra(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "delexit"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    delExit(cgi, session.getAccountID());
    sendShowExit(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "newexit"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    makeNewExit(cgi, session.getAccountID());
    sendShowExit(session.getAccountID(), convertTo<int>(**vnum));
    return 0;    
  } else if(**state_form == "showexit"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;
    
    sendShowExit(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "saveexit"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;

    saveExit(cgi, session.getAccountID());
    sendShowExit(session.getAccountID(), convertTo<int>(**vnum));
    return 0;
  } else if(**state_form == "saveroom"){
    form_iterator vnum=cgi.getElement("vnum");
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("SneezyMUD Room Editor") << endl;
    cout << head() << body() << endl;
    
    saveRoom(cgi, session.getAccountID());
    sendShowRoom(session.getAccountID(), convertTo<int>(**vnum),
		session.hasWizPower(POWER_WIZARD));
    return 0;
  } else if(**state_form == "logout"){
    session.logout();
    cout << HTTPRedirectHeader("roomeditor.cgi").setCookie(session.getCookie());
    cout << endl;
    return 0;
  }
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("SneezyMUD Room Editor") << endl;
  cout << head() << body() << endl;
  cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
  cout << **state_form << endl;
  cout << body() << endl;
  cout << html() << endl;
  
  return 0;
}

void delRoom(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from room where vnum=%s and owner='%s' and block=1",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from roomextra where vnum=%s and owner='%s' and block=1",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

  db.query("delete from roomexit where vnum=%s and owner='%s' and block=1",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str());

}

void delExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from roomextra where vnum=%s and owner='%s' and block=1 and name='%s'",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
           (**(cgi.getElement("name"))).c_str());
}


void delExit(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  db.query("delete from roomexit where vnum=%s and owner='%s' and block=1 and direction=%s",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
           (**(cgi.getElement("direction"))).c_str());
}


void makeNewExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db.query("insert into roomextra (vnum, owner, block, name, description) values (%s, '%s', 1, '%s', '')",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("name"))).c_str());

}


void makeNewExit(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db.query("insert into roomexit (vnum, owner, block, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination) values (%s, '%s', 1, %s, '', '', 0, 0, -1, -1, -1, 0)",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("direction"))).c_str());
}


void makeNewRoom(Cgicc cgi, int account_id, sstring vnum, sstring templatevnum)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db_sneezy(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  if(templatevnum.empty() ||
     templatevnum == "0"){
    db_sneezy.setDB(DB_SNEEZY);
    db_sneezy.query("select vnum, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec from room where vnum=0");
  } else {
    db_sneezy.query("select vnum, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec from room where vnum=%s and block=1 and owner in (%r)", templatevnum.c_str(), getPlayerNames(account_id).c_str());
  }
  db_sneezy.fetchRow();
    
  db.query("insert into room (owner, vnum, block, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec) values ('%s', %s, 1, %s, %s, %s, '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   vnum.c_str(),
	   db_sneezy["x"].c_str(), 
	   db_sneezy["y"].c_str(), 
	   db_sneezy["z"].c_str(), 
	   db_sneezy["name"].c_str(), 
	   db_sneezy["description"].c_str(), 
	   db_sneezy["room_flag"].c_str(), 
	   db_sneezy["sector"].c_str(), 
	   db_sneezy["teletime"].c_str(), 
	   db_sneezy["teletarg"].c_str(), 
	   db_sneezy["telelook"].c_str(), 
	   db_sneezy["river_speed"].c_str(), 
	   db_sneezy["river_dir"].c_str(), 
	   db_sneezy["capacity"].c_str(), 
	   db_sneezy["height"].c_str(),
	   db_sneezy["spec"].c_str());

  if(templatevnum.empty()){
    db_sneezy.query("select vnum, name, description from roomextra where vnum=0");
  } else {
    db_sneezy.query("select vnum, name, description from roomextra where vnum=%s and block=1 and owner in (%r)", templatevnum.c_str(), getPlayerNames(account_id).c_str());
  }

  while(db_sneezy.fetchRow()){
    db.query("insert into roomextra (vnum, owner, block, name, description) values (%s, '%s', 1, '%s', '%s')", 
	     vnum.c_str(),
	     (**(cgi.getElement("owner"))).c_str(),
	     db_sneezy["name"].c_str(),
	     db_sneezy["description"].c_str());
  }
}

void saveExtra(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }
  
  db.query("delete from roomextra where owner='%s' and vnum=%s and block=1 and name='%s'",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("name"))).c_str());

  db.query("insert into roomextra (vnum, owner, block, name, description) values (%s, '%s', 1, '%s', '%s')",
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("description"))).c_str());
  
  cout << "Saved for keyword " << (**(cgi.getElement("name"))) << ".<br>";
}

void saveExit(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);
  sstring destination=(**(cgi.getElement("destination")));

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  if((**(cgi.getElement("destination"))) == "new"){
    destination=getNextVnum(account_id);
    makeNewRoom(cgi, account_id, destination, 
		(**(cgi.getElement("vnum"))));
    cout << "Created new destination room " << destination << ".<br>";
  }


  
  db.query("delete from roomexit where owner='%s' and vnum=%s and block=1 and direction=%s",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("direction"))).c_str());
  
  db.query("insert into roomexit (vnum, owner, block, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination) values (%s, '%s', 1, %s, '%s', '%s', %s, %s, %s, %s, %s, %s)", 
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("direction"))).c_str(),
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("description"))).c_str(),
	   (**(cgi.getElement("type"))).c_str(),
	   (**(cgi.getElement("condition_flag"))).c_str(),
	   (**(cgi.getElement("lock_difficulty"))).c_str(),
	   (**(cgi.getElement("weight"))).c_str(),
	   (**(cgi.getElement("key_num"))).c_str(),
	   destination.c_str());

  cout << "Saved for direction " << (**(cgi.getElement("direction")))<<".<br>";
  
  
  ///
  db.query("delete from roomexit where owner='%s' and vnum=%s and block=1 and direction=%i",
  	   (**(cgi.getElement("owner"))).c_str(), 
	   destination.c_str(),
	   rev_dir[convertTo<int>((**(cgi.getElement("direction"))))]);

  db.query("insert into roomexit (vnum, owner, block, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination) values (%s, '%s', 1, %i, '%s', '%s', %s, %s, %s, %s, %s, %s)", 
	   destination.c_str(),
	   (**(cgi.getElement("owner"))).c_str(),
	   rev_dir[convertTo<int>((**(cgi.getElement("direction"))))],
	   (**(cgi.getElement("name"))).c_str(),
	   (**(cgi.getElement("description"))).c_str(),
	   (**(cgi.getElement("type"))).c_str(),
	   (**(cgi.getElement("condition_flag"))).c_str(),
	   (**(cgi.getElement("lock_difficulty"))).c_str(),
	   (**(cgi.getElement("weight"))).c_str(),
	   (**(cgi.getElement("key_num"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str());


  cout << "Created opposite exit in " << destination <<".<br>";
}


void saveRoom(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  if(!checkPlayerName(account_id, **(cgi.getElement("owner")))){
    cout << "Owner name didn't match - security violation.";
    return;
  }

  // calculate room_flag value
  int room_flag=0;
  for(int i=0;i<MAX_ROOM_BITS;++i){
    if(cgi.getElement(room_bits[i]) != cgi.getElements().end()){
      room_flag|=(1<<i);
    }
  }

  int terrain=convertTo<int>(**(cgi.getElement("sector")));
  int terrain_file=mapSectorToFile((sectorTypeT)terrain);

  db.query("delete from room where owner='%s' and vnum=%s and block=1",
  	   (**(cgi.getElement("owner"))).c_str(), 
  	   (**(cgi.getElement("vnum"))).c_str());


  db.query("insert into room (owner, vnum, block, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec) values ('%s', %s, 1, %s, %s, %s, '%s', '%s', %i, %i, %s, %s, %s, %s, %s, %s, %s, %s)",
	   (**(cgi.getElement("owner"))).c_str(),
	   (**(cgi.getElement("vnum"))).c_str(),
	   (**(cgi.getElement("x"))).c_str(), 
	   (**(cgi.getElement("y"))).c_str(), 
	   (**(cgi.getElement("z"))).c_str(), 
	   (**(cgi.getElement("name"))).c_str(), 
	   (**(cgi.getElement("description"))).c_str(), 
	   room_flag,
	   terrain_file,
	   (**(cgi.getElement("teletime"))).c_str(), 
	   (**(cgi.getElement("teletarg"))).c_str(), 
	   (**(cgi.getElement("telelook"))).c_str(), 
	   (**(cgi.getElement("river_speed"))).c_str(), 
	   (**(cgi.getElement("river_dir"))).c_str(), 
	   (**(cgi.getElement("capacity"))).c_str(), 
	   (**(cgi.getElement("height"))).c_str(),
	   (**(cgi.getElement("spec"))).c_str());


  cout << "Saved.<br>";
}

void sendShowExtra(int account_id, int vnum)
{
  TDatabase db(DB_IMMORTAL);

  db.query("select owner from room where lower(owner) in (%r) and vnum=%i and block=1 group by owner",
	   getPlayerNames(account_id).c_str(), vnum);
  db.fetchRow();


  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<button name=state value=main type=submit>room list</button>";
  cout << "<button name=state value=showroom type=submit>edit room</button>";
  cout << "<button name=state value=showexit type=submit>edit exits</button>";
  cout << "<input type=hidden name=vnum value=" << vnum << ">";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "<p></form>" << endl;

  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=newextra type=submit>new extra</button>";
  cout << "<input type=text name=name>";
  cout << "<input type=hidden name=vnum value=" << vnum << ">";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "</form>";
  cout << endl;
  

  db.query("select owner, vnum, name, description from roomextra where vnum=%i and block=1 and owner in (%r) order by name", vnum, getPlayerNames(account_id).c_str());

  while(db.fetchRow()){
    cout << "<form action=\"roomeditor.cgi\" method=post name=saveextra>" << endl;
    cout << "<input type=hidden name=state value=saveextra>" << endl;

    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "<table border=1>";

    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];
    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % db["name"];
    cout << format("<tr><td>%s</td><td><textarea name=description cols=90 rows=5>%s</textarea></td></tr>\n") % "description" % db["description"];

    cout << format("<tr><td></td><td bgcolor=black>%s</td></tr>\n") %
      mudColorToHTML(db["description"]);
    
    cout << "</table>";    
    cout << "<table width=100%><tr><td align left>";
    cout << "<input type=submit value='save changes'>";
    cout << "</form></td><td width=100% align=right></td><td>";

    cout << "<form method=post action=roomeditor.cgi>";
    cout << "<button name=state value=delextra type=submit>delete</button>";
    cout << "<input type=hidden name=name value='" << db["name"] << "'>";
    cout << "<input type=hidden name=vnum value=" << vnum << ">";
    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "</form></td></tr></table>";
    

    cout << "<hr>";
  }


  cout << body() << endl;
  cout << html() << endl;

}

sstring getDirectionForm(int selected)
{
  sstring buf="<tr><td>direction</td><td><select name=direction>\n";
  for(int i=0;i<MAX_DIR;++i){
    buf+=format("<option value=%i %s>%s</option>\n") %
      i % ((i==selected)?"selected":"") % dirs[i];
  }
  buf+="</select>\n";

  return buf;
}

sstring getTypeForm(int selected)
{
  sstring buf="<tr><td>type</td><td><select name=type>\n";
  for(int i=0;i<MAX_DOOR_TYPES;++i){
    buf+=format("<option value=%i %s>%s</option>\n") %
      i % ((i==selected)?"selected":"") % door_types[i];
  }
  buf+="</select>\n";

  return buf;
}

sstring getDestinationForm(int selected, int account_id)
{
  TDatabase db(DB_IMMORTAL);

  db.query("select vnum, name from room where block=1 and owner in (%r)", getPlayerNames(account_id).c_str());

  sstring buf="<tr><td>destination</td><td><select name=destination>\n";
  buf+="<option bgcolor=black value=new>New Room</option>\n";
  while(db.fetchRow()){
    buf+=format("<option bgcolor=black value=%s %s>%s - %s</option>\n") %
       db["vnum"] % ((convertTo<int>(db["vnum"])==selected)?"selected":"") % 
      mudColorToHTML(db["name"], false) % db["vnum"];
  }
  buf+="</select>\n";

  return buf;
}


void sendShowExit(int account_id, int vnum)
{
  TDatabase db(DB_IMMORTAL);

  db.query("select owner from room where lower(owner) in (%r) and block=1 and vnum=%i group by owner",
	   getPlayerNames(account_id).c_str(), vnum);
  db.fetchRow();

  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<button name=state value=main type=submit>room list</button>";
  cout << "<button name=state value=showroom type=submit>edit room</button>";
  cout << "<button name=state value=showextra type=submit>edit extras</button>";
  cout << "<input type=hidden name=vnum value=" << vnum << ">";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "<p></form>" << endl;


  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=newexit type=submit>new exit</button>";
  cout << getDirectionForm(0);
  cout << "<input type=hidden name=vnum value=" << vnum << ">";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "</form>";
  cout << endl;
  


  db.query("select owner, vnum, direction, name, description, type, condition_flag, lock_difficulty, weight, key_num, destination from roomexit where vnum=%i and block=1 and owner in (%r) order by type", vnum, getPlayerNames(account_id).c_str());

  while(db.fetchRow()){
    cout << "<form action=\"roomeditor.cgi\" method=post name=saveexit>" << endl;
    cout << "<input type=hidden name=state value=saveexit>" << endl;

    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "<table border=1>";

    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];

    cout << getDirectionForm(convertTo<int>(db["direction"]));

  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % db["name"];
  cout << format("<tr><td></td><td bgcolor=black>%s</td></tr>\n") % 
    mudColorToHTML(db["name"]);

  cout << format("<tr><td>%s</td><td><textarea name=description cols=90 rows=5>%s</textarea></td></tr>\n") % "description" % db["description"];
  cout << format("<tr><td></td><td bgcolor=black>%s</td></tr>\n") %
    mudColorToHTML(db["description"]);

    cout << getTypeForm(convertTo<int>(db["type"]));
    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "condition_flag" % "condition_flag" % db["condition_flag"];
    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "lock_difficulty" % "lock_difficulty" % db["lock_difficulty"];
    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "weight" % "weight" % db["weight"];
    cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "key_num" % "key_num" % db["key_num"];


    cout << getDestinationForm(convertTo<int>(db["destination"]), account_id);
    
    cout << "</table>";    
    cout << "<table width=100%><tr><td align left>";
    cout << "<input type=submit value='save changes'>";
    cout << "</form></td><td width=100% align=right></td><td>";

    cout << "<form method=post action=roomeditor.cgi>";
    cout << "<button name=state value=delexit type=submit>delete</button>";
    cout << "<input type=hidden name=direction value='" << db["direction"] << "'>";
    cout << "<input type=hidden name=vnum value=" << vnum << ">";
    cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
    cout << "</form></td></tr></table>";
    

    cout << "<hr>";
  }


  cout << body() << endl;
  cout << html() << endl;

}



sstring getRiverDirForm(int selected)
{
  sstring buf="<tr><td>river_dir</td><td><select name=river_dir>\n";
  buf+=format("<option value=%i %s>%s</option>\n") %
   -1 % ((-1==selected)?"selected":"") % "none";
  for(int i=0;i<MAX_DIR;++i){
    buf+=format("<option value=%i %s>%s</option>\n") %
      i % ((i==selected)?"selected":"") % dirs[i];
  }
  buf+="</select>\n";

  return buf;
}


sstring getTerrainForm(int selected)
{
  multimap <sstring, int, std::less<sstring> > m;
  multimap <sstring, int, std::less<sstring> >::iterator it;

  selected = mapFileToSector(selected);

  for (sectorTypeT i = MIN_SECTOR_TYPE; i < MAX_SECTOR_TYPES; i++) {
    if (!*TerrainInfo[i]->name)
      continue;
    
    m.insert(pair<sstring,int>(TerrainInfo[i]->name, i));
  }

  sstring buf="<tr><td>sector</td><td><select name=sector>\n";
  for(it=m.begin();it!=m.end();++it){
    buf+=format("<option value=%i %s>%s</option>\n") %
      (*it).second % (((*it).second==selected)?"selected":"") % 
      (*it).first;
  }
  buf+="</select>\n";

  return buf;
}


sstring getProcForm(int selected, bool wizard)
{
  multimap <sstring, int, std::less<sstring> > m;
  multimap <sstring, int, std::less<sstring> >::iterator it;

  m.insert(pair<sstring,int>("-- none", 0));

  for(int i=1;i<NUM_ROOM_SPECIALS-1;++i){
    if(roomSpecials[i].name!="BOGUS")
      m.insert(pair<sstring,int>(roomSpecials[i].name, i));
  }

  sstring buf="<tr><td>spec_proc</td><td><select name=spec>\n";
  for(it=m.begin();it!=m.end();++it){
    if(roomSpecials[(*it).second].assignable || 
       (*it).second==selected ||
       wizard){
      buf+=format("<option value=%i %s>%s</option>\n") %
	(*it).second % (((*it).second==selected)?"selected":"") % 
	(*it).first;
    }
  }
  buf+="</select>\n";

  return buf;
}



void sendShowRoom(int account_id, int vnum, bool wizard)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db_exits(DB_IMMORTAL);
  sstring buf;

  assign_item_info();
  assignTerrainInfo();

  db.query("select owner, vnum, x, y, z, name, description, room_flag, sector, teletime, teletarg, telelook, river_speed, river_dir, capacity, height, spec from room where block=1 and vnum=%i and owner in (%r)", vnum, getPlayerNames(account_id).c_str());
  db.fetchRow();

  cout << "<form method=post action=roomeditor.cgi name=pickroom>" << endl;
  cout << "<table width=100%><tr>";
  cout << "<td align=left><button name=state value=logout type=submit>logout</button></td>";
  cout << "<td align=left><button name=state value=main type=submit>room list</button></td>";
  cout << "<td align=left><button name=state value=showextra type=submit>edit extras</button></td>";
  cout << "<td align=left><button name=state value=showexit type=submit>edit exits</button></td>";
  cout << "<td width=100% align=right><button name=state value=delroom type=submit>delete</button></td>";
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  cout << "<input type=hidden name=vnum value='" << vnum << "'>";
  cout << "</table><p></form>" << endl;

  // exit navigation
  
  db_exits.query("select direction, destination from roomexit where vnum=%i and owner in (%r) and block=1 and destination in (select vnum from room where owner in (%r) and block=1)", vnum, getPlayerNames(account_id).c_str(), getPlayerNames(account_id).c_str());
  int exits[MAX_DIR];

  for(dirTypeT i=MIN_DIR;i<MAX_DIR;i++){
    exits[i]=-1;
  }
  while(db_exits.fetchRow()){
    exits[convertTo<int>(db_exits["direction"])]=convertTo<int>(db_exits["destination"]);
  }


  cout << "<form action=\"roomeditor.cgi\" method=post>" << endl;
  cout << "<input type=hidden name=state value=showroom>" << endl;
  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";
  
  cout << "<table border=1>";
  cout << "<tr><td><button name=vnum value="<< exits[DIR_NORTHWEST] << 
    ((exits[DIR_NORTHWEST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_NORTHWEST] <<"</button></td>" << endl;

  cout << "<td><button name=vnum value="<< exits[DIR_NORTH] << 
    ((exits[DIR_NORTH]==-1)?" disabled> ":"> ") <<
    dirs[DIR_NORTH] <<"</button></td>";

  cout << "<td><button name=vnum value="<< exits[DIR_NORTHEAST] << 
    ((exits[DIR_NORTHEAST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_NORTHEAST] <<"</button></td>";

  cout << "<td><button name=vnum value="<< exits[DIR_UP] << 
    ((exits[DIR_UP]==-1)?" disabled> ":"> ") <<
    dirs[DIR_UP] <<"</button></td></tr>";

  cout << "<tr><td align=right><button name=vnum value="<< exits[DIR_WEST] << 
    ((exits[DIR_WEST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_WEST] <<"</button></td>" << endl;

  cout << "<td></td>" << endl;

  cout << "<td><button name=vnum value="<< exits[DIR_EAST] << 
    ((exits[DIR_EAST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_EAST] <<"</button></td></tr>";

  cout << "<tr><td><button name=vnum value="<< exits[DIR_SOUTHWEST] << 
    ((exits[DIR_SOUTHWEST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_SOUTHWEST] <<"</button></td>" << endl;

  cout << "<td><button name=vnum value="<< exits[DIR_SOUTH] << 
    ((exits[DIR_SOUTH]==-1)?" disabled> ":"> ") <<
    dirs[DIR_SOUTH] <<"</button></td>";

  cout << "<td><button name=vnum value="<< exits[DIR_SOUTHEAST] << 
    ((exits[DIR_SOUTHEAST]==-1)?" disabled> ":"> ") <<
    dirs[DIR_SOUTHEAST] <<"</button></td>";

  cout << "<td><button name=vnum value="<< exits[DIR_DOWN] << 
    ((exits[DIR_DOWN]==-1)?" disabled> ":"> ") <<
    dirs[DIR_DOWN] <<"</button></td></tr>";


  cout << "</table>";

  cout << "</form>";
  /////


  cout << "<form action=\"roomeditor.cgi\" method=post name=saveroom>" << endl;
  cout << "<input type=hidden name=state value=saveroom>" << endl;

  cout << "<input type=hidden name=owner value='" << db["owner"] << "'>";


  cout << "<table border=1>";


  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "vnum" % "vnum" % db["vnum"];

  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "x" % "x" % db["x"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "y" % "y" % db["y"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "z" % "z" % db["z"];

  buf=db["name"];
  while (buf.find("'") != sstring::npos)
    buf.replace(buf.find("'"), 1, "&#39;");
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "name" % "name" % buf;
  buf=format("%s%s") % 
    getSectorNameColor(mapFileToSector(convertTo<int>(db["sector"])), NULL) %
    db["name"];
  cout << format("<tr><td></td><td bgcolor=black>%s</td></tr>\n") % 
    mudColorToHTML(buf);

  buf=db["description"];
  while (buf.find("'") != sstring::npos)
    buf.replace(buf.find("'"), 1, "&#39;");
  cout << format("<tr><td>%s</td><td><textarea name=description cols=90 rows=5>%s</textarea></td></tr>\n") % "description" % buf;
  buf=format("%s%s") % 
    getSectorDescrColor(mapFileToSector(convertTo<int>(db["sector"])), NULL) %
    db["description"];
  cout << format("<tr><td></td><td width=80 bgcolor=black>%s</td></tr>\n") %
    mudColorToHTML(buf);


  // room flag
  cout << "<tr><td>room_flag</td><td><table><tr>" << endl;
  int room_flag=convertTo<int>(db["room_flag"]);
  for(int i=0;i<MAX_ROOM_BITS;++i){
    cout << format("<td><input type=checkbox %s name='%s'> %s") %
      ((room_flag & (1<<i))?"checked":"") % room_bits[i] % room_bits[i];

    cout << "</td>";

    if(!((i+1) % 6))
      cout << "</tr><tr>";
  }
  cout <<"</tr></table></td></tr>";
  //



  cout << getTerrainForm(convertTo<int>(db["sector"]));

  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "teletime" % "teletime" % db["teletime"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "teletarg" % "teletarg" % db["teletarg"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "telelook" % "telelook" % db["telelook"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "river_speed" % "river_speed" % db["river_speed"];
  cout << getRiverDirForm(convertTo<int>(db["river_dir"]));

  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "capacity" % "capacity" % db["capacity"];
  cout << format("<tr><td>%s</td><td><input type=text size=127 name='%s' value='%s'></td></tr>\n") % "height" % "height" % db["height"];
  
  cout << getProcForm(convertTo<int>(db["spec"]), wizard);

  cout << "</table>";

  cout << "<input type=submit value='save changes'>";
  cout << "</form>" << endl;

  cout << body() << endl;
  cout << html() << endl;
  
  
}

void sendRoomlist(int account_id)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db_exits(DB_IMMORTAL);
  TDatabase db_extras(DB_IMMORTAL);
  sstring buf;

  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("SneezyMUD Room Editor") << endl;
  sendJavaScript();
  cout << head() << body() << endl;

  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>";

  sstring buildername;

  db.query("select owner, max(vnum)+1 as nvnum from room where block=1 and lower(owner) in (%r) group by owner",
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

  cout << "<form method=post action=roomeditor.cgi>" << endl;
  cout << "<button name=state value=newroom type=submit>new room</button>";
  cout << "vnum <input type=text name=vnum value=" << db["nvnum"] << ">";
  cout << "template <input type=text name=template value=";
  cout << max((convertTo<int>(db["nvnum"])-1),0) << ">";
  cout << "<input type=hidden name=owner value='" << buildername << "'>";
  cout << "</form>";
  cout << endl;

  cout << "<form action=\"roomeditor.cgi\" method=post name=pickroom>" << endl;
  cout << "<input type=hidden name=vnum>" << endl;
  cout << "<input type=hidden name=state>" << endl;

  cout << "<table border=1>";
  cout << "<tr><td>vnum</td><td>name</td><td>extras</td><td>exits</td></tr>";

  db.query("select vnum, name, sector from room r where block=1 and lower(owner) in (%r) order by vnum asc", getPlayerNames(account_id).c_str());
  
  db_exits.query("select vnum, count(*) as count from roomexit where block=1 and lower(owner) in (%r) group by vnum order by vnum asc", getPlayerNames(account_id).c_str());
  db_exits.fetchRow();

  db_extras.query("select vnum, count(*) as count from roomextra where block=1 and lower(owner) in (%r) group by vnum order by vnum asc", getPlayerNames(account_id).c_str());
  db_extras.fetchRow();

  int affcount=0, extracount=0;
  while(db.fetchRow()){
    affcount=extracount=0;
    while(convertTo<int>(db_exits["vnum"]) < 
	  convertTo<int>(db["vnum"]))
      if(!db_exits.fetchRow())
	break;
    while(convertTo<int>(db_extras["vnum"]) < 
	  convertTo<int>(db["vnum"]))
      if(!db_extras.fetchRow())
	break;
    
    if(db_exits["vnum"]==db["vnum"]){
      affcount=convertTo<int>(db_exits["count"]);
      db_exits.fetchRow();
    }
    if(db_extras["vnum"]==db["vnum"]){
      extracount=convertTo<int>(db_extras["count"]);
      db_extras.fetchRow();
    }

    cout << "<tr><td>" << "<a href=javascript:pickroom('" << db["vnum"];
    cout << "','showroom')>" << db["vnum"] << "</a>" << endl;
    buf=format("%s%s") % 
      getSectorNameColor(mapFileToSector(convertTo<int>(db["sector"])), NULL) %
      db["name"];
    cout << "</td><td bgcolor=black>" << mudColorToHTML(buf, false);
    cout << "</td>"<< endl;
    cout << "<td><a href=javascript:pickroom('" << db["vnum"];
    cout << "','showextra')>" << extracount << "</a></td>" << endl;
    cout << "<td><a href=javascript:pickroom('" << db["vnum"];
    cout << "','showexit')>" << affcount << "</a></td>" << endl;
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

  cout << "function pickroom(vnum, state)" << endl;
  cout << "{" << endl;
  cout << "document.pickroom.state.value = state;" << endl;
  cout << "document.pickroom.vnum.value = vnum;" << endl;
  cout << "document.pickroom.submit();" << endl;
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

