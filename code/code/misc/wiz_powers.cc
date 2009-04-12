#include <unistd.h>

#include "being.h"
#include "database.h"
#include "person.h"
#include "extern.h"
#include "handler.h"

bool TBeing::powerCheck(wizPowerT wpt) const
{
  if (!hasWizPower(wpt)) {
    sendTo(format("%s: You lack this power.\n\r") % getWizPowerName(wpt));
    return true;
  }
  return false;
}

bool TBeing::limitPowerCheck(cmdTypeT cmd, int vnum) {
  if (!desc) {
    vlogf(LOG_BUG,format("%s got to limitPowerCheck() without desc, very bad.") %  getName());
    return TRUE; // if we return FALSE here it could cause a crash
  }
  if (hasWizPower(POWER_NO_LIMITS))
    return TRUE;
  int as, ae, bs, be, o;
  as = desc->blockastart;
  ae = desc->blockaend;
  bs = desc->blockbstart;
  be = desc->blockbend;
  o = desc->office;



  switch(cmd) {
    case CMD_FORCE:
    case CMD_TRANSFER:
    case CMD_RESTORE:
    case CMD_SWITCH:
    case CMD_GIVE:
    case CMD_OUTFIT:
    case CMD_STEAL:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be)) 
	return TRUE;
      break;
    case CMD_GOTO:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
	  vnum == o || (vnum >= 0 && vnum <= 100))
	return TRUE;
      break;
    case CMD_EDIT:
    case CMD_REDIT:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
	  vnum == o)
        return TRUE;
      break;
    case CMD_STAT:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
          vnum == o || isGenericMob(vnum) || isGenericObj(vnum))
        return TRUE;
      break;
    case CMD_LOAD:
    case CMD_SHOW:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
          isGenericMob(vnum) || isGenericObj(vnum))
        return TRUE;
      break;
    case CMD_OEDIT:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
          isGenericObj(vnum))
        return TRUE;
      break;
    case CMD_MEDIT:
      if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
          isGenericMob(vnum))
        return TRUE;
      break;
    default:
      vlogf(LOG_DASH, format("%s called limits check with undefined command type (%d)") % 
	    getName() % (int)cmd);
      break;
  }
  return FALSE;
}

bool TBeing::isGenericObj(int vnum)
{
  if ((vnum >= 950 && vnum <= 1013) // training eq
      || (vnum >= 20 && vnum <= 34) // generic furniture
      || (vnum >= 100 && vnum <= 110) // generic lightables, window
      || (vnum >= 130 && vnum <= 139) // generic windows
      || (vnum >= 300 && vnum <= 342) // generic weapons
      || (vnum >= 400 && vnum <= 417) // generic food
      || (vnum >= 420 && vnum <= 443) // generic drink
      || (vnum < 0) // oed loaded object
      ) // add other generics here
    return TRUE;
  return FALSE;
}

bool TBeing::isGenericMob(int vnum)
{
  if ((vnum >= 1701 && vnum <= 1750) // testmobs
      || (vnum <= 0)
      ) // add other generics here
    return TRUE;
  return FALSE;
}


void setWizPowers(const TBeing *doer, TBeing *ch, const sstring &arg)
{
  // this is intended to "package" powers into groupings.  Yes, it's sort
  // of like the old "levels" idea that we moved away from, but since there
  // are *SO* many powers (mostly cheesy), it's good to just pump them
  // into groups and assign them this way.
  // beyond "god" (the old L54), assign powers on case-by-case basis.  Let's
  // avoid packaging powers above god

  if (is_abbrev(arg, "basic")) {
    ch->setWizPower(POWER_BUILDER);
    ch->setWizPower(POWER_WIZNET);
    ch->setWizPower(POWER_POWERS);
    ch->setWizPower(POWER_GOTO);
    ch->setWizPower(POWER_IMMORTAL_HELP);
    ch->setWizPower(POWER_SETSEV);
  } else if (is_abbrev(arg, "rembasic")) {
    ch->remWizPower(POWER_BUILDER);
    ch->remWizPower(POWER_WIZNET);
    ch->remWizPower(POWER_POWERS);
    ch->remWizPower(POWER_GOTO);
    ch->remWizPower(POWER_IMMORTAL_HELP);
    ch->remWizPower(POWER_SETSEV);
  } else if (is_abbrev(arg, "rooms")) {
    ch->setWizPower(POWER_REDIT);  
    ch->setWizPower(POWER_RSAVE);
    ch->setWizPower(POWER_EDIT);
    ch->setWizPower(POWER_RLOAD);
    ch->setWizPower(POWER_STAT);
    ch->setWizPower(POWER_SHOW);
    ch->setWizPower(POWER_PURGE);
  } else if (is_abbrev(arg, "remrooms")) {
    ch->remWizPower(POWER_REDIT);  
    ch->remWizPower(POWER_RSAVE);
    ch->remWizPower(POWER_EDIT);
    ch->remWizPower(POWER_RLOAD);
    ch->remWizPower(POWER_STAT);
    ch->remWizPower(POWER_SHOW);
    ch->remWizPower(POWER_PURGE);
  } else if (is_abbrev(arg, "mobs")) {
    ch->setWizPower(POWER_MEDIT);
    ch->setWizPower(POWER_STAT_MOBILES);
    ch->setWizPower(POWER_SHOW_MOB);
    ch->setWizPower(POWER_SEDIT);
    ch->setWizPower(POWER_IMMORTAL_OUTFIT);
    ch->setWizPower(POWER_WIZNET_ALWAYS);
    ch->setWizPower(POWER_LOAD);  // load rooms backdoor   
  } else if (is_abbrev(arg, "remmobs")) {
    ch->remWizPower(POWER_MEDIT);
    ch->remWizPower(POWER_STAT_MOBILES);
    ch->remWizPower(POWER_SHOW_MOB);
    ch->remWizPower(POWER_SEDIT);
    ch->remWizPower(POWER_IMMORTAL_OUTFIT);
    ch->remWizPower(POWER_WIZNET_ALWAYS);
    ch->remWizPower(POWER_LOAD);  // load rooms backdoor   
  } else if (is_abbrev(arg, "objs")) {
    ch->setWizPower(POWER_LOAD_SET);
    ch->setWizPower(POWER_STAT_OBJECT);
    ch->setWizPower(POWER_SHOW_OBJ);
    ch->setWizPower(POWER_OEDIT);
    ch->setWizPower(POWER_OEDIT_APPLYS);
    ch->setWizPower(POWER_OEDIT_WEAPONS);
    ch->setWizPower(POWER_OEDIT_COST);
  } else if (is_abbrev(arg, "remobjs")) {
    ch->remWizPower(POWER_LOAD_SET);
    ch->remWizPower(POWER_STAT_OBJECT);
    ch->remWizPower(POWER_SHOW_OBJ);
    ch->remWizPower(POWER_OEDIT);
    ch->remWizPower(POWER_OEDIT_APPLYS);
    ch->remWizPower(POWER_OEDIT_WEAPONS);
    ch->remWizPower(POWER_OEDIT_COST);
  } else if (is_abbrev(arg, "quest")) {
    ch->setWizPower(POWER_SWITCH);
    ch->setWizPower(POWER_NOSHOUT);
    ch->setWizPower(POWER_STEALTH);
    ch->setWizPower(POWER_QUEST);
    ch->setWizPower(POWER_AT);
    ch->setWizPower(POWER_WHERE);
    ch->setWizPower(POWER_SYSTEM);
    ch->setWizPower(POWER_LOAD_NOPROTOS);
  } else if (is_abbrev(arg, "remquest")) {
    ch->remWizPower(POWER_SWITCH);
    ch->remWizPower(POWER_NOSHOUT);
    ch->remWizPower(POWER_STEALTH);
    ch->remWizPower(POWER_QUEST);
    ch->remWizPower(POWER_AT);
    ch->remWizPower(POWER_WHERE);
    ch->remWizPower(POWER_SYSTEM);
    ch->remWizPower(POWER_LOAD_NOPROTOS);
  } else if (is_abbrev(arg, "demi")) {
    ch->setWizPower(POWER_COLOR_LOGS);
    ch->setWizPower(POWER_LONGDESC);
    ch->setWizPower(POWER_COMMENT);
    ch->setWizPower(POWER_FINDEMAIL);
    ch->setWizPower(POWER_CLIENTS);
    ch->setWizPower(POWER_TRACEROUTE);
    ch->setWizPower(POWER_HOSTLOG);
    ch->setWizPower(POWER_DEATHCHECK);
    ch->setWizPower(POWER_SNOWBALL);
    ch->setWizPower(POWER_PEE);
    ch->setWizPower(POWER_WIZLOCK);
    ch->setWizPower(POWER_CUTLINK);
    ch->setWizPower(POWER_SEE_COMMENTARY);  // view bugs, ideas, typos
    ch->setWizPower(POWER_ECHO);
    ch->setWizPower(POWER_TRANSFER);
    ch->setWizPower(POWER_TOGGLE);
    ch->setWizPower(POWER_VISIBLE);  // PCs can see them now
    ch->setWizPower(POWER_HEAVEN);
    ch->setWizPower(POWER_ZONEFILE_UTILITY);
    ch->setWizPower(POWER_INFO);
  } else if (is_abbrev(arg, "remdemi")) {
    ch->remWizPower(POWER_COLOR_LOGS);
    ch->remWizPower(POWER_LONGDESC);
    ch->remWizPower(POWER_COMMENT);
    ch->remWizPower(POWER_FINDEMAIL);
    ch->remWizPower(POWER_CLIENTS);
    ch->remWizPower(POWER_TRACEROUTE);
    ch->remWizPower(POWER_HOSTLOG);
    ch->remWizPower(POWER_DEATHCHECK);
    ch->remWizPower(POWER_SNOWBALL);
    ch->remWizPower(POWER_PEE);
    ch->remWizPower(POWER_WIZLOCK);
    ch->remWizPower(POWER_CUTLINK);
    ch->remWizPower(POWER_SEE_COMMENTARY);  // view bugs, ideas, typos
    ch->remWizPower(POWER_ECHO);
    ch->remWizPower(POWER_TRANSFER);
    ch->remWizPower(POWER_TOGGLE);
    ch->remWizPower(POWER_VISIBLE);  // PCs can see them now
    ch->remWizPower(POWER_HEAVEN);
    ch->remWizPower(POWER_ZONEFILE_UTILITY);
    ch->remWizPower(POWER_INFO);
  } else if (is_abbrev(arg, "trust")) {
    ch->setWizPower(POWER_INFO_TRUSTED);
    ch->setWizPower(POWER_GAMESTATS);
    ch->setWizPower(POWER_FLAG);
    ch->setWizPower(POWER_SHOW_TRUSTED);
    ch->setWizPower(POWER_RESTORE);
    ch->setWizPower(POWER_ACCESS);
    ch->setWizPower(POWER_USERS);
    ch->setWizPower(POWER_ACCOUNT);
  } else if (is_abbrev(arg, "remtrust")) {
    ch->remWizPower(POWER_INFO_TRUSTED);
    ch->remWizPower(POWER_GAMESTATS);
    ch->remWizPower(POWER_FLAG);
    ch->remWizPower(POWER_SHOW_TRUSTED);
    ch->remWizPower(POWER_RESTORE);
    ch->remWizPower(POWER_ACCESS);
    ch->remWizPower(POWER_USERS);
    ch->remWizPower(POWER_ACCOUNT);
  } else if (arg=="god") {
    ch->setWizPower(POWER_LOW);
    ch->setWizPower(POWER_GOD);
    ch->setWizPower(POWER_COMPARE);
    ch->setWizPower(POWER_REDIT_ENABLED);  // redit an enabled zone
    ch->setWizPower(POWER_STAT_SKILL);
    ch->setWizPower(POWER_RESTORE_MORTAL);
    ch->setWizPower(POWER_IMM_EVAL);
    ch->setWizPower(POWER_FORCE);
    ch->setWizPower(POWER_LOG);
    ch->setWizPower(POWER_PURGE_PC);
    ch->setWizPower(POWER_PURGE_ROOM);
    ch->setWizPower(POWER_EGOTRIP);
    ch->setWizPower(POWER_CHECKLOG);
    ch->setWizPower(POWER_LOGLIST);
    ch->setWizPower(POWER_REPLACE);
    ch->setWizPower(POWER_RESIZE);
    ch->setWizPower(POWER_NO_LIMITS);
  } else if (arg=="remgod") {
    ch->remWizPower(POWER_LOW);
    ch->remWizPower(POWER_GOD);
    ch->remWizPower(POWER_COMPARE);
    ch->remWizPower(POWER_REDIT_ENABLED);  // redit an enabled zone
    ch->remWizPower(POWER_STAT_SKILL);
    ch->remWizPower(POWER_RESTORE_MORTAL);
    ch->remWizPower(POWER_IMM_EVAL);
    ch->remWizPower(POWER_FORCE);
    ch->remWizPower(POWER_LOG);
    ch->remWizPower(POWER_PURGE_PC);
    ch->remWizPower(POWER_PURGE_ROOM);
    ch->remWizPower(POWER_EGOTRIP);
    ch->remWizPower(POWER_CHECKLOG);
    ch->remWizPower(POWER_LOGLIST);
    ch->remWizPower(POWER_REPLACE);
    ch->remWizPower(POWER_RESIZE);
    ch->remWizPower(POWER_NO_LIMITS);
  } else if (arg=="allpowers") {
    wizPowerT wpt;
    for (wpt = MIN_POWER_INDEX; wpt < MAX_POWER_INDEX; wpt++)
      ch->setWizPower(wpt);
      ch->remWizPower(POWER_IDLED);
  } else if (is_abbrev(arg, "allpowers")) {
    doer->sendTo("This gives them *ALL* powers, don't do it unless you really really mean to.\n\r");
    doer->sendTo("You have to type the whole word 'allpowers' to do it too.\n\r");
    return;
  } else if (arg=="remall") {
    wizPowerT wpt;
    for (wpt = MIN_POWER_INDEX; wpt < MAX_POWER_INDEX; wpt++)
      ch->remWizPower(wpt);
      ch->setWizPower(POWER_IDLED);
  } else if (is_abbrev(arg, "remall")) {
    doer->sendTo("This removes *ALL* powers, don't do it unless you really really mean to.\n\r");
    doer->sendTo("You have to type the whole word 'remall' to do it too.\n\r");
    return;
  } else {
    doer->sendTo("Outside of range.\n\r");
    doer->sendTo("Syntax: @set wizpower <person> <power>.\n\r");
    doer->sendTo("Syntax: @set wizpower <person> <\"basic\" | \"rooms\" | \"mobs\" | \"objs\" | \"demi\" | \"quest\" | \"trust\" | \"allpowers\">\n\r");
    doer->sendTo("Syntax: @set wizpower <person> <\"rembasic\" | \"remrooms\" | \"remmobs\" | \"remobjs\" | \"remdemi\" | \"remquest\" | \"remtrust\" | \"remall\">\n\r");
    return;
  }
  doer->sendTo("Wizpowers have been toggled.\n\r");
}

bool TBeing::hasWizPower(wizPowerT) const
{
  return false;
}

void TBeing::setWizPower(wizPowerT)
{
}

void TBeing::remWizPower(wizPowerT)
{
}

bool TPerson::hasWizPower(wizPowerT value) const
{
  if (value >= MAX_POWER_INDEX) {
    vlogf(LOG_BUG, format("Bad check of hasWizPower(%d)") %  value);
    return FALSE;
  }

  if (wizPowers[POWER_IDLED] && !wizPowers[POWER_WIZARD])
    switch (value) {
      case POWER_BUILDER:
      case POWER_GOD:
      case POWER_WIZARD:
      case POWER_GOTO:
      case POWER_IDLED:
        break;
      default:
        return false;
    }

  return (wizPowers[value]);
}

void TPerson::setWizPower(wizPowerT value)
{
  if (value >= MAX_POWER_INDEX) {
    vlogf(LOG_BUG, format("Bad check of setWizPower(%d)") %  value);
    return;
  }

  wizPowers[value] |= 0x1;
}

void TPerson::remWizPower(wizPowerT value)
{
  if (value >= MAX_POWER_INDEX) {
    vlogf(LOG_BUG, format("Bad check of remWizPower(%d)") %  value);
    return;
  }

  wizPowers[value] &= ~(0x1);
}

void TPerson::saveWizPowers()
{
  if (GetMaxLevel() <= MAX_MORT)
    return;

  TDatabase db(DB_SNEEZY);

  for (wizPowerT num = MIN_POWER_INDEX; num < MAX_POWER_INDEX; num++) {
    if (wizPowers[num] != wizPowersOriginal[num]) {
      if (!wizPowersOriginal[num])
        db.query("insert into wizpower (player_id, wizpower) values (%i, %i)", getPlayerID(), mapWizPowerToFile(num));
      else
        db.query("delete from wizpower where player_id=%i and wizpower=%i", getPlayerID(), mapWizPowerToFile(num));
    }
    wizPowersOriginal[num] = wizPowers[num];
  }
}

void TPerson::loadWizPowers()
{
  if (GetMaxLevel() <= MAX_MORT)
    return;
  memset(&wizPowersOriginal, 0, sizeof(wizPowersOriginal));

  TDatabase db(DB_SNEEZY);

  db.query("select wizpower from wizpower where player_id=%i",
	   getPlayerID());
  
  while(db.fetchRow()) {
    setWizPower(mapFileToWizPower(convertTo<int>(db["wizpower"])));
    wizPowersOriginal[mapFileToWizPower(convertTo<int>(db["wizpower"]))] |= 0x1;
  }
}

void TBeing::doPowers(const sstring &) const
{
  sendTo("Mobs don't get powers, go away!");
  return;
}

void TPerson::doPowers(const sstring &argument) const
{
  if (!hasWizPower(POWER_POWERS)) {
    incorrectCommand();
    return;
  }
  if (!desc)
    return;

  sstring    tStName, tStPower, tStString, tString;
  const     TBeing *ch;
  wizPowerT tWizPower;
  bool      wizPowerList[MAX_POWER_INDEX];

  memset(&wizPowerList, 0, sizeof(wizPowerList));

  tStName=argument.word(0);
  tStPower=argument.word(1);

  if (!tStName.empty()) {
    ch = get_pc_world(this, tStName, EXACT_YES);

    if (!ch)
      ch = get_pc_world(this, tStName, EXACT_NO);

    if (!ch) {
      TDatabase db(DB_SNEEZY);

      db.query("select wizpower from wizpower w, player p where p.name='%s' and p.id=w.player_id", tStName.lower().c_str());

      if(db.fetchRow()){
        sendTo("Player not logged in but file found.  Reading in...\n\r");

	do {
          wizPowerList[mapFileToWizPower(convertTo<int>(db["wizpower"]))]|=0x1;
	} while(db.fetchRow());
      } else {
        sendTo("Unable to locate them anywhere in the world or in the files.\n\r");
        return;
      }
    } else {
      for (tWizPower = MIN_POWER_INDEX; tWizPower < MAX_POWER_INDEX; tWizPower++)
         wizPowerList[tWizPower] = ch->hasWizPower(tWizPower);
    }
  } else {
    ch = this;

    for (tWizPower = MIN_POWER_INDEX; tWizPower < MAX_POWER_INDEX; tWizPower++)
      wizPowerList[tWizPower] = ch->hasWizPower(tWizPower);
  }

  tString = format("%s%s Wiz-Powers:\n\r") %
    (ch == this ? "Your" : (ch ? ch->getName() : tStName)) %
    (ch == this ? "" : "'s");
  tStString += tString;

  for (tWizPower = MIN_POWER_INDEX; tWizPower < MAX_POWER_INDEX; tWizPower++) {
    tString = tStPower;

    if (tStPower.empty() ||
        (is_number(tString) && convertTo<int>(tStPower) == (tWizPower + 1)) ||
        (!is_number(tString) &&
         is_abbrev(tStPower, getWizPowerName(tWizPower)))) {
      tString = format("%3d.) [%c] %-25.25s") %
              (tWizPower + 1) %
              (wizPowerList[tWizPower] ? '*' : ' ') %
              getWizPowerName(tWizPower);
      tStString += tString;

      if ((tWizPower % 2) || !tStPower.empty())
        tStString += "\n\r";
      else
        tStString += "      ";
    }
  }

  if ((tWizPower % 2))
    tStString += "\n\r";

  desc->page_string(tStString, SHOWNOW_NO, ALLOWREP_YES);  
}

const sstring getWizPowerName(wizPowerT wpt)
{
  // powers command truncs at 20 chars
  switch (wpt) {
    case POWER_BUILDER:
      return "Builder-Title";
    case POWER_GOD:
      return "God-Title";
    case POWER_WIZARD:
      return "Wizard-Title";
    case POWER_WIZNET:
      return "Wiznet";
    case POWER_WIZNET_ALWAYS:
      return "Wiznet-nobuilders";
    case POWER_REDIT:
      return "REdit";
    case POWER_REDIT_ENABLED:
      return "REdit-enabled zone";
    case POWER_EDIT:
      return "Edit";
    case POWER_RLOAD:
      return "RLoad";
    case POWER_RSAVE:
      return "RSave";
    case POWER_POWERS:
      return "Powers";
    case POWER_MEDIT:
      return "MEdit";
    case POWER_MEDIT_LOAD_ANYWHERE:
      return "MEdit-load anywhere";
    case POWER_MEDIT_IMP_POWER:
      return "MEdit-imp power";
    case POWER_OEDIT:
      return "OEdit";
    case POWER_OEDIT_COST:
      return "OEdit-cost";
    case POWER_OEDIT_APPLYS:
      return "OEdit-applies";
    case POWER_OEDIT_WEAPONS:
      return "OEdit-weapons";
    case POWER_OEDIT_NOPROTOS:
      return "OEdit-noprotos";
    case POWER_OEDIT_IMP_POWER:
      return "OEdit-imp power";
    case POWER_GOTO:
      return "Goto";
    case POWER_GOTO_IMP_POWER:
      return "Goto-imp power";
    case POWER_ECHO:
      return "Echo";
    case POWER_LONGDESC:
      return "Longdesc";
    case POWER_COMMENT:
      return "Comment";
    case POWER_FINDEMAIL:
      return "Findemail";
    case POWER_CLIENTS:
      return "Clients";
    case POWER_LOW:
      return "Low";
    case POWER_TRACEROUTE:
      return "Traceroute";
    case POWER_HOSTLOG:
      return "Hostlog";
    case POWER_ACCESS:
      return "Access";
    case POWER_DEATHCHECK:
      return "Deathcheck";
    case POWER_SNOWBALL:
      return "Snowball";
    case POWER_PEE:
      return "Pee";
    case POWER_SWITCH:
      return "Switch";
    case POWER_USERS:
      return "Users";
    case POWER_STEALTH:
      return "Stealth";
    case POWER_WIZLOCK:
      return "Wizlock";
    case POWER_CUTLINK:
      return "Cutlink";
    case POWER_TRANSFER:
      return "Transfer";
    case POWER_LOG:
      return "Log";
    case POWER_TOGGLE:
      return "Toggle";
    case POWER_TOGGLE_INVISIBILITY:
      return "Toggle-invisibility";
    case POWER_BREATHE:
      return "Breathe";
    case POWER_SHOW:
      return "Show";
    case POWER_SHOW_MOB:
      return "Show-mobiles";
    case POWER_SHOW_OBJ:
      return "Show-objects";
    case POWER_SHOW_TRUSTED:
      return "Show-trusted";
    case POWER_FLAG:
      return "Flag";
    case POWER_FLAG_IMP_POWER:
      return "Flag-nosnoop";
    case POWER_STAT:
      return "Stat";
    case POWER_STAT_MOBILES:
      return "Stat-mobiles";
    case POWER_STAT_OBJECT:
      return "Stat-objects";
    case POWER_STAT_SKILL:
      return "Stat-skills";
    case POWER_LOAD:
      return "Load";
    case POWER_LOAD_SET:
      return "Load-set";
    case POWER_LOAD_NOPROTOS:
      return "Load-noprotos";
    case POWER_LOAD_LIMITED:
      return "Load-limited";
    case POWER_LOAD_IMP_POWER:
      return "Load-imp power";
    case POWER_PURGE:
      return "Purge";
    case POWER_PURGE_PC:
      return "Purge-PC";
    case POWER_PURGE_ROOM:
      return "Purge-room";
    case POWER_PURGE_LINKS:
      return "Purge-links";
    case POWER_EGOTRIP:
      return "Egotrip";
    case POWER_FORCE:
      return "Force";
    case POWER_SNOOP:
      return "Snoop";
    case POWER_CHANGE:
      return "Change";
    case POWER_SET:
      return "@set";
    case POWER_SET_IMP_POWER:
      return "@set-imp power";
    case POWER_WIPE:
      return "Wipe";
    case POWER_COLOR_LOGS:
      return "Color Logs";
    case POWER_VISIBLE:
      return "Visible";
    case POWER_MULTIPLAY:
      return "Multiplay";
    case POWER_RENAME:
      return "Rename Mortal";
    case POWER_IMM_EVAL:
      return "Immortal Evaluation";
    case POWER_BOARD_POLICE:
      return "Board Police";
    case POWER_VIEW_IMM_ACCOUNTS:
      return "View Imm Accounts";
    case POWER_COMPARE:
      return "Compare";
    case POWER_IMMORTAL_HELP:
      return "Help - immortal";
    case POWER_QUEST:
      return "Quest";
    case POWER_SEE_FACTION_SENDS:
      return "See Faction Sends";
    case POWER_SEE_COMMENTARY:
      return "See Bug/Idea/Typo";
    case POWER_AT:
      return "At";
    case POWER_WHERE:
      return "Where";
    case POWER_SYSTEM:
      return "System";
    case POWER_GAMESTATS:
      return "Gamestats";
    case POWER_HEAVEN:
      return "Heaven";
    case POWER_ACCOUNT:
      return "Account";
    case POWER_ZONEFILE_UTILITY:
      return "Zonefile Utility";
    case POWER_RESTORE:
      return "Restore";
    case POWER_RESTORE_MORTAL:
      return "Restore Mortal";
    case POWER_INFO:
      return "Info";
    case POWER_INFO_TRUSTED:
      return "Info-trusted";
    case POWER_NOSHOUT:
      return "Noshout";
    case POWER_CHECKLOG:
      return "Checklog";
    case POWER_LOGLIST:
      return "Loglist";
    case POWER_REPLACE:
      return "Replace";
    case POWER_REPLACE_PFILE:
      return "Replace-playerfile";
    case POWER_RESIZE:
      return "Resize";
    case POWER_SEDIT:
      return "SEdit";
    case POWER_SEDIT_IMP_POWER:
      return "SEdit-Imp";
    case POWER_CRIT:
      return "Crit";
    case POWER_SHUTDOWN:
      return "Shutdown";
    case POWER_SLAY:
      return "Slay";
    case POWER_TIMESHIFT:
      return "Timeshift";
    case POWER_RESET:
      return "Reset";
    case POWER_IMMORTAL_OUTFIT:
      return "Outfit-immortal";
    case POWER_SETSEV:
      return "Setsev";
    case POWER_SETSEV_IMM:
      return "Setsev-Advanced";
    case POWER_IDLED:
      return "Inactive";
    case POWER_NO_LIMITS:
      return "No Limitations";
    case POWER_CLONE:
      return "Clone";
    case MAX_POWER_INDEX:
      break;
  }
  return "";
}

