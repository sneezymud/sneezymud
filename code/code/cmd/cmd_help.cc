#include <stdio.h>

#include "extern.h"
#include "being.h"

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <dirent.h>
}
 
#include "help.h"
#include "obj_component.h"
#include "statistics.h"
#include "systemtask.h"
#include "database.h"

#define ARTICLE_LIST_WIDTH 80

static std::vector<sstring>helpIndex(0);
static std::vector<char *>immortalIndex(0);
static std::vector<char *>builderIndex(0);
static std::vector<char *>skillIndex(0);
static std::vector<char *>spellIndex(0);

#if 0
static const char *start_name(byte num)
{
  if (num <= 5)
    return "Neophyte";
  else if (num <= 10)
    return "Novice";
  else if (num <= 20)
    return "Beginner";
  else if (num <= 30)
    return "Student";
  else if (num <= 40)
    return "Scholar";
  else if (num <= 50)
    return "Expositor";
  else if (num <= 70)
    return "Proctor";
  else
    return "Master";
}
#endif

wizPowerT wizPowerFromCmd(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_CHANGE:
      return POWER_CHANGE;
      break;
    case CMD_ECHO:
      return POWER_ECHO;
      break;
    case CMD_FORCE:
      return POWER_FORCE;
      break;
    case CMD_TRANSFER:
      return POWER_TRANSFER;
      break;
    case CMD_STAT:
      return POWER_STAT;
      break;
    case CMD_LOAD:
      return POWER_LOAD;
      break;
    case CMD_PURGE:
      return POWER_PURGE;
      break;
    case CMD_AT:
      return POWER_AT;
      break;
    case CMD_SNOOP:
      return POWER_SNOOP;
      break;
    case CMD_AS:
    case CMD_SWITCH:
      return POWER_SWITCH;
      break;
    case CMD_SNOWBALL:
      return POWER_SNOWBALL;
      break;
    case CMD_INFO:
      return POWER_INFO;
      break;
    case CMD_WHERE:
      return POWER_WHERE;
      break;
    case CMD_PEE:
      return POWER_PEE;
      break;
    case CMD_WIZNET:
      return POWER_WIZNET;
      break;
    case CMD_RESTORE:
      return POWER_RESTORE;
      break;
    case CMD_USERS:
      return POWER_USERS;
      break;
    case CMD_SYSTEM:
      return POWER_SYSTEM;
      break;
    case CMD_BESTOW:
    case CMD_SET:
      return POWER_SET;
      break;
    case CMD_RSAVE:
      return POWER_RSAVE;
      break;
    case CMD_RLOAD:
      return POWER_RLOAD;
      break;
    case CMD_WIZLOCK:
      return POWER_WIZLOCK;
      break;
    case CMD_SHOW:
      return POWER_SHOW;
      break;
    case CMD_TOGGLE:
      return POWER_TOGGLE;
      break;
    case CMD_BREATH:
      return POWER_BREATHE;
      break;
    case CMD_LOG:
      return POWER_LOG;
      break;
    case CMD_WIPE:
      return POWER_WIPE;
      break;
    case CMD_CUTLINK:
      return POWER_CUTLINK;
      break;
    case CMD_CHECKLOG:
      return POWER_CHECKLOG;
      break;
    case CMD_OFFICE:
      return POWER_GOTO;
      break;
    case CMD_LOGLIST:
      return POWER_LOGLIST;
      break;
    case CMD_DEATHCHECK:
      return POWER_DEATHCHECK;
      break;
    case CMD_REDIT:
      return POWER_REDIT;
      break;
    case CMD_OEDIT:
      return POWER_OEDIT;
      break;
    case CMD_MEDIT:
      return POWER_MEDIT;
      break;
    case CMD_CLONE:
    case CMD_ACCESS:
      return POWER_ACCESS;
      break;
    case CMD_REPLACE:
      return POWER_REPLACE;
      break;
    case CMD_GAMESTATS:
      return POWER_GAMESTATS;
      break;
    case CMD_HOSTLOG:
      return POWER_HOSTLOG;
      break;
    case CMD_TRACEROUTE:
      return POWER_TRACEROUTE;
      break;
    case CMD_LOW:
      return POWER_LOW;
      break;
    case CMD_RESIZE:
      return POWER_RESIZE;
      break;
    case CMD_HEAVEN:
      return POWER_HEAVEN;
      break;
    case CMD_ACCOUNT:
      return POWER_ACCOUNT;
      break;
    case CMD_CLIENTS:
      return POWER_CLIENTS;
      break;
    case CMD_FINDEMAIL:
      return POWER_FINDEMAIL;
      break;
    case CMD_COMMENT:
      return POWER_COMMENT;
      break;
    case CMD_EGOTRIP:
      return POWER_EGOTRIP;
      break;
    case CMD_POWERS:
      return POWER_POWERS;
      break;
    case CMD_SHUTDOWN:
    case CMD_SHUTDOW:
      return POWER_SHUTDOWN;
      break;
    case CMD_RESET:
      return POWER_RESET;
      break;
    case CMD_SLAY:
      return POWER_SLAY;
      break;
    case CMD_TIMESHIFT:
      return POWER_TIMESHIFT;
      break;
    case CMD_CRIT:
      return POWER_CRIT;
      break;
    case CMD_RELEASE: // ???
    case CMD_CAPTURE: // ???
    case CMD_TASKS:
    case CMD_TEST_FIGHT:
    case CMD_PEELPK:
    case CMD_TESTCODE:
    case CMD_BRUTTEST:
      return POWER_WIZARD;
      break;
    break;
    default:
      break;
  }

  return MAX_POWER_INDEX;
}

static const char *learn_name(byte num)
{
  if (num <= 1)
    return "Really Slow";
  else if (num == 2)
    return "Slow";
  else if (num == 3)
    return "Somewhat Slow";
  else if (num == 4)
    return "Moderate";
  else if (num == 5)
    return "Somewhat Fast";
  else if (num == 6)
    return "Fast";
  else
    return "very fast";
}

static const sstring wikiColors[][2] = {
  { "darkred", "<R>" },
  { "red", "<r>" },
  { "darkblue", "<B>" },
  { "blue", "<b>" },
  { "darkgreen", "<G>" },
  { "green", "<g>" },
  { "cyan", "<c>" },
  { "turquoise", "<C>" },
  { "magenta", "<p>" },
  { "purple", "<P>" },
  { "orange", "<o>" },
  { "yellow", "<y>" },
  { "gray", "<k>" },
  { "black", "<K>" },
  { "ghostwhite", "<w>" },
  { "white", "<W>" },
  { "invert", "<i>" },
};

// sucks the data out of wiki tables and re-gens them for text
void replaceWikiTable(sstring &data)
{
  size_t tablePos = data.find("{|");
  size_t tableEnd = data.find("|}", tablePos+1);
  while(tablePos != sstring::npos && tableEnd != sstring::npos)
  {
    static const char * colColorHdr[] = { "<p>", "", "<g>", "", "<c>" };
    static const char * colColorFtr[] = { "<z>", "", "<z>", "", "<z>" };
    sstring table = data.substr(tablePos, tableEnd-tablePos+2);
    sstring *rgTableData;
    sstring tableReplace;
    int cData = 0;
    int cRows = table.countSubstr("|-\n!");
    int cCols = 0;

    if (cRows <= 0)
      cRows = table.countSubstr("|-\n|");

    if (cRows <= 0)
      return;

    // normalize table into simple delimited
    table.inlineRemoveBetween("{|", "|-", true);
    table.inlineRemoveBetween("!", "|", false);
    table.inlineReplaceString("!|", "||");
    table.inlineReplaceString("\n", "");
    table.inlineReplaceString("\r", ""); // not really used in wikitext
    table.inlineReplaceString("|}", "");
    table.inlineReplaceString("|-", "\255");
    table.inlineReplaceString("||", "\255");

    // remove all data parts from table
    cData = table.split('\255', NULL);
    if (cData <= 0)
      return;
    rgTableData = new sstring[cData];
    cData = table.split('\255', rgTableData);
    cCols = cData / cRows;

    // trim strings, calc width for each column
    int *rgWidth = new int[cCols];
    memset(rgWidth, 0, sizeof(rgWidth)*cCols);
    for(int iWidth = 0;iWidth < cData; iWidth++)
    {
      rgTableData[iWidth] = rgTableData[iWidth].trim();
      rgWidth[iWidth%cCols] = max(rgWidth[iWidth%cCols], int(rgTableData[iWidth].lengthNoColor()));
    }

    // write the table
    for(int iData = 0;iData < cData; iData++)
    {
      int iCol = iData % cCols;
      rgTableData[iData].resize(rgTableData[iData].length() +
                                rgWidth[iCol] - rgTableData[iData].lengthNoColor(), ' ');
      tableReplace += format("%s%s%s") % colColorHdr[iCol%cElements(colColorHdr)] %
                        rgTableData[iData] % colColorFtr[iCol%cElements(colColorFtr)];
      if (iCol == cCols-1)
        tableReplace += "\n";
      else
        tableReplace += " : ";
    }

    // cleanup, replace
    delete[] rgTableData;
    delete[] rgWidth;

    // re-generate table as text into tableReplace
    data.replace(tablePos, tableEnd-tablePos+2, tableReplace.c_str(), tableReplace.length());

    tablePos = data.find("{|", tablePos + tableReplace.length());
    tableEnd = data.find("|}", tablePos+1);
  }
}

void /*TBeing::*/displayHelpFile(TBeing *ch, char *helppath, char *namebuf){
  int j;
  struct stat timestat;
  char timebuf[1024], buf2[1024];
  sstring str;

  // make the topic name upper case
  for (j = 0;namebuf[j] != '\0';j++)
    namebuf[j] = UPPER(namebuf[j]);

  // find the last modified time on the file
  if (stat(helppath, &timestat)) {
    vlogf(LOG_BUG,format("bad call to help function %s, rebuilding indices") %  namebuf);
    buildHelpIndex();
    ch->sendTo("There was an error, try again.\n\r");
    return;
  }
  strcpy(timebuf, ctime(&(timestat.st_mtime)));
  timebuf[strlen(timebuf) - 1] = '\0';
  sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", ch->green(),
	  namebuf,timebuf, ch->norm());
  str = buf2;
  
  
  // special message for nextversion file
  if (!strcmp(namebuf, "NEXTVERSION")) {
    str += "THIS HELP FILE REFLECTS WHAT THE \"news\" COMMAND WILL SHOW NEXT TIME THERE\n\r";
    str += "IS A CHANGE IN CODE (PROBABLY IN THE NEXT FEW DAYS).  IT IS HERE TO GIVE\n\r";
    str += "YOU SOME IDEA OF WHAT THINGS HAVE BEEN FIXED ALREADY, OR WHAT FEATURES ARE\n\r";
    str += "FORTHCOMING...\n\r\n\r";
  }

  // now print the file
  file_to_sstring(helppath, str, CONCAT_YES);
  str += "\n\r";
  ch->desc->page_string(str);
}

void TBeing::doHelp(const char *arg)
{
  sstring str;
  int j;
  bool found = FALSE;
  int helpnum = 0;
  unsigned int i;
  char helppath[256], ansipath[256];
  struct stat timestat;
  char namebuf[1024], timebuf[1024];
  char buf2[MAX_STRING_LENGTH];

  if (!desc)
    return;

  for (; isspace(*arg); arg++);

  char searchBuf[256];

  one_argument(arg, searchBuf, cElements(searchBuf));

  // this prevents "help ../../being.h" and "help _skills"
  const char *c;
  for (c = arg; *c; c++) {
    if (!isalnum(*c) && !isspace(*c)) {
      sendTo("Illegal argument.\n\r");
      return;
    }
  }
  help_used_num++;
  total_help_number++;
  save_game_stats();

  if (!*arg) {
    desc->start_page_file(File::HELP_PAGE, "General help unavailable.\n\r");
    return;
  }

  if(!strcasecmp(arg, "index")){
    FILE *index=popen("bin/helpindex", "r");
    
    while(fread(buf2, 1, MAX_STRING_LENGTH, index)){
      str+=buf2;
    }
    pclose(index);
    
    desc->page_string(str);

    return;
  }

  if (isImmortal() && hasWizPower(POWER_IMMORTAL_HELP)) {
    for (i = 0; i < immortalIndex.size(); i++) {
      if (!strcasecmp(arg, immortalIndex[i])) {
        found = TRUE;
        helpnum = i;
        sprintf(helppath, "%s/%s", Path::IMMORTAL_HELP, immortalIndex[i]);
        break;
      } else if (is_abbrev(arg, immortalIndex[i])) {
        found = TRUE;
        helpnum = i;
        sprintf(helppath, "%s/%s", Path::IMMORTAL_HELP, immortalIndex[i]);
      }
    }
    if (found) {
      i = helpnum;
      strcpy(namebuf, immortalIndex[i]);
      if (hasColorVt()) {
        sprintf(ansipath, "%s.ansi", helppath);
        if (file_to_sstring(ansipath, str)) {
          // an ansi file was found, swap helppath request with ansi
          strcpy(helppath, ansipath);
        }
      }
      for (j = 0;namebuf[j] != '\0';j++)
        namebuf[j] = UPPER(namebuf[j]);

      if (stat(helppath, &timestat)) {
	vlogf(LOG_BUG,format("bad call to help function %s, rebuilding indices") %  namebuf);
	buildHelpIndex();
	sendTo("There was an error, try again.\n\r");
	return;
      }
      strcpy(timebuf, ctime(&(timestat.st_mtime)));
      timebuf[strlen(timebuf) - 1] = '\0';
      sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", green(),
            namebuf,timebuf, norm());
      str = buf2;
      file_to_sstring(helppath, str, CONCAT_YES);
      str += "\n\r";
      desc->page_string(str);
      return;
    }
  }
  if (GetMaxLevel() >= GOD_LEVEL1 && isImmortal()) {
    for (i = 0; i < builderIndex.size(); i++) {
      if (!strcasecmp(arg, builderIndex[i])) {
        sprintf(helppath, "%s/%s", Path::BUILDER_HELP, builderIndex[i]);
        helpnum = i;
        found = TRUE;
        break;
      } else if (is_abbrev(arg, builderIndex[i])) {
        sprintf(helppath, "%s/%s", Path::BUILDER_HELP, builderIndex[i]);
        helpnum = i;
        found = TRUE;
      }
    }
    if (found) {
      i = helpnum;
      strcpy(namebuf, builderIndex[i]);
      if (hasColorVt()) {
        sprintf(ansipath, "%s.ansi", helppath);
        if (file_to_sstring(ansipath, str)) {
          // an ansi file was found, swap helppath request with ansi
          strcpy(helppath, ansipath);
        }
      }
      for (j = 0;namebuf[j] != '\0';j++)
        namebuf[j] = UPPER(namebuf[j]);
      if (stat(helppath, &timestat)) {
	vlogf(LOG_BUG,format("bad call to help function %s, rebuilding indices") %  namebuf);
	buildHelpIndex();
	sendTo("There was an error, try again.\n\r");
	return;
      }
      strcpy(timebuf, ctime(&(timestat.st_mtime)));
      timebuf[strlen(timebuf) - 1] = '\0';
      sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", green(),
            namebuf,timebuf, norm());
      str = buf2;
      file_to_sstring(helppath, str, CONCAT_YES);
      str += "\n\r";
      desc->page_string(str);
      return;
    }
  }
  for (i = 0; i < helpIndex.size(); i++) {
    // this is a kludge
    // force help armor to hit spell, not armor proficiency
    // force help bleed to hit prayer, not bleeding
    // force help sharpen to hit skill, not sharpener
    if (!strcasecmp(arg, "armor") ||
        !strcasecmp(arg, "sharpen") ||
        !strcasecmp(arg, "bleed"))
      break;
    if(sstring(arg).lower() == helpIndex[i].lower()){
      sprintf(helppath, "%s/%s", Path::HELP, helpIndex[i].c_str());
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, helpIndex[i], MULTIPLE_YES)) {
      sprintf(helppath, "%s/%s", Path::HELP, helpIndex[i].c_str());
      helpnum = i;
      found = TRUE;
    }
  }
  if (found) {
    strcpy(namebuf, helpIndex[helpnum].c_str());
    displayHelpFile(this, helppath, namebuf);
    return;
  }
  for (i = 0; i < spellIndex.size(); i++) {
    // this is a kludge
    // some normal help files are masked by spells
    if (!strcasecmp(arg, "steal"))  // stealth
      break;
    if (!strcasecmp(arg, spellIndex[i])) {
      sprintf(helppath, "%s/%s", Path::SPELL_HELP, spellIndex[i]);
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, spellIndex[i], MULTIPLE_YES, EXACT_YES)) {
      sprintf(helppath, "%s/%s", Path::SPELL_HELP, spellIndex[i]);
      helpnum = i;
      found = TRUE;
    } else if (is_abbrev(arg, spellIndex[i])) {
      sprintf(helppath, "%s/%s", Path::SPELL_HELP, spellIndex[i]);
      helpnum = i;
      found = TRUE;
    }
  }
  if (found) {
    i = helpnum;
    strcpy(namebuf, spellIndex[i]);
    if (hasColorVt()) {
      sprintf(ansipath, "%s.ansi", helppath);
      if (file_to_sstring(ansipath, str)) {
        // an ansi file was found, swap helppath request with ansi
        strcpy(helppath, ansipath);
      }
    }
    for (j = 0;namebuf[j] != '\0';j++)
      namebuf[j] = UPPER(namebuf[j]);
    if (stat(helppath, &timestat)) {
      vlogf(LOG_BUG,format("bad call to help function %s, rebuilding indices") %  namebuf);
      buildHelpIndex();
      sendTo("There was an error, try again.\n\r");
      return;
    }
    strcpy(timebuf, ctime(&(timestat.st_mtime)));
    timebuf[strlen(timebuf) - 1] = '\0';
    sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", green(),
            namebuf,timebuf, norm());
    str = buf2;
    spellNumT skill;
    discNumT disc_num;
    spellNumT snt;

    // first, see if we can find a matching skill that the player has
    // this is here so skills with same name (for different classes) will
    // be isolated.
    for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
      if (hideThisSpell(snt))
        continue;

      if (strcasecmp(discArray[snt]->name, spellIndex[i]))
        continue;
   
      if (doesKnowSkill(snt))
        break;
    }

    // if we can't find match on name for skill they have, just use name match
    if (snt >= MAX_SKILL) {
      for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
  
        if (!strcasecmp(discArray[snt]->name, spellIndex[i]))
          break;
      }
    }

    skill = snt;
    if (skill >= MAX_SKILL) {
      vlogf(LOG_BUG,format("Bogus spell help file: %s") %  spellIndex[i]);
      return;
    }
    skill = getSkillNum(skill);
    if (skill < 0) {
      vlogf(LOG_BUG,format("Bogus spell help file: %s") %  spellIndex[i]);
      return;
    }
    disc_num = getDisciplineNumber(skill, FALSE);
    if (disc_num != DISC_NONE) {
      str += purple();
      str += "Discipline       : ";
      str += norm();
      str += discNames[disc_num].properName;
      str += purple();
      if (isImmortal()) {
        sprintf(buf2, "    (disc: %d, spell %d)", mapDiscToFile(disc_num), skill);
        str += buf2;
      }
    } else {
      vlogf(LOG_BUG, format("Bad skill %d to getDisciplineNumber in doHelp") %  skill);
    }
    str += purple();
    str += "\n\rSpecialization   : ";
    str += norm();
    str += discNames[(discArray[skill]->assDisc)].properName;
    str += purple();
    if (isImmortal()) {
      sprintf(buf2, "    (disc: %d)", mapDiscToFile(discArray[skill]->assDisc));
      str += buf2;
    }

    str += "\n\rLearned in Disc. : ";
    str += norm();

    // PC's know what the learning of thie disc is, so may as well tell them
    sprintf(buf2, "%d%%", discArray[skill]->start);
    str += buf2;
    str += purple();

    str += "\n\rDisc. Learn Rate : ";
    str += norm();
    str +=learn_name(discArray[skill]->learn);
    str += purple();

    if (isImmortal()) {
      sprintf(buf2, " (%d)", discArray[skill]->learn);
      str += buf2;
    }
    str += "\n\rLearn By Doing   : ";
    str += norm();
    str += ((discArray[skill]->startLearnDo == -1) ? "No" : "Yes");

    if (isImmortal()) {
      sprintf(buf2, "  %s(%d) (%d) %s", purple(), discArray[skill]->startLearnDo, discArray[skill]->amtLearnDo, norm());
      str += buf2;
    }

    str += "\n\r";

    if (discArray[skill]->holyStrength) {
#if 0
  // this is calculated differently now, so lets not mislead folks
      sprintf(buf2, "\n\r%sSymbol Stress    :%s %d\n\r",
          purple(), norm(), discArray[skill]->holyStrength);
      str += buf2;
#endif
    } else if (skill == SPELL_MATERIALIZE ||
               skill == SPELL_SPONTANEOUS_GENERATION) {
      sprintf(buf2, "\n\r%sSpell Component  :%s SPECIAL (see below)\n\r", purple(), norm());
      str += buf2;
    } else if (IS_SET(discArray[skill]->comp_types, COMP_MATERIAL)) {
      unsigned int comp;
      for (comp = 0; (comp < CompInfo.size()) &&
                (skill != CompInfo[comp].spell_num);comp++);
      if (comp != CompInfo.size() && CompInfo[comp].comp_num >= 0) {
        sprintf(buf2, "\n\r%sSpell Component  :%s %s\n\r",
		purple(), norm(), 
		sstring(obj_index[real_object(CompInfo[comp].comp_num)].short_desc).cap().c_str());
        str += buf2;
      } else
        vlogf(LOG_BUG, format("Problem in help file for skill=%d, comp=%d.  (component definition)") %  skill % comp);
    } else {
      sprintf(buf2, "\n\r%sSpell Component  :%s NONE\n\r", purple(), norm());
      str += buf2;
    }
    sprintf(buf2, "%sDifficulty       :%s %s\n\r",
         purple(), norm(), displayDifficulty(skill).c_str());
    str += buf2;

    int immy = getTypeImmunity(skill);
    sprintf(buf2, "%sImmunity Type    :%s %s\n\r", purple(), norm(),
         ((immy != -1) ? immunity_names[immy] : "NONE"));
    str += buf2;

    if (IS_SET(discArray[skill]->comp_types, SPELL_TASKED)) {
        sprintf(buf2, "%sCasting rounds   :%s %d casting rounds\n\r" ,purple(), norm(), discArray[skill]->lag + 2);
        str += buf2;

        sprintf(buf2, "%sCombat rounds    :%s %d combat rounds\n\r" ,purple(), norm(), discArray[skill]->lag + 1);
        str += buf2;
    } else {
      lag_t lag = discArray[skill]->lag;
      if (lag > LAG_0) {
        sprintf(buf2, "%sCommand lock-out :%s %.1f seconds",purple(), norm(), lagAdjust(lag) * combatRound(1)/Pulse::ONE_SECOND);
        str += buf2;

        if (isImmortal()) {
          sprintf(buf2, " %s(%d rounds)%s", purple(), lag, norm());
          str += buf2;
        }
        str += "\n\r";
      } else {
        sprintf(buf2, "%sCommand lock-out :%s None\n\r",purple(), norm());
        str += buf2;
      }
    }
    if (discArray[skill]->minMana) {
      if (doesKnowSkill(skill) && (IS_SET(discArray[skill]->comp_types, SPELL_TASKED))) {
        sprintf(buf2, "%sMinimum Mana     :%s %d, per round amount : %d\n\r",
            purple(), norm(),
            ((discArray[skill]->minMana / (discArray[skill]->lag +2)) * (discArray[skill]->lag +2)),
            (discArray[skill]->minMana / (discArray[skill]->lag +2)));
        sprintf(buf2 + strlen(buf2), "%sCurrent Mana     :%s %d, per round amount : %d\n\r",
            purple(), norm(),
            useMana(skill) * (discArray[skill]->lag +2),
            useMana(skill));
      } else if (doesKnowSkill(skill)) {
        sprintf(buf2, "%sMinimum Mana     :%s %d, current : %d\n\r",
              purple(),  norm(), discArray[skill]->minMana, useMana(skill));
      } else {
        sprintf(buf2, "%sMana (min/cur)   :%s %d/spell-not-known\n\r",
                       purple(),  norm(), discArray[skill]->minMana);
      }
    }
    if (discArray[skill]->minLifeforce) {
      if (doesKnowSkill(skill) && (IS_SET(discArray[skill]->comp_types, SPELL_TASKED))) {
        sprintf(buf2, "%sMinimum Lifeforce:%s %d, per round amount : %d\n\r",
            purple(), norm(),
            ((discArray[skill]->minLifeforce / (discArray[skill]->lag +2)) * (discArray[skill]->lag +2)),
            (discArray[skill]->minLifeforce / (discArray[skill]->lag +2)));
        sprintf(buf2 + strlen(buf2), "%sCurrent Lifeforce:%s %d, per round amount : %d\n\r",
            purple(), norm(),
            useLifeforce(skill) * (discArray[skill]->lag +2),
            useLifeforce(skill));
      } else if (doesKnowSkill(skill)) {
        sprintf(buf2, "%sMinimum Lifeforce:%s %d, current : %d\n\r",
              purple(),  norm(), discArray[skill]->minLifeforce, useLifeforce(skill));
      } else {
        sprintf(buf2, "%sLifeforce:min/cur:%s %d/spell-not-known\n\r",
                       purple(),  norm(), discArray[skill]->minLifeforce);
      }
    }
    if (discArray[skill]->minPiety) {
      if (doesKnowSkill(skill) && (IS_SET(discArray[skill]->comp_types, SPELL_TASKED))) {

        sprintf(buf2, "%sMinimum Piety    :%s %.2f, per round amount : %.2f\n\r",
              purple(), norm(),
              ((discArray[skill]->minPiety / (discArray[skill]->lag +2)) *
(discArray[skill]->lag +2)),
              (discArray[skill]->minPiety / (discArray[skill]->lag +2)));
        sprintf(buf2 + strlen(buf2), "%sCurrent Piety    :%s %.2f, per round amount : %.2f\n\r",
              purple(), norm(),
                 (usePiety(skill) * (discArray[skill]->lag +2)),
               usePiety(skill));
      } else if (doesKnowSkill(skill)) {
        sprintf(buf2, "%sMinimum Piety    :%s %.2f  Current Piety  : %.2f\n\r",
                    purple(),  norm(), discArray[skill]->minPiety, usePiety(skill));
      } else {
        sprintf(buf2, "%sPiety (min/cur)  :%s %.2f/prayer-not-known\n\r",
              purple(),  norm(), discArray[skill]->minPiety);
      }
    }
    str += buf2;
    if (discArray[skill]->comp_types) {
      sprintf(buf2,  "%sRequires         :%s ", purple(), norm());
      str += buf2;

      if (discArray[skill]->comp_types & COMP_GESTURAL)
        str += "Gestural Moves";
      if ((discArray[skill]->comp_types & COMP_GESTURAL) &&
          (discArray[skill]->comp_types & COMP_VERBAL))
        str += ", ";
      if (discArray[skill]->comp_types & COMP_VERBAL) {
        if (discArray[skill]->holyStrength)
          str += "Spoken Mantra";
        else
          str += "Spoken Incantation";
      }
      str += "\n\r";
    }
    sprintf(buf2, "%sOffensive        : %s%s\t%sArea Effect          : %s%s\n\r", 
      purple(), norm(), 
      (discArray[skill]->targets & TAR_VIOLENT) ? "Yes" : "No",
      purple(), norm(),
      (discArray[skill]->targets & TAR_AREA) ? "Yes" : "No");
    str += buf2;

    sprintf(buf2, "%sCast on Self     : %s%s\t%sObject Castable      : %s%s\n\r", 
      purple(), norm(), 
      (discArray[skill]->targets & TAR_SELF_NONO ? "No" :
       (discArray[skill]->targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY)) ? "Yes" : "No"),
      purple(), norm(), 
      (discArray[skill]->targets & (TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_WORLD | TAR_OBJ_EQUIP)) ? "Yes" : "No");
    str += buf2;

    sprintf(buf2, "%sCast on Others   : %s%s \n\r", 
      purple(), norm(), 
      (discArray[skill]->targets & TAR_SELF_ONLY) ? "No" :
      (discArray[skill]->targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD)) ? "Yes" : "No");
    str += buf2;

    str += "\n\r";

    file_to_sstring(helppath, str, CONCAT_YES);
    str += "\n\r";
    desc->page_string(str);
    return;
  }
  for (i = 0; i < skillIndex.size(); i++) {
    // this is a kludge
    // some normal help files are masked by skills
    if (!strcasecmp(arg, "cast"))  // casting
      break;
    if (!strcasecmp(arg, skillIndex[i])) {
      sprintf(helppath, "%s/%s", Path::SKILL_HELP, skillIndex[i]);
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, skillIndex[i], MULTIPLE_YES)) {
      sprintf(helppath, "%s/%s", Path::SKILL_HELP, skillIndex[i]);
      helpnum = i;
      found = TRUE;
    }
  }
  if (found) {
    i = helpnum;
    strcpy(namebuf, skillIndex[i]);
    if (hasColorVt()) {
      sprintf(ansipath, "%s.ansi", helppath);
      if (file_to_sstring(ansipath, str)) {
        // an ansi file was found, swap helppath request with ansi
        strcpy(helppath, ansipath);
      }
    }
    for (j = 0;namebuf[j] != '\0';j++)
      namebuf[j] = UPPER(namebuf[j]);

    if (stat(helppath, &timestat)) {
      vlogf(LOG_BUG,format("bad call to help function %s, rebuilding indices") %  namebuf);
      buildHelpIndex();
      sendTo("There was an error, try again.\n\r");
      return;
    }
    strcpy(timebuf, ctime(&(timestat.st_mtime)));
    timebuf[strlen(timebuf) - 1] = '\0';
    sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", green(),
            namebuf,timebuf, norm());
    str = buf2;

    spellNumT skill;
    discNumT disc_num;
    spellNumT snt;

    // first, see if we can find a matching skill that the player has
    // this is here so skills with same name (for different classes) will
    // be isolated.
    for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
      if (hideThisSpell(snt))
        continue;

      if (strcasecmp(discArray[snt]->name, skillIndex[i]))
        continue;
   
      if (doesKnowSkill(snt))
        break;
    }

    // if we can't find match on name for skill they have, just use name match
    if (snt >= MAX_SKILL) {
      for (snt = MIN_SPELL; snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
  
        if (!strcasecmp(discArray[snt]->name, skillIndex[i]))
          break;
      }
    }

    skill = snt;

    if (skill >= MAX_SKILL) {
      vlogf(LOG_BUG,format("Bogus skill help file: %s") %  skillIndex[i]);
      return;
    }
    disc_num = getDisciplineNumber(skill, FALSE);
    if (disc_num != DISC_NONE) {
      str += purple();
      str += "Discipline       : ";
      str += norm();
      str += discNames[disc_num].properName;
      str += purple();
      if (isImmortal()) {
        sprintf(buf2, "    (disc: %d, skill %d)", mapDiscToFile(disc_num), skill);
        str += buf2;
      }
    } else 
      vlogf(LOG_BUG, format("Bad disc for skill %d in doHelp()") %  skill);
    
    str += purple();
    str += "\n\rSpecialization   : ";
    str += norm();
    str += discNames[(discArray[skill]->assDisc)].properName;
    str += purple();
    if (isImmortal()) {
      sprintf(buf2, "    (disc: %d)", mapDiscToFile(discArray[skill]->assDisc));
      str += buf2;
    }

    str += norm();
    str += purple();
    str += "\n\rStart Learning   : ";
    str += norm();

    // PC's know what the learning of thie disc is, so may as well tell them
    sprintf(buf2, "%d%%", discArray[skill]->start);
    str += buf2;
    str += purple();

    str += "\n\rDisc. Learn Rate : ";
    str += norm();
    str += learn_name(discArray[skill]->learn);
    if (isImmortal()) {
      sprintf(buf2, "%s(%d)%s", purple(), discArray[skill]->learn, norm());
      str += buf2;
    }

    str += purple();
    str += "\n\rLearn By Doing   : ";
    str += norm();
    str += ((discArray[skill]->startLearnDo == -1) ? "No" : "Yes");
    if (isImmortal()) {
      sprintf(buf2, "  %s(%d) (%d) %s", 
         purple(), discArray[skill]->startLearnDo, discArray[skill]->amtLearnDo, norm());
      str += buf2;
    }
    str += "\n\r";

    sprintf(buf2, "%sDifficulty       :%s %s\n\r",
         purple(), norm(), displayDifficulty(skill).c_str());
    str += buf2;

    lag_t lag = discArray[skill]->lag;
    if (lag > LAG_0) {
      sprintf(buf2, "%sCommand lock-out :%s %.1f seconds",
	      purple(), norm(), lagAdjust(lag) * combatRound(1)/Pulse::ONE_SECOND);
      str += buf2;

      if (isImmortal()) {
        sprintf(buf2, " %s(%d rounds)%s", purple(), lag, norm());
        str += buf2;
      }
      str += "\n\r";
    } else {
      sprintf(buf2, "%sCommand lock-out :%s None\n\r",purple(), norm());
      str += buf2;
    }

    str += "\n\r";
    file_to_sstring(helppath, str, CONCAT_YES);
    str += "\n\r";
    desc->page_string(str);
    return;
  }

  sendTo("No such help file available.\n\r");
}

void TBeing::doBuildhelp(const char* arg)
{
  if (!desc)
    return;
  for (; isspace(*arg); arg++);

  if (!isImmortal() || GetMaxLevel() < GOD_LEVEL1 || !hasWizPower(POWER_BUILDER))
  {
    sendTo("Sorry, only builders may access build help files.\n\r");
    return;
  }
}

void TBeing::doWizhelp(const char *arg)
{
  sstring sbuf, buf, tString;
  int       no,
            tLength = 2;
  unsigned int i;
  wizPowerT tPower;

  if (!isImmortal())
    return;

  if (!desc)
    return;
  for (; isspace(*arg); arg++);

  for (i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;

    if ((GetMaxLevel() >= commandArray[i]->minLevel) &&
        (commandArray[i]->minLevel > MAX_MORT) &&
        ((tPower = wizPowerFromCmd(cmdTypeT(i))) == MAX_POWER_INDEX ||
         hasWizPower(tPower)))
      tLength = max(strlen(commandArray[i]->name), (size_t) tLength);
  }

  tString = format("%c-%ds") % '%' % (tLength + 1);
  tLength = (79 / tLength);

  sendTo("The following privileged commands are available:\n\r\n\r");

  if ((tPower = wizPowerFromCmd(CMD_AS)) == MAX_POWER_INDEX ||
      hasWizPower(tPower))
    buf = format(tString) % "as";

  for (no = 2, i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;

    if ((GetMaxLevel() >= commandArray[i]->minLevel) &&
        (commandArray[i]->minLevel > MAX_MORT) &&
        ((tPower = wizPowerFromCmd(cmdTypeT(i))) == MAX_POWER_INDEX ||
         hasWizPower(tPower))) {

      sbuf = format(tString) % commandArray[i]->name;
      buf += sbuf;

      if (!(no % (tLength - 1)))
	buf += "\n\r";

      no++;
    }
  }

  buf += "\n\r      Check out HELP GODS (or HELP BUILDERS) for an index of help files.\n\r";
  desc->page_string(buf);
}


void buildHelpIndex()
{
  DIR *dfd;
  struct dirent *dp;

  // set a reasonable initial size
  immortalIndex.clear();
  immortalIndex.reserve(128);
  if (!(dfd = opendir(Path::IMMORTAL_HELP))) {
    vlogf(LOG_FILE, "Can't open immortal help directory for indexing!");
    exit(0);
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
        (strlen(dp->d_name) >= 5 &&
         !strcmp(&dp->d_name[strlen(dp->d_name) - 5], ".ansi")))
      continue; 

    char *tmpc = mud_str_dup(dp->d_name);
    immortalIndex.push_back(tmpc);
  }
  closedir(dfd);

  // set a reasonable initial size
  builderIndex.clear();
  builderIndex.reserve(64);
  if (!(dfd = opendir(Path::BUILDER_HELP))) {
    vlogf(LOG_FILE, "Can't open builder help directory for indexing!");
    exit(0);
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
        (strlen(dp->d_name) >= 5 &&
         !strcmp(&dp->d_name[strlen(dp->d_name) - 5], ".ansi")))
      continue;

    char *tmpc = mud_str_dup(dp->d_name);
    builderIndex.push_back(tmpc);
  }
  closedir(dfd);

  // set a reasonable initial size
  helpIndex.clear();
  helpIndex.reserve(512);
  if (!(dfd = opendir(Path::HELP))) {
    vlogf(LOG_FILE, "Can't open help directory for indexing!");
    exit(0);
  }
  // COSMO STRING
  sstring str;
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
        (strlen(dp->d_name) >= 5 &&
         !strcmp(&dp->d_name[strlen(dp->d_name) - 5], ".ansi")))
      continue;
    str = dp->d_name;
    helpIndex.push_back(str);
  }
// COSMO STRING  
//  delete str;
  closedir(dfd);

  // set a reasonable initial size
  skillIndex.clear();
  skillIndex.reserve(256);
  if (!(dfd = opendir(Path::SKILL_HELP))) {
    vlogf(LOG_FILE, "Can't open skill help directory for indexing!");
    exit(0);
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
        (strlen(dp->d_name) >= 5 &&
         !strcmp(&dp->d_name[strlen(dp->d_name) - 5], ".ansi")))
      continue;

    char *tmpc = mud_str_dup(dp->d_name);
    skillIndex.push_back(tmpc);
  }
  closedir(dfd);

  // set a reasonable initial size
  spellIndex.clear();
  spellIndex.reserve(256);
  if (!(dfd = opendir(Path::SPELL_HELP))) {
    vlogf(LOG_FILE, "Can't open spell help directory for indexing!");
    exit(0);
  }
  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
        (strlen(dp->d_name) >= 5 &&
         !strcmp(&dp->d_name[strlen(dp->d_name) - 5], ".ansi")))
      continue;

    char *tmpc = mud_str_dup(dp->d_name);
    spellIndex.push_back(tmpc);
  }
  closedir(dfd);
}


void cleanUpHelp()
{
  unsigned int i;
  for (i = 0; i < immortalIndex.size(); i++)
    delete [] immortalIndex[i];
  for (i = 0; i < builderIndex.size(); i++)
    delete [] builderIndex[i];
  for (i = 0; i < skillIndex.size(); i++)
    delete [] skillIndex[i];
  for (i = 0; i < spellIndex.size(); i++)
    delete [] spellIndex[i];
}













