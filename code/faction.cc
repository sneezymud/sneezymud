//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      faction.cc : Functions related to factions
//
//////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
}
#include <cmath>

#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"

TFactionInfo FactionInfo[MAX_FACTIONS];

double avg_faction_power = 0.0;
spellNumT your_deity_val = TYPE_UNDEFINED;

// start new faction stuff
vector<TFaction *>faction_table(0);


int TFactionInfo::getMoney() const {
  TCorporation corp(corp_id);

  return corp.getMoney();
}

void TFactionInfo::setMoney(int money){
  TCorporation corp(corp_id);
  
  corp.setMoney(money);
}

void TFactionInfo::addToMoney(int money){
  TCorporation corp(corp_id);
  
  corp.setMoney(corp.getMoney() + money);
}


// open recruitment factions in the office are taken care of by the registrar
void TBeing::doJoin(const char * args) {
  char buf[256];
  TFaction *f = NULL;

  if(!TestCode5) {
    sendTo("The new faction system is currently disabled.  You may not join a faction now.\n\r");
    return;
  }
  if(!(f = get_faction(args)) || (IS_SET(f->flags, FACT_HIDDEN) && !hasOffer(f)) || 
     (!IS_SET(f->flags, FACT_ACTIVE))) {
    // logic: faction doesn't exist, faction is hidden and hasn't extended an offer, or
    // faction is inactive - deny.
    sendTo("There is no such faction for you to join.\n\r");
    return;
  }
  if(faction.whichfaction) {
    sprintf(buf, "You are already a member of %s, you may not join a second faction.\n\r", f->getName());
    sendTo(COLOR_BASIC, buf);
    return;
  }
  if(recentlyDefected()) {
    sprintf(buf, "You recently defected from your faction, you'll have to wait to join another.");
     sendTo(buf);
    return;
  }

  if(!hasOffer(f)) {
    sprintf(buf,"%s has not extended a recruitment offer to you.\n\r", f->getName());
    sendTo(COLOR_BASIC, buf);
    if(IS_SET(f->flags, FACT_OPEN_RECRUITMENT)) {
      sendTo("However, you may join at the Grimhaven Bureau of Faction Affairs.\n\r");
    }
    return;
  }
  sprintf(buf,"You have accepted %s's offer and joined their faction!\n\r", f->getName());
  faction.whichfaction = f->ID;
  faction.rank = f->ranks - 1; // this starts them off as the lowest level rank
  sendTo(COLOR_BASIC, buf);
  removeOffers();
  saveFactionStats();
  return;
}


void TBeing::doDefect(const char * args) {
  if(!TestCode5) {
    sendTo("The new faction system is currently disabled.  You may not defect now.\n\r");
    return;
  }

  char buf[80];
  if(!faction.whichfaction) {
    sendTo("You are not a member of any faction - no need to defect.\n\r");
    return;
  }
  sscanf(args, "%s", buf);
  if(!strcmp(buf, "yes")) {
    sprintf(buf, "You have defected from %s.\n\r", newfaction()->getName());
    sendTo(COLOR_BASIC, buf);
    vlogf(LOG_FACT, fmt("%s defected from %s.") %  getName() % newfaction()->getName());
    faction.whichfaction = 0;
    faction.rank = 0;
    saveFactionStats();
    setDefected();
  } else {
    sendTo("In order to insure you really meant to defect, you have to type 'defect yes'.\n\r");
  }
  return;
}  

void TBeing::doRecruit(const char * args) {
  if(!TestCode5) {
    sendTo("The new faction system is currently disabled.  You may not recruit now.\n\r");
    return;
  }
  TBeing *targ;
  TObj *obj;
  int bits = generic_find(args, FIND_CHAR_ROOM, this, &targ, &obj);
  if(!bits) {
    sendTo("They don't seem to be here. It will be hard to recruit them.\n\r");
    return;
  }
  
  if(faction.whichfaction == 0) {
    sendTo("You are unaffiliated, you have no faction to recruit into!\n\r");
    return;
  }
  if(!IS_SET(newfaction()->permissions[faction.rank], PERM_RECRUIT)) {
    sendTo("You do not have permission to recruit new members.\n\r");
    return;
  }
  if(!targ->isPc()) {
    sendTo("You can only recruit players.\n\r");
    return;
  }
  if(!isPc()) {
    sendTo("Mobiles can't recruit!\n\r");
    return;
  }
  if(targ->faction.whichfaction) {
    sendTo("You cannot recruit players who are already in another faction.\n\r");
    return;
  }
  if (targ->hasOffer(newfaction())) {
    sendTo("Your faction has already extended an offer of recruitment to them.\n\r");
    return;
  }
  char buf[256];
  sprintf(buf, "You extend an offer of recruitment to %s.\n\r", targ->getName());
  sendTo(buf);
  
  sprintf(buf, "<o>%s <o>has extended you an offer of recruitment into %s<o>.<1>\n\r",
	  this->getName(), newfaction()->getName());
  targ->sendTo(COLOR_BASIC,buf);
  sprintf(buf, "You may accept this offer by typing 'join <faction>'. It will expire in one day.\n\r");
  targ->sendTo(buf);
  targ->addOffer(newfaction());

  return;
}

bool TBeing::hasOffer(TFaction * f) {
  
  affectedData *hjp;

  for (hjp = affected; hjp; hjp = hjp->next) {
    if (hjp->type == AFFECT_OFFER && hjp->modifier == f->ID)
      return TRUE;
  }
  return FALSE;
}

void TBeing::removeOffers() {
  affectedData *hjp = NULL, *next_aff = NULL;

  for (hjp = affected; hjp; hjp = next_aff) {
    next_aff = hjp->next;
    if (hjp->type == AFFECT_OFFER) {
      affectRemove(hjp);
    }
  }
}

void TBeing::addOffer(TFaction * f) {
  affectedData aff;

  aff.type = AFFECT_OFFER;
  aff.duration = UPDATES_PER_MUDHOUR * 24;
  aff.modifier = f->ID;
  aff.location = APPLY_NONE;

  affectTo(&aff);
  return;
}

bool TBeing::recentlyDefected() {
  affectedData *hjp;

  for (hjp = affected; hjp; hjp = hjp->next) {
    if (hjp->type == AFFECT_DEFECTED)
      return TRUE;
  }
  return FALSE;
}

void TBeing::setDefected() {
  affectedData aff;

  aff.type = AFFECT_DEFECTED;
  aff.duration = UPDATES_PER_MUDHOUR * 24;

  affectTo(&aff);
  return;
}



void TBeing::add_faction(const char * args) {
  int idnum;
  idnum = get_unused_ID();
  TFaction *f = new TFaction;
  
  if (idnum == -1) {
    sendTo("It appears there is no room for more factions - sorry.\n\r");
    return; // no room for more factions
  }

  if (f->keywords)
    delete [] f->keywords;
  f->keywords = mud_str_dup(args);
  f->ID = idnum;

  int i;
 
  for (i = 0; i < NUM_MAX_RANK; i++) {

    switch(i) {
      case 0:
	f->permissions[i] = 
	  (PERM_RECRUIT | PERM_PROMOTE | PERM_TREASURER | PERM_EDIT | PERM_LOCK |
	   PERM_AMBASSADOR | PERM_SCRIBE);
        if (f->rank[i])
          delete [] f->rank[i];
        f->rank[i] = mud_str_dup("Leader");
	break;
      case 1:
        f->permissions[i] = (PERM_RECRUIT);
	if (f->rank[i])
	  delete [] f->rank[i];
	f->rank[i] = mud_str_dup("Member");
	break;
      case 2:
	f->permissions[i] = 0;
        if (f->rank[i])
          delete [] f->rank[i];
        f->rank[i] = mud_str_dup("New Recruit");
	break;
      default:
	f->permissions[i] = 0;
    }
  }

  for (i = 0; i < MAX_FACT_COLORS; i++) {
    f->colors[i] = 0;
  }
  f->flags = 0;
  f->treasury = 0;
  f->alignx = 0;
  f->aligny = 0;
  f->actx = 0;
  f->acty = 0;
  f->ranks = DEFAULT_RANKS;
  f->power = 0.0;
  f->patron = deityTypeT(0);
  faction_table.push_back(f);
  char buf[128];
  if (isImmortal()) {
    sprintf(buf,"Faction: '%s' added with unique ID #%d\n\r", f->keywords, f->ID);
    sendTo(buf);
  } else {
    faction.whichfaction = f->ID;
    faction.rank = 0; // leader slot
    saveFactionStats();
  }  
  vlogf(LOG_FACT, fmt("%s founded a new faction: [%s] (%d)") %  getName() % f->keywords % f->ID);
  save_newfactions();
  return;
}



bool TBeing::canCreateFaction(bool silent = false) {
  char buf[256];
  if(isImmortal())
    return TRUE;
  if(inRoom() != ROOM_FACTION_BUREAU) {
    if (!silent) {
      sendTo("You must be in the Grimhaven Bureau of Factions to create a new faction\n\r");
    }
    return FALSE;
  }
  if(faction.whichfaction) {
    if (!silent) {
      sprintf(buf, "You are already a member of %s.\n\r", newfaction()->getName());
      sendTo(COLOR_BASIC, buf);
      sendTo("You must first disband from that faction before you may create another.\n\r");
    }      
    return FALSE;
  }
  if(!hasQuestBit(TOG_HAS_PAID_FACT_FEE)) {
    if (!silent) {
      sendTo("You have not yet paid the fee to register a new faction.\n\r");
      sendTo("Until that time, you may not create a faction.\n\r");
    }
    return FALSE;
  }
  if(hasQuestBit(TOG_HAS_CREATED_FACTION)) {
    if(!silent) {
      sendTo("You have already created a faction!\n\r");
      sendTo("The King forbids me to let players create more than one faction.\n\r");
    }
    return FALSE;
  }
  return TRUE;
}

// spec_mob proc for Miya in the bureau
int factionRegistrar(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  char field[80], values[80];
  char buf[256];
  char tell[256];

  strcpy(values, "");
  
  if(!ch || !ch->awake() || ch->fight())
    return FALSE;
  

  if(cmd == CMD_FEDIT) {
    int count = sscanf(arg, "%s %[0-9a-z-A-Z '<>]", field, values);
    if (!is_abbrev(field, "create")) {
      return FALSE;
    }
    if (count == 1) {
      myself->doTell(fname(ch->name), "If you want me to help you, you have to use the right syntax.");
      myself->doTell(fname(ch->name), "Try 'fedit create <keywords>'");
      return TRUE;
    }
    myself->doTell(fname(ch->name), "Ah, so you wish to found a new faction?");
    myself->doTell(fname(ch->name), "Let me just make sure your paperwork is in order.");
    
    
    if(ch->isImmortal()) {
      myself->doTell(fname(ch->name), "Actually... for an immortal I think I can skip the paperwork.");
      myself->doAction(fname(ch->name), CMD_SMILE);
    } else {
      if(ch->inRoom() != ROOM_FACTION_BUREAU) {
	myself->doAction("", CMD_BLINK);
	myself->doSay("Uh, it seem I've misplaced my office.");
	myself->doTell(fname(ch->name), "Tell ya what, I'll meet you there, and then we'll go over your paperwork.");
	act("$n hurries off back to $s office.", 0, myself, 0, 0, TO_ROOM);
	--(*myself);
	*real_roomp(ROOM_FACTION_BUREAU) += *myself;
	act("$n hurries into the office.", 0, myself, 0, 0, TO_ROOM); 
	return TRUE;
      }
      act("$n gets a folder from a filing cabinet in the corner.", 0, myself, 0, 0, TO_ROOM);
      act("$n quickly scans a few of the pages from the file.", 0, myself, 0, 0, TO_ROOM);
      if(ch->GetMaxLevel() < FACTION_CREATE_LEVEL) {
	myself->doSay("Awwww, I'm sorry, it appears you're too low level to create a faction.");
	myself->doAction(fname(ch->name), CMD_COMFORT);
	myself->doSay("You can come back later when you've become more powerful.");
	return TRUE;
      }
      if(ch->faction.whichfaction) {
	sprintf(buf, "Hmmmn.  %s, it appears you are already a member of a faction.", ch->getName());
	myself->doSay(buf);
	myself->doAction("", CMD_SHAKE);
	sprintf(buf, "You must first defect from your current faction before creating another.");
	myself->doSay(buf);
	sprintf(buf, "There is also a twenty-four hour wait period after you disband.");
	myself->doSay(buf);
	sprintf(buf, "After that, you may come back and create a faction.");
	myself->doAction(fname(ch->name), CMD_SMILE);
	
	return TRUE;
      }
      if(ch->recentlyDefected()) {
	myself->doAction("", CMD_FROWN);
	sprintf(buf, "You recently defected from your faction, and wont be able to create another.");
	myself->doSay(buf);
	sprintf(buf, "Well, at least until the waiting period is over.");
	myself->doSay(buf);
	//	myself->doActiom("", CMD_SHRUG);
	return TRUE;
      }
      if(!ch->hasQuestBit(TOG_HAS_PAID_FACT_FEE)) {
	myself->doAction("", CMD_FROWN);
	sprintf(buf, "It appears you have not paid the factions registration fee.");
	myself->doSay(buf);
	sprintf(buf, "The fee is 100000 talens, payable to me.");
	myself->doSay(buf);
	
	return TRUE;
      }
      if(ch->hasQuestBit(TOG_HAS_CREATED_FACTION)) {
	myself->doAction("", CMD_ARCH);
	sprintf(buf, "My records show that you have already created a faction..");
	myself->doSay(buf);
	sprintf(buf, "The King has forbidden us to let players create more than one faction.");
	myself->doSay(buf);
	
	return TRUE;
      }
      
    }
    sprintf(buf, "Well, it appears you check out.");
    myself->doSay(buf);
    myself->doAction("", CMD_SMILE);
    sprintf(buf, "Lets get started on the forms.");
    myself->doSay(buf);
    act("$n gets a sheet of paper and a pen from the desk.",
	FALSE, myself, 0, 0, TO_ROOM);
    sprintf(buf, "The only thing I need from you are the keywords for your new faction.");
    myself->doSay(buf);
    sprintf(buf, "Keywords will be used to identify your faction, and can be changed later.");
    myself->doSay(buf);
    sprintf(buf, "What do you want the keywords for your new faction to be?");
    myself->doSay(buf);
    ch->doSay(values);
    ch->removeOffers();
    sprintf(buf, "Excellent.");
    myself->doSay(buf);
    act("$n jots down a few notes on $s paper.",
	FALSE, myself, 0, 0, TO_ROOM);
    ch->add_faction(values);
    sprintf(buf, "Ok, I've created your faction with those keywords, and added you to the roster.");
    myself->doSay(buf);
    sprintf(buf, "Naturally, you have been given the top spot.");
    myself->doSay(buf);
    act("$n carefully places the paper into a folder, and files it away in one of the cabinets.",
	FALSE, myself, 0, 0, TO_ROOM);
    sprintf(buf, "The rest is up to you.  I suggest you read up on HELP FEDIT and HELP FACTIONS.");
    myself->doSay(buf);
    myself->doAction(fname(ch->name), CMD_SHAKE);
    sprintf(buf, "Good luck with your new faction.");
    myself->doSay(buf);
    ch->saveFactionStats();
    return TRUE;
    
  } else if (cmd == CMD_JOIN) {
    
    
    if(!TestCode5) {
      ch->sendTo("The new faction system is currently disabled.  You may not join a faction now.");
      return TRUE;
    }
    
    
    TFaction *f = NULL;
    sprintf(buf, "You wish to join a faction?  Lets see....");
    myself->doSay(buf);
    act("$n gets a folder from a filing cabinet along the wall.", 0, myself, 0, 0, TO_ROOM);
    act("$n quickly scans a few of the pages from the file.", 0, myself, 0, 0, TO_ROOM);
    
    
    
    if(!(f = get_faction(arg)) || (IS_SET(f->flags, FACT_HIDDEN) && !ch->hasOffer(f)) ||
       (!IS_SET(f->flags, FACT_ACTIVE)) || f->ID == 0) {
      // logic: faction doesn't exist, faction is hidden and hasn't extended an offer, or
      // faction is inactive - deny.
      
      myself->doTell(fname(ch->name), "I'm sorry, it appears that faction does not show up in any of my records.");
      
      return TRUE;
    }
    if(ch->faction.whichfaction) {
      myself->doTell(fname(ch->name), "You are already a member of a faction... you'll have to disband before you join another.");
      myself->doTell(fname(ch->name), "There is also a twenty four hour wait period before you may join another faction.");
      
      return TRUE;
    }
    if(ch->recentlyDefected()) {
      myself->doTell(fname(ch->name), "You recently defected from your faction, you'll have to wait to join another.");
      return TRUE;
    }
    
    if(!ch->hasOffer(f)) {
      myself->doTell(fname(ch->name), fmt("%s has not extended a recruitment offer to you.") % f->getName());
      if(!IS_SET(f->flags, FACT_OPEN_RECRUITMENT)) {
	return TRUE;
      } else {
	myself->doTell(fname(ch->name), "However, they offer open recruitment, so I can sign you up anyway.");
      }
      
    }
    sprintf(buf, "Well it looks like everything checks out, lets add you to the roster.");
    myself->doSay(buf);
    act("$n carefully places the paper into a folder, and files it away in one of the cabinets.",
	FALSE, myself, 0, 0, TO_ROOM);
    sprintf(buf, "Congratulations, your new title is %s of the %s.", f->rank[f->ranks - 1], f->getName());
    if (IS_SET(f->flags, FACT_HIDDEN)) {
      sprintf(tell, "%s %s", fname(ch->name).c_str(), buf);
      myself->doWhisper(tell);
    } else {
      myself->doSay(buf);
    }
    ch->faction.whichfaction = f->ID;
    ch->faction.rank = f->ranks - 1;
    myself->doAction(fname(ch->name), CMD_SHAKE);
    ch->saveFactionStats();
    return TRUE;
   
    
  }
  if (cmd == CMD_LIST) {
    myself->doTell(fname(ch->name), "I currently have these factions registered as active.");
    act("$n gets a sheet of paper from $s desk and holds it out to you.\n\rIt reads as follows:\n\r",
        FALSE, myself, 0, ch, TO_VICT);
    act("$n says something to $N.\n\r$n gets a sheet of paper from $s desk and holds it out to $N.\n\r",
        FALSE, myself, 0, ch, TO_NOTVICT);
    
    ch->show_faction("showallfactions");
    ch->sendTo("\n\r");
    myself->doTell(fname(ch->name), "I've marked the factions that have open recruitment with an [<R>X<1>].");
    return TRUE;
  }
  return FALSE;
}


// fedit command
void TBeing::edit_faction(const char * args) {
  // there are two ways this will be called - god version and player version of the
  // command
  // thus, we need a variable for all the syntax stuff.
  char SYNTAX[30] = "";

  char buf[128];
  char fact[80];
  char field[80];
  char values[80];
  TFaction *f = NULL;

  // ok 'args' is going to be of the format:
  // <faction> <field> <value(s)>
  if (isImmortal()) {
    strcpy(SYNTAX, "fedit <faction>");
    int count = sscanf(args, "%s %s %[0-9a-zA-Z '<>]", fact, field, values);
    if (!(*args)) {
      show_faction(NULL);
      return;
    } else if (count == 1) {
      if(is_abbrev(fact, "save")) {
	save_newfactions();
	sendTo("Factions saved.\n\r");
	return;
      }
      show_faction(fact);
      return;
    } else if (count == 2) {
      sendTo("Syntax: fedit <faction> [<field> <value(s)>]\n\r");
      return;
    }
    f = get_faction(fact);
    if (!f) {
      sprintf(buf,"Unable to find faction '%s'\n\r", fact);
      sendTo(buf);
      return;
    }
  } else {
    if(!TestCode5) {
      sendTo("The new faction code is currently disabled.  You will not be able to use this command.\n\r");
      return;
    }
    strcpy(SYNTAX, "fedit");
    int count = sscanf(args, "%s %[0-9a-z-A-Z '<>]", field, values);
    if (is_abbrev(field, "create")) {
      sendTo("You must speak to the Faction Registrar if you wish to create a faction.\n\r");
      sendTo("She can be found in the Grimhaven Bureau of Factions.\n\r");
      return;
#if 0
      // removed this in favor of the registrar proc
      if(canCreateFaction()) {
	add_faction(values);
	return;
      } else {
	return;
      }
    
#endif
    }
    f = newfaction();
    
    if (!f || f->ID == 0) {
      sprintf(buf,"You are not a member of any faction.\n\r");
      sendTo(buf);
      return;
    }
    if (!(*args)) {
      sprintf(buf,"%d", f->ID);
      show_faction(buf);
      return;
    } else if (count == 1) {
      if(is_abbrev(fact, "save")) {
        save_newfactions();
        sendTo("Faction saved.\n\r");
	return;
      }
      sendTo("Syntax: fedit [<field> <value(s)>]\n\r");
      return;
    }
    
  }
  

  // ok, so we matched all 3 parts
  // now we need to figure out which faction we have
  // fact will either be a number or keywords
  
  
  // ok, we have the faction, lets determine which field to edit
  
  if (is_abbrev(field, "name")) {
    if(f->proper_name)
      delete [] f->proper_name;
    f->proper_name = mud_str_dup(values);
    sprintf(buf,"Faction name changed to %s\n\r", values);
    sendTo(COLOR_BASIC, buf);
    return;
  } else if (is_abbrev(field, "save")) {
    save_newfactions();
    sendTo("Saved.\n\r");
    return;
  } else if (is_abbrev(field, "shortname")) {
    if(f->slang_name)
      delete [] f->slang_name;
    f->slang_name = mud_str_dup(values);
    sprintf(buf,"Faction short name changed to %s\n\r", values);
    sendTo(COLOR_BASIC, buf);
    return;

  } else if (is_abbrev(field, "keywords")) {
    if(f->keywords)
      delete [] f->keywords;
    f->keywords = mud_str_dup(values);
    sprintf(buf,"Faction keywords changed to %s\n\r", values);
    sendTo(buf);
    return;

  } else if (is_abbrev(field, "password")) {
    char passwd[80];
    if(sscanf(values, "%s %s", passwd, buf) != 2)
      strcpy(passwd, values);
    if(f->password)
      delete [] f->password;
    f->password = mud_str_dup(values);
    sprintf(buf,"Faction password changed to %s\n\r", passwd);
    sendTo(buf);
    return;

  } else if (is_abbrev(field, "numranks")) {
    int number;
    if(sscanf(values, "%d", &number) != 1) {
      sprintf(buf,"Syntax: fedit <faction> numranks <ranks>\n\r");
      sendTo(buf);
      return;
    }
    if (number < 1 || number > NUM_MAX_RANK) {
      sprintf(buf, "<ranks> must be between 1 and %d.\n\r", NUM_MAX_RANK);
      sendTo(buf);
      return;
    }
    f->ranks = number;
    sprintf(buf,"Number of ranks in faction changed to %d.\n\r", number);
    sendTo(buf);
    return;

  } else if (is_abbrev(field, "ranks")) {
    // format: <faction> ranks <rank#> <title>
    char title[80];
    int which;
    if(sscanf(values, "%d %[a-zA-Z '<>]", &which, title) != 2) {
      sprintf(buf,"Syntax: %s ranks <rank#> <title>\n\r", SYNTAX);
      sendTo(buf);
      return;
    }
    if(which < 1 || which > f->ranks) {
      sprintf(buf, "<rank#> must be between 1 and %d.\n\r", f->ranks);
      sendTo(buf);
      return;
    }
    if(f->rank[which-1])
      delete [] f->rank[which-1];
    f->rank[which-1] = mud_str_dup(title);
    sprintf(buf,"Rank #%d changed to %s\n\r", which, title);
    sendTo(COLOR_BASIC,buf);
    return;

  } else if (is_abbrev(field, "permissions")) {
    // format fedit <faction> permissions <rank> <rptelas>
    unsigned int perms = 0;
    int which;
    char bits[20];
    if(sscanf(values, "%d %[rptelasRPTELAS ]", &which, bits) != 2) {
      sprintf(buf,"Syntax: %s permissions <rank#> [rptelas]\n\r", SYNTAX);
      sendTo(buf);
      sendTo("Read HELP FEDIT for details on what the permissions do.\n\r");
      return;
    }

    if(which < 1 || which > f->ranks) {
      sprintf(buf, "<rank#> must be between 1 and %d.\n\r", f->ranks);
      sendTo(buf);
      return;
    }
    if (strchr(bits, 'r') || (strchr(bits, 'R')))
      SET_BIT(perms, PERM_RECRUIT);
    if (strchr(bits, 'p') || (strchr(bits, 'P'))) 
      SET_BIT(perms, PERM_PROMOTE);
    if (strchr(bits, 't') || (strchr(bits, 'T')))
      SET_BIT(perms, PERM_TREASURER);  
    if (strchr(bits, 'e') || (strchr(bits, 'E')))
      SET_BIT(perms, PERM_EDIT);
    if (strchr(bits, 'l') || (strchr(bits, 'L')))
      SET_BIT(perms, PERM_LOCK);
    if (strchr(bits, 'a') || (strchr(bits, 'A')))
      SET_BIT(perms, PERM_AMBASSADOR);
    if (strchr(bits, 's') || (strchr(bits, 'S')))
      SET_BIT(perms, PERM_SCRIBE);
    f->permissions[which-1] = perms;
    
    sendTo("Permissions updated.\n\r");
    sprintf(buf, "New permissions for rank #%d: %s\n\r", which, display_permission(perms));
    return;
    
  } else if (is_abbrev(field, "colors")) {
    int i,j;
    char c[3][20];
    if(sscanf(values, "%s %s %s", c[0], c[1], c[2]) != 3) {
      sprintf(buf,"Syntax: %s  colors <color> <color> <color>\n\r", SYNTAX);
      sendTo(buf);
      return;
    }
    for (i = 0; i < 3; i++) {
      j = 0;
      while (strcmp(heraldcolors[j], "\n")) {
	if (is_abbrev(c[i], heraldcolors[j])) {
	  f->colors[i] = j;
	  break;
	}
	j++;
      }
    }
    sprintf(buf, "Colors for %s set to %s, %s, and %s.\n\r", f->getName(), heraldcolors[f->colors[0]], 
	    heraldcolors[f->colors[1]],  heraldcolors[f->colors[2]]);
    sendTo(COLOR_BASIC,buf);
    return;
  } else if (is_abbrev(field, "alignment")) {
    sendTo("Not yet implemented.\n\r");
  } else if (is_abbrev(field, "flags")) {

    unsigned int whichbit;
    if (is_abbrev(values, "active")) {
      if(!isImmortal()) {
	sendTo("Mortals are not allowed to change this setting.\n\r");
	sendTo("Please speak to a god if your faction is ready to be activated.\n\r");
	return;
      }
      whichbit = FACT_ACTIVE;
    } else if (is_abbrev(values, "locked")) {
      if(!isImmortal()) {
        sendTo("Mortals are not allowed to change this setting.\n\r");
        sendTo("Please speak to a god if your faction settings needs to be locked or unlocked.\n\r");
        return;
      }
      whichbit = FACT_LOCKED;
    } else if (is_abbrev(values, "recruitment")) {
      whichbit = FACT_OPEN_RECRUITMENT;
    } else if (is_abbrev(values, "hidden")) {
      whichbit = FACT_HIDDEN;
    } else if (is_abbrev(values, "members")) {
      whichbit = FACT_HIDE_MEMBERS;
      //    } else if (is_abbrev(values, "leaders")) {
      //      whichbit = FACT_HIDE_LEADERS;
    } else if (is_abbrev(values, "ranks")) {
      whichbit = FACT_HIDE_RANKS;
    } else {
      if (!isImmortal())
	sendTo("Syntax: fedit flags [ recruitment | hidden | members | ranks ]");
      else
	sendTo("Syntax: fedit <faction> flags [ active | locked | recruitment | hidden | members | ranks ]");
      return;
    }
    
    if(IS_SET(f->flags, whichbit)) {
      REMOVE_BIT(f->flags, whichbit);
      sendTo("Flag removed.\n\r");
    } else {
      SET_BIT(f->flags, whichbit);
      sendTo("Flag added.\n\r");
    }
    sprintf(buf, "<c>Faction Flags:<1>");
    sendTo(COLOR_BASIC, buf);
    sendTo(COLOR_BASIC, display_faction_flags(f->flags));
    sendTo("\n\r");
    return;
  } else if (is_abbrev(field, "relations")) {
    char fname[80];
    char frela[80];
    if(sscanf(values, "%s %s", fname, frela) != 2) {
      sprintf(buf, "%s relations <faction> [ war | peace | none ]n\r", SYNTAX);
      sendTo(buf);
      return;
    }
    TFaction *f2 = get_faction(fname);
    if (!f2) {
      sendTo("Could not locate the target faction.\n\r");
      return;
    }
    if (is_abbrev(frela, "war")) {
      f->setRelation(f2->ID, RELATION_WAR);
      sprintf(buf, "%s<1> has declared <o>war<1> on %s<1>!\n\r",
	      f->getName(), f2->getName());
      sendTo(COLOR_BASIC, buf);
      return;
    } else if (is_abbrev(frela, "peace")) {
      f->setRelation(f2->ID, RELATION_PEACE);
      sprintf(buf, "%s<1> has called for <c>peace<1> with %s<1>!\n\r",
	      f->getName(), f2->getName());
      sendTo(COLOR_BASIC, buf);
      return;
    } else if (is_abbrev(frela, "none")) {
      f->setRelation(f2->ID, RELATION_NONE);
      sprintf(buf, "%s<1> has withdrawn all relations with %s<1>!\n\r",
              f->getName(), f2->getName());
      sendTo(COLOR_BASIC, buf);
      return;
    } 
    sprintf(buf, "%s relations <faction> [ war | peace | none ]n\r", SYNTAX);
    sendTo(buf);
    return;
  } else if (is_abbrev(field, "patron")) {
    char dname[80];
    if(sscanf(values, "%s %s", dname, buf) != 2)
      strcpy(dname, values);
    int i;
    for(i = 0; i < MAX_DEITIES; i++) {
      if(isname(sstring(dname).lower(), sstring(deities[i]).lower())) {
        f->patron = deityTypeT(i);
        sprintf(buf, "The patron deity for %s<1> has been set to %s.\n\r", f->getName(), deities[i]);
	sendTo(COLOR_BASIC, buf);
	return;
      }
    }
    sendTo("Invalid deity name, HELP PANTHEON to list valid deities.\n\r");
    return;
  }
  sendTo("Invalid option - HELP FEDIT for valid choices.\n\r");
  return;
}

void TBeing::show_faction(const char * args) {
#if 0
  vlogf(LOG_DASH, fmt("show_faction() called with args = %s") %  args);
#endif
  char buf[4096];
  if (args && strcmp(args, "showallfactions")) {
    TFaction *f = NULL;
    f = get_faction(args);
    if (!f) {
      sendTo("Unable to find faction by that name or ID.\n\r");
      return;
    } 
    
    sprintf(buf, "<c>Faction ID:<1> %-5d <1><c>Keywords:<1> [%s]\n\r", 
	    f->ID, (f->keywords) ? f->keywords : "(null)");
    sendTo(COLOR_BASIC,buf);
    
    sprintf(buf, " <1><c>Name:<1> %s   <1><p>Power:<1> %5.2f\n\r",
	    (f->proper_name) ? f->proper_name : "(null)", f->power);
    sendTo(COLOR_BASIC, buf);
    
    sprintf(buf, "<1><c> Short Name:<1> %s   <1><c>Password:<1> %s\n\r",
	    (f->slang_name) ? f->slang_name : "(null)",
	    (f->password) ? f->password : "(null)");
    sendTo(COLOR_BASIC,buf);
    
    sprintf(buf, "<1><c> Treasury:<1> %d talen%s \n\r<1><c>Faction Flags: <1>",
	    f->treasury, (f->treasury == 1) ? "" : "s");//, display_faction_flags(f->flags));
    sendTo(COLOR_BASIC,buf);
    sendTo(COLOR_BASIC, display_faction_flags(f->flags));
    sendTo("\n\r");

    int j;

    sendTo(COLOR_BASIC, "<1><c>Rank  Permissions  Title:\n\r");
    for (j = 0; j < f->ranks; j++) {
      sprintf(buf,"<1><p> #%-3d <k>%-10s  <1> %s\n\r", (j+1), display_permission(f->permissions[j]),
	      (f->rank[j]) ? f->rank[j] : "(null)");
      sendTo(COLOR_BASIC,buf);
    }
    sprintf(buf,"<1><c>Colors:<1> %s%s<1>, %s%s<1>, and %s%s<1>\n\r",
      heraldcodes[f->colors[0]], heraldcolors[f->colors[0]],
      heraldcodes[f->colors[1]], heraldcolors[f->colors[1]], 
      heraldcodes[f->colors[2]], heraldcolors[f->colors[2]]);
    sendTo(COLOR_BASIC,buf);
    int relationcount = 0;
    sendTo(COLOR_BASIC, "<1><c>Relations:<1>\n\r");
    vector<TFaction *>::iterator i;
    TFaction *f2 = NULL;
    for( i = faction_table.begin(); i != faction_table.end(); ++i) {
      f2 = (*i);
      if (f->getRelation(f2->ID) == RELATION_NONE && f2->getRelation(f->ID) == RELATION_NONE)
	continue; // no relations, so don't display
      else if (f->getRelation(f2->ID) == RELATION_NONE && f2->getRelation(f->ID) == RELATION_PEACE)
        sprintf(buf," An offer of <c>peace<1> has been extended to us by %s<1>\n\r", f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_NONE && f2->getRelation(f->ID) == RELATION_WAR)
	sprintf(buf," <r>War<1> has been declared on us by %s<1>!\n\r", f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_PEACE && f2->getRelation(f->ID) == RELATION_NONE)
	sprintf(buf," We have extended an offer of <c>peace<1> to %s<1>.\n\r", f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_PEACE && f2->getRelation(f->ID) == RELATION_PEACE)
	sprintf(buf," We have formed an <C>alliance of peace<1> with %s<1>\n\r", f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_PEACE && f2->getRelation(f->ID) == RELATION_WAR)
	sprintf(buf," We have offered <c>peace<1> with %s<1>, but they have declared <r>war<1> on us.\n\r",
		f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_WAR && f2->getRelation(f->ID) == RELATION_NONE)
	sprintf(buf," We have declared <r>war<1> on %s<1>\n\r", f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_WAR && f2->getRelation(f->ID) == RELATION_PEACE)
	sprintf(buf," We have declared <r>war<1> on %s<1>, but they have offered <c>peace<1> with us.\n\r",
		f2->getName());
      else if (f->getRelation(f2->ID) == RELATION_WAR && f2->getRelation(f->ID) == RELATION_WAR)
	sprintf(buf," We are <R>at war<1> with %s<1>.\n\r", f2->getName());
      
      sendTo(COLOR_BASIC, buf);
      relationcount++;
    } 
    if (!relationcount) {
      sendTo(" None!\n\r");
    }
    return;
  }

  sprintf(buf, "<c>ID          Name\n\r");
  sendTo(COLOR_BASIC, buf);
  vector<TFaction *>::iterator i;
  for(i = faction_table.begin();i != faction_table.end(); ++i) {
    if (isImmortal() || (*i) == newfaction() ||
	!(IS_SET((*i)->flags, FACT_HIDDEN) || (*i)->ID == 0 || !IS_SET((*i)->flags, FACT_ACTIVE))) {
      sprintf(buf, "<1>%-4d%s%s<1>%s%s<1>%s%s<1> [<R>%s<1>] <1>%s%s<k>%s%s%s<1>\n\r", (*i)->ID, 
	      heraldcodes[(*i)->colors[0]], ((*i)->colors[0]) ? "*" : " ",
	      heraldcodes[(*i)->colors[1]], ((*i)->colors[1]) ? "*" : " ",
	      heraldcodes[(*i)->colors[2]], ((*i)->colors[2]) ? "*" : " ",
	      (IS_SET((*i)->flags, FACT_OPEN_RECRUITMENT)) ? "X" : " ",
	      ((*i)->proper_name) ? (*i)->proper_name : "",
	      ((*i)->proper_name) ? " " : "",
	       isImmortal() ? "[" : "",
	      (isImmortal() ? (((*i)->keywords) ? (*i)->keywords : "(null)") : ""),
	      isImmortal() ? "]" : "");

      sendTo(COLOR_BASIC,buf);
    }
  }

  return;
}

TFaction * TBeing::newfaction() const {
  return get_faction_by_ID(faction.whichfaction);
}

const char * TBeing::rank() {
  return newfaction()->rank[faction.rank];
}

bool TBeing::hasPermission(unsigned int bit) {
  return IS_SET(newfaction()->permissions[faction.rank-1], bit);
}

int TFaction::getRelation(TFaction * target) {
  return getRelation(target->ID);
}

int TFaction::getRelation(int target) {
  vector<TRelation *>::iterator i;
  for (i = relations.begin(); i != relations.end(); ++i) {
    if((*i)->targ_fact == target) {
      return (*i)->relation;
    }
  }
  return RELATION_NONE;
}
 
void TFaction::setRelation(TFaction * target, int state) {
  setRelation(target->ID, state);
}

void TFaction::setRelation(int target, int state) {
  vector<TRelation *>::iterator i;
  for (i = relations.begin(); i != relations.end(); ++i) {
    if((*i)->targ_fact == target) {
      if (state == RELATION_NONE) {
	relations.erase(i);
	return;
      }
      (*i)->relation = state;
      return;
    }
  }
  if (state == RELATION_NONE)
    return;
  
  TRelation *r = new TRelation;
  r->targ_fact = target;
  r->relation = state;
  relations.push_back(r);
  return;
}


TFaction * get_faction(const char *args) {
  int num = 0;
  int count = 0;
  vlogf(LOG_DASH, fmt("get_faction called with args = '%s'") %  args);
  count = sscanf(args,"%d", &num);
  if(count)
    return get_faction_by_ID(num);
  else
    return get_faction_by_keywords(args);
  return NULL;
}

TFaction * get_faction_by_ID(int idnum) {
  vector<TFaction *>::iterator i;
  for (i = faction_table.begin();i != faction_table.end();++i) {
    if ((*i)->ID == idnum) {
      return (*i);
    }
  }
  return NULL;
}

TFaction * get_faction_by_keywords(const char * args) {
  vector<TFaction *>::iterator i;
  for (i = faction_table.begin(); i != faction_table.end();++i) {
    if (isname(args, (*i)->keywords)) {
      return (*i);
    }
  }
  return NULL;
}


// this function takes an unsigned int and converts it to a char sstring
// to display when showing permissions for each rank
char * display_permission(unsigned int perms) {
  char buf[10];
  sprintf(buf, "%s%s%s%s%s%s%s",
	  IS_SET(perms, PERM_RECRUIT) ?    "r" : " ",
	  IS_SET(perms, PERM_PROMOTE) ?    "p" : " ",
	  IS_SET(perms, PERM_TREASURER) ?  "t" : " ",
	  IS_SET(perms, PERM_EDIT) ?       "e" : " ",
	  IS_SET(perms, PERM_LOCK) ?       "l" : " ",
	  IS_SET(perms, PERM_AMBASSADOR) ? "a" : " ",
	  IS_SET(perms, PERM_SCRIBE) ?     "s" : " ");
  return mud_str_dup(buf);
}

char * display_faction_flags(unsigned int flags) {
  char buf[256];
  sprintf(buf, "%s%s%s%s%s%s%s",
	  (IS_SET(flags, FACT_ACTIVE) ?  
	   "\n\r This faction is activated." : "\n\r This faction is NOT activated."),
	  (IS_SET(flags, FACT_LOCKED) ?    
	   "\n\r This faction's attributes are locked." : ""),
	  (IS_SET(flags, FACT_OPEN_RECRUITMENT) ?
	   "\n\r This faction is openly recruiting." : "\n\r New members must be recruited by hand."),
	  (IS_SET(flags, FACT_HIDDEN) ?      
	   "\n\r The existance of this faction is hidden." : ""),
	  (IS_SET(flags, FACT_HIDE_MEMBERS) ?
	   "\n\r The members of this faction are hidden from the masses." : ""),
	  (IS_SET(flags, FACT_HIDE_LEADERS) ?
	   "\n\r The leaders of this faction are hidden from the masses." : ""),
	  (IS_SET(flags, FACT_HIDE_RANKS) ? 
	   "\n\r The ranks of those in this faction are hidden from the masses." : "" ));
  return mud_str_dup(buf);
}

// get_unused_ID finds an unused faction ID (0 to MAX_FACT_ID) and returns it.
// if all ID's are currently taken (ie 200 factions - shouldn't happen)
// then it returns -1
int get_unused_ID() {
  int i, j;
  bool found = FALSE;
  for (i = 0; i <= MAX_FACT_ID; i++) {
    for (j = 0;(int)j < (int)faction_table.size(); j++) {
      found = FALSE;
      if(faction_table[j]->ID == i) {
	found = TRUE;
	break;
      }
    }
    if (!found)
      return i;
  }
  return -1;
}


// this function loads the faction info for the PLAYER.
void TBeing::saveFactionStats()
{
  FILE *fp;
  char buf[160];
  int current_version = 1;

  if (!isPc() || !desc)
    return;

  sprintf(buf, "player/%c/%s.faction", LOWER(name[0]), sstring(name).lower().c_str());

  if (!(fp = fopen(buf, "w"))) {
    vlogf(LOG_FILE, fmt("Unable to open file (%s) for saving faction stats. (%d)") %  buf % errno);
    return;
  }
  if(!get_faction_by_ID(faction.whichfaction)) {
    vlogf(LOG_FACT, fmt("%s had bad faction during saveFactionStats() ... making unaffiliated") %  getName());
    faction.whichfaction = 0;
    faction.rank = 0;
  }

  if(faction.rank < 0 || faction.rank >= newfaction()->ranks) {
    vlogf(LOG_FACT, fmt("%s had bad rank - setting to lowest in faction.") %  getName());
    faction.rank = newfaction()->ranks - 1;
  }
  fprintf(fp, "%u\n",
	  current_version);

  fprintf(fp,"%d %d\n", faction.whichfaction, faction.rank);
  fprintf(fp,"%d %d\n", faction.align_ge, faction.align_lc);

  if (fclose(fp))
    vlogf(LOG_FILE, fmt("Problem closing %s's faction stats") %  name);
}

//this loads the faction data for the PLAYER
void TBeing::loadFactionStats()
{
  FILE *fp = NULL;
  char buf[160];
  int current_version;
  int num1, num2, num3, num4;

  if (!isPc() || !desc)
    return;

  sprintf(buf, "player/%c/%s.faction", LOWER(name[0]), sstring(name).lower().c_str());


  if (!(fp = fopen(buf, "r"))) {    // file may not exist
    return;
  }

  if (fscanf(fp, "%d\n",
	     &current_version) != 1) {
    vlogf(LOG_BUG, fmt("Bad data in faction stat read (%s)") %  getName());
    fclose(fp);
    return;
  }

  if (fscanf(fp, "%d %d\n", &num1, &num2) != 2) {
    vlogf(LOG_BUG, fmt("Bad data in factionss stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  faction.whichfaction = num1;
  if(!get_faction_by_ID(faction.whichfaction)) {
    vlogf(LOG_FACT, fmt("%s had bad faction during loadFactionStats() ... making unaffiliated") %  getName());
    faction.whichfaction = 0;
    faction.rank = 0;
  }
  faction.rank = num2;
  if(faction.rank < 0 || faction.rank >= newfaction()->ranks) {
    vlogf(LOG_FACT, fmt("%s had bad rank during loadFactionStats - setting to lowest in faction.") %  getName());
    faction.rank = newfaction()->ranks - 1;
  }

  if (fscanf(fp, "%d %d\n", &num3, &num4) != 2) {
    vlogf(LOG_BUG, fmt("Bad data in factionss stat read (%s)") %  getName());
    fclose(fp);
    return;
  }
  faction.align_ge = num3;
  faction.align_lc = num4;
  
  fclose(fp);
}

int load_newfactions() {
  FILE *fp;
  char buf[256];
  int i1, i2, i3, i4;
  float f1;
  char c1[80];
  int line = 0;

  if (!(fp = fopen(NEWFACT_FILE, "r"))) {
    vlogf(LOG_FILE, "Couldn't open factionlist file in function load_newfactions()!");
    return FALSE;
  }

  faction_table.clear();

  while(fp) {
    TFaction *f = new TFaction;
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);
      fclose(fp);
      return FALSE;
    }
    line++;
    if(strchr(buf,'$')) // eof
      break;
    sscanf(buf, "#%d\n\r", &i1);
    f->ID = i1;
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;

    strcpy(c1, ""); // just to make sure
    sscanf(buf, "keywords: %[a-zA-Z '<>]\n\r", c1);
    f->keywords = mud_str_dup(c1);
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    strcpy(c1, ""); // just to make sure
    sscanf(buf, "name: %[a-zA-Z '<>]\n\r", c1);
    f->proper_name = mud_str_dup(c1);
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;

    }
    line++;
    
    strcpy(c1, ""); // just to make sure
    sscanf(buf, "shortname: %[a-zA-Z '<>]\n\r", c1);
    f->slang_name = mud_str_dup(c1);
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }
    strcpy(c1, ""); // just to make sure
    sscanf(buf, "password: %s\n\r", c1);
    f->password = mud_str_dup(c1);
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    sscanf(buf, "%d %d %d %f\n\r", &i1, &i2, &i3, &f1);
    f->treasury = i1;
    f->ranks = i2;
    f->flags = (unsigned int)(i3);
    f->power = f1;
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    sscanf(buf, "%d %d %d %d\n\r", &i1, &i2, &i3, &i4);
    f->alignx = i1;
    f->aligny = i2;
    f->actx = i3;
    f->acty = i4;

    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    sscanf(buf, "%d %d %d\n\r", &i1, &i2, &i3);
    f->colors[0] = i1;
    f->colors[1] = i2;
    f->colors[2] = i3;

    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    sscanf(buf, "%d\n\r", &i1);
    f->patron = deityTypeT(i1);
    for(int j = 0; j < NUM_MAX_RANK; j++) {
      if(fgets(buf, 256, fp) == NULL) {
	vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

	fclose(fp);
	return FALSE;
      }

      line++;
      strcpy(c1, ""); // just to make sure
      sscanf(buf, "rank %d %d %[a-zA-Z '<>]\n\r", &i1, &i2, c1);
      f->rank[j] = mud_str_dup(c1);
      f->permissions[j] = (unsigned int)(i2);
    }
    TRelation *r;
    if(fgets(buf, 256, fp) == NULL) {
      vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

      fclose(fp);
      return FALSE;
    }

    line++;
    f->relations.clear();
    while(!strchr(buf, '!')) {
      r = new TRelation;
      sscanf(buf, "R %d %d\n\r", &i1, &i2);
      r->targ_fact = i1;
      r->relation = i2;
      f->relations.push_back(r);
      if(fgets(buf, 256, fp) == NULL) {
	vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: %d") %  line);

        fclose(fp);
        return FALSE;
      }

      line++;
    }
    faction_table.push_back(f);
  }
  fclose(fp);

  sprintf(buf, "cp %s %s", NEWFACT_FILE, NEWFACT_BAK);
  vsystem(buf);

  return TRUE;
}

int load_factions()
{
  FILE *fp;
  char buf[256];
  int num, j;
  int inum1, inum2, inum3, inum4;
  unsigned int uinum;
  long ln;
  float num1, num2;

  if (!(fp = fopen(FACTION_FILE, "r"))) {
    vlogf(LOG_FILE, "Couldn't open factionlist file in function load_factions()!");
    return FALSE;
  }
  for (factionTypeT i = MIN_FACTION;i < MAX_FACTIONS;i++) {
    if (fgets(buf,256,fp) == NULL) {
      vlogf(LOG_FILE,"ERROR: bogus line in FACTION_FILE");
      fclose(fp);
      return FALSE;
    }
    if (!strcmp(buf,"$"))   // EOF
      break;
    if (sscanf(buf,"#%d\n\r",&num) == 1) {   //   new faction
      if (fgets(buf,256,fp) == NULL) {
        vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: faction %d") % num);
        fclose(fp);
        return FALSE;
      }
      if (!strcmp(buf, "")) {
        vlogf(LOG_FILE, "ERROR: Null faction name.");
        fclose(fp);
        return FALSE;
      }
      // strip off the trailing newline
      buf[strlen(buf) - 1] = '\0';
      FactionInfo[i].faction_name = new char[strlen(buf) + 1];
      strcpy(FactionInfo[i].faction_name,buf);

      for (j = 0; j < FACT_LEADER_SLOTS;j++) {
        if (fgets(buf,256,fp) == NULL) {
          vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: faction %d") % num);
          fclose(fp);
          return FALSE;
        }
        // strip off the trailing newline
        buf[strlen(buf) - 1] = '\0';
        FactionInfo[i].leader[j] = new char[strlen(buf) + 1];
        strcpy(FactionInfo[i].leader[j],buf);
      }
      if (fgets(buf,256,fp) == NULL) {
        vlogf(LOG_FILE,fmt("ERROR: bogus line in FACTION_FILE: faction %d") % num);
        fclose(fp);
        return FALSE;
      }
      // strip off the trailing newline
      buf[strlen(buf) - 1] = '\0';
      FactionInfo[i].faction_password = new char[strlen(buf) + 1];
      strcpy(FactionInfo[i].faction_password,buf);

      factionTypeT ij;
      for (ij = MIN_FACTION; ij < MAX_FACTIONS; ij++) {
        if (fscanf(fp, "%f %f\n", &num1, &num2) != 2) {
          vlogf(LOG_FILE, fmt("ERROR: bogus faction array faction (%d) (j=%2)") % i %ij);
          fclose(fp);
          return FALSE;
        }
        FactionInfo[i].faction_array[ij][0] = (double) num1;
        FactionInfo[i].faction_array[ij][1] = (double) num2;
      }
      if (fscanf(fp, "%f\n", &num1) != 1) {
        vlogf(LOG_FILE, fmt("ERROR: bogus setting of faction power for faction(%d)") % i);
        fclose(fp);
        return FALSE;
      }
      FactionInfo[i].faction_power = (double) num1;
      if (fscanf(fp, "%ld %f\n", &ln, &num1) != 2) {
        vlogf(LOG_FILE, fmt("ERROR: bogus setting of faction wealth/tithe for faction(%d)") % i);
        fclose(fp);
        return FALSE;
      }
      FactionInfo[i].corp_id=ln;
      FactionInfo[i].faction_tithe = (double) num1;

      if (fscanf(fp, "%d %d %d %d\n", &inum1, &inum2, &inum3, &inum4) != 4) {
        vlogf(LOG_FILE, fmt("ERROR: bogus setting of faction caravan info 1 for faction(%d)") % i);
        fclose(fp);
        return FALSE;
      }
      FactionInfo[i].caravan_interval = inum1;
      FactionInfo[i].caravan_counter = inum2;
      FactionInfo[i].caravan_value = inum3;
      FactionInfo[i].caravan_defense = inum4;

      if (fscanf(fp, "%d %d %u\n", &inum1, &inum2, &uinum) != 3) {
        vlogf(LOG_FILE, fmt("ERROR: bogus setting of faction caravan info 2 for faction(%d)") % i);
        fclose(fp);
        return FALSE;
      }
      FactionInfo[i].caravan_attempts = inum1;
      FactionInfo[i].caravan_successes = inum2;
      FactionInfo[i].caravan_flags = uinum;
    }
  }
  fclose(fp);

  // data read was all ok, save this in case of corruption
  sprintf(buf, "cp %s %s", FACTION_FILE, FACTION_BAK);
  vsystem(buf);

  return TRUE;
}

void save_newfactions()
{
  FILE *fp;
  
  if (!(fp = fopen(NEWFACT_FILE, "w+"))) {
    vlogf(LOG_FILE, "Couldn't open newfactions datafile in save_newfactions()");
    return;
  }
  vector<TFaction *>::iterator i;
  TFaction *f = NULL;
  for(i = faction_table.begin(); i != faction_table.end(); ++i) {
    f = (*i);
    fprintf(fp, "#%d\n", f->ID);
    fprintf(fp, "keywords: %s\n", f->keywords);
    fprintf(fp, "name: %s\n", f->proper_name);
    fprintf(fp, "shortname: %s\n", f->slang_name);
    fprintf(fp, "password: %s\n", f->password);
    fprintf(fp, "%d %d %d %f\n", f->treasury, f->ranks, f->flags, f->power);
    fprintf(fp, "%d %d %d %d\n", f->alignx, f->aligny, f->actx, f->acty);
    fprintf(fp, "%d %d %d\n", f->colors[0], f->colors[1], f->colors[2]);
    fprintf(fp, "%d\n", (int)f->patron);
    for(int j = 0; j < NUM_MAX_RANK; j++) {
      fprintf(fp, "rank %d %d %s\n", j, f->permissions[j], f->rank[j]);
    }
    vector<TRelation *>::iterator k;
    for(k = f->relations.begin(); k != f->relations.end(); ++k) {
      fprintf(fp, "R %d %d\n", (*k)->targ_fact, (*k)->relation);
    }
    fprintf(fp, "!\n");
  }
  fprintf(fp, "$\n");
  fclose(fp);
}


void save_factions()
{
  FILE *fp;
  int j;

  if (!(fp = fopen(FACTION_FILE, "w+"))) {
    vlogf(LOG_FILE, "Couldn't open factionlist file in function load_factions()!");
    return;
  }
  for (factionTypeT i = MIN_FACTION;i < MAX_FACTIONS;i++) {
    fprintf(fp,"#%d\n",i);
    fprintf(fp,"%s\n",FactionInfo[i].faction_name);
    for (j = 0; j < FACT_LEADER_SLOTS;j++) {
      fprintf(fp,"%s\n",FactionInfo[i].leader[j]);
    }
    fprintf(fp,"%s\n",FactionInfo[i].faction_password);
    for (j = 0;j < MAX_FACTIONS;j++) {
      fprintf(fp, "%.1f %.1f\n",
            (float) FactionInfo[i].faction_array[j][0],
            (float) FactionInfo[i].faction_array[j][1]);
    }
    fprintf(fp, "%.4f\n", (float) FactionInfo[i].faction_power);
    fprintf(fp, "%d %.4f\n", FactionInfo[i].corp_id, FactionInfo[i].faction_tithe);
    fprintf(fp, "%d %d %d %d\n", FactionInfo[i].caravan_interval, 
             FactionInfo[i].caravan_counter,
             FactionInfo[i].caravan_value,
             FactionInfo[i].caravan_defense);
    fprintf(fp, "%d %d %u\n", FactionInfo[i].caravan_attempts, 
             FactionInfo[i].caravan_successes,
             FactionInfo[i].caravan_flags);
  }
  fprintf(fp,"$\n");
  fclose(fp);
}


factionTypeT factionNumber(const sstring name)
{
  if (is_abbrev(name,"brotherhood") || is_abbrev(name,"galek"))
    return FACT_BROTHERHOOD;
  else if (is_abbrev(name,"cult") || is_abbrev(name,"chaos") ||
            is_abbrev(name,"logrus"))
    return FACT_CULT;
  else if (is_abbrev(name,"unaffiliated") || is_abbrev(name,"none"))
    return FACT_NONE;
  else if (is_abbrev(name,"order") || is_abbrev(name,"serpents") ||
            is_abbrev(name,"snakes"))
    return FACT_SNAKE;
  else
    return FACT_UNDEFINED;
}

static const sstring factionLeaderTitle(factionTypeT faction, int slot)
{
  // display length for this is typically 30 chars
  char buf[64];

  mud_assert(faction >= MIN_FACTION && faction < MAX_FACTIONS,
    "factionLeaderTitle(): faction outside range %d", faction);
  mud_assert(slot >= 0 && slot < FACT_LEADER_SLOTS,
    "factionLeaderTitle(): slot outside range %d", slot);

  if (faction == FACT_CULT) {
    if (slot == 0)
      mud_str_copy(buf, "Grand Master", 64);
    else if (slot == 1)
      mud_str_copy(buf, "Zenith of Death", 64);
    else if (slot == 2)
      mud_str_copy(buf, "Magus of the Damned", 64);
    else if (slot == 3)
      mud_str_copy(buf, "Master Assassin", 64);
    else if (slot == 4)
      mud_str_copy(buf, "Unholy Cardinal",64);
  } else if (faction == FACT_BROTHERHOOD) {
    if (slot == 0)
      mud_str_copy(buf, "Chief Councillor of Three", 64);
    else if (slot == 1 || slot == 2)
      mud_str_copy(buf, "Councillor of Three", 64);
    else if (slot == 3)
      mud_str_copy(buf, "Master Guardian of the Steps", 64);
  } else if (faction == FACT_SNAKE) {
    if (slot == 0)
      mud_str_copy(buf, "Overlord of the Serpents", 64);
    else if (slot == 1)
      mud_str_copy(buf, "Lord of the Purse", 64);
    else if (slot == 2)
      mud_str_copy(buf, "Lord of the Shadow", 64);
    else if (slot == 3)
      mud_str_copy(buf, "Lord of the Fist", 64);
  } else {
    if (slot == 0)
      mud_str_copy(buf, "Faction Leader", 64);
    else if (slot == 1)
      mud_str_copy(buf, "Sub-Leader 1", 64);
    else if (slot == 2)
      mud_str_copy(buf, "Sub-Leader 2", 64);
    else if (slot == 3)
      mud_str_copy(buf, "Sub-Leader 3", 64);
  }

  return buf;
}

// for determining leadership position within the faction
// -1 if not a leader
// FALSE if a leader but lacking "power"
// TRUE if leader with "power"
int TBeing::getFactionAuthority(factionTypeT fnum, int power)
{
  if (isImmortal() && hasWizPower(POWER_WIZARD)) {
    return TRUE;
  }
  for (int i = 0;i < FACT_LEADER_SLOTS;i++) {
    if (!strcmp(getName(),FactionInfo[fnum].leader[i]))
      return (power >= i);
  }
  return -1;
}
 
void TBeing::doMakeLeader(const char *arg)
{
  char namebuf[100];
  factionTypeT fnum = getFaction();
  int which;
  charFile st;
  TBeing *vict;
  bool doNoone = FALSE; 

  arg = one_argument(arg, namebuf);
  if (!*namebuf) {
    sendTo("Whom do you wish to make a leader??\n\r");
    sendTo("Syntax: makeleader <name> <leader slot>\n\r");
    return;
  }
  if (!*arg) {
    sendTo("You need to define a leader_slot.\n\r");   
    sendTo("Syntax: makeleader <name> <leader slot>\n\r");
    return;
  }
  which = convertTo<int>(arg);
  /* Capitalize name and faction */
  strcpy(namebuf, sstring(namebuf).cap().c_str());

  if (!strcmp("Noone", namebuf) || !strcmp("noone", namebuf))
    doNoone = TRUE;

  if (!doNoone && !load_char(namebuf, &st)) {
    sendTo("No such person exists.\n\r");
    return;
  }

  if (getFactionAuthority(fnum,which) <= 0) {
    sendTo("You lack the authority to change that.\n\r");
    return;
  }
  if (!doNoone && st.f_type != fnum) {
    sendTo("You can't make them a leader, they aren't even a member.\n\r");
    return;
  }
  if (fnum == FACT_NONE) {
    sendTo(fmt("%s don't have or want leaders.\n\r") %
         sstring(FactionInfo[FACT_NONE].faction_name).cap());
    return;
  }
  if (which == 0 && doNoone) {
    sendTo("You must appoint a new head leader.\n\r");
    return;
  }
  if ((which < 0) || (which >= FACT_LEADER_SLOTS)) {
    sendTo("Syntax: makeleader <name> <leader slot>\n\r");
    return;
  } else {
    vlogf(LOG_FACT,fmt("Leader slot %d for faction %s changed.") % which %
           FactionInfo[fnum].faction_name);
    vlogf(LOG_FACT,fmt("Changed from %s to %s.") % FactionInfo[fnum].leader[which] %
           namebuf);
    sendTo(fmt("You have set %s's leader %d to %s.\n\r") % 
           FactionInfo[fnum].faction_name % which % namebuf);

    if (strcmp(namebuf, "Noone")) {
      strcpy(FactionInfo[fnum].leader[which],namebuf);
      if ((vict = get_char(namebuf, EXACT_YES)) ||
          (vict = get_char(namebuf, EXACT_NO))) {
        vict->sendTo(COLOR_MOBS, fmt("%s has made you leader %d of %s.\n\r") %
            getName() % which % FactionInfo[fnum].faction_name);
      }
    } else
      strcpy(FactionInfo[fnum].leader[which],"");

    save_factions();
    return;
  }
}

void TBeing::doNewMember(const char *arg)
{
  TBeing *vict;
  char namebuf[100];
  factionTypeT fnum = getFaction();

  if (isUnaff()) {
    sendTo(fmt("You can't apppoint members to %s.\n\r") %
               FactionInfo[FACT_NONE].faction_name);
    return;
  }
  if (getFactionAuthority(fnum,FACT_LEADER_SLOTS - 1) <= 0) {
    sendTo("You lack the authority to add members.\n\r");
    return;
  }

  arg = one_argument(arg, namebuf);
  if (!*namebuf) {
    sendTo("Whom do you wish to make a member of your faction??\n\r");
    sendTo("Syntax: newmember <name>\n\r");
    return;
  }
  if (!(vict = get_pc_world(this, namebuf, EXACT_YES))) {
    if (!(vict = get_pc_world(this, namebuf, EXACT_NO))) {
      sendTo("They don't want to be a member of a faction, they aren't here.\n\r");
      return;
    }
  }
  if (!vict->isPc()) {
    sendTo("Only PCs can be added.\n\r");
    return;
  } else if (vict->desc && !IS_SET(vict->desc->autobits, AUTO_JOIN)) {
    sendTo("That person doesn't want to join factions.\n\r");
    return;
  } else if (vict->getFaction() == fnum) {
    sendTo("That person is already a member of your faction.\n\r");
    return;
  } else if (!vict->isUnaff()) {
    sendTo("You can't add them, they are already a member somewhere else.\n\r");
    return;
  }
  vict->setFaction(fnum);

#if FACTIONS_IN_USE
  vict->setPerc(0.00);
  factionTypeT i;
  for (i = MIN_FACTION; i < MAX_FACTIONS; i++)
    vict->setPercX(0.0, i);
#endif

  vict->sendTo(fmt("You have been made a member of the %s.\n\r") %
        FactionInfo[fnum].faction_name);
  sendTo(COLOR_MOBS, fmt("You have added %s to the %s.\n\r") % vict->getName() %
        FactionInfo[fnum].faction_name);
  vlogf(LOG_FACT, fmt("Newmember: %s adding %s to %s.") % getName() %vict->getName() %
        FactionInfo[fnum].faction_name); 
}

void TBeing::doRMember(const char *arg)
{
  TBeing *vict = NULL;
  char namebuf[128];
  factionTypeT fnum = getFaction();
  int j;
  
  arg = one_argument(arg, namebuf);
  if (!*namebuf) {
    sendTo("Whom do you wish to remove as a member?\n\r");
    sendTo("Syntax: RMember <name>\n\r");
    return;
  }
  if (getFactionAuthority(fnum,FACT_LEADER_SLOTS - 1) <= 0) {
    sendTo("You lack the authority to remove members.\n\r");
    return;
  }
  if (fnum == FACT_NONE) {
    sendTo(fmt("You can't remove someone from %s!\n\r") %
           FactionInfo[FACT_NONE].faction_name);
    return;
  }
  if (!(vict = get_pc_world(this, namebuf, EXACT_YES))) {
    if (!(vict = get_pc_world(this, namebuf, EXACT_NO))) {
      sendTo("You can't remove that person, they aren't here.\n\r");
      return;
    }
  }
  if (vict == this) {
    sendTo("Use the disband command to remove yourself from a faction.\n\r");
    return;
  }
  if (vict->getFaction() != fnum) {
    sendTo("That person is not a member of that faction.\n\r");
    return;
  }
  for (j = 0;j < FACT_LEADER_SLOTS;j++) {
    if (!strcmp(FactionInfo[fnum].leader[j],vict->getName())) {
      sendTo("Sorry, that person is a leader of that faction and can't be removed.\n\r");
      sendTo(fmt("Appoint a new leader to slot %d and then you may remove them.\n\r") %j);
      return;
    }
  }
  vict->setFaction(FACT_NONE);
#if FACTIONS_IN_USE
  vict->setPerc(0.00);
  factionTypeT i;
  for (i = MIN_FACTION; i < MAX_FACTIONS; i++)
    vict->setPercX(0.0, i);
#endif

  vict->sendTo(fmt("You have been kicked out of the %s.\n\r") %
        FactionInfo[fnum].faction_name);
  sendTo(COLOR_MOBS, fmt("You have removed %s from the %s.\n\r") % vict->getName() %
        FactionInfo[fnum].faction_name);
  vlogf(LOG_FACT, fmt("RMember: %s removing %s from %s.") % getName() %vict->getName() %
        FactionInfo[fnum].faction_name);
}
  
void TBeing::doDisband()
{
  factionTypeT fnum = getFaction();

  if (fnum == FACT_NONE) {
    sendTo("You don't seem to be affiliated to any faction.\n\r");
    return;
  }

#if 0
  sendTo("No disbanding is allowed at the present time.\n\r");
  return;
#endif

  int ij;
  for (ij = 0;ij < FACT_LEADER_SLOTS;ij++) {
    if (!strcmp(FactionInfo[fnum].leader[ij],getName())) {
      sendTo("What?!?  And leave them leaderless?\n\r");
      sendTo(fmt("Appoint a new leader for slot %d and then you may disband.\n\r") %ij);
      return;
    }
  }
  sendTo(fmt("You have disbanded from %s.\n\r") %FactionInfo[fnum].faction_name);
  vlogf(LOG_FACT,fmt("Disband: %s left %s.") % getName() %FactionInfo[fnum].faction_name);
  setFaction(FACT_NONE);
#if FACTIONS_IN_USE
  setPerc(0.0);
  factionTypeT i;
  for (i = MIN_FACTION; i < MAX_FACTIONS; i++)
    setPercX(0.0, i);
#endif
}

void sendToFaction(factionTypeT fnum, const char *who, const char *arg)
{
  Descriptor *d, *d_next;
  TBeing *tmpch;

  for (d = descriptor_list; d; d = d_next) {
    d_next = d->next;
    if (d->connected)
      continue;

    tmpch = (d->original ? d->original : d->character);

    if ((tmpch->getFaction() != fnum) &&
        !tmpch->hasWizPower(POWER_SEE_FACTION_SENDS))
      continue;

    d->character->sendTo(COLOR_SHOUTS, fmt("<g>%s <c>%s<1>: %s\n\r") %
			 FactionInfo[fnum].faction_name % who % arg);

    if (!d->m_bIsClient && IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT))
      if (d->character->isImmortal())
        d->clientf(fmt("%d|%d|%s|%s") % CLIENT_FTELL % fnum % who % arg);
      else
        d->clientf(fmt("%d|%s|%s") % CLIENT_FTELL % who % arg);
  }
}


void TBeing::doSend(sstring arg)
{ 
  factionTypeT fnum = getFaction();
  sstring msg, faction, new_arg;

  // allow immortals to send to any faction via "send snake mess"
  bool wizSent = false;
  if (isImmortal()) {
    new_arg = one_argument(arg, faction);
    factionTypeT fnum2 = factionNumber(faction.c_str());
    if (fnum2 != FACT_UNDEFINED) {
      fnum = fnum2;
      wizSent = true;
      msg = new_arg;
    }
  }
  if (!wizSent)
    msg=arg;

#if 0
  if ((getFactionAuthority(fnum,FACT_LEADER_SLOTS - 1) <= 0) &&
      !isImmortal()) {
    sendTo("You lack the authority to send faction announcements.\n\r");
    return;
  }
#endif
  if (fnum == FACT_NONE) {
    sendTo("You are not a member of a faction.\n\r");
    return;
  }

  if (desc)
    desc->talkCount = time(0);

  if (applySoundproof())
    return;

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return;
  }
  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return;
  }

  if (isPc() && ((desc && IS_SET(desc->autobits, AUTO_NOSHOUT)) || isPlayerAction(PLR_GODNOSHOUT))) {
    sendTo("You can't send a faction message!!\n\r");
    return;
  }
  if (isAffected(AFF_CHARM) && master) {
    if (!isPc()) {
      sendTo("What a dumb master you have, charmed mobiles can't shout.\n\r");
      master->sendTo("Stop ordering your charms to shout.  *scold*  \n\r");
    } else {
      sendTo("You're charmed, you can't shout.\n\r");
    }
    return;
  }
  if (!dynamic_cast<TMonster *>(this) && (Silence == 1) && !isImmortal()) {
    sendTo("Faction messages has been banned.\n\r");
    return;
  }
  if (master && isAffected(AFF_CHARM)) {
    master->sendTo("I don't think so :-)\n\r");
    return;
  }
  if (rider) {
    rider->sendTo("I don't think so :-)\n\r");
    return;
  }
  if ((getMove() < 15) && isPc()) {
    sendTo("You don't have the energy to send a faction message!\n\r");
    return;
  }

  if (msg.empty()) {
    sendTo("What message do you wish to send?\n\r");
    sendTo("Syntax: send <message>\n\r");
    return;
  }
  if (isPc())
    addToMove(-15);

  addToWait(combatRound(0.5));


  sendToFaction(fnum, getName(), msg.c_str());
}

void TBeing::doRelease(const sstring & arg)
{
  TBeing *targ;
  sstring buf;

  one_argument(arg, buf);

  if (is_abbrev(buf, "all")) {
    for (targ = getCaptive(); targ; targ = getCaptive()) 
      doRelease(fname(targ->name));
    
    return;
  }
  if (!(targ = get_char_room_vis(this, buf))) {
    sendTo("Release whom?\n\r");
    return;
  }
  if (targ->getCaptiveOf() != this) {
    sendTo("They don't seem to be a captive of yours.\n\r");
    return;
  }

  sendTo(COLOR_MOBS, fmt("You release %s.\n\r") % targ->getName());
  act("$n releases you.", TRUE, this, 0, targ, TO_VICT);
  act("$n releases $N.", TRUE, this, 0, targ, TO_NOTVICT);
  remCaptive(targ);  
  targ->stopFollower(FALSE);
  return;
}

void TBeing::doCapture(const sstring & arg)
{
  TBeing *targ;
  sstring buf;

  one_argument(arg, buf);

  if (buf.empty()) {
    sendTo("Your captives: ");
    if (!getCaptive())
      sendTo("No one.\n\r");
    else {
      for(targ = getCaptive(); targ; targ = targ->getNextCaptive()) {
        sendTo(COLOR_MOBS, fmt(" %s") %targ->getName());
        if (targ->getNextCaptive())
          sendTo(",");
      }
      sendTo("\n\r");
    }
    return;
  }
  if (!(targ = get_char_room_vis(this, buf))) {
    sendTo("Capture whom?\n\r");
    return;
  }
  if (targ->getCaptiveOf()) {
    sendTo("They are someone else's captive already.\n\r");
    return;
  }
  if (targ->isImmortal()) {
    sendTo("I don't _THINK_ so...\n\r");
    return;
  }
  if (rider) {
    sendTo("You can't capture them while they are being ridden.\n\r");
    return;
  }
  if (targ->master) {
    targ->stopFollower(TRUE);
  }
  sendTo(COLOR_MOBS, fmt("You capture %s.\n\r") % targ->getName());
  targ->sendTo(COLOR_MOBS, fmt("You have been captured by %s!\n\r") % getName());
  act("$n captures $N!", TRUE, this, 0, targ, TO_NOTVICT);

  // Don't let captives have captives...
  while (targ->getCaptive())
    targ->doRelease(fname(targ->getCaptive()->name));

  addCaptive(targ);  

  if (getPosition() < POSITION_CRAWLING)
    targ->doStand();

  if (master)
    targ->stopFollower(TRUE);

  addFollower(targ);
  return;
}

void TBeing::doFactions(const sstring &arg)
{
  factionTypeT which;
  sstring buf, sbuf;
  struct stat timestat;
  char timebuf[256];

  if (!desc)
    return;

  if (!isImmortal() || arg.empty())
    which = getFaction();
  else { 
    which = factionNumber(arg);
    if (which == -1) {
      sendTo("No such faction.\n\r");
      sendTo("Syntax: list faction <faction>\n\r");
      return;
    }
  }

  buf = fmt("You are allied to: %s\n\r") % FactionInfo[which].faction_name;
  sbuf += buf;

  if (which != FACT_NONE) {
#if 0
    buf = fmt("Your faction has a potency of: %.2f, which is %s.\n\r") %
      FactionInfo[which].faction_power %
      ((FactionInfo[which].faction_power == avg_faction_power) ? "average" :
       ((FactionInfo[which].faction_power > avg_faction_power) ?
          "above average" :"below average"));
    sbuf+=buf;

#endif
    buf = fmt("Your faction has %ld talens of wealth, and a tithe percentage of %.2f%c.\n\r") %
	     FactionInfo[which].getMoney() %
	     FactionInfo[which].faction_tithe % '%';
    sbuf+=buf;
  }
#if 0
  if (which != FACT_NONE || isImmortal()) {
    buf =fmt("\n\rOne of your faction's caravans departed %d hour%s ago bound for %s.\n\r") %
      (FactionInfo[which].caravan_counter / 2) %
      (FactionInfo[which].caravan_counter / 2 == 1 ? "" : "s") %
      CaravanDestination(-which-1);
    sbuf+=buf;
    buf=fmt("It was carrying %d talens of goods, and paid %d talens for defenders.\n\r") %
            FactionInfo[which].caravan_value %
            FactionInfo[which].caravan_defense;
    sbuf+=buf;

    if (FactionInfo[which].caravan_interval != -1) {
      buf=fmt("The next caravan is scheduled to leave in %d hour%s.\n\r") %
            ((FactionInfo[which].caravan_interval -
	      FactionInfo[which].caravan_counter)/2) %
            (((FactionInfo[which].caravan_interval -
            FactionInfo[which].caravan_counter)/2 == 1 ? "" : "s"));
      sbuf+=buf;
      buf="Currently, caravan destinations are: ";
      sbuf+=buf;

      if (IS_SET(FactionInfo[which].caravan_flags, CARAVAN_DEST_GH)){
        buf = fmt(" %s") % CaravanDestination(CARAVAN_DEST_GH);
	sbuf+=buf;
      }
      if (IS_SET(FactionInfo[which].caravan_flags, CARAVAN_DEST_BM)){
	buf = fmt(" %s") % CaravanDestination(CARAVAN_DEST_BM);
	sbuf+=buf;
      }
      if (IS_SET(FactionInfo[which].caravan_flags, CARAVAN_DEST_LOG)){
        buf = fmt(" %s") % CaravanDestination(CARAVAN_DEST_LOG);
	sbuf+=buf;
      }
      if (IS_SET(FactionInfo[which].caravan_flags, CARAVAN_DEST_AMBER)){
        buf = fmt(" %s") % CaravanDestination(CARAVAN_DEST_AMBER);
	sbuf+=buf;
      }
      if (!IS_SET(FactionInfo[which].caravan_flags, 
                CARAVAN_DEST_BM | CARAVAN_DEST_GH |
		  CARAVAN_DEST_LOG | CARAVAN_DEST_AMBER)){
	sbuf+=" None";
      }
      sbuf+="\n\r";
    } else {
      sbuf += "At this time, no caravans are scheduled.\n\r";
    }
    buf=fmt("Your faction has sent %d caravan%s, and %d of them have arrived successfully.\n\r") %
            FactionInfo[which].caravan_attempts %
            (FactionInfo[which].caravan_attempts == 1 ? "" : "s") %
            FactionInfo[which].caravan_successes;
    sbuf+=buf;
  }

  sbuf += "\n\rContribution ratios:\n\r";
  factionTypeT i;
  for (i = MIN_FACTION;i < MAX_FACTIONS; i++) {
    buf = fmt("     %-25.25s : helping: %4.1f     harming: %4.1f\n\r") %
            FactionInfo[i].faction_name %
            FactionInfo[which].faction_array[i][OFF_HELP] %
            FactionInfo[which].faction_array[i][OFF_HURT];
    sbuf += buf;
  }
#endif

  if (which != FACT_NONE) {
    buf = fmt("%-50.50s:     %-20.20s\n\r") %
      factionLeaderTitle(which, 0) % FactionInfo[which].leader[0];
    sbuf += buf;
    buf = fmt("%-50.50s:        %-20.20s\n\r") %
      factionLeaderTitle(which, 1) % FactionInfo[which].leader[1];
    sbuf += buf;
    buf = fmt("%-50.50s:        %-20.20s\n\r") %
          factionLeaderTitle(which, 2) % FactionInfo[which].leader[2];
    sbuf += buf;
    buf = fmt("%-50.50s:        %-20.20s\n\r") %
      factionLeaderTitle(which, 3) % FactionInfo[which].leader[3];
    sbuf += buf;

    char factname[8];

    if (which == FACT_BROTHERHOOD)
      strncpy(factname, "brother", 8);
    else if (which == FACT_CULT)
      strncpy(factname, "cult", 8);
    else if (which == FACT_SNAKE)
      strncpy(factname, "snake", 8);

    // reveal membership only to leaders, otherwise someone can cross reference
    if ((getFactionAuthority(which,FACT_LEADER_SLOTS - 1) > 0) ||
        isImmortal()) {
      mud_str_copy(timebuf, ctime(&(timestat.st_mtime)), 256);
      timebuf[strlen(timebuf) - 1] = '\0';

      buf = fmt("\n\rMembership as of last rollcall (%s):\n\r") % timebuf;
      sbuf += buf;

      TDatabase db(DB_SNEEZY);
      db.query("select name, level from factionmembers where faction='%s'",
	       factname);

      while(db.fetchRow()){
	buf = fmt("      %-10.10s    Level: %s\n\r") %
		db["name"] % db["level"];
	sbuf+=buf;
      }
    }
  }
  desc->page_string(sbuf);

  return;
}

void TBeing::doAdjust(const char *arg)
{
  char tmpbuf[128];
  char tmpbuf2[128];
  factionTypeT fnum = getFaction();
  int verses = -1, hval;
  float value;
  float old_val;
  int amount;
  unsigned int uamount;

  enum adjTypeT {
       ADJ_HELP,
       ADJ_HURT,
       ADJ_TITHE,
       ADJ_NAME,
       ADJ_CARAVAN
  };

  arg = one_argument(arg, tmpbuf);
  if (is_abbrev(tmpbuf, "help")) {
    hval = ADJ_HELP;
  } else if (is_abbrev(tmpbuf, "harm")) {
    hval = ADJ_HURT;
  } else if (is_abbrev(tmpbuf, "tithe")) {
    hval = ADJ_TITHE;
  } else if (is_abbrev(tmpbuf, "name")) {
    hval = ADJ_NAME;
  } else if (is_abbrev(tmpbuf, "caravan")) {
    hval = ADJ_CARAVAN;
  } else {
    sendTo("Specify either help, harm or tithe.\n\r");
    sendTo("Syntax: adjust <\"help\" | \"harm\"> <verses faction> <value>\n\r");
    sendTo("Syntax: adjust tithe <value>\n\r");
    sendTo("Syntax: adjust name <new name>\n\r");
    sendTo("Syntax: adjust caravan ...\n\r");
    return;
  }
  if (hval == ADJ_NAME) {
    if (strcmp(getName(), "Batopr")) {
      // this works, but it screws up factionNumber() functionality.
      sendTo("Contact Batopr to change the name of the faction.\n\r");
      return; 
    }
    if ((getFactionAuthority(fnum,0) <= 0) && !isImmortal()) {
      // must be primary leader
      sendTo("You lack the authority to change the faction's name.\n\r");
      return;
    }
    char oldname[256];
    sprintf(oldname, FactionInfo[fnum].faction_name);
    for (; arg && *arg && isspace(*arg);arg++);

    if (!arg || !*arg) {
      sendTo("Please specify a new faction name.\n\r");
      return;
    }
    vlogf(LOG_FACT, fmt("Faction name changed from %s to %s.\n\r") % 
          oldname % arg);
    sendTo(fmt("You change the name of the faction from %s to %s.\n\r") %
          oldname % arg);
    sendTo("Remember to update factionNumber() in faction.cc with the new abbreviations.\n\r");
    sprintf(FactionInfo[fnum].faction_name, arg);
    save_factions();
    return;
  }
  if (hval == ADJ_CARAVAN) {
    if (!isImmortal()) {
      sendTo("Caravans are currently disabled.\n\r");
      return;
    }

    if ((getFactionAuthority(fnum,FACT_LEADER_SLOTS-1) <= 0) && !isImmortal()) {
      sendTo("You lack the authority to change caravan parameters.\n\r");
      return;
    }
    arg = one_argument(arg, tmpbuf);
    arg = one_argument(arg, tmpbuf2);
    if (is_abbrev(tmpbuf, "interval")) {
      if (!*tmpbuf2) {
        sendTo("Specify an interval.\n\r");
        sendTo("Syntax: adjust caravan interval <interval>\n\r");
        return;
      }
      amount = convertTo<int>(tmpbuf2);
      if (amount < MIN_CARAVAN_INTERVAL  && amount != -1) {
        sendTo(fmt("You can't specify an interval less than %d.\n\r") % MIN_CARAVAN_INTERVAL);
        return;
      }
      FactionInfo[fnum].caravan_interval = amount;
      sendTo(fmt("You set %s's caravan interval to %d ticks.\n\r") %
          FactionInfo[fnum].faction_name %
          FactionInfo[fnum].caravan_interval);
      return;
    } else if (is_abbrev(tmpbuf, "value")) {
      if (!*tmpbuf2) {
        sendTo("Specify the value the caravans should transport.\n\r");
        sendTo("Syntax: adjust caravan value <value>\n\r");
        return;
      }
      amount = convertTo<int>(tmpbuf2);
      if (amount <= 0) {
        sendTo("Caravans must transport something.\n\r");
        return;
      }
      if (amount % CARAVAN_TRADE) {
        sendTo(fmt("Caravans may only transport goods in multiples of %d.\n\r") % CARAVAN_TRADE);
        return;
      }
      FactionInfo[fnum].caravan_value = amount;
      sendTo(fmt("You set %s's caravan value to %d talens.\n\r") %          FactionInfo[fnum].faction_name %
          FactionInfo[fnum].caravan_value);
       return;
    } else if (is_abbrev(tmpbuf, "defense")) {
      if (!*tmpbuf2) {
        sendTo("Specify the amount per caravan to spend on defenses.\n\r");
        sendTo("Syntax: adjust caravan defense <amount>\n\r");
        return;
      }
      amount = convertTo<int>(tmpbuf2);
      if (amount < 0) {
        sendTo("You can't spend a negative amount on defense.\n\r");
        return;
      }
      FactionInfo[fnum].caravan_defense = amount;
      sendTo(fmt("You set %s's caravan defense value to %d talens.\n\r") %          FactionInfo[fnum].faction_name %
          FactionInfo[fnum].caravan_defense);
       return;
    } else if (is_abbrev(tmpbuf, "destination")) {
      if (!*tmpbuf2) {
        sendTo("Specify the caravan destination city.\n\r");
        sendTo("Syntax: adjust caravan destination <city>\n\r");
        sendTo("Valid <cities>:\n\r");
        sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
        return;
      }
      mud_str_copy(tmpbuf2, sstring(tmpbuf2).lower(), 128);
      if (is_abbrev(tmpbuf2, "grimhaven")) {
        uamount = CARAVAN_DEST_GH;
      } else if (is_abbrev(tmpbuf2, "brightmoon")) {
        uamount = CARAVAN_DEST_BM;
      } else if (is_abbrev(tmpbuf2, "logrus")) {
        uamount = CARAVAN_DEST_LOG;
      } else if (is_abbrev(tmpbuf2, "amber")) {
        uamount = CARAVAN_DEST_AMBER;
      } else {
        sendTo("Undefined city.\n\r");
        sendTo("Valid <cities>:\n\r");
        sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
        return;
      }
      if (fnum == FACT_NONE) {
        if (uamount == CARAVAN_DEST_GH) {
          sendTo("You are based in that city, and can't just trade with yourself.\n\r");
          sendTo("Valid <city number>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
      } else if (fnum == FACT_BROTHERHOOD) {
        if (uamount == CARAVAN_DEST_BM) {
          sendTo("You are based in that city, and can't just trade with yourself.\n\r");
          sendTo("Valid <cities>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
        if (uamount == CARAVAN_DEST_LOG) {
          sendTo("Direct trade with an enemy city is forbidden.\n\r");
          sendTo("Valid <cities>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
      } else if (fnum == FACT_CULT) {
        if (uamount == CARAVAN_DEST_LOG) {
          sendTo("You are based in that city, and can't just trade with yourself.\n\r");
          sendTo("Valid <cities>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
        if (uamount == CARAVAN_DEST_BM) {
          sendTo("Direct trade with an enemy city is forbidden.\n\r");
          sendTo("Valid <cities>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
      } else if (fnum == FACT_SNAKE) {
        if (uamount == CARAVAN_DEST_AMBER) {
          sendTo("You are based in that city, and can't just trade with yourself.\n\r");
          sendTo("Valid <cities>:\n\r");
          sendTo("Grimhaven, Brightmoon, Logrus, Amber\n\r");
          return;
        }
      }
      if (!IS_SET(FactionInfo[fnum].caravan_flags, uamount)) {
        SET_BIT(FactionInfo[fnum].caravan_flags, uamount);
        sendTo(fmt("You set %s's caravan destination to %s.\n\r") %
            FactionInfo[fnum].faction_name %
            CaravanDestination(uamount));
      } else {
        REMOVE_BIT(FactionInfo[fnum].caravan_flags, uamount);
        sendTo(fmt("Future caravans of %s will no longer go to %s.\n\r") %            FactionInfo[fnum].faction_name %
            CaravanDestination(uamount));
      }
      return;
    } else {
      sendTo("Unknown caravan option.\n\r");
      sendTo("Syntax: adjust caravan interval <interval>\n\r");
      sendTo("Syntax: adjust caravan value <value>\n\r");
      sendTo("Syntax: adjust caravan defense <defense>\n\r");
      sendTo("Syntax: adjust caravan destination <city>\n\r");
    }
    return;
  }
  if (hval == ADJ_HELP || hval == ADJ_HURT) {
    arg = one_argument(arg, tmpbuf);
    if ((verses = factionNumber(tmpbuf)) == -1) {
      sendTo("That faction doesn't exist.\n\r");
      sendTo("Syntax: adjust <help | harm> <verses faction> <value>\n\r");
      return;
    }
  }
  if (!arg || !*arg || (sscanf(arg, " %f ", &value) != 1)) {
    sendTo("Specify a legitimate value.\n\r");
    sendTo("Syntax: adjust <\"help\" | \"harm\"> <verses faction> <value>\n\r");
    sendTo("Syntax: adjust tithe <value>\n\r");
    sendTo("Syntax: adjust name <new name>\n\r");
    return;
  }
  if (getFactionAuthority(fnum,FACT_LEADER_SLOTS - 1) <= 0 && !isImmortal()) {
    sendTo("You lack the authority to adjust values.\n\r");
    return;
  }

  if (hval == ADJ_HELP || hval == ADJ_HURT) {
#if !FACTIONS_IN_USE
    sendTo("You are not permitted to alter the help/harm values because faction percent is not in use.\n\r");
    return;
#else
    old_val = FactionInfo[fnum].faction_array[verses][(hval == ADJ_HURT ? OFF_HURT : OFF_HELP)];

    if (value < -4.0 || value > 4.0) {
      sendTo("Absolute value of <value> may not exceed 4.0\n\r");
      return;
    }
    if (((FactionInfo[fnum].faction_array[verses][(hval == ADJ_HURT ? OFF_HELP : OFF_HURT)] > 0.0) &&
        value > 0.0) ||
        ((FactionInfo[fnum].faction_array[verses][(hval == ADJ_HURT ? OFF_HELP : OFF_HURT)] < 0.0) &&
        value < 0.0)) {
      sendTo("Helping and harming can't BOTH be in same direction.\n\r");
      if (isImmortal())
        sendTo("Ignoring this error due to immortality.  Please fix this though.\n\r");
      else
        return;
    }
    if ((hval == ADJ_HURT && fabs(FactionInfo[fnum].faction_array[verses][OFF_HELP]) < fabs(value)) ||
        (hval == ADJ_HELP && fabs(FactionInfo[fnum].faction_array[verses][OFF_HURT]) > fabs(value))) {
      sendTo("The absolute value of \"helping\" must exceed \"harming\".\n\r");
      return;
    }
    if (fnum == verses) {
      if (hval == ADJ_HURT && value > -1.0) {
        sendTo("Sorry, harming your own faction must cause some detriments.\n\r");
        return;
      }
      if (hval == ADJ_HELP && value < 1.0) {
        sendTo("Sorry, helping your own faction must cause some benefit.\n\r");
        return;
      }
      if ((hval == ADJ_HURT && value > 0.0) ||
          (hval == ADJ_HELP && value < 0.0)) {
        sendTo("That would be a little self-destructive.\n\r");
        return;
      }
    }
    if (verses == FACT_NONE) {
      if (hval == ADJ_HURT && value > -1.0) {
        sendTo(fmt("You have no quarrel with %s, and can not benefit from their destruction.\n\r") % FactionInfo[FACT_NONE].faction_name);
        return;
      }
    }
    if (fnum == FACT_BROTHERHOOD) {
      if (verses == FACT_CULT) {
        if (hval == ADJ_HURT && value < 2.0) {
          sendTo("You may not reduce this value below 2.0\n\r");
          return;
        }
        if (hval == ADJ_HELP && value > -2.0) {
          sendTo("You may not raise this value above -2.0\n\r");
          return;
        }
      }
    } else if (fnum == FACT_CULT) {
      if (verses == FACT_BROTHERHOOD) {
        if (hval == ADJ_HURT && value < 2.0) {
          sendTo("You may not reduce this value below 2.0\n\r");
          return;
        }
        if (hval == ADJ_HELP && value > -2.0) {
          sendTo("You may not raise this value above -2.0\n\r");
          return;
        }
      }
    } else if (fnum == FACT_SNAKE) {
      if (verses == FACT_BROTHERHOOD) {
        if ((hval == ADJ_HELP) &&
            (value + FactionInfo[fnum].faction_array[FACT_CULT][OFF_HELP] > 1.0)) {
          sendTo(fmt("At present, that value is limited to %5.2f.\n\r") %
              1.0 - FactionInfo[fnum].faction_array[FACT_CULT][OFF_HELP]);
          return;
        }
        if ((hval == ADJ_HURT) &&
            (value + FactionInfo[fnum].faction_array[FACT_CULT][OFF_HURT] > 1.0)) {
          sendTo(fmt("At present, that value is limited to %5.2f.\n\r") %
              1.0 - FactionInfo[fnum].faction_array[FACT_CULT][OFF_HURT]);
          return;
        }
      } else if (verses == FACT_CULT) {
        if ((hval == ADJ_HELP) &&
            (value + FactionInfo[fnum].faction_array[FACT_BROTHERHOOD][OFF_HELP] > 1.0)) {
          sendTo(fmt("At present, that value is limited to %5.2f.\n\r") %
              1.0 - FactionInfo[fnum].faction_array[FACT_BROTHERHOOD][OFF_HELP]);
          return;
        }
        if ((hval == ADJ_HURT) &&
            (value + FactionInfo[fnum].faction_array[FACT_BROTHERHOOD][OFF_HURT] > 1.0)) {
          sendTo(fmt("At present, that value is limited to %5.2f.\n\r") %
              1.0 - FactionInfo[fnum].faction_array[FACT_BROTHERHOOD][OFF_HURT]);
          return;
        }
      }
    }

    sendTo(fmt("You change %s's %s rating verses %s from %5.2f to %5.2f.\n\r") %
          FactionInfo[fnum].faction_name,
          (hval == ADJ_HURT ? "harm" : "help"),
          FactionInfo[verses].faction_name,
          old_val, value);
    FactionInfo[fnum].faction_array[verses][(hval == ADJ_HURT ? OFF_HURT : OFF_HELP)] = value;
#endif
  } else if (hval == ADJ_TITHE) {
    old_val = FactionInfo[fnum].faction_tithe;

    if (value < 0.0 || value > 15.0) {
      sendTo("Tithe <value> must be in range 0.0 to 15.0\n\r");
      return;
    }
    sendTo(fmt("You change %s's tithe rate from %5.2f to %5.2f.\n\r") %          FactionInfo[fnum].faction_name % old_val % value);
    FactionInfo[fnum].faction_tithe = value;
  }
  
  save_factions();
  return;
}

bool TBeing::isOppositeFaction(const TBeing *v) const
{
  if ((isCult() && v->isBrother()) ||
      (isBrother() && v->isCult()))
    return TRUE;

  if ((FactionInfo[getFaction()].faction_array[v->getFaction()][OFF_HURT] > 2.0) &&
     (FactionInfo[getFaction()].faction_array[v->getFaction()][OFF_HELP] < -2.0))
    return TRUE;

  return FALSE;
}

int TBeing::doTithe()
{
  sendTo("You can only tithe at a bank.\n\r");
  return FALSE;
}

sstring TBeing::yourDeity(spellNumT skill, personTypeT self, const TBeing *who) const
{
  char buf[256];
  deityTypeT deity = DEITY_NONE;
  factionTypeT fnum = getFaction();

  // switch for specifc skill
  if (deity == DEITY_NONE) {
    switch (skill) {
      case SPELL_REFRESH_DEIKHAN:
      case SPELL_CURSE_DEIKHAN:
      case SPELL_REMOVE_CURSE_DEIKHAN:
      case SPELL_BLESS_DEIKHAN:
      case SPELL_CREATE_FOOD_DEIKHAN:
      case SPELL_CREATE_WATER_DEIKHAN:
      case SPELL_HEROES_FEAST_DEIKHAN:
      case SPELL_BLESS:
      case SPELL_REFRESH:
      case SPELL_SECOND_WIND:
      case SPELL_CREATE_FOOD:
      case SPELL_CREATE_WATER:
      case SPELL_HEROES_FEAST:
      case SPELL_CURSE:
      case SPELL_REMOVE_CURSE:
        if (fnum == FACT_SNAKE)
          deity = DEITY_LUNA;
        break;
      case SPELL_PORTAL:
      case SPELL_ASTRAL_WALK:
      case SPELL_SUMMON:
      case SPELL_WORD_OF_RECALL:
      case SPELL_EXPEL_DEIKHAN:
      case SPELL_EXPEL:
        if (fnum == FACT_SNAKE)
          deity = DEITY_ICON;
        break;
      case SPELL_CURE_PARALYSIS:
        if (fnum == FACT_SNAKE)
          deity = DEITY_SINSUKEY;
        break;
      case SPELL_CURE_BLINDNESS:
      case SPELL_PARALYZE:
      case SPELL_PARALYZE_LIMB:
      case SPELL_NUMB_DEIKHAN:
      case SPELL_NUMB:
      case SPELL_CURE_POISON_DEIKHAN:
      case SPELL_CURE_POISON:
        if (fnum == FACT_SNAKE)
          deity = DEITY_SASUKEY;
        break;
      default:
        break;
    }
  }

  // switch for discipline
  if (deity == DEITY_NONE) {
    switch (getDisciplineNumber(skill, FALSE)) {
      case DISC_AEGIS:
        if (fnum == FACT_NONE)
          deity = DEITY_SASUKEY;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_SASUKEY;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_MENANON;
        else if (fnum == FACT_CULT)
          deity = DEITY_SALUREL;
        break;
      case DISC_CURES:
        if (fnum == FACT_NONE)
          deity = DEITY_LUNA;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_LUNA;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_AMANA;
        else if (fnum == FACT_CULT)
          deity = DEITY_LESPRIT;
        break;
      case DISC_AFFLICTIONS:
        if (fnum == FACT_NONE)
          deity = DEITY_SINSUKEY;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_SINSUKEY;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_ELYON;
        else if (fnum == FACT_CULT)
          deity = DEITY_SHROUD;
        break;
      case DISC_DEIKHAN:
      case DISC_WRATH:
        if (fnum == FACT_NONE)
          deity = DEITY_ELYON;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_SINSUKEY;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_ELYON;
        else if (fnum == FACT_CULT)
          deity = DEITY_MALSHYRA;
        break;
      case DISC_HAND_OF_GOD:
        if (fnum == FACT_NONE)
          deity = DEITY_JEVON;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_SASUKEY;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_JEVON;
        else if (fnum == FACT_CULT)
          deity = DEITY_SALUREL;
        break;
      case DISC_SHAMAN:
        if (fnum == FACT_NONE)
          deity = DEITY_SHROUD;
        else if (fnum == FACT_SNAKE)
          deity = DEITY_ICON;
        else if (fnum == FACT_BROTHERHOOD)
          deity = DEITY_OMNON;
        else if (fnum == FACT_CULT)
          deity = DEITY_SHROUD;
        break;
      case DISC_THEOLOGY:
        if (fnum == FACT_NONE)
          deity = DEITY_MEZAN;
        break;
      case DISC_CLERIC:
        if (fnum == FACT_NONE)
          deity = DEITY_MEZAN;
        break;
      default:
        break;
    }
  }

  if (self == FIRST_PERSON) {
    if ((deity <= DEITY_NONE) || (deity >= MAX_DEITIES))
      sprintf(buf, "your deity");
    else
      sprintf(buf, "%s", deities[deity]);
  } else if (self == SECOND_PERSON) 
    sprintf(buf, "%s deity", hshr());
  else if (self == THIRD_PERSON) {
    if (who) 
      sprintf(buf, "%s's deity", who->pers(this));
    else
      sprintf(buf, "%s's deity", getName());
  } 
  return buf;
}

int bestFactionPower()
{
  factionTypeT i,cur_best;
  double cur_score = 0.0;

  // skip over power of unaffiliated
  for (i = cur_best = MIN_FACTION;i < MAX_FACTIONS; i++) {
    if (FactionInfo[i].faction_power > cur_score) {
      cur_best = i;
      cur_score = FactionInfo[i].faction_power;
    }
  }
  return cur_best;
}

void recalcFactionPower()
{
  factionTypeT i;

  avg_faction_power = 0;

  // skip over power of unaffiliated
  for (i = factionTypeT(FACT_NONE+1);i < MAX_FACTIONS; i++) {
    avg_faction_power += FactionInfo[i].faction_power;
  }
 
  // subtract 1 due to skipping FACT_NONE
  avg_faction_power /= (double) ( MAX_FACTIONS - 1.0);
 
  if (!avg_faction_power)
    avg_faction_power = 1.0;
}

void TPerson::reconcileHelp(TBeing *victim, double amp)
{
#if FACTIONS_IN_USE

#undef ALIGN_STUFF
  double abso;
  double perc, mob_perc;
  double value;
  double tmp;

  if (this == victim)
    return;

  if (victim && !victim->isPc() && (victim->master == this))
    return;

  perc = getPerc();
  if (victim)
    mob_perc = victim->getPerc();
  else
    mob_perc = 100.0;

//  double old_perc = getPerc();

  // KLUDGE TO get things going faster
  amp *= 17.00;

  if (amp > 0.0) {
    // helping
    if (victim)
      value = mob_perc *
        FactionInfo[getFaction()].faction_array[victim->getFaction()][OFF_HELP] / 100.0;
    else
      value = mob_perc *
        FactionInfo[getFaction()].faction_array[getFaction()][OFF_HELP] / 100.0;
  } else {
    // harming
    if (victim)
      value = -mob_perc *
        FactionInfo[getFaction()].faction_array[victim->getFaction()][OFF_HURT] / 100.0;
    else
      value = -mob_perc *
        FactionInfo[getFaction()].faction_array[getFaction()][OFF_HURT] / 100.0;
  }

  abso = value * amp;

#ifdef ALIGN_STUFF 
  vlogf(LOG_MISC, fmt("align thing: value %2.6f, amp %2.6f abso  %2.6f, %10.10s (%d) vs %10.10s (%d)") % 
       value % amp % abso % getName() % getFaction() % victim->getName() %
       victim->getFaction());
#endif

  if (victim) {
    addToPercX(abso, victim->getFaction());
  } else
    addToPercX(abso, getFaction());

  double avg = (getPercX(FACT_NONE) + getPercX(FACT_BROTHERHOOD) + getPercX(FACT_CULT) + getPercX(FACT_SNAKE)) / 4.0;
  double sigma;

  sigma =  (getPercX(FACT_NONE) - avg) * (getPercX(FACT_NONE) - avg);
  sigma += (getPercX(FACT_BROTHERHOOD) - avg) * (getPercX(FACT_BROTHERHOOD) - avg);
  sigma += (getPercX(FACT_CULT) - avg) * (getPercX(FACT_CULT) - avg);
  sigma += (getPercX(FACT_SNAKE) - avg) * (getPercX(FACT_SNAKE) - avg);
  sigma /= 4.0;
  sigma = sqrt(sigma);

  setPerc(avg - (0.33*sigma));
#ifdef ALIGN_STUFF
  vlogf(LOG_MISC, fmt("avg %2.6f    sigma %2.6f") %  avg % sigma);
  vlogf(LOG_MISC,fmt("real %2.6f    0: %2.6f    1: %2.6f    2: %2.6f    3: %2.6f") % 
     getPerc() % getPercX(FACT_NONE) % getPercX(FACT_BROTHERHOOD) % getPercX(FACT_CULT) % getPercX(FACT_SNAKE)); 
#endif

  // adjust faction pool
  // redefine abso as amount to change faction-power by
  // this effectively changes faction-power even if faction % doesn't
  abso /= 4.25;

  if (FactionInfo[getFaction()].faction_power + abso < 0.0)
    abso = -FactionInfo[getFaction()].faction_power;

  if (!isUnaff()) {
    FactionInfo[getFaction()].faction_power += abso;
    if (desc)
      desc->session.perc += abso;
    if (victim && !victim->isUnaff()) {
      // amp > 0 if helpful act, < 0 if harmful
      // helping victims faction, raise victims
      // harming victims faction, lower victims
      tmp = mob_perc * amp / 100.0;
      tmp /= 6.0;  // semi-arbitrary (2.0 causes drop)

      if (FactionInfo[victim->getFaction()].faction_power + tmp < 0.0)
        tmp = -FactionInfo[victim->getFaction()].faction_power;

      FactionInfo[victim->getFaction()].faction_power += tmp;
    }
  } else {
    // unaffiliated don't have their own power, they simply drain/add to
    // other factions.

    if (victim && !victim->isUnaff()) {
      // amp > 0 if helpful act, < 0 if harmful
      // helping victims faction, raise victims
      // harming victims faction, lower victims

      tmp = mob_perc * amp / 100.0;
//      tmp /= 12.0;  // semi-arbitrary (2.0 causes drop)

      if (FactionInfo[victim->getFaction()].faction_power + tmp < 0.0)
        tmp = -FactionInfo[victim->getFaction()].faction_power;

      FactionInfo[victim->getFaction()].faction_power += tmp;
    }
  }
#endif  // factions_in_use
}

void TPerson::reconcileHurt(TBeing *victim, double amp)
{
  reconcileHelp(victim, -amp);
}

const char * CaravanDestination(int which)
{
  int i;

  if (which < 0)
    i = FactionInfo[-which - 1].caravan_flags & 
           (CARAVAN_CUR_DEST_GH | CARAVAN_CUR_DEST_BM | 
           CARAVAN_CUR_DEST_LOG | CARAVAN_CUR_DEST_AMBER);
  else
    i = which;

  switch (i) {
    case 0:
      return "Nowhere";
    case CARAVAN_DEST_GH:
    case CARAVAN_CUR_DEST_GH:
      return "Grimhaven";
    case CARAVAN_DEST_BM:
    case CARAVAN_CUR_DEST_BM:
      return "Brightmoon";
    case CARAVAN_DEST_LOG:
    case CARAVAN_CUR_DEST_LOG:
      return "Logrus";
    case CARAVAN_DEST_AMBER:
    case CARAVAN_CUR_DEST_AMBER:
      return "Amber";
    default:
      vlogf(LOG_BUG, fmt("CaravanDestination had an i of %d (which %d)") %  i % which);
      vlogf(LOG_BUG, "bad carvan");
      return "unknown";
  }
}

void launch_caravans()
{
  factionTypeT i;
  TMonster *mob;

  // don't launch caravans if in shutdown mode
  if (timeTill)
    return;

  for (i = MIN_FACTION; i < MAX_FACTIONS;i++) {
    if (!FactionInfo[i].caravan_value) // no gold on board, why go?
      continue;
    FactionInfo[i].caravan_counter++;
    if ((FactionInfo[i].caravan_interval == -1) ||
        (!IS_SET(FactionInfo[i].caravan_flags, 
                CARAVAN_DEST_BM | CARAVAN_DEST_GH |
                CARAVAN_DEST_LOG | CARAVAN_DEST_AMBER))) {
      // no caravans will be launched
      // don't let caravan interval grow out of bounds
      FactionInfo[i].caravan_counter = min(FactionInfo[i].caravan_counter, 
                                             MIN_CARAVAN_INTERVAL);
      continue;
    }
    if (FactionInfo[i].caravan_counter < FactionInfo[i].caravan_interval) {
      // not time yet
      continue;
    }
    // launch a caravan
    FactionInfo[i].caravan_counter = 0;
    if (!(mob = read_mobile(MOB_CARAVAN_MASTER, VIRTUAL))) {
      vlogf(LOG_BUG, "No caravan master read in load_caravan");
      return;
    }
    mob->setFaction(i);
    switch (i) {
      case FACT_NONE:
        thing_to_room(mob, CAR_GH_HOME);
        break;
      case FACT_BROTHERHOOD:
        thing_to_room(mob, CAR_BM_HOME);
        break;
      case FACT_CULT:
        thing_to_room(mob, CAR_LOG_HOME);
        break;
      case FACT_SNAKE:
        thing_to_room(mob, CAR_AMBER_HOME);
        break;
      case MAX_FACTIONS:
      case FACT_UNDEFINED:
        vlogf(LOG_BUG, fmt("Bad faction (%d) in LaunchCaravans") %  i);
        break;
    }
  }
  return;
}

void TBeing::deityIgnore(silentTypeT silent_caster) const
{
  int num = ::number(0,2);
  switch (num) {
    case 0:
    default:
      if (!silent_caster)
        act("$d ignores you.", FALSE, this, NULL, NULL, TO_CHAR, ANSI_RED);
      act("$n's request is ignored by $d.", TRUE, this, NULL, NULL, TO_ROOM);
      break;
    case 1:
      if (!silent_caster)
        act("$d fails to come to your aid.", FALSE, this, NULL, NULL, TO_CHAR, ANSI_RED);

      act("$n is not aided by $d.", TRUE, this, NULL, NULL, TO_ROOM);
      break;
    case 2:
      if (!silent_caster)
        act("$d does not answer your prayer.", FALSE, this, NULL, NULL, TO_CHAR, ANSI_RED);
      act("$n's prayer is not answered by $d.", TRUE, this, NULL, NULL, TO_ROOM);
      break;
  }
}

// a generic modifier for prayer effects
float TBeing::percModifier() const
{
#if FACTIONS_IN_USE
  return (getPerc() + 100.0) / 200.0;
#else
  return 0.75;
#endif
}
