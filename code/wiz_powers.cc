//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: wiz_powers.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.2  1999/10/12 04:10:12  lapsos
// Added power <name> <power> single power display output.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include <unistd.h>

#include "stdsneezy.h"

bool TBeing::powerCheck(wizPowerT wpt) const
{
  if (!hasWizPower(wpt)) {
    sendTo("%s: You lack this power.\n\r", getWizPowerName(wpt).c_str());
    return true;
  }
  return false;
}

void setWizPowers(const TBeing *doer, TBeing *ch, const char *arg)
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
    ch->setWizPower(POWER_REDIT);
    ch->setWizPower(POWER_RSAVE);
    ch->setWizPower(POWER_EDIT);
    ch->setWizPower(POWER_RLOAD);
    ch->setWizPower(POWER_POWERS);
    ch->setWizPower(POWER_STAT);
    ch->setWizPower(POWER_GOTO);
    ch->setWizPower(POWER_SHOW);
    ch->setWizPower(POWER_PURGE);
    ch->setWizPower(POWER_LOAD);  // load rooms backdoor
  } else if (is_abbrev(arg, "mobiles")) {
    ch->setWizPower(POWER_MEDIT);
    ch->setWizPower(POWER_STAT_MOBILES);
    ch->setWizPower(POWER_SHOW_MOB);
    ch->setWizPower(POWER_SEDIT);
    ch->setWizPower(POWER_IMMORTAL_OUTFIT);
    ch->setWizPower(POWER_WIZNET_ALWAYS);
  } else if (is_abbrev(arg, "objects")) {
    ch->setWizPower(POWER_LOAD_SET);
    ch->setWizPower(POWER_STAT_OBJECT);
    ch->setWizPower(POWER_SHOW_OBJ);
    ch->setWizPower(POWER_OEDIT);
    ch->setWizPower(POWER_OEDIT_APPLYS);
    ch->setWizPower(POWER_OEDIT_WEAPONS);
    ch->setWizPower(POWER_OEDIT_COST);
  } else if (is_abbrev(arg, "demigod")) {
    ch->setWizPower(POWER_COLOR_LOGS);
    ch->setWizPower(POWER_LONGDESC);
    ch->setWizPower(POWER_COMMENT);
    ch->setWizPower(POWER_FINDEMAIL);
    ch->setWizPower(POWER_CLIENTS);
    ch->setWizPower(POWER_TRACEROUTE);
    ch->setWizPower(POWER_HOSTLOG);
    ch->setWizPower(POWER_NOSHOUT);
    ch->setWizPower(POWER_DEATHCHECK);
    ch->setWizPower(POWER_SNOWBALL);
    ch->setWizPower(POWER_PEE);
    ch->setWizPower(POWER_SWITCH);
    ch->setWizPower(POWER_STEALTH);
    ch->setWizPower(POWER_WIZLOCK);
    ch->setWizPower(POWER_CUTLINK);
    ch->setWizPower(POWER_IMMORTAL_HELP);
    ch->setWizPower(POWER_QUEST);
    ch->setWizPower(POWER_SEE_COMMENTARY);  // view bugs, ideas, typos
    ch->setWizPower(POWER_ECHO);
    ch->setWizPower(POWER_TRANSFER);
    ch->setWizPower(POWER_TOGGLE);
    ch->setWizPower(POWER_VISIBLE);  // PCs can see them now
    ch->setWizPower(POWER_HEAVEN);
    ch->setWizPower(POWER_ZONEFILE_UTILITY);
    ch->setWizPower(POWER_INFO);
  } else if (is_abbrev(arg, "trusted")) {
    ch->setWizPower(POWER_INFO_TRUSTED);
    ch->setWizPower(POWER_AT);
    ch->setWizPower(POWER_WHERE);
    ch->setWizPower(POWER_SYSTEM);
    ch->setWizPower(POWER_GAMESTATS);
    ch->setWizPower(POWER_FLAG);
    ch->setWizPower(POWER_SHOW_TRUSTED);
    ch->setWizPower(POWER_RESTORE);
    ch->setWizPower(POWER_ACCESS);
    ch->setWizPower(POWER_USERS);
    ch->setWizPower(POWER_ACCOUNT);
  } else if (!strcmp(arg, "god")) {
    ch->setWizPower(POWER_LOW);
    ch->setWizPower(POWER_GOD);
    ch->setWizPower(POWER_COMPARE);
    ch->setWizPower(POWER_REDIT_ENABLED);  // redit an enabled zone
    ch->setWizPower(POWER_STAT_SKILL);
    ch->setWizPower(POWER_RESTORE_MORTAL);
    ch->setWizPower(POWER_IMM_EVAL);
    ch->setWizPower(POWER_FORCE);
    ch->setWizPower(POWER_LOG);
    ch->setWizPower(POWER_LOAD_NOPROTOS);
    ch->setWizPower(POWER_PURGE_PC);
    ch->setWizPower(POWER_PURGE_ROOM);
    ch->setWizPower(POWER_EGOTRIP);
    ch->setWizPower(POWER_CHECKLOG);
    ch->setWizPower(POWER_LOGLIST);
    ch->setWizPower(POWER_REPLACE);
    ch->setWizPower(POWER_RESIZE);
  } else if (!strcmp(arg, "allpowers")) {
    wizPowerT wpt;
    for (wpt = MIN_POWER_INDEX; wpt < MAX_POWER_INDEX; wpt++)
      ch->setWizPower(wpt);
  } else if (is_abbrev(arg, "allpowers")) {
    doer->sendTo("This gives them *ALL* powers, don't do it unless you really really mean to.\n\r");
    doer->sendTo("You have to type the whole word 'allpowers' to do it too.\n\r");
    return;
  } else {
    doer->sendTo("Outside of range.\n\r");
    doer->sendTo("Syntax: @set wizpower <person> <power>.\n\r");
    doer->sendTo("Syntax: @set wizpower <person> <\"basic\" | \"mobiles\" | \"objects\" | \"demigod\" | \"trusted\" | \"allpowers\">\n\r");
    return;
  }
  doer->sendTo("Wiz Powers set OK!\n\r");
}

// this is a temp function, meant to migrate to new "power" system from
// the old level-based setup
void giveGodsTheirPowers(TBeing *ch)
{
#if 0
  int lev = ch->GetMaxLevel();
  if (lev <= MAX_MORT)
    return;
  if (lev >= GOD_LEVEL1) {
    ch->setWizPower(POWER_BUILDER);
    ch->setWizPower(POWER_WIZNET);
    ch->setWizPower(POWER_REDIT);
    ch->setWizPower(POWER_RSAVE);
    ch->setWizPower(POWER_EDIT);
    ch->setWizPower(POWER_RLOAD);
    ch->setWizPower(POWER_POWERS);
    ch->setWizPower(POWER_STAT);
    ch->setWizPower(POWER_GOTO);
    ch->setWizPower(POWER_SHOW);
    ch->setWizPower(POWER_RESTORE);
    ch->setWizPower(POWER_INFO);
  }
  if (lev >= GOD_LEVEL2) {
    ch->setWizPower(POWER_VISIBLE);
    ch->setWizPower(POWER_WIZNET_ALWAYS);
    ch->setWizPower(POWER_MEDIT);
    ch->setWizPower(POEWR_SEDIT);
    ch->setWizPower(POWER_OEDIT);
    ch->setWizPower(POWER_OEDIT_COST);
    ch->setWizPower(POWER_STAT_OBJECT);
    ch->setWizPower(POWER_STAT_MOBILES);
    ch->setWizPower(POWER_COLOR_LOGS);
    ch->setWizPower(POWER_ECHO);
    ch->setWizPower(POWER_TRANSFER);
    ch->setWizPower(POWER_TOGGLE);
    ch->setWizPower(POWER_LOAD);
    ch->setWizPower(POWER_PURGE);
    ch->setWizPower(POWER_SHOW_MOB);
    ch->setWizPower(POWER_SHOW_OBJ);
    ch->setWizPower(POWER_LONGDESC);
    ch->setWizPower(POWER_COMMENT);
    ch->setWizPower(POWER_FINDEMAIL);
    ch->setWizPower(POWER_CLIENTS);
    ch->setWizPower(POWER_TRACEROUTE);
    ch->setWizPower(POWER_HOSTLOG);
    ch->setWizPower(POWER_ACCESS);
    ch->setWizPower(POWER_DEATHCHECK);
    ch->setWizPower(POWER_SNOWBALL);
    ch->setWizPower(POWER_PEE);
    ch->setWizPower(POWER_SWITCH);
    ch->setWizPower(POWER_USERS);
    ch->setWizPower(POWER_STEALTH);
    ch->setWizPower(POWER_WIZLOCK);
    ch->setWizPower(POWER_CUTLINK);
    ch->setWizPower(POWER_IMMORTAL_HELP);
    ch->setWizPower(POWER_QUEST);
    ch->setWizPower(POWER_SEE_COMMENTARY);
  }
  if (lev >= GOD_LEVEL3) {
    ch->setWizPower(POWER_HEAVEN);
    ch->setWizPower(POWER_NOSHOUT);
    ch->setWizPower(POWER_INFO_TRUSTED);
    ch->setWizPower(POWER_AT);
    ch->setWizPower(POWER_WHERE);
    ch->setWizPower(POWER_SYSTEM);
    ch->setWizPower(POWER_GAMESTATS);
    ch->setWizPower(POWER_MEDIT_LOAD_ANYWHERE);
    ch->setWizPower(POWER_OEDIT_APPLYS);
    ch->setWizPower(POWER_FLAG);
    ch->setWizPower(POWER_SHOW_TRUSTED);
    ch->setWizPower(POWER_ACCOUNT);
    ch->setWizPower(POWER_ZONEFILE_UTILITY);
  }
  if (lev >= GOD_LEVEL4) {
    ch->setWizPower(POWER_RESTORE_MORTAL);
    ch->setWizPower(POWER_GOD);
    ch->setWizPower(POWER_COMPARE);
    ch->setWizPower(POWER_REDIT_ENABLED);
    ch->setWizPower(POWER_STAT_SKILL);
    ch->setWizPower(POWER_IMM_EVAL);
    ch->setWizPower(POWER_OEDIT_WEAPONS);
    ch->setWizPower(POWER_FORCE);
    ch->setWizPower(POWER_LOG);
    ch->setWizPower(POWER_LOAD_SET);
    ch->setWizPower(POWER_LOAD_NOPROTOS);
    ch->setWizPower(POWER_PURGE_PC);
    ch->setWizPower(POWER_PURGE_ROOM);
    ch->setWizPower(POWER_EGOTRIP);
    ch->setWizPower(POWER_CHECKLOG);
    ch->setWizPower(POWER_LOGLIST);
    ch->setWizPower(POWER_REPLACE);
    ch->setWizPower(POWER_RESIZE);
  }
  if (lev >= GOD_LEVEL5) {
    ch->setWizPower(POWER_LOW);
    ch->setWizPower(POWER_BOARD_POLICE);
    ch->setWizPower(POWER_SNOOP);
    ch->setWizPower(POWER_BREATHE);
  }
  if (lev >= GOD_LEVEL6) {
    ch->setWizPower(POWER_VIEW_IMM_ACCOUNTS);
    ch->setWizPower(POWER_SET);
    ch->setWizPower(POWER_MULTIPLAY);
    ch->setWizPower(POWER_CHANGE);
    ch->setWizPower(POWER_LOAD_LIMITED);
    ch->setWizPower(POWER_PURGE_LINKS);
  }
  if (lev >= GOD_LEVEL7) {
    ch->setWizPower(POWER_WIZARD);
    ch->setWizPower(POWER_OEDIT_NOPROTOS);
    ch->setWizPower(POWER_RENAME);
    ch->setWizPower(POWER_TOGGLE_INVISIBILITY);
    ch->setWizPower(POWER_FLAG_IMP_POWER);
    ch->setWizPower(POWER_GOTO_IMP_POWER);
  }
  if (lev >= GOD_LEVEL9) {
    ch->setWizPower(POWER_MEDIT_IMP_POWER);
    ch->setWizPower(POWER_OEDIT_IMP_POWER);
    ch->setWizPower(POWER_SEDIT_IMP_POEWR);
    ch->setWizPower(POWER_SET_IMP_POWER);
    ch->setWizPower(POWER_LOAD_IMP_POWER);
    ch->setWizPower(POWER_WIPE);
  }
#endif
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
    vlogf(6, "Bad check of hasWizPower(%d)", value);
    return FALSE;
  }

  return (wizPowers[value]);
}

void TPerson::setWizPower(wizPowerT value)
{
  if (value >= MAX_POWER_INDEX) {
    vlogf(6, "Bad check of setWizPower(%d)", value);
    return;
  }

  wizPowers[value] |= 0x1;
}

void TPerson::remWizPower(wizPowerT value)
{
  if (value >= MAX_POWER_INDEX) {
    vlogf(6, "Bad check of remWizPower(%d)", value);
    return;
  }

  wizPowers[value] &= ~(0x1);
}

void TPerson::saveWizPowers()
{
  if (GetMaxLevel() <= MAX_MORT)
    return;

  char caFilebuf[128];
  FILE *fp;

  sprintf(caFilebuf, "player/%c/%s.wizpower", LOWER(name[0]), lower(name).c_str());

  if (!(fp = fopen(caFilebuf, "w")))
    return;

  unsigned int total = 0;
  wizPowerT num;
  for (num = MIN_POWER_INDEX; num < MAX_POWER_INDEX; num++) {
    if (hasWizPower(num)) {
      total++;
      fprintf(fp, "%u ", mapWizPowerToFile(num));
    }
  }
  fclose(fp);

  if (!total)
    unlink(caFilebuf);
}

void TPerson::loadWizPowers()
{
  if (GetMaxLevel() <= MAX_MORT)
    return;

  char caFilebuf[128];
  FILE *fp;

  sprintf(caFilebuf, "player/%c/%s.wizpower", LOWER(name[0]), lower(name).c_str());

  if (!(fp = fopen(caFilebuf, "r")))
    return;

  unsigned int num;
  while (fscanf(fp, "%u ", &num) == 1)
    setWizPower(mapFileToWizPower(num));

  fclose(fp);
}

void TBeing::doPowers(const char *) const
{
  sendTo("Mobs don't get powers, go away!");
  return;
}

void TPerson::doPowers(const char *argument) const
{
  if (!hasWizPower(POWER_POWERS)) {
    incorrectCommand();
    return;
  }
  if (!desc)
    return;

  string    tStName(""),
            tStPower(""),
            tStString("");
  char      tString[MAX_INPUT_LENGTH];
  const     TBeing *ch;
  wizPowerT tWizPower;

  two_arg(argument, tStName, tStPower);

  if (!tStName.empty()) {
    ch = get_pc_world(this, tStName.c_str(), EXACT_YES);

    if (!ch)
      ch = get_pc_world(this, tStName.c_str(), EXACT_NO);

    if (!ch) {
      sendTo("Unable to locate them anywhere in the world.\n\r");
      return;
    }
  } else
    ch = this;

  sprintf(tString, "%s%s Wiz-Powers:\n\r",
          (ch == this ? "Your" : ch->getName()),
          (ch == this ? "" : "'s"));
  tStString += tString;

  for (tWizPower = MIN_POWER_INDEX; tWizPower < MAX_POWER_INDEX; tWizPower++) {
    strcpy(tString, tStPower.c_str());

    if (tStPower.empty() ||
        (is_number(tString) && atoi(tStPower.c_str()) == (tWizPower + 1)) ||
        (!is_number(tString) &&
         is_abbrev(tStPower.c_str(), getWizPowerName(tWizPower).c_str()))) {
      sprintf(tString, "%3d.) [%c] %-25.25s",
              (tWizPower + 1),
              (ch->hasWizPower(tWizPower) ? '*' : ' '),
              getWizPowerName(tWizPower).c_str());
      tStString += tString;

      if ((tWizPower % 2) || !tStPower.empty())
        tStString += "\n\r";
      else
        tStString += "      ";
    }
  }

  if (!(tWizPower % 2))
    tStString += "\n\r";

  desc->page_string(tStString.c_str(), 0, true);  

  /*
  char arg[MAX_INPUT_LENGTH];
  one_argument(argument, arg);  

  const TBeing *ch;
  if (arg && *arg) {
    ch = get_pc_world(this, arg, EXACT_YES);
    if (!ch)
      ch = get_pc_world(this, arg, EXACT_NO);
    if (!ch) {
      sendTo("Couldn't locate any such PC in world.\n\r");
      return;
    }
  } else
    ch = this;

  string str;
  char buf[256];
  sprintf(buf, "%s%s Wiz-Powers:\n\r",
    ch == this ? "Your" : ch->getName(),
    ch == this ? "" : "'s");
  str += buf;

  wizPowerT wpt;
  for (wpt = MIN_POWER_INDEX; wpt < MAX_POWER_INDEX; wpt++) {
    sprintf(buf, "%3d.) [%c] %-25.25s",
        wpt+1,
        ch->hasWizPower(wpt) ? '*' : ' ',
        getWizPowerName(wpt).c_str());
    str += buf;
    if ((wpt % 2) == 1)
      str += "\n\r";
    else
      str += "      ";
  }

  str += "\n\r";

  desc->page_string(str.c_str(), 0, true);  
  */
}

const string getWizPowerName(wizPowerT wpt)
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
    case MAX_POWER_INDEX:
      break;
  }
  return "";
}

