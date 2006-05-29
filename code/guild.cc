#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"
#include "guild.h"

// start new faction stuff
vector<TGuild *>guild_table(0);


// open recruitment factions in the office are taken care of by the registrar
void TBeing::doJoin(const char * args) {
  char buf[256];
  TGuild *f = NULL;

  if(!toggleInfo[TOG_TESTCODE5]->toggle) {
    sendTo("The new faction system is currently disabled.  You may not join a faction now.\n\r");
    return;
  }
  if(!(f = get_guild(args)) || (IS_SET(f->flags, GUILD_HIDDEN) && !hasOffer(f)) || 
     (!IS_SET(f->flags, GUILD_ACTIVE))) {
    // logic: faction doesn't exist, faction is hidden and hasn't extended an offer, or
    // faction is inactive - deny.
    sendTo("There is no such faction for you to join.\n\r");
    return;
  }
  if(faction.whichguild) {
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
    if(IS_SET(f->flags, GUILD_OPEN_RECRUITMENT)) {
      sendTo("However, you may join at the Grimhaven Bureau of Faction Affairs.\n\r");
    }
    return;
  }
  sprintf(buf,"You have accepted %s's offer and joined their faction!\n\r", f->getName());
  faction.whichguild = f->ID;
  faction.rank = f->ranks - 1; // this starts them off as the lowest level rank
  sendTo(COLOR_BASIC, buf);
  removeOffers();
  saveFactionStats();
  return;
}


void TBeing::doDefect(const char * args) {
  if(!toggleInfo[TOG_TESTCODE5]->toggle) {
    sendTo("The new faction system is currently disabled.  You may not defect now.\n\r");
    return;
  }

  char buf[80];
  if(!faction.whichguild) {
    sendTo("You are not a member of any faction - no need to defect.\n\r");
    return;
  }
  sscanf(args, "%s", buf);
  if(!strcmp(buf, "yes")) {
    sprintf(buf, "You have defected from %s.\n\r", newfaction()->getName());
    sendTo(COLOR_BASIC, buf);
    vlogf(LOG_FACT, fmt("%s defected from %s.") %  getName() % newfaction()->getName());
    faction.whichguild = 0;
    faction.rank = 0;
    saveFactionStats();
    setDefected();
  } else {
    sendTo("In order to insure you really meant to defect, you have to type 'defect yes'.\n\r");
  }
  return;
}  

void TBeing::doRecruit(const char * args) {
  if(!toggleInfo[TOG_TESTCODE5]->toggle) {
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
  
  if(faction.whichguild == 0) {
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
  if(targ->faction.whichguild) {
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

bool TBeing::hasOffer(TGuild * f) {
  
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

void TBeing::addOffer(TGuild * f) {
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
  TGuild *f = new TGuild;
  
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

  for (i = 0; i < MAX_GUILD_COLORS; i++) {
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
  guild_table.push_back(f);
  char buf[128];
  if (isImmortal()) {
    sprintf(buf,"Faction: '%s' added with unique ID #%d\n\r", f->keywords, f->ID);
    sendTo(buf);
  } else {
    faction.whichguild = f->ID;
    faction.rank = 0; // leader slot
    saveFactionStats();
  }  
  vlogf(LOG_FACT, fmt("%s founded a new faction: [%s] (%d)") %  getName() % f->keywords % f->ID);
  save_guilds();
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
  if(faction.whichguild) {
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
      if(ch->GetMaxLevel() < GUILD_CREATE_LEVEL) {
	myself->doSay("Awwww, I'm sorry, it appears you're too low level to create a faction.");
	myself->doAction(fname(ch->name), CMD_COMFORT);
	myself->doSay("You can come back later when you've become more powerful.");
	return TRUE;
      }
      if(ch->faction.whichguild) {
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
    
    
    if(!toggleInfo[TOG_TESTCODE5]->toggle) {
      ch->sendTo("The new faction system is currently disabled.  You may not join a faction now.");
      return TRUE;
    }
    
    
    TGuild *f = NULL;
    sprintf(buf, "You wish to join a faction?  Lets see....");
    myself->doSay(buf);
    act("$n gets a folder from a filing cabinet along the wall.", 0, myself, 0, 0, TO_ROOM);
    act("$n quickly scans a few of the pages from the file.", 0, myself, 0, 0, TO_ROOM);
    
    
    
    if(!(f = get_guild(arg)) || (IS_SET(f->flags, GUILD_HIDDEN) && !ch->hasOffer(f)) ||
       (!IS_SET(f->flags, GUILD_ACTIVE)) || f->ID == 0) {
      // logic: faction doesn't exist, faction is hidden and hasn't extended an offer, or
      // faction is inactive - deny.
      
      myself->doTell(fname(ch->name), "I'm sorry, it appears that faction does not show up in any of my records.");
      
      return TRUE;
    }
    if(ch->faction.whichguild) {
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
      if(!IS_SET(f->flags, GUILD_OPEN_RECRUITMENT)) {
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
    if (IS_SET(f->flags, GUILD_HIDDEN)) {
      sprintf(tell, "%s %s", fname(ch->name).c_str(), buf);
      myself->doWhisper(tell);
    } else {
      myself->doSay(buf);
    }
    ch->faction.whichguild = f->ID;
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
  // there are two ways this will be called - god version and player
  // version of the command thus, we need a variable for all the
  // syntax stuff.
  char SYNTAX[30] = "";

  char buf[128];
  char fact[80];
  char field[80];
  char values[80];
  TGuild *f = NULL;

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
	save_guilds();
	sendTo("Factions saved.\n\r");
	return;
      }
      show_faction(fact);
      return;
    } else if (count == 2) {
      if(is_abbrev(fact, "remove")){
	if(!remove_guild(field)){
	  sendTo(fmt("Unable to find faction '%s'\n\r") % field);
	} else {
	  sendTo("Faction removed.\n\r");
	}
	return;
      }
      sendTo("Syntax: fedit <faction> [<field> <value(s)>]\n\r");
      return;
    }
    f = get_guild(fact);
    if (!f) {
      sprintf(buf,"Unable to find faction '%s'\n\r", fact);
      sendTo(buf);
      return;
    }
  } else {
    if(!toggleInfo[TOG_TESTCODE5]->toggle) {
      sendTo("The new faction code is currently disabled.  You will not be able to use this command.\n\r");
      return;
    }
    strcpy(SYNTAX, "fedit");
    int count = sscanf(args, "%s %[0-9a-z-A-Z '<>]", field, values);
    if (is_abbrev(field, "create")) {
      sendTo("You must speak to the Faction Registrar if you wish to create a faction.\n\r");
      sendTo("She can be found in the Grimhaven Bureau of Factions.\n\r");
      return;
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
        save_guilds();
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
    save_guilds();
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
      whichbit = GUILD_ACTIVE;
    } else if (is_abbrev(values, "locked")) {
      if(!isImmortal()) {
        sendTo("Mortals are not allowed to change this setting.\n\r");
        sendTo("Please speak to a god if your faction settings needs to be locked or unlocked.\n\r");
        return;
      }
      whichbit = GUILD_LOCKED;
    } else if (is_abbrev(values, "recruitment")) {
      whichbit = GUILD_OPEN_RECRUITMENT;
    } else if (is_abbrev(values, "hidden")) {
      whichbit = GUILD_HIDDEN;
    } else if (is_abbrev(values, "members")) {
      whichbit = GUILD_HIDE_MEMBERS;
      //    } else if (is_abbrev(values, "leaders")) {
      //      whichbit = GUILD_HIDE_LEADERS;
    } else if (is_abbrev(values, "ranks")) {
      whichbit = GUILD_HIDE_RANKS;
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
    TGuild *f2 = get_guild(fname);
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
    TGuild *f = NULL;
    f = get_guild(args);
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
    vector<TGuild *>::iterator i;
    TGuild *f2 = NULL;
    for( i = guild_table.begin(); i != guild_table.end(); ++i) {
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
  vector<TGuild *>::iterator i;
  for(i = guild_table.begin();i != guild_table.end(); ++i) {
    if (isImmortal() || (*i) == newfaction() ||
	!(IS_SET((*i)->flags, GUILD_HIDDEN) || (*i)->ID == 0 || !IS_SET((*i)->flags, GUILD_ACTIVE))) {
      sprintf(buf, "<1>%-4d%s%s<1>%s%s<1>%s%s<1> [<R>%s<1>] <1>%s%s<k>%s%s%s<1>\n\r", (*i)->ID, 
	      heraldcodes[(*i)->colors[0]], ((*i)->colors[0]) ? "*" : " ",
	      heraldcodes[(*i)->colors[1]], ((*i)->colors[1]) ? "*" : " ",
	      heraldcodes[(*i)->colors[2]], ((*i)->colors[2]) ? "*" : " ",
	      (IS_SET((*i)->flags, GUILD_OPEN_RECRUITMENT)) ? "X" : " ",
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

TGuild * TBeing::newfaction() const {
  return get_guild_by_ID(faction.whichguild);
}

const char * TBeing::rank() {
  return newfaction()->rank[faction.rank];
}

bool TBeing::hasPermission(unsigned int bit) {
  return IS_SET(newfaction()->permissions[faction.rank-1], bit);
}

int TGuild::getRelation(TGuild * target) {
  return getRelation(target->ID);
}

int TGuild::getRelation(int target) {
  vector<TRelation *>::iterator i;
  for (i = relations.begin(); i != relations.end(); ++i) {
    if((*i)->targ_fact == target) {
      return (*i)->relation;
    }
  }
  return RELATION_NONE;
}
 
void TGuild::setRelation(TGuild * target, int state) {
  setRelation(target->ID, state);
}

void TGuild::setRelation(int target, int state) {
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


bool remove_guild(const char *args)
{
  int num = 0;
  int count = 0;

  count = sscanf(args,"%d", &num);
  if(count)
    return remove_guild_by_ID(num);
  else
    return remove_guild_by_keywords(args);
}

bool remove_guild_by_ID(int idnum) {
  vector<TGuild *>::iterator i;
  for (i = guild_table.begin();i != guild_table.end();++i) {
    if ((*i)->ID == idnum) {
      guild_table.erase(i);
      return true;
    }
  }
  return false;
}

bool remove_guild_by_keywords(const char * args) {
  vector<TGuild *>::iterator i;
  for (i = guild_table.begin(); i != guild_table.end();++i) {
    if (isname(args, (*i)->keywords)) {
      guild_table.erase(i);
      return true;
    }
  }
  return false;
}

TGuild * get_guild(const char *args) {
  int num = 0;
  int count = 0;
  //  vlogf(LOG_DASH, fmt("get_guild called with args = '%s'") %  args);
  count = sscanf(args,"%d", &num);
  if(count)
    return get_guild_by_ID(num);
  else
    return get_guild_by_keywords(args);
  return NULL;
}

TGuild * get_guild_by_ID(int idnum) {
  vector<TGuild *>::iterator i;
  for (i = guild_table.begin();i != guild_table.end();++i) {
    if ((*i)->ID == idnum) {
      return (*i);
    }
  }
  return NULL;
}

TGuild * get_guild_by_keywords(const char * args) {
  vector<TGuild *>::iterator i;
  for (i = guild_table.begin(); i != guild_table.end();++i) {
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
	  (IS_SET(flags, GUILD_ACTIVE) ?  
	   "\n\r This faction is activated." : "\n\r This faction is NOT activated."),
	  (IS_SET(flags, GUILD_LOCKED) ?    
	   "\n\r This faction's attributes are locked." : ""),
	  (IS_SET(flags, GUILD_OPEN_RECRUITMENT) ?
	   "\n\r This faction is openly recruiting." : "\n\r New members must be recruited by hand."),
	  (IS_SET(flags, GUILD_HIDDEN) ?      
	   "\n\r The existance of this faction is hidden." : ""),
	  (IS_SET(flags, GUILD_HIDE_MEMBERS) ?
	   "\n\r The members of this faction are hidden from the masses." : ""),
	  (IS_SET(flags, GUILD_HIDE_LEADERS) ?
	   "\n\r The leaders of this faction are hidden from the masses." : ""),
	  (IS_SET(flags, GUILD_HIDE_RANKS) ? 
	   "\n\r The ranks of those in this faction are hidden from the masses." : "" ));
  return mud_str_dup(buf);
}

// get_unused_ID finds an unused faction ID (0 to MAX_GUILD_ID) and returns it.
// if all ID's are currently taken (ie 200 factions - shouldn't happen)
// then it returns -1
int get_unused_ID() {
  int i, j;
  bool found = FALSE;
  for (i = 0; i <= MAX_GUILD_ID; i++) {
    for (j = 0;(int)j < (int)guild_table.size(); j++) {
      found = FALSE;
      if(guild_table[j]->ID == i) {
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
  if(!get_guild_by_ID(faction.whichguild)) {
    vlogf(LOG_FACT, fmt("%s had bad faction during saveFactionStats() ... making unaffiliated") %  getName());
    faction.whichguild = 0;
    faction.rank = 0;
  }

  if(faction.rank < 0 || faction.rank >= newfaction()->ranks) {
    vlogf(LOG_FACT, fmt("%s had bad rank - setting to lowest in faction.") %  getName());
    faction.rank = newfaction()->ranks - 1;
  }
  fprintf(fp, "%u\n",
	  current_version);

  fprintf(fp,"%d %d\n", faction.whichguild, faction.rank);
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
  faction.whichguild = num1;
  if(!get_guild_by_ID(faction.whichguild)) {
    vlogf(LOG_FACT, fmt("%s had bad faction during loadFactionStats() ... making unaffiliated") %  getName());
    faction.whichguild = 0;
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

int load_guilds() {
  FILE *fp;
  char buf[256];
  int i1, i2, i3, i4;
  float f1;
  char c1[80];
  int line = 0;

  if (!(fp = fopen(GUILD_FILE, "r"))) {
    vlogf(LOG_FILE, "Couldn't open factionlist file in function load_guilds()!");
    return FALSE;
  }

  guild_table.clear();

  while(fp) {
    TGuild *f = new TGuild;
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
    guild_table.push_back(f);
  }
  fclose(fp);

  sprintf(buf, "cp %s %s", GUILD_FILE, GUILD_BAK);
  vsystem(buf);

  return TRUE;
}

void save_guilds()
{
  FILE *fp;
  
  if (!(fp = fopen(GUILD_FILE, "w+"))) {
    vlogf(LOG_FILE, "Couldn't open newfactions datafile in save_guilds()");
    return;
  }
  vector<TGuild *>::iterator i;
  TGuild *f = NULL;
  for(i = guild_table.begin(); i != guild_table.end(); ++i) {
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


