#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;

Cgicc cgi;

void showTeamChops(sstring, TDatabase);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  cout << HTTPHTMLHeader() << endl;
  cout << html() << endl;
  cout << head(title("Quest for Limbs")) << endl;
  cout << body() << endl;
  
  vector <sstring> teams;
  TDatabase db(DB_SNEEZY);
  db.query("select distinct team from quest_limbs_teams order by team");
  
  if(!db.fetchRow()) {
    cout << "What are the teams again?" << endl;
    cout << body() << endl;
    cout << html() << endl;
    return 0;
  }

  while(db.fetchRow()) {
    teams.push_back(db["team"]);
  }
  
  for (unsigned int step = 0; step < teams.size(); step++) {
    showTeamChops(teams[step], db);
  }
  
  cout << body() << endl;
  cout << html() << endl;
  return 0;
}


void showTeamChops(sstring team, TDatabase db) {
  db.query("select t1.team, t1.player, q1.mob_vnum, m1.short_desc, round((m1.ac + m1.hpbonus + m1.damage_level) / 3) as mob_level, q1.slot_name from quest_limbs q1 join quest_limbs_team t1 on q1.player = t1.player and t1.team = '%s' left join mob m1 on q1.mob_vnum = m1.vnum order by q1.date_submitted desc, q1.slot_num", team.c_str());
  
  if(!db.fetchRow()) {
    cout << (fmt("No chops for team %s.") % team) << endl;
    return;
  }
    
  cout << "<table>\n" <<endl;
  cout << (fmt("<tr><td colspan='3'>%s</td></tr>\n") % team) <<endl;
  cout << "<tr><td>Chopped</td><td>Choppee</td><td>Chopper</td></tr>\n" <<endl;
  while(db.fetchRow()) {
    cout << (fmt("<tr><td>%s</td><td>%s (%s)</td><td>%s</td></tr>\n") % db["slot_name"] % db["short_desc"] % db["mob_level"] % db["player"]) <<endl;
  }
  cout << "</table>\n" <<endl;
  return;
}



/*
 create table quest_limbs 
 (
 player varchar(80) not null,
 team varchar(30) null,
 mob_vnum int(11) not null,
 slot_num int(11) not null,
 slot_name varchar(80) not null,
 date_submitted timestamp not null default CURRENT_TIMESTAMP
 );
 
 create table quest_limbs_team
 (
 team varchar(30) null,
 player varchar(80) not null
 );
 
 * 
 * 
 * 
 */
