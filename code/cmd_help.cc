#include "stdsneezy.h"

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

static vector<sstring>helpIndex(0);
static vector<char *>immortalIndex(0);
static vector<char *>builderIndex(0);
static vector<char *>skillIndex(0);
static vector<char *>spellIndex(0);

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

void TBeing::displayHelpFile(char *helppath, char *namebuf){
  int j;
  struct stat timestat;
  char timebuf[1024], buf2[1024];
  sstring str;

  // make the topic name upper case
  for (j = 0;namebuf[j] != '\0';j++)
    namebuf[j] = UPPER(namebuf[j]);

  // find the last modified time on the file
  if (stat(helppath, &timestat)) {
    vlogf(LOG_BUG,fmt("bad call to help function %s, rebuilding indices") %  namebuf);
    buildHelpIndex();
    sendTo("There was an error, try again.\n\r");
    return;
  }
  strcpy(timebuf, ctime(&(timestat.st_mtime)));
  timebuf[strlen(timebuf) - 1] = '\0';
  sprintf(buf2,"%s%-30.30s (Last Updated: %s)%s\n\r\n\r", green(),
	  namebuf,timebuf, norm());
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
  desc->page_string(str);
  return;

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

  one_argument(arg, searchBuf);

  if (!strncmp(searchBuf, "-l", 2)) {
    sendTo(COLOR_BASIC, "<r>Help search functionality currently disabled.<1>\n\r");
    return;
  } 

  // this prevents "help ../../being.h" and "help _skills"
  const char *c;
  for (c = arg; *c; c++) {
    if (!isalpha(*c) && !isspace(*c)) {
      sendTo("Illegal argument.\n\r");
      return;
    }
  }
  help_used_num++;
  total_help_number++;
  save_game_stats();

  if (!*arg) {
    desc->start_page_file(HELP_PAGE_FILE, "General help unavailable.\n\r");
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
        sprintf(helppath, "%s/%s", IMMORTAL_HELP_PATH, immortalIndex[i]);
        break;
      } else if (is_abbrev(arg, immortalIndex[i])) {
        found = TRUE;
        helpnum = i;
        sprintf(helppath, "%s/%s", IMMORTAL_HELP_PATH, immortalIndex[i]);
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
	vlogf(LOG_BUG,fmt("bad call to help function %s, rebuilding indices") %  namebuf);
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
        sprintf(helppath, "%s/%s", BUILDER_HELP_PATH, builderIndex[i]);
        helpnum = i;
        found = TRUE;
        break;
      } else if (is_abbrev(arg, builderIndex[i])) {
        sprintf(helppath, "%s/%s", BUILDER_HELP_PATH, builderIndex[i]);
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
	vlogf(LOG_BUG,fmt("bad call to help function %s, rebuilding indices") %  namebuf);
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
      sprintf(helppath, "%s/%s", HELP_PATH, helpIndex[i].c_str());
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, helpIndex[i], MULTIPLE_YES)) {
      sprintf(helppath, "%s/%s", HELP_PATH, helpIndex[i].c_str());
      helpnum = i;
      found = TRUE;
    }
  }
  if (found) {
    strcpy(namebuf, helpIndex[helpnum].c_str());
    displayHelpFile(helppath, namebuf);
    return;
  }
  for (i = 0; i < spellIndex.size(); i++) {
    // this is a kludge
    // some normal help files are masked by spells
    if (!strcasecmp(arg, "steal"))  // stealth
      break;
    if (!strcasecmp(arg, spellIndex[i])) {
      sprintf(helppath, "%s/%s", SPELL_HELP_PATH, spellIndex[i]);
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, spellIndex[i], MULTIPLE_YES, EXACT_YES)) {
      sprintf(helppath, "%s/%s", SPELL_HELP_PATH, spellIndex[i]);
      helpnum = i;
      found = TRUE;
    } else if (is_abbrev(arg, spellIndex[i])) {
      sprintf(helppath, "%s/%s", SPELL_HELP_PATH, spellIndex[i]);
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
      vlogf(LOG_BUG,fmt("bad call to help function %s, rebuilding indices") %  namebuf);
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
      vlogf(LOG_BUG,fmt("Bogus spell help file: %s") %  spellIndex[i]);
      return;
    }
    skill = getSkillNum(skill);
    if (skill < 0) {
      vlogf(LOG_BUG,fmt("Bogus spell help file: %s") %  spellIndex[i]);
      return;
    }
    disc_num = getDisciplineNumber(skill, FALSE);
    if (disc_num != DISC_NONE) {
      str += purple();
      str += "Discipline       : ";
      str += norm();
      str += disc_names[disc_num];
      str += purple();
      if (isImmortal()) {
        sprintf(buf2, "    (disc: %d, spell %d)", mapDiscToFile(disc_num), skill);
        str += buf2;
      }
    } else {
      vlogf(LOG_BUG, fmt("Bad skill %d to getDisciplineNumber in doHelp") %  skill);
    }
    str += purple();
    str += "\n\rSpecialization   : ";
    str += norm();
    str += disc_names[(discArray[skill]->assDisc)];
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
        vlogf(LOG_BUG, fmt("Problem in help file for skill=%d, comp=%d.  (component definition)") %  skill % comp);
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
        sprintf(buf2, "%sCommand lock-out :%s %.1f seconds",purple(), norm(), lagAdjust(lag) * combatRound(1)/ONE_SECOND);
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
      sprintf(helppath, "%s/%s", SKILL_HELP_PATH, skillIndex[i]);
      helpnum = i;
      found = TRUE;
      break;
    } else if (is_abbrev(arg, skillIndex[i], MULTIPLE_YES)) {
      sprintf(helppath, "%s/%s", SKILL_HELP_PATH, skillIndex[i]);
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
      vlogf(LOG_BUG,fmt("bad call to help function %s, rebuilding indices") %  namebuf);
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
      vlogf(LOG_BUG,fmt("Bogus skill help file: %s") %  skillIndex[i]);
      return;
    }
    disc_num = getDisciplineNumber(skill, FALSE);
    if (disc_num != DISC_NONE) {
      str += purple();
      str += "Discipline       : ";
      str += norm();
      str += disc_names[disc_num];
      str += purple();
      if (isImmortal()) {
        sprintf(buf2, "    (disc: %d, skill %d)", mapDiscToFile(disc_num), skill);
        str += buf2;
      }
    } else 
      vlogf(LOG_BUG, fmt("Bad disc for skill %d in doHelp()") %  skill);
    
    str += purple();
    str += "\n\rSpecialization   : ";
    str += norm();
    str += disc_names[(discArray[skill]->assDisc)];
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
         purple(), norm(), lagAdjust(lag) * combatRound(1)/ONE_SECOND);
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

void buildHelpIndex()
{
  DIR *dfd;
  struct dirent *dp;

  // set a reasonable initial size
  immortalIndex.clear();
  immortalIndex.reserve(128);
  if (!(dfd = opendir(IMMORTAL_HELP_PATH))) {
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
  if (!(dfd = opendir(BUILDER_HELP_PATH))) {
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
  if (!(dfd = opendir(HELP_PATH))) {
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
  if (!(dfd = opendir(SKILL_HELP_PATH))) {
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
  if (!(dfd = opendir(SPELL_HELP_PATH))) {
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













