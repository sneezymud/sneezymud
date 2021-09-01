#include <stdio.h>
#include <tuple>

#include "extern.h"
#include "being.h"
#include "stats.h"

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
static std::vector<sstring>immortalIndex(0);
static std::vector<sstring>builderIndex(0);
static std::vector<sstring>skillIndex(0);
static std::vector<sstring>spellIndex(0);

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
    return "Very Fast";
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

void displayHelpFile(TBeing *ch, const sstring &helppath, sstring namebuf) {
  // make the topic name upper case
  namebuf = static_cast<sstring>(namebuf).upper();

  // find the last modified time on the file
  struct stat timestat;
  if (stat(helppath.c_str(), &timestat)) {
    vlogf(LOG_BUG, format("bad call to help function %s, rebuilding indices") % namebuf);
    buildHelpIndex();
    ch->sendTo("There was an error, try again.\n\r");
    return;
  }
  sstring timebuf = ctime(&(timestat.st_mtim.tv_sec));
  timebuf.pop_back(); // Drop newline
  sstring buffer = format("%s%-30.30s (Last Updated: %s)%s\n\r\n\r") % ch->green() % namebuf %
                   timebuf % ch->norm();

  // special message for nextversion file
  if (!namebuf.compareCaseless("NEXTVERSION")) {
    buffer += "THIS HELP FILE REFLECTS WHAT THE \"news\" COMMAND WILL SHOW NEXT TIME THERE\n\r";
    buffer += "IS A CHANGE IN CODE (PROBABLY IN THE NEXT FEW DAYS).  IT IS HERE TO GIVE\n\r";
    buffer += "YOU SOME IDEA OF WHAT THINGS HAVE BEEN FIXED ALREADY, OR WHAT FEATURES ARE\n\r";
    buffer += "FORTHCOMING...\n\r\n\r";
  }

  // now print the file
  file_to_sstring(helppath.c_str(), buffer, CONCAT_YES);
  buffer += "\n\r";
  ch->desc->page_string(buffer);
}

void TBeing::doHelp(const char *arg) {
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

  if (!static_cast<sstring>(arg).compareCaseless("index")) {
    FILE *index = popen("bin/helpindex", "r");

    sstring str;
    char buffer[MAX_STRING_LENGTH];
    while (fread(buffer, 1, MAX_STRING_LENGTH, index)) {
      str += buffer;
    }
    pclose(index);
    desc->page_string(str);
    return;
  }

  enum resultType { ERROR, MATCH_FOUND, MATCH_NOT_FOUND };

  auto searchIndex = [this, arg](std::vector<sstring> &index, const sstring &path,
                                 std::function<bool(sstring)> skipArgs = nullptr,
                                 bool multiExact = false) {
    sstring helpName, helpPath, buffer, _;

    for (const auto &indexEntry : index) {
      if (skipArgs && skipArgs(arg))
        break;

      bool isMatch = !static_cast<sstring>(arg).compareCaseless(indexEntry);

      if (isMatch || (multiExact ? is_abbrev(arg, indexEntry, MULTIPLE_YES, EXACT_YES) : false) ||
          is_abbrev(arg, indexEntry)) {
        helpName = indexEntry.upper();
        helpPath = format("%s/%s") % path % indexEntry;
        if (isMatch)
          break;
      }
    }

    if (helpName.empty())
      return std::make_tuple(resultType::MATCH_NOT_FOUND, helpName, helpPath, buffer);
  
    if (hasColorVt()) {
      sstring ansiPath = format("%s.ansi") % helpPath;
      if (file_to_sstring(ansiPath.c_str(), _))
        helpPath = ansiPath;
    }

    struct stat timestat;
    if (stat(helpPath.c_str(), &timestat)) {
      vlogf(LOG_BUG, format("bad call to help function %s, rebuilding indices") % helpName);
      buildHelpIndex();
      sendTo("There was an error, try again.\n\r");
      return std::make_tuple(resultType::ERROR, helpName, helpPath, buffer);
    }

    sstring time = ctime(&(timestat.st_mtim.tv_sec));
    time.pop_back(); // Remove newline
    buffer = format("%s%-30.30s (Last Updated: %s)%s\n\r\n\r") % green() % helpName % time % norm();

    return std::make_tuple(resultType::MATCH_FOUND, helpName, helpPath, buffer);
  };

  auto searchImmortalIndexGeneric = [this, searchIndex](std::vector<sstring> &index, sstring path) {
    auto [result, helpName, helpPath, buffer] = searchIndex(index, path);

    if (result == resultType::MATCH_NOT_FOUND)
      return false;

    if (result == resultType::MATCH_FOUND) {
      file_to_sstring(helpPath.c_str(), buffer, CONCAT_YES);
      buffer += "\n\r";
      desc->page_string(buffer);
    }

    return true;
  };

  auto searchImmortalIndex = [this, searchImmortalIndexGeneric]() {
    if (hasWizPower(POWER_IMMORTAL_HELP) &&
        searchImmortalIndexGeneric(immortalIndex, Path::IMMORTAL_HELP))
      return true;
    return false;
  };

  auto searchBuilderIndex = [this, searchImmortalIndexGeneric]() {
    if (GetMaxLevel() >= GOD_LEVEL1 && searchImmortalIndexGeneric(builderIndex, Path::BUILDER_HELP))
      return true;
    return false;
  };

  auto searchImmortalIndexes = [this, searchImmortalIndex, searchBuilderIndex]() {
    if (!isImmortal())
      return false;

    if (searchImmortalIndex() || searchBuilderIndex())
      return true;    

    return false;
  };

  auto searchHelpIndex = [this, searchIndex]() {
    auto skipArgs = [](sstring sArg) {
      return !sArg.compareCaseless("armor") || !sArg.compareCaseless("sharpen") ||
             !sArg.compareCaseless("bleed");
    };

    auto [result, helpName, helpPath, buffer] = searchIndex(helpIndex, Path::HELP, skipArgs);

    if (result == resultType::MATCH_NOT_FOUND)
      return false;

    if (result == resultType::MATCH_FOUND)
        displayHelpFile(this, helpPath, helpName);

    return true;
  };

  auto findSkill = [this](const sstring &helpName) {
    // first, see if we can find a matching skill that the player has
    // this is here so skills with same name (for different classes) will
    // be isolated.
    spellNumT skillNumber = MIN_SPELL;
    for (; skillNumber < MAX_SKILL; skillNumber++) {
      if (hideThisSpell(skillNumber))
        continue;

      if (helpName.compareCaseless(discArray[skillNumber]->name) != 0)
        continue;

      if (doesKnowSkill(skillNumber))
        break;
    }

    // if we can't find match on name for skill they have, just use name match
    if (skillNumber >= MAX_SKILL) {
      for (skillNumber = MIN_SPELL; skillNumber < MAX_SKILL; skillNumber++) {
        if (hideThisSpell(skillNumber))
          continue;

        if (!helpName.compareCaseless(discArray[skillNumber]->name))
          break;
      }
    }

    spellInfo *skill = nullptr;

    if (skillNumber >= MAX_SKILL || (skillNumber = getSkillNum(skillNumber)) < 0) {
      vlogf(LOG_BUG, format("Bogus help file: %s") % helpName);
      return std::make_tuple(skill, MAX_SKILL);
    }

    skill = discArray[skillNumber];

    if (skill && skill->disc == DISC_NONE) {
      vlogf(LOG_BUG, format("Skill/spell %s with DISC_NONE in doHelp()") % helpName);
      return std::make_tuple(skill, skillNumber);
    }

    return std::make_tuple(skill, skillNumber);
  };

  auto bufferSkillInfo = [this](sstring &buffer, spellInfo *skill, spellNumT skillNumber) {
    buffer +=
        format("%sDiscipline       :%s %s") % purple() % norm() % discNames[skill->disc].properName;
    if (isImmortal())
      buffer += format("    %s(disc: %d, spell %d)%s") % purple() % mapDiscToFile(skill->disc) %
                skillNumber % norm();

    buffer += format("\n\r%sSpecialization   :%s %s") % purple() % norm() %
              discNames[skill->assDisc].properName;
    if (isImmortal())
      buffer += format("    %s(disc: %d)%s") % purple() % mapDiscToFile(skill->assDisc) % norm();

    buffer += format("\n\r%sStart Learning   :%s %d%%") % purple() % norm() % skill->start;

    buffer +=
        format("\n\r%sDisc. Learn Rate :%s %s") % purple() % norm() % learn_name(skill->learn);
    if (isImmortal())
      buffer += format(" %s(%d)%s") % purple() % skill->learn % norm();

    buffer += format("\n\r%sLearn By Doing   :%s %s") % purple() % norm() %
              (skill->startLearnDo == -1 ? "No" : "Yes");
    if (isImmortal())
      buffer +=
          format("  %s(%d) (%d) %s") % purple() % skill->startLearnDo % skill->amtLearnDo % norm();

    buffer += format("\n\r%sModifier Stat    :%s %s\n\r") % purple() % norm() %
              statToString(skill->modifierStat).cap();

    buffer += format("%sDifficulty       :%s %s\n\r") % purple() % norm() %
              displayDifficulty(skillNumber);
  };

  auto bufferSkillLagInfo = [this](sstring &buffer, lag_t lag) {
    if (lag > LAG_0) {
      buffer += format("%sCommand lock-out :%s %.1f seconds") % purple() % norm() %
                (lagAdjust(lag) * combatRound(1) / Pulse::ONE_SECOND);
      if (isImmortal())
        buffer += format(" %s(%d rounds)%s") % purple() % lag % norm();
      buffer += "\n\r";
    } else
      buffer += format("%sCommand lock-out :%s None\n\r") % purple() % norm();
  };

  auto bufferHelpFileContents = [](sstring &buffer, const sstring &helpPath) {
    buffer += "\n\r";
    file_to_sstring(helpPath.c_str(), buffer, CONCAT_YES);
    buffer += "\n\r";
  };

  auto searchSpellIndex = [this, searchIndex, findSkill, bufferSkillInfo, bufferSkillLagInfo,
                           bufferHelpFileContents]() {
    auto skipArgs = [](sstring sArg) { return !sArg.compareCaseless("steal"); };

    auto [result, helpName, helpPath, buffer] =
        searchIndex(spellIndex, Path::SPELL_HELP, skipArgs, true);

    if (result == resultType::ERROR)
      return true;

    if (result == resultType::MATCH_NOT_FOUND)
      return false;

    spellInfo *spell = nullptr;
    spellNumT spellNumber = MAX_SKILL;

    // Have to use std::tie here because lambda used in find_if below
    // can't capture structured binding in C++17 apparently
    std::tie(spell, spellNumber) = findSkill(helpName);
    if (!spell || spellNumber == MAX_SKILL)
      return true;

    bufferSkillInfo(buffer, spell, spellNumber);

    unsigned int compTypes = spell->comp_types;
    bool isTasked = IS_SET(compTypes, SPELL_TASKED);

    if (spellNumber == SPELL_MATERIALIZE || spellNumber == SPELL_SPONTANEOUS_GENERATION)
      buffer += format("%sSpell Component  :%s SPECIAL (see below)\n\r") % purple() % norm();
    else if (isTasked) {
      auto comp = std::find_if(CompInfo.begin(), CompInfo.end(), [spellNumber](compInfo comp) {
        return comp.spell_num == spellNumber;
      });

      if (comp != CompInfo.end()) {
        sstring compName = obj_index[real_object(comp->comp_num)].short_desc.cap();
        buffer += format("%sSpell Component  :%s %s\n\r") % purple() % norm() % compName;
      } else
        vlogf(LOG_BUG,
              format("Problem in help file for skill=%d, comp=%d.  (component definition)") %
                  spellNumber % comp->comp_num);
    } else
      buffer += format("%sSpell Component  :%s NONE\n\r") % purple() % norm();

    int immunityType = getTypeImmunity(spellNumber);
    buffer += format("%sImmunity Type    :%s %s\n\r") % purple() % norm() %
              (immunityType != -1 ? immunity_names[immunityType] : "NONE");

    lag_t lag = spell->lag;
    if (isTasked) {
      buffer +=
          format("%sCasting rounds   :%s %d casting rounds\n\r") % purple() % norm() % (lag + 2);
      buffer +=
          format("%sCombat rounds    :%s %d combat rounds\n\r") % purple() % norm() % (lag + 1);
    } else
      bufferSkillLagInfo(buffer, lag);

    sstring minResource, curResource, combinedResource;
    bool bufferSpellResourceInfo = false;
    int minResourceAmt = 0;
    if (spell->minMana) {
      bufferSpellResourceInfo = true;
      minResourceAmt = spell->minMana;
      minResource = "Minimum Mana     :";
      curResource = "Current Mana     :";
      combinedResource = "Mana (min/cur)   :";
    } else if (spell->minLifeforce) {
      bufferSpellResourceInfo = true;
      minResourceAmt = spell->minLifeforce;
      minResource = "Minimum Lifeforce:";
      curResource = "Current Lifeforce:";
      combinedResource = "Lifeforce:min/cur:";
    } else if (spell->minPiety) {
      bufferSpellResourceInfo = true;
      minResourceAmt = spell->minPiety;
      minResource = "Minimum Piety    :";
      curResource = "Current Piety    :";
      combinedResource = "Piety (min/cur)  :";
    }

    if (bufferSpellResourceInfo) {
      if (doesKnowSkill(spellNumber)) {
        if (isTasked) {
          buffer += format("%s%s%s %d, per round amount : %d\n\r") % purple() % minResource %
                    norm() % ((minResourceAmt / (lag + 2)) * (lag + 2)) %
                    (minResourceAmt / (lag + 2));

          buffer += format("%s%s%s %d, per round amount : %d\n\r") % purple() % curResource %
                    norm() % (useMana(spellNumber) * (lag + 2)) % useMana(spellNumber);
        } else
          buffer += format("%s%s%s %d, current : %d\n\r") % purple() % minResource % norm() %
                    minResourceAmt % useMana(spellNumber);
      } else
        buffer += format("%s%s%s %d/%s-not-known\n\r") % purple() % combinedResource % norm() %
                  minResourceAmt % (spell->minPiety ? "prayer" : "spell");
    }

    if (compTypes) {
      buffer += format("%sRequires         :%s ") % purple() % norm();
      if (compTypes & COMP_GESTURAL) {
        buffer += "Gestural Moves";
        if (compTypes & COMP_VERBAL)
          buffer += format(", Spoken%s") % (spell->holyStrength > 0 ? " Mantra" : " Incantation");
      }
      buffer += "\n\r";
    }

    unsigned int targets = spell->targets;

    sstring isOffensiveSpell = (targets & TAR_VIOLENT) ? "Yes" : "No";
    sstring isAOESpell = (targets & TAR_AREA) ? "Yes" : "No";
    buffer += format("%sOffensive        :%s %s\t") % purple() % norm() % isOffensiveSpell;
    buffer += format("%sArea Effect          :%s %s\n\r") % purple() % norm() % isAOESpell;

    sstring canCastOnSelf = (targets & TAR_SELF_NONO) 
        ? "No"
        : (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY))
            ? "Yes"
            : "No";

    sstring canCastOnObject =
        (targets & (TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_WORLD | TAR_OBJ_EQUIP))
            ? "Yes" 
            : "No";

    buffer += format("%sCast on Self     :%s %s\t") % purple() % norm() % canCastOnSelf;
    buffer += format("%sObject Castable      :%s %s\n\r") % purple() % norm() % canCastOnObject;

    sstring canCastOnOthers = (targets & TAR_SELF_ONLY)
        ? "No"
        : (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD))
            ? "Yes"
            : "No";

    buffer += format("%sCast on Others   : %s%s \n\r") % purple() % norm() % canCastOnOthers;

    bufferHelpFileContents(buffer, helpPath);
    desc->page_string(buffer);

    return true;
  };

  auto searchSkillIndex = [this, searchIndex, findSkill, bufferSkillInfo, bufferSkillLagInfo,
                           bufferHelpFileContents]() {
    auto skipArgs = [](sstring sArg) { return !sArg.compareCaseless("cast"); };

    auto [result, helpName, helpPath, buffer] =
        searchIndex(skillIndex, Path::SKILL_HELP, skipArgs, true);

    if (result == resultType::ERROR)
      return true;

    if (result == resultType::MATCH_NOT_FOUND)
      return false;

    spellInfo *skill = nullptr;
    spellNumT skillNumber = MAX_SKILL;

    // Have to use std::tie here because lambda used in find_if below
    // can't capture structured binding in C++17 apparently
    std::tie(skill, skillNumber) = findSkill(helpName);
    if (!skill || skillNumber == MAX_SKILL)
      return true;

    bufferSkillInfo(buffer, skill, skillNumber);
    bufferSkillLagInfo(buffer, skill->lag);
    bufferHelpFileContents(buffer, helpPath);
    desc->page_string(buffer);
    return true;
  };

  if (searchImmortalIndexes() || searchHelpIndex() || searchSpellIndex() || searchSkillIndex())
    return;

  sendTo("No such help file available.\n\r");
}

void TBeing::doBuildhelp(const char *arg) {
  if (!desc)
    return;

  for (; isspace(*arg); arg++)
    ;

  if (!isImmortal() || GetMaxLevel() < GOD_LEVEL1 || !hasWizPower(POWER_BUILDER)) {
    sendTo("Sorry, only builders may access build help files.\n\r");
    return;
  }
}

void TBeing::doWizhelp(const char *arg) {
  if (!isImmortal())
    return;

  if (!desc)
    return;

  for (; isspace(*arg); arg++);

  int i;
  wizPowerT tPower;
  int tLength = 2;
  for (i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;

    if ((GetMaxLevel() >= commandArray[i]->minLevel) && (commandArray[i]->minLevel > MAX_MORT) &&
        ((tPower = wizPowerFromCmd(cmdTypeT(i))) == MAX_POWER_INDEX || hasWizPower(tPower)))
      tLength = max(strlen(commandArray[i]->name), (size_t)tLength);
  }

  sstring tString = format("%c-%ds") % '%' % (tLength + 1);
  tLength = (79 / tLength);

  sendTo("The following privileged commands are available:\n\r\n\r");

  sstring buf, sbuf;
  if ((tPower = wizPowerFromCmd(CMD_AS)) == MAX_POWER_INDEX || hasWizPower(tPower))
    buf = format(tString) % "as";

  for (int no = 2, i = 0; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;

    if ((GetMaxLevel() >= commandArray[i]->minLevel) && (commandArray[i]->minLevel > MAX_MORT) &&
        ((tPower = wizPowerFromCmd(cmdTypeT(i))) == MAX_POWER_INDEX || hasWizPower(tPower))) {
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

void buildHelpIndex() {
  auto buildIndex = [](std::vector<sstring> &index, const sstring &path, const sstring &name = "") {
    // set a reasonable initial size
    index.clear();
    index.reserve(128);

    DIR *directory = nullptr;
    if (!(directory = opendir(path.c_str()))) {
      vlogf(LOG_FILE,
            format("Can't open%s help directory for indexing!") % (name.empty() ? "" : " " + name));
      exit(0);
    }

    struct dirent *directoryItem = nullptr;
    while ((directoryItem = readdir(directory))) {
      if (!strcmp(directoryItem->d_name, ".") || !strcmp(directoryItem->d_name, "..") ||
          (strlen(directoryItem->d_name) >= 5 &&
           !strcmp(&directoryItem->d_name[strlen(directoryItem->d_name) - 5], ".ansi")))
        continue;

      index.emplace_back(directoryItem->d_name);
    }

    closedir(directory);
  };

  buildIndex(immortalIndex, Path::IMMORTAL_HELP, "immortal");
  buildIndex(builderIndex, Path::BUILDER_HELP, "builder");
  buildIndex(helpIndex, Path::HELP);
  buildIndex(skillIndex, Path::SKILL_HELP, "skill");
  buildIndex(spellIndex, Path::SPELL_HELP, "spell");
}
