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

// Class and member functions for handling helpfile requests in doHelp
namespace {
  class HelpFileHandler {
    public:
      HelpFileHandler() = delete;
      HelpFileHandler(sstring args, TBeing* user) : originalArgs(std::move(args)), user(user){};

      void displayOutput();

    private:
      const sstring originalArgs;
      const TBeing* user;
      spellNumT skillNumber{MAX_SKILL};
      spellInfo* skill{nullptr};
      bool isSpell{false};
      sstring fileName;
      sstring filePath;
      sstring output;

      struct stat timestat {};

      std::vector<std::pair<sstring, sstring>> partialMatches;

      void addComponentInfo();
      void addHeader();
      void addImmunityInfo();
      void addFileContents();
      void addSkillInfo();
      void addSkillLagInfo();
      void addSpellRequirements();
      void addSpellResourceInfo();
      void addSpellTargetInfo();
      sstring infoLine(const sstring& label, const sstring& value);
      bool initSkillInfo();
      void setIsSpell();
      bool isValidRequest() const;
      bool returnEarly();

      sstring purple() const { return user->purple(); };

      sstring norm() const { return user->norm(); };

      bool searchAllIndexes();
      bool searchBuilderIndex();
      bool searchGeneralIndex();
      bool searchImmortalIndex();
      bool searchIndex(const std::vector<sstring>& index, const sstring& path);
      bool searchSkillIndex();
      bool searchSpellIndex();
      void setFilePath(const sstring& path);
      bool updateFileStats();
  };

  void HelpFileHandler::addComponentInfo() {
    if (!isSpell || skill->minPiety > 0)
      return;

    sstring compName = "NONE";

    if (skillNumber == SPELL_MATERIALIZE || skillNumber == SPELL_SPONTANEOUS_GENERATION) {
      compName = "SPECIAL (see below)";
    }

    auto comp = std::find_if(CompInfo.begin(), CompInfo.end(), [this](const compInfo& comp) {
      return comp.spell_num == skillNumber;
    });

    if (comp != CompInfo.end()) {
      auto index = static_cast<size_t>(real_object(comp->comp_num));
      compName = obj_index[index].short_desc.cap();
    }

    output += infoLine("Spell Component", compName);
  }

  void HelpFileHandler::addFileContents() {
    output += "\n\r\n\r";
    file_to_sstring(filePath, output, CONCAT_YES);
  }

  void HelpFileHandler::addHeader() {
    sstring time = ctime(&(timestat.st_mtim.tv_sec));
    time.pop_back();  // Drop newline
    output += format("%s%-30.30s (Last Updated: %s)%s\n\r") % user->green() % fileName.upper() %
              time % user->norm();

    if (fileName.compareCaseless("NEXTVERSION") != 0)
      return;

    // special message for nextversion file
    output += "THIS HELP FILE REFLECTS WHAT THE \"news\" COMMAND WILL SHOW NEXT TIME THERE\n\r";
    output += "IS A CHANGE IN CODE (PROBABLY IN THE NEXT FEW DAYS).  IT IS HERE TO GIVE\n\r";
    output += "YOU SOME IDEA OF WHAT THINGS HAVE BEEN FIXED ALREADY, OR WHAT FEATURES ARE\n\r";
    output += "FORTHCOMING...\n\r\n\r";
  };

  void HelpFileHandler::addImmunityInfo() {
    if (!skill)
      return;

    const immuneTypeT immunityType = getTypeImmunity(skillNumber);
    const sstring immunityStr = immunityType >= IMMUNE_NONE ? immunity_names[immunityType] : "NONE";

    output += format("%sImmunity Type    :%s %s\n\r") % purple() % norm() % immunityStr;
  }

  void HelpFileHandler::addSkillInfo() {
    if (!skill)
      return;

    const sstring properName = discNames[skill->disc].properName;

    sstring discStr = infoLine("Discipline", properName);
    sstring spec = infoLine("Specialization", properName);
    sstring startLearn = infoLine("Start Learning", std::to_string(skill->start));
    sstring learnRate = infoLine("Disc. Learn Rate", learn_name(skill->learn));
    sstring lbd = infoLine("Learn By Doing", skill->startLearnDo == -1 ? "No" : "Yes");
    sstring modStat = infoLine("Modifier Stat", statToString(skill->modifierStat).cap());
    sstring diff = infoLine("Difficulty", displayDifficulty(skillNumber));

    if (user->isImmortal()) {
      discStr += format(" %s(disc: %d, spell %d)%s") % purple() % mapDiscToFile(skill->disc) %
                 skillNumber % norm();
      spec += format(" %s(disc: %d)%s") % purple() % mapDiscToFile(skill->assDisc) % norm();
      learnRate += format(" %s(%d)%s") % purple() % skill->learn % norm();
      lbd +=
        format(" %s(start: %d) (amount: %d)%s") % purple() % skill->startLearnDo % skill->amtLearnDo % norm();
    }

    output += discStr + spec + startLearn + learnRate + lbd + modStat + diff;
  };

  void HelpFileHandler::addSkillLagInfo() {
    if (!skill)
      return;

    const lag_t lag = skill->lag;
    bool isTasked = IS_SET(skill->comp_types, SPELL_TASKED);

    if (isTasked) {
      output += infoLine("Casting Rounds", std::to_string(lag + 2));      
      output += infoLine("Combat Rounds", std::to_string(lag + 1));
    }

    sstring label = "Command Lock-Out";
    if (lag > LAG_0) {
      output += infoLine(label, format("%.1f seconds") % (user->lagAdjust(lag) * combatRound(1) / Pulse::ONE_SECOND));

      if (user->isImmortal())
        output += format(" %s(%d round%s)%s") % purple() % lag % (lag != 1 ? "s" : "") % norm();
    } else
      output += infoLine(label, "None");
  };

  void HelpFileHandler::addSpellRequirements() {
    if (!skill || !isSpell)
      return;

    const uint32_t compTypes = skill->comp_types;

    if (!compTypes)
      return;

    const sstring gestures = compTypes & COMP_GESTURAL ? "Gestures" : "";
    const sstring spokenType = skill->holyStrength > 0 ? "Mantra" : "Incantation";
    const sstring spoken = compTypes & COMP_VERBAL ? "Spoken " + spokenType : "";
    const sstring both = !gestures.empty() && !spoken.empty() ? ", " : "";

    output += infoLine("Requires", format("%s%s%s") % gestures % both % spoken);
  }

  void HelpFileHandler::addSpellResourceInfo() {
    if (!skill || !isSpell)
      return;

    sstring resource;
    sstring type;
    double minAmt = 0;
    double curAmt = 0;

    if (skill->minMana > 0) {
      resource = "Mana";
      type = "spell";
      minAmt = skill->minMana;
      curAmt = user->useMana(skillNumber);
    } else if (skill->minLifeforce > 0) {
      resource = "Lifeforce";
      type = "ritual";
      minAmt = skill->minLifeforce;
      curAmt = user->useLifeforce(skillNumber);
    } else if (skill->minPiety > 0) {
      resource = "Piety";
      type = "prayer";
      minAmt = skill->minPiety;
      curAmt = user->usePiety(skillNumber);
    } else
      return;

    sstring minLabel = format("Minimum %-9.9s") % resource;
    sstring curLabel = format("Current %-9.9s") % resource;
    sstring combinedLabel = format("%s %-10.10s") % resource % "(min/cur)";

    if (user->doesKnowSkill(skillNumber)) {
      if (IS_SET(skill->comp_types, SPELL_TASKED)) {
        lag_t lag = skill->lag;
        output += infoLine(minLabel,
                           format("%d, per round amount: %d") % ((minAmt / (lag + 2)) * (lag + 2)) %
                             (minAmt / (lag + 2)));
        output +=
          infoLine(curLabel, format("%d, per round amount: %d") % (curAmt * (lag + 2)) % curAmt);
      } else
        output += infoLine(minLabel, format("%d, current: %d") % minAmt % curAmt);
    } else
      output += infoLine(combinedLabel, format("%d/%s-not-known") % minAmt % type);
  }

  void HelpFileHandler::addSpellTargetInfo() {
    if (!skill || !isSpell)
      return;

    uint32_t targets = skill->targets;

    sstring isOffensiveSpell = (targets & TAR_VIOLENT) ? "Yes" : "No";
    sstring isAOESpell = (targets & TAR_AREA) ? "Yes" : "No";
    sstring canCastOnSelf =
      (targets & TAR_SELF_NONO)                                                       ? "No"
      : (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY)) ? "Yes"
                                                                                      : "No";
    sstring canCastOnOthers = (targets & TAR_SELF_ONLY)                      ? "No"
                              : (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD)) ? "Yes"
                                                                             : "No";
    sstring canCastOnObject =
      (targets & (TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_WORLD | TAR_OBJ_EQUIP)) ? "Yes" : "No";

    output += infoLine("Offensive", isOffensiveSpell);
    output += infoLine("Area Effect", isAOESpell);
    output += infoLine("Cast on Self", canCastOnSelf);
    output += infoLine("Cast on Others", canCastOnOthers);
    output += infoLine("Cast on Object", canCastOnObject);
  }

  void HelpFileHandler::displayOutput() {
    if (returnEarly())
      return;

    addHeader();
    addSkillInfo();
    addComponentInfo();
    addSkillLagInfo();
    addSpellResourceInfo();
    addSpellRequirements();
    addSpellTargetInfo();
    addFileContents();

    user->desc->page_string(output);
  }

  // Standardize printing skill/spell helpfile header lines
  sstring HelpFileHandler::infoLine(const sstring& label, const sstring& value) {
      return format("\n\r%s%-16.16s:%s %s") % purple() % label % norm() % value;
  }

  bool HelpFileHandler::initSkillInfo() {
    struct MatchedSkill {
        spellNumT skillNumber{MAX_SKILL};
        spellInfo* skill{nullptr};

        MatchedSkill() = default;
        MatchedSkill(spellNumT sn, spellInfo* sk) : skillNumber(sn), skill(sk){};
    };

    auto* begin = std::begin(discArray);
    auto* end = std::end(discArray);

    std::vector<MatchedSkill> matches{};

    auto* nameMatch = begin;
    while (nameMatch != end) {
      nameMatch = std::find_if(nameMatch, end, [this](spellInfo* si) {
        return (si && !fileName.compareCaseless(si->name));
      });

      if (nameMatch != end) {
        // Can calculate index of iterator by subtracting .begin(). Index of
        // discArray is actually the value of the spellNumT enum for the entry
        // at that index. Have to obtain it this way because it's not stored in
        // the actual spellInfo object for some reason.
        auto skillNum = static_cast<spellNumT>(nameMatch - begin);
        matches.emplace_back(skillNum, *nameMatch);
        nameMatch = std::next(nameMatch);
      }
    }

    if (matches.empty()) {
      return false;
    }

    auto nameMatchesKnownSkill =
      std::find_if(matches.begin(), matches.end(), [this](MatchedSkill& sk) {
        return (!hideThisSpell(sk.skillNumber) && user->doesKnowSkill(sk.skillNumber));
      });

    // If one of the matches is also a skill known by the character, use it.
    // Otherwise just grab the first skill with a matching name.
    auto match = nameMatchesKnownSkill != matches.end() ? *nameMatchesKnownSkill : matches.front();
    skillNumber = match.skillNumber;
    skill = match.skill;
    setIsSpell();
    return true;
  }

  bool HelpFileHandler::isValidRequest() const {
    // this prevents "help ../../being.h" and "help _skills"
    return originalArgs.isOnlyAlnum();
  }

  bool HelpFileHandler::searchAllIndexes() {
    if (searchImmortalIndex() || searchBuilderIndex() || searchGeneralIndex())
      return true;

    if (searchSpellIndex() || searchSkillIndex())
      return initSkillInfo();

    if (partialMatches.empty())
      return false;

    // Sort helpfile names for which arg was a partial match by size and use the shortest one, as
    // longer helpfile names can be accessed by passing a more specific arg.
    std::sort(partialMatches.begin(),
              partialMatches.end(),
              [](std::pair<sstring, sstring>& a, const std::pair<sstring, sstring>& b) {
                return a.first.size() < b.first.size();
              });

    const auto& [indexEntry, path] = partialMatches.front();
    fileName = indexEntry;
    setFilePath(path);
    return (path == Path::SKILL_HELP || path == Path::SPELL_HELP) ? initSkillInfo() : true;
  }

  bool HelpFileHandler::searchBuilderIndex() {
    return user->isImmortal() && user->GetMaxLevel() >= GOD_LEVEL1 &&
           searchIndex(builderIndex, Path::BUILDER_HELP);
  }

  bool HelpFileHandler::searchGeneralIndex() {
    return searchIndex(helpIndex, Path::HELP);
  }

  bool HelpFileHandler::searchImmortalIndex() {
    return user->isImmortal() && user->hasWizPower(POWER_IMMORTAL_HELP) &&
           searchIndex(immortalIndex, Path::IMMORTAL_HELP);
  }

  bool HelpFileHandler::searchIndex(const std::vector<sstring>& index, const sstring& path) {
    fileName = originalArgs;

    bool exactMatch = false;
    for (const auto& indexEntry : index) {
      exactMatch = !fileName.compareCaseless(indexEntry);
      if (exactMatch) {
        fileName = indexEntry;
        setFilePath(path);
        partialMatches.clear();
        break;
      }

      // Use different variations of is_abbrev to ensure finding all possible abbreviations. Keep
      // track of partial matches through all indexes. If an exact match is never found, use these
      // to determine which file to display.
      if (is_abbrev(fileName, indexEntry) ||
          is_abbrev(fileName, indexEntry, MULTIPLE_YES, EXACT_NO) ||
          is_abbrev(fileName, indexEntry, MULTIPLE_YES, EXACT_YES))
        partialMatches.emplace_back(indexEntry, path);
    }

    return exactMatch;
  }

  bool HelpFileHandler::searchSkillIndex() {
    return searchIndex(skillIndex, Path::SKILL_HELP);
  }

  bool HelpFileHandler::searchSpellIndex() {
    return searchIndex(spellIndex, Path::SPELL_HELP);
  }

  void HelpFileHandler::setFilePath(const sstring& path) {
    filePath = format("%s%s%s") % path % (filePath == Path::HELP ? "" : "/") % fileName;
    sstring ansiPath = format("%s.ansi") % filePath;
    if (user->hasColorVt() && fileExists(ansiPath))
      filePath = ansiPath;
  }

  void HelpFileHandler::setIsSpell() {
    if (!skill)
      return;

    isSpell = skill->minMana > 0 || skill->minLifeforce > 0 || skill->minPiety > 0;
  }

  bool HelpFileHandler::updateFileStats() {
    if (!stat(filePath.c_str(), &timestat))
      return true;

    vlogf(LOG_BUG, format("bad call to help function %s, rebuilding indices") % fileName.upper());
    buildHelpIndex();
    user->sendTo("There was an error, try again.\n\r");
    return false;
  }

  // Define a series of tests and resulting actions on test failure. Before
  // building the output, execute all tests and if any fail, execute the
  // included action then end the helpfile request.
  bool HelpFileHandler::returnEarly() {
    struct EarlyReturn {
        std::function<bool()> test;
        std::function<void()> action;
    };

    const std::vector<EarlyReturn> returnCases = {
      {
        [this]() { return !user || !user->desc; },
        []() {},
      },
      {
        [this]() { return originalArgs.trim().empty(); },
        [this]() { user->desc->start_page_file(File::HELP_PAGE, "General help unavailable.\n\r"); },
      },
      {
        [this]() { return !isValidRequest(); },
        [this]() { user->sendTo("Illegal argument.\n\r"); },
      },
      {
        [this]() { return !originalArgs.compareCaseless("index"); },
        [this]() {
          // bin/helpindex is a perl script. Use popen to execute the script in
          // a separate process then pull the output in to a stream for reading.
          FILE* index = popen("bin/helpindex", "r");
          if (!index) {
            vlogf(LOG_FILE, "popen failed to open bin/helpindex for read in cmd_help.cc");
            return;
          }

          char* line = nullptr;
          size_t len = 0;
          while (getline(&line, &len, index) != -1) {
            output += line;
          }
          pclose(index);
          user->desc->page_string(output);
        },
      },
      {
        [this]() { return !searchAllIndexes(); },
        [this]() { user->sendTo("No such help file available.\n\r"); },
      },
      {
        [this]() { return !updateFileStats(); },
        []() {},
      }};

    for (const auto& returnCase : returnCases) {
      if (!returnCase.test())
        continue;

      returnCase.action();
      return true;
    }

    help_used_num++;
    total_help_number++;
    save_game_stats();

    return false;
  }
}  // namespace

void TBeing::doHelp(const sstring& arg) {
  HelpFileHandler handler(arg, this);
  handler.displayOutput();
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
