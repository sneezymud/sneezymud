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
#include <unistd.h>

using namespace cgicc;


void sendSelectForm(int);
void adjustObjs(Cgicc, int);

void sendJavaScript();
sstring mudColorToHTML(sstring, bool spacer=true);


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
    session.doLogin(cgi, "eqcalc.cgi");
    return 0;
  }

  if(!session.hasWizPower(POWER_BUILDER)){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head(title("Object Load Logs")) << endl;
    cout << body() << endl;
    cout << "You don't have permission to use this.";
    cout << body() << endl;
    return 0;
  }

    

  if(state_form == cgi.getElements().end() || **state_form == "main"){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Eqcalc") << endl;
    sendJavaScript();
    cout << head() << body() << endl;

    sendSelectForm(session.getAccountID());
    return 0;
  } else if(**state_form == "adjust"){
    cout << HTTPHTMLHeader() << endl;
    cout << html() << head() << title("Eqcalc") << endl;
    sendJavaScript();
    cout << head() << body() << endl;

    adjustObjs(cgi, session.getAccountID());
    sendSelectForm(session.getAccountID());
    return 0;
  } else if(**state_form == "logout"){
    session.logout();
    cout << HTTPRedirectHeader("eqcalc.cgi").setCookie(session.getCookie());
    cout << endl;
    return 0;
  }
  
  cout << HTTPHTMLHeader() << endl;
  cout << html() << head() << title("Eqcalc") << endl;
  cout << head() << body() << endl;
  cout << "Fell through state switch.  Bad.<p><hr><p>" << endl;
  cout << **state_form << endl;
  cout << body() << endl;
  cout << html() << endl;
  
  return 0;
}



float getRaceClassConstant(race_t eq_race, int action_flag)
{
  float min=0;
  if (eq_race==RACE_HUMAN) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.86;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.86;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.71;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.71;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.64;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.57;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.57;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.50;
  } else if (eq_race==RACE_DWARF) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.86;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.79;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.66;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.66;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.59;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.52;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.52;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.46;
  } else if (eq_race==RACE_ELVEN) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.71;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.71;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.60;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.60;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.57;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.54;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.48;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.43;
  } else if (eq_race==RACE_GNOME) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.71;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.71;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.71;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.60;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.54;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.48;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.48;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.43;
  } else if (eq_race==RACE_HOBBIT) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.64;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.64;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.64;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.52;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.52;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.43;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.43;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.37;
  } else if (eq_race==RACE_OGRE) {
    if (!(ITEM_ANTI_WARRIOR & action_flag)) min = 0.79;
    if (!(ITEM_ANTI_DEIKHAN & action_flag)) min = 0.64;
    if (!(ITEM_ANTI_CLERIC & action_flag))  min = 0.52;
    if (!(ITEM_ANTI_RANGER & action_flag))  min = 0.52;
    if (!(ITEM_ANTI_THIEF & action_flag))   min = 0.48;
    if (!(ITEM_ANTI_MAGE & action_flag))    min = 0.43;
    if (!(ITEM_ANTI_SHAMAN & action_flag))  min = 0.43;
    if (!(ITEM_ANTI_MONK & action_flag))    min = 0.37;
  }
  return min;
}

void getSlotData(int wear_flag, wearSlotT &slot, float &slot_c, float &slot_s, sstring &fav)
{
  switch(wear_flag){
    case 3:   // finger
      slot=WEAR_FINGER_R;
      slot_c=0.01;
      slot_s=0.0454;
      break;
    case 5:   // neck
      slot=WEAR_NECK;
      slot_c=0.04;
      slot_s=0.0908;
      break;
    case 9:   // body
      slot=WEAR_BODY;
      slot_c=0.15;
      slot_s=0.0908;
      fav=fmt("%i,%i,%i") % mapApplyToFile(APPLY_STR) % mapApplyToFile(APPLY_BRA) % mapApplyToFile(APPLY_CON) ;
      break;
    case 17:  // head
      slot=WEAR_HEAD;
      slot_c=0.07;
      slot_s=0.0908;
      fav=fmt("%i,%i,%i,%i,%i") % mapApplyToFile(APPLY_INT) % mapApplyToFile(APPLY_FOC) % mapApplyToFile(APPLY_WIS) % mapApplyToFile(APPLY_PER) % mapApplyToFile(APPLY_VISION);
      break;
    case 33:  // legs
      slot=WEAR_LEG_R;
      slot_c=0.05;
      slot_s=0.0454;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_AGI) % mapApplyToFile(APPLY_MOVE) ;
      break;
    case 65:  // feet
      slot=WEAR_FOOT_R;
      slot_c=0.02;
      slot_s=0.0454;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_SPE) % mapApplyToFile(APPLY_MOVE) ;
      break;
    case 129: // hands
      slot=WEAR_HAND_R;
      slot_c=0.03;
      slot_s=0.0454;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_DEX) % mapApplyToFile(APPLY_SPE) ;
      break;
    case 257: // arms
      slot=WEAR_ARM_R;
      slot_c=0.04;
      slot_s=0.0454;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_AGI) % mapApplyToFile(APPLY_STR) ;
      break;
    case 1025: // back
      slot=WEAR_BACK;
      slot_c=0.07;
      slot_s=0.0908;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_CON) % mapApplyToFile(APPLY_BRA) ;
      break;
    case 2049: // waist
      slot=WEAR_WAIST;
      slot_c=0.08;
      slot_s=0.0908;
      fav=fmt("%i,%i") % mapApplyToFile(APPLY_CON) % mapApplyToFile(APPLY_STR) ;
      break;
    case 4097: // wrist
      slot=WEAR_WRIST_R;
      slot_c=0.02;
      slot_s=0.0454;
      fav=fmt("%i,%i,%i") % mapApplyToFile(APPLY_DEX) % mapApplyToFile(APPLY_AGI) % mapApplyToFile(APPLY_SPE) ;
      break;
    case 16385: // hold
      slot=HOLD_LEFT;
      slot_c=0.25;
      slot_s=0.0908;
      break;
  }

  fav=fmt("%s,%i,%i,%i,%i,%i,%i") % fav % mapApplyToFile(APPLY_MANA) % mapApplyToFile(APPLY_HIT) % mapApplyToFile(APPLY_NOISE) % mapApplyToFile(APPLY_AGE) % mapApplyToFile(APPLY_CHA) % mapApplyToFile(APPLY_KAR);
}


int getStatCount(sstring vnum, sstring owner, sstring fav)
{
  TDatabase db2(DB_IMMORTAL);

    db2.query("select sum(mod1) as statcount from objaffect where vnum=%s and owner='%s' and type in (%s)", vnum.c_str(), owner.c_str(), fav.c_str());
    db2.fetchRow();
    int statcount=convertTo<int>(db2["statcount"]);

    sstring unfav=fmt("%s,%i,%i,%i,%i,%i") % fav % mapApplyToFile(APPLY_ARMOR) % mapApplyToFile(APPLY_IMMUNITY) % mapApplyToFile(APPLY_DISCIPLINE) % mapApplyToFile(APPLY_SPELL_EFFECT) % mapApplyToFile(APPLY_SPELL) ;
    db2.query("select sum(mod1) as statcount from objaffect where vnum=%s and owner='%s' and type not in (%s)", vnum.c_str(), owner.c_str(), unfav.c_str());
    db2.fetchRow();
    statcount+=convertTo<int>(db2["statcount"]);

    db2.query("select sum(mod2)*2 as statcount from objaffect where vnum=%s and owner='%s' and type=%i and mod1 in (%i, %i, %i)",
	      vnum.c_str(), owner.c_str(),
	      mapApplyToFile(APPLY_IMMUNITY), IMMUNE_SLASH, IMMUNE_BLUNT, IMMUNE_PIERCE);
    db2.fetchRow();
    statcount += convertTo<int>(db2["statcount"]);

    db2.query("select sum(mod2)*3 as statcount from objaffect where vnum=%s and owner='%s' and type=%i and mod1 in (%i)",
	      vnum.c_str(), owner.c_str(),
	      mapApplyToFile(APPLY_IMMUNITY), IMMUNE_NONMAGIC);
    db2.fetchRow();
    statcount += convertTo<int>(db2["statcount"]);

    return statcount;
}

void lowerStats(sstring vnum, sstring owner, sstring fav, float maxes)
{
  TDatabase db2(DB_IMMORTAL);
  sstring unfav=fmt("%s,%i,%i,%i,%i,%i") % fav % mapApplyToFile(APPLY_ARMOR) % mapApplyToFile(APPLY_IMMUNITY) % mapApplyToFile(APPLY_DISCIPLINE) % mapApplyToFile(APPLY_SPELL_EFFECT) % mapApplyToFile(APPLY_SPELL);
  int count=0, lcount=0, breakout=0;

  while(((count=getStatCount(vnum, owner, fav)) > maxes)){
    if(count==lcount)
      if((++breakout) > 10)
	break;

    switch(::number(0,3)){
      case 0:
	db2.query("update objaffect set mod1=mod1-1 where mod1 > 0 and vnum=%s and owner='%s' and type in (%s)", vnum.c_str(), owner.c_str(), fav.c_str());
	db2.query("delete from objaffect where mod1=0 and vnum=%s and owner='%s' and type in (%s)", vnum.c_str(), owner.c_str(), fav.c_str());
	break;
      case 1:
	db2.query("update objaffect set mod1=mod1-1 where mod1 > 0 and vnum=%s and owner='%s' and type not in (%s)", vnum.c_str(), owner.c_str(), unfav.c_str());
	db2.query("delete from objaffect where mod1=0 and vnum=%s and owner='%s' and type not in (%s)", vnum.c_str(), owner.c_str(), unfav.c_str());
	break;
      case 2:
	db2.query("update objaffect set mod2=mod2-1 where mod2 > 0 and vnum=%s and owner='%s' and type=%i and mod1 in (%i, %i, %i)",
		  vnum.c_str(), owner.c_str(),
		  mapApplyToFile(APPLY_IMMUNITY), IMMUNE_SLASH, IMMUNE_BLUNT, IMMUNE_PIERCE);
	db2.query("delete from objaffect where mod2=0 and vnum=%s and owner='%s' and type=%i and mod1 in (%i, %i, %i)",
		  vnum.c_str(), owner.c_str(),
		  mapApplyToFile(APPLY_IMMUNITY), IMMUNE_SLASH, IMMUNE_BLUNT, IMMUNE_PIERCE);

	break;
      case 3:
	db2.query("update objaffect set mod2=mod2-1 where mod2 > 0 and vnum=%s and owner='%s' and type=%i and mod1 in (%i)",
		  vnum.c_str(), owner.c_str(),
		  mapApplyToFile(APPLY_IMMUNITY) ,  IMMUNE_NONMAGIC);
	db2.query("delete from objaffect where mod2=0  and vnum=%s and owner='%s' and type=%i and mod1 in (%i)",
                  vnum.c_str(), owner.c_str(),
                  mapApplyToFile(APPLY_IMMUNITY) , IMMUNE_NONMAGIC);
	break;
    }
    lcount=count;
  }
}


void adjustObjs(Cgicc cgi, int account_id)
{
  TDatabase db(DB_IMMORTAL);
  TDatabase db2(DB_IMMORTAL);
  int moblevel=convertTo<int>((**(cgi.getElement("moblevel"))));
  vector <FormEntry> objlist;
  cgi.getElement("objlist", objlist);
  sstring buf;
  bool is_artifact, any_race;
  float volume, slot_c, slot_s;
  sstring fav;

  if(!moblevel){
    cout << "You need to enter a valid mob level.<p>";
    return;
  }
  
  if(!objlist.size()){
    cout << "You need to select object(s).<p>";
    return;
  }

  buf="";
  for(unsigned int i=0;i<objlist.size();++i){
    if(!buf.empty())
      buf+=",";
    buf+=*objlist[i];
  }
  
  db.query("select vnum, owner, type, max_exist, volume, wear_flag, action_flag, short_desc from obj o where lower(owner) in (%r) and vnum in (%s) order by vnum asc", getPlayerNames(account_id).c_str(), buf.c_str());

  
  while(db.fetchRow()){
    is_artifact=((convertTo<int>(db["max_exist"])==1)?true:false);
    
    volume=convertTo<float>(db["volume"]);
    
    wearSlotT slot=MAX_WEAR;
    slot_c=0.0;
    slot_s=0.0;

    getSlotData(convertTo<int>(db["wear_flag"]), slot, slot_c, slot_s, fav);

    // initialize race data
    chdir("/mud/prod/lib");
    for(race_t rindex=RACE_NORACE;rindex<MAX_RACIAL_TYPES;rindex++)
      Races[rindex] = new Race(rindex);

    // list of player usable races
    vector <race_t> player_races;
    player_races.push_back(RACE_HUMAN);
    player_races.push_back(RACE_ELVEN);
    player_races.push_back(RACE_DWARF);
    player_races.push_back(RACE_GNOME);
    player_races.push_back(RACE_HOBBIT);
    player_races.push_back(RACE_OGRE);
  
    int avg_height=0;
    float eq_size=0;
    race_t eq_race=RACE_NORACE;
    any_race=false;


    // figure out what player race can use this eq
    for(unsigned int i=0;i<player_races.size();++i){
      avg_height=Races[player_races[i]]->getBaseMaleHeight();
      avg_height+=(int)((float)Races[player_races[i]]->getMaleHtNumDice() * (float)((float)Races[player_races[i]]->getMaleHtDieSize() / 2.0));

      eq_size=race_vol_constants[mapSlotToFile(slot)] * avg_height;

      if(volume <= (eq_size * 1.15) &&
	 volume >= (eq_size *0.85)){
	eq_race=player_races[i];
	break;
      }
    }

    if(slot == WEAR_NECK || 
       convertTo<int>(db["type"]) == ITEM_JEWELRY ||
       slot == HOLD_LEFT){
      eq_race=RACE_HUMAN;
      any_race=true;
    }
    if(eq_race==RACE_NORACE){
      cout << "Couldn't find racial size for ";
      cout << stripColorCodes(db["short_desc"]);
      cout << ".  Skipping.<br>" << endl;
    }

    ////////


    // calculate appropriate ac
    int action_flag=convertTo<int>(db["action_flag"]);
    float race_class=getRaceClassConstant(eq_race, action_flag);
    float aclevel = ceil(((float)moblevel * race_class));

    if(convertTo<int>(db["type"]) == ITEM_JEWELRY)
      aclevel /= 2;

    float ac = ((aclevel * 25.0) + 500.0) * slot_c;


    // stats
    float stats = ceil((moblevel * 0.5) * 0.86);
    float maxes = ceil(slot_s * stats * 2);

    
    // adjust the eq
    db2.query("update objaffect set mod1=%i where type=11 and vnum=%s and owner='%s'", (int)-ac, db["vnum"].c_str(), db["owner"].c_str());

    cout << "Adjusted " << stripColorCodes(db["short_desc"]);
    cout << " (" << (any_race?"any":Races[eq_race]->getProperName()) << ")";
    if(convertTo<int>(db["type"]) == ITEM_JEWELRY)
      cout << "(jewelry)";
    cout << ": AC = " << (int)-ac << " (L" << aclevel << ").";


    if(getStatCount(db["vnum"], db["owner"], fav) > maxes){
      lowerStats(db["vnum"], db["owner"], fav, maxes);
      cout <<" Current stat total is now ";
      cout << getStatCount(db["vnum"], db["owner"], fav);
      cout << " (out of " << maxes << ").";
    }

    cout << "<br>" << endl;

  }
  
  //  cout << "Adjusted";
}

void sendSelectForm(int account_id)
{
  TDatabase db(DB_IMMORTAL);

  cout << "<form method=post action=eqcalc.cgi>" << endl;
  cout << "<button name=state value=logout type=submit>logout</button>";
  cout << "<p></form>";

  cout << "<form method=post action=eqcalc.cgi>" << endl;

  cout <<"Level of weakest mob suit loads on: <input type=text name=moblevel>";
  cout << "<p>" << endl;

  // objects
  cout << "Select the eq to adjust:<br>" << endl;
  cout << "(Hold down control to select multiple pieces)<br>" << endl;
  cout << "<select name=objlist multiple=true>" << endl;

  db.query("select vnum, short_desc from obj o where lower(owner) in (%r) order by vnum asc", getPlayerNames(account_id).c_str());
  
  while(db.fetchRow()){
    cout << "<option value=" << db["vnum"] << ">";
    cout << db["vnum"] << " - " << mudColorToHTML(db["short_desc"], false);
    cout << endl;
  }
  cout << "</select><p>";

  cout << "<button name=state value=adjust type=submit>adjust</button>";

  cout << "</form>";
  

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

  return fmt("<span style=\"color:white\"><font face=\"courier\">%s%s</font></span>") % spacing_strip % str;
}

