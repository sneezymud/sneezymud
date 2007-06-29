#include "stdsneezy.h"
#include "database.h"

#include <vector>
#include "sstring.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;

Cgicc cgi;

void replaceString(sstring &, sstring, sstring);
sstring mudColorToHTML(sstring);
sstring ambiguousLevel(int);
sstring genericPart(int);

int main(int argc, char **argv)
{
  // trick the db code into using the prod database
  gamePort = PROD_GAMEPORT;
  TDatabase db(DB_SNEEZY);
  
  // get the query string
  Cgicc cgi;
  CgiEnvironment ce = cgi.getEnvironment();
  sstring team = ce.getQueryString();
  replaceString(team, "%20", " ");
  
  int level, avg_level, max_level, part_num, tmp;
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << endl;
  cout << "<head><title>SneezyMUD: Quest for Limbs</title>\n" 
       << "<style>\n"
       << "A {color: red;}\n"
       << "BODY, P {font-family: \"Verdana\", \"Arial\", sans-serif; background: #070909; color: white;}\n"
       << "TD, TH {padding-left: 10px; padding-right: 10px; font-family: \"Verdana\", \"Arial\", sans-serif; background: #070909; color: white; align: left;}\n"
       << "table.sortable thead {font-weight: bold; align: left; color: gray}\n"
       << "</style>\n"
       << endl;
  if (!team.empty())
    cout << "<script src=\"http://www.sneezymud.com/Metrohep/sorttable.js\"></script>\n" << endl;
  cout << "</head>\n" << endl;
  cout << body() << endl;

  if (team.empty() || team.length() > 30) {
    // show list of available teams
    vector <sstring> teams; // for the team summaries
    cout << "<table align=\"center\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\"><tr><td valign=\"top\" colspan=\"4\" align=\"center\"><h3><a href=\"limb_quest.cgi\">Welcome to SneezyMUD: Quest for Limbs III</a></h3></td></tr>" << endl;
    cout << "<tr><td colspan=\"2\" align=\"right\"><img src=\"http://www.sneezymud.com/Metrohep/handy.jpg\"></td><td colspan=\"2\" align=\"left\">" << endl;
    db.query("select distinct team from quest_limbs_team order by team");
    if(!db.isResults()){
      cout << "There are no teams designated right now." << endl;
    } else {
      cout << (fmt("View limb-ventory for :<br>") % db["team"] % db["team"]) << endl;
      while(db.fetchRow()) {
        teams.push_back(db["team"]);
        cout << (fmt("<a href=\"limb_quest.cgi?%s\">%s</a><br>") % db["team"] % db["team"]) << endl;
      }
    }
    cout << "</td></tr>\n" << endl;
    
    // summaries
    cout << "<tr><td colspan=\"4\" align=\"center\">&nbsp;<br><b>Team Summaries</b></td></tr>\n" << endl;
    for (unsigned int step = 0; step < teams.size(); step++) {
      db.query("select player from quest_limbs_team where team = '%s' order by player", teams[step].c_str());
      cout << fmt("<tr><td colspan=\"4\">&nbsp;</td></tr><tr><td colspan=\"4\" align=\"center\" style=\"border: 1px solid grey;\"><b style=\"color:red;\">%s</b><br>") % teams[step] << endl;
      tmp = 0;
      while (db.fetchRow()) {
        tmp++;
        cout << " " << db["player"] << endl;
        if (!(tmp % 7))
        cout << "<br>" << endl;
      }
      cout << "</td></tr><tr><td colspan=\"4\">&nbsp;</td></tr>" << endl;
      db.query("select case q1.slot_name when 'heart' then -1 when 'jumblies' then -2 when 'tooth' then -3 when 'eyeballs' then -4 else q1.slot_num end as slot_num, round(avg((m1.ac + m1.hpbonus + m1.damage_level) / 3)) as avg_level, round(max((m1.ac + m1.hpbonus + m1.damage_level) / 3)) as max_level, count(*) as tally from quest_limbs q1 left join mob m1 on q1.mob_vnum = m1.vnum where q1.team = '%s' and q1.date_submitted < '2007-06-25 01:30:00' group by case q1.slot_name when 'heart' then -1 when 'jumblies' then -2 when 'eyeballs' then -3 else q1.slot_num end order by slot_num", teams[step].c_str());
      if(!db.isResults()){
        cout << "<tr><td colspan=\"4\">This team is lazy, and has no limbs yet.</td></tr>" << endl;
      } else {
        cout << "<tr><td><b>Organ</b></td><td align=\"right\"><b>Number</b></td><td><b>Avg Level</b></td><td><b>Max Level</b></td></tr>" <<endl;
        while(db.fetchRow()) {
          if (is_number(db["avg_level"]))
            avg_level = convertTo<int>(db["avg_level"]);
          else
            avg_level = -1;
          if (is_number(db["max_level"]))
            max_level = convertTo<int>(db["max_level"]);
          else
            max_level = -1;
          if (!(part_num = convertTo<int>(db["slot_num"])))
            part_num = 0;
          cout << (fmt("<tr><td>%s</td><td align=\"right\">%s</td>%s%s</tr>") % genericPart(part_num) % db["tally"] % ambiguousLevel(avg_level) % ambiguousLevel(max_level)) <<endl;
        }
      }
      while (db.fetchRow()) {
        cout << fmt(" %s") % db["player"] << endl;
      }
    }
    cout << "</table>"  << endl;
    
  } else {
    // view details for team
    cout << "<table align=\"center\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\"><tr><td align=\"center\">"
         << "<table width=\"100%\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\"><tr><td valign=\"top\" colspan=\"4\" align=\"center\"><h3><a href=\"limb_quest.cgi\">Welcome to SneezyMUD: Quest for Limbs III</a></h3></td></tr>" << endl;
    cout << fmt("<tr><td colspan=\"4\" align=\"center\" style=\"border: 1px solid grey; color: red; font-weight: bold\">%s</td></tr>") % team 
         << "<tr><td colspan=\"4\">&nbsp;</td></tr>"
         << endl;
    db.query("select q1.player, q1.mob_vnum, m1.short_desc, round((m1.ac + m1.hpbonus + m1.damage_level) / 3) as mob_level, q1.slot_name from quest_limbs q1 left join mob m1 on q1.mob_vnum = m1.vnum where q1.team = '%s' and q1.date_submitted < '2007-06-25 01:30:00' order by q1.date_submitted desc, q1.slot_num;", team.c_str());
    if(!db.isResults()){
      cout << fmt("<tr><td colspan=\"4\">No limbs submitted. Get to work, %s!</td></tr>") % team << endl;
    } else {
      cout << "</table>"  << endl;
      cout << "<table width=\"100%\" class=\"sortable\" align=\"center\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\"><thead><tr><th align=\"left\"><a href=\"javascript:return false;\">Organ</a></th><th align=\"left\"><a href=\"javascript:return false;\">Donor</a></th><th align=\"left\"><a href=\"javascript:return false;\">Level</a></th><th align=\"left\"><a href=\"javascript:return false;\">Chopper</a></th></tr></thead>" <<endl;
      while(db.fetchRow()) {
        if (is_number(db["mob_level"]))
          level = convertTo<int>(db["mob_level"]);
        else
          level = -1;
        cout << (fmt("<tr><td>%s</td><td>%s</td>%s<td>%s</td></tr>") % db["slot_name"] % mudColorToHTML(db["short_desc"]) % ambiguousLevel(level) % db["player"]) <<endl;
      }
    }
    cout << "</table></td></tr></table>"  << endl;
  }
  cout << body() << endl;
  cout << html() << endl;
  return 0;
}

// candidate for inclusion in sstring
void replaceString(sstring &str, sstring find, sstring replace)
{
  while(str.find(find)!=sstring::npos){
    str.replace(str.find(find), find.size(), replace);
  }
}

sstring genericPart(int part)
{
  // doctoring values:
  // heart = -1
  // genitals = -2
  // tooth = -3
  // eyes = -4
  // too bad those don't have wearslot values
  switch (part) {
    case -4:
      return "eyes";
      break;
    case -3:
      return "tooth";
      break;
    case -2:
      return "jumblies";
      break;
    case -1:
      return "heart";
      break;
    case 1:
      return "head";
      break;
    case 2:
      return "neck";
      break;
    case 3:
      return "body";
      break;
    case 4:
      return "back";
      break;
    case 5:
      return "right arm";
      break;
    case 6:
      return "left arm";
      break;
    case 7:
      return "right wrist";
      break;
    case 8:
      return "left wrist";
      break;
    case 9:
      return "right hand";
      break;
    case 10:
      return "left hand";
      break;
    case 11:
      return "right finger";
      break;
    case 12:
      return "left finger";
      break;
    case 13:
      return "waist";
      break;
    case 14:
      return "right leg";
      break;
    case 15:
      return "left leg";
      break;
    case 16:
      return "right foot";
      break;
    case 17:
      return "left foot";
      break;
    case 18:
      return "right hand";
      break;
    case 19:
      return "left hand";
      break;
    case 20:
      return "rear right leg";
      break;
    case 21:
      return "rear left leg";
      break;
    case 22:
      return "rear right foot";
      break;
    case 23:
      return "rear left foot";
      break;
    default:
      return "giblets";
      break;
  }
  return "giblets";
}

sstring ambiguousLevel(int n)
{
  if (n < 0)
    return "<td sorttable_customkey=\"1\">no level</td>";
  else if (n < 10)
    return "<td sorttable_customkey=\"2\">very low</td>";
  else if (n < 20)
    return "<td sorttable_customkey=\"3\">low</td>";
  else if (n < 40)
    return "<td sorttable_customkey=\"4\">medium</td>";
  else if (n < 50)
    return "<td sorttable_customkey=\"5\">medium high</td>";
  else if (n < 60)
    return "<td sorttable_customkey=\"6\">high</td>";
  else if (n < 70)
    return "<td sorttable_customkey=\"7\">very high</td>";
  else if (n < 80)
    return "<td sorttable_customkey=\"8\">extremely high</td>";
  else
    return "<td sorttable_customkey=\"9\">crazy high</td>";
}

// candidate for some sort of global cgi tools library
sstring mudColorToHTML(sstring str)
{
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

  return fmt("<span style=\"color:white;\">%s</span>") % str;
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
