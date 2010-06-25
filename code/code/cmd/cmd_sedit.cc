/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_sedit.cc"
  All functions and routines related to the Script Editor.

  Created 6/12/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
}

#include <algorithm>

#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "configuration.h"
#include "combat.h"
#include "dirsort.h"
#include "person.h"
#include "monster.h"

void seditAddMenu        (TBeing *, TMonster *, const char *, int);
void seditDeleteMenu     (TBeing *, TMonster *, const char *, int);
void seditModifyMenu     (TBeing *, TMonster *, const char *, int);
void seditDisplayMenu    (TBeing *, TMonster *, const char *, int);
void seditSaveMenu       (TBeing *, TMonster *, const char *, int);
void seditLoadMenu       (TBeing *, TMonster *, const char *, int);
void seditClearMenu      (TBeing *, TMonster *, const char *, int);
void seditLoad           (TBeing *, TMonster *, sstring, bool = false);
void seditSave           (TBeing *, TMonster *, bool = false);
void update_sedit_menu   (TBeing *, TMonster *, bool = false);
void sedit               (TBeing *, TMonster *);
void seditClear          (TBeing *, TMonster *);
void seditList           (TBeing *);
void seditPurge          (TBeing *);
FILE * seditVerifyDirTree(TBeing *, char * = NULL, bool = false, bool = false);
void seditDisplayResponse(TBeing *, resp *, bool, int, bool = false, sstring * = NULL);
resp * seditFindResponse (resp *, sstring, bool *, int = -1);
sstring seditExtraWords   (cmdTypeT);
void seditReadUntil      (sstring &, sstring &, char);
cmdTypeT seditCmdFromText(sstring, bool);
void seditCoreAdd        (TBeing *, TMonster *, cmdTypeT, sstring, cmdTypeT, sstring);
void seditCoreDelete     (TBeing *, TMonster *, cmdTypeT, sstring, cmdTypeT, sstring);
void seditCoreBreakdown  (sstring, cmdTypeT &, sstring &);

const char *editor_types_sedit[] =
{
  "modify",
  "add",
  "delete",
  "display",
  "save",
  "load",
  "clear",
  "list",
  "purge",
  "purge!",
  "\n\r"
};

static const char *script_edit_menu =
"%s1)%s Add Trigger\n\r"
"%s2)%s Delete Trigger\n\r"
"%s3)%s Modify Trigger\n\r"
"%s4)%s Display Current Triggers\n\r"
"%s5)%s Save Current Triggers to File\n\r"
"%s6)%s Load Current Triggers from File\n\r"
"%s7)%s Clear Current Triggers in File and on Mobile\n\r"
"\n\r";

void send_sedit_menu(TBeing *ch)
{
  ch->sendTo(format(script_edit_menu) %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm() %
             ch->cyan() % ch->norm());
}

void stSpaceOut(sstring & tStString)
{
  while (!tStString.empty() && isspace((tStString.c_str())[0]))
    tStString.erase(tStString.begin());
}

// Add game mobs with scripts that can be messed with here.
// 0 = Cannot Modify
// 1 = Can Modify
// 2 = Can only view
// Keep in mind that at no time should a mob with a vnum
// greater than -1 return 1.  At most it should return 2.
// else we give builders a loop where they can make a script
// change to load some really awesome stuff and blow everything
// out of the water.
int seditCanModify(TBeing *tBeing, TMonster *tMonster)
{
  // Creators and lows can modify anything for world modifications.
  // This is the one exception to the above rule and should remain
  // unchanged.  -Lapsos
  if (tBeing->hasWizPower(POWER_SEDIT_IMP_POWER))
    return 1;

  if (tMonster->mobVnum() >= 0)
    switch (tMonster->mobVnum()) {
      // Add new mob vnums here which return 2

      default:
        return 0;
    };

  return 1;
}

void TBeing::doSEdit(const char *)
{
  sendTo("Silly monster, you may not modify scripts.\n\r");
}

void TPerson::doSEdit(const char *tArg)
{
  int       field,
            tCount = 0,
            tShift = -1;
  char      tString[256];
  TThing   *tThing;
  TMonster *tMonster = NULL;
  sstring    tStArg(""),
            tStMobile(""),
            tStInit(""),
            tStString(""),
            tStBuffer("");
  resp     *respIndex;
  bool      tForm = false;
  cmdTypeT  tCmd,
            tCmdB;

  if (!hasWizPower(POWER_SEDIT)) {
    sendTo("You don't have the power to modify scripts.\n\r");
    return;
  }

  if (strcmp(getName(), "Lapsos") != 0 &&
      strcmp(getName(), "Damescena") != 0) {
    sendTo("This code is under development...do not use.\n\r");
    return;
  }

  if (!desc)
    return;

  if (!tArg || !*tArg) {
    tStString = "\0";
    tStString += "Syntax:\n\r";
    tStString += "  sedit modify <mobile>  - Go into SEdit menu interface\n\r";
    tStString += "  sedit add <mobile> <{trigger}> <what-to-add> - Add to trigger/action\n\r";
    tStString += "  sedit delete <mobile> <{trigger}/action> - Remove trigger/action\n\r";
    tStString += "  sedit display <mobile> <tofind> - Display current triggers/actions\n\r";
    tStString += "  sedit save <mobile> - Save triggers on <mobile>\n\r";
    tStString += "  sedit load <mobile> - Load triggers for <mobile>\n\r";
    tStString += "  sedit clear <mobile> - Clear all triggers from <mobile> (see helpfile)\n\r";
    tStString += "  sedit list - Display all stored scripts\n\r";
    tStString += "  sedit purge! - Purge *ALL* scripts in your directory (see helpfile)\n\r";
    tStString += "----------------------------------------------------------------------\n\r";
    tStString += "  Anywhere {} was used it means you have to include that in the line:\n\r";
    tStString += "  sedit add elite-guard {black gate} tell %%n I saw that!  To the East!\n\r";
    tStString += "  sedit delete elite-guard {black gate} tell %%n Go East!\n\r";

    sendTo(tStString);
    return;
  }

  bisect_arg(tArg, &field, tString, editor_types_sedit);

  if (field != 8 && field != 9 && field != 10) {
    tStInit = tString;
    tStArg = one_argument(tStInit, tStMobile);


    if (!(tThing = searchLinkedListVis(this, tStMobile, roomp->stuff))) {
      sendTo("I don't see that here, do you?\n\r");
      return;
    } else if (!(tMonster = dynamic_cast<TMonster *>(tThing))) {
      sendTo("You're funny,  This is used on mobiles.\n\r");
      return;
    } else if (tMonster->isPc()) {
      sendTo("I'm sure they would Love that, perhaps after a good lobotomy...\n\r");
      return;
    } else if (tMonster->desc || tMonster->orig) {
      sendTo("At this stage I think that would be unwise.\n\r");
      return;
    }
  }

  switch (field) {
    case 1: // Modify
      if (!seditCanModify(this, tMonster)) {
        sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
        return;
      }

      if (gamePort != Config::Port::PROD)
        sedit(this, tMonster);
      else
        sendTo("There are still problems with the menu system, please don't use it yet.\n\r");

      return;
    case 2: // Add
      if (seditCanModify(this, tMonster) != 1) {
        sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
        return;
      }

      stSpaceOut(tStInit);
      seditReadUntil(tStInit, tStMobile, '}');
      seditCoreBreakdown(tStMobile, tCmd, tStString);
      seditCoreBreakdown(tStInit, tCmdB, tStBuffer);

      if (tCmd == MAX_CMD_LIST || tCmdB == MAX_CMD_LIST) {
        sendTo("Syntax: sedit add <mobile> <{trigger}> <response>");
        return;
      }

      seditCoreAdd(this, tMonster, tCmd, tStString, tCmdB, tStBuffer);

      return;
    case 3: // Delete
      if (seditCanModify(this, tMonster) != 1) {
        sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
        return;
      }

      if (!tMonster->resps || !tMonster->resps->respList) {
        sendTo("This monster has no triggers, how can you delete one of them?\n\r");
        return;
      }

      stSpaceOut(tStInit);
      seditReadUntil(tStInit, tStMobile, '}');
      seditCoreBreakdown(tStMobile, tCmd, tStString);
      seditCoreBreakdown(tStInit, tCmdB, tStBuffer);

      if (tCmd == MAX_CMD_LIST) {
        sendTo("Syntax: sedit delete <mobile> <{trigger}> <response>\n\r");
        return;
      }

      seditCoreDelete(this, tMonster, tCmd, tStString, tCmdB, tStBuffer);

      if (tMonster->resps && !tMonster->resps->respList) {
        delete tMonster->resps;
        tMonster->resps = NULL;
      }

      return;
    case 4: // Display
      if (!seditCanModify(this, tMonster)) {
        sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
        return;
      }

      if (!tMonster->resps)
        sendTo("No Response quest information on this mob yet.\n\r");
      else {
        tStInit = tStArg;
        stSpaceOut(tStInit);
        tStArg  = one_argument(tStInit, tStMobile);
        stSpaceOut(tStMobile);
        stSpaceOut(tStArg);
        strcpy(tString, tStMobile.c_str());

        if (!tStArg.empty() && is_number(tString)) {
          if ((tShift = convertTo<int>(tString)) <= 0)
            tShift = -1;

          tStInit = tStArg;
          stSpaceOut(tStInit);
        }

        if ((respIndex = seditFindResponse(tMonster->resps->respList, tStInit, &tForm, tShift))) {
          do {
            seditDisplayResponse(this, respIndex, tForm, ++tCount);
            tForm = false;
          } while (tShift == -1 &&
                   (respIndex = seditFindResponse(respIndex->next, tStInit, &tForm, tShift)));
        } else
          sendTo("No Matches.\n\r");
      }
      return;
    case 5: // Save
      seditSave(this, tMonster);
      return;
    case 6: // Load
      seditLoad(this, tMonster, tStArg);
      return;
    case 7: // Clear
      seditClear(this, tMonster);
      return;
    case 8: // List
      seditList(this);
      return;
    case 9: // Purge
      sendTo("This will wipe *ALL* scripts from your directory, you must type out the entire purge!\n\r");
      return;
    case 10: // Purge!
      seditPurge(this);
      return;
    default:
      sendTo("Unknown extension.  Please try again.\n\r");
      return;
  }
}

void seditCoreBreakdown(sstring tStString, cmdTypeT &tCmd, sstring &tStArgs)
{
  sstring tStBuffer(""),
         tStCommand("");

  stSpaceOut(tStString);
  tStArgs = one_argument(tStString, tStCommand);
  stSpaceOut(tStCommand);
  stSpaceOut(tStArgs);

  if (tStArgs.empty()) {
    tStCommand.replace(tStString.find("{"), 1, "");
    tStCommand.replace(tStString.find("}"), 1, "");
  } else {
    tStCommand.replace(tStString.find("{"), 1, "");
    tStArgs.replace(tStBuffer.find("}"), 1, "");
  }

  tCmd = seditCmdFromText(tStString, true);
}

void seditCoreAdd(TBeing *ch, TMonster *tMonster,
                  cmdTypeT blockCmd, sstring tStBlock,
                  cmdTypeT tCmd, sstring tStCommand)
{
  bool     tForm = true;
  resp    *respIndex;
  char     tString[256];
  command *tRespCmd;

  if (!tMonster->resps)
    tMonster->resps = new Responses();

  if (!(respIndex = seditFindResponse(tMonster->resps->respList, tStBlock, &tForm, -1))) {
    for (respIndex = tMonster->resps->respList;
         respIndex && respIndex->next;
         respIndex = respIndex->next);

    strcpy(tString, tStBlock.c_str());

    if (!respIndex)
      respIndex = (tMonster->resps->respList = new resp(blockCmd, tString));
    else
      respIndex = (respIndex->next = new resp(blockCmd, tString));
  }

  for (tRespCmd = respIndex->cmds;
       tRespCmd && tRespCmd->next;
       tRespCmd = tRespCmd->next);

  strcpy(tString, tStCommand.c_str());

  if (tRespCmd)
    tRespCmd = (tRespCmd->next = new command(tCmd, tString));
  else
    tRespCmd = (respIndex->cmds = new command(tCmd, tString));

  ch->sendTo(format("\t%s%s%s;\n\rAdded to Trigger {%s%s%s}\n\r") %

             seditExtraWords(tRespCmd->cmd) %
             (tRespCmd->args ? " " : "") %
             (tRespCmd->args ? tRespCmd->args : "") %
             seditExtraWords(respIndex->cmd) %
             (respIndex->args ? " " : "") %
             (respIndex->args ? respIndex->args : ""));
}

void seditCoreDelete(TBeing *ch, TMonster *tMonster,
                     cmdTypeT blockCmd, sstring tStBlock,
                     cmdTypeT tCmd, sstring tStCommand)
{
  bool     tForm = true;
  resp    *respIndex;
  command *tRespCmd;

  if (!(respIndex = seditFindResponse(tMonster->resps->respList, tStBlock, &tForm, -1))) {
    ch->sendTo("That block was not found.  Sorry.\n\r");
    return;
  }

  if (tStCommand.empty() && tCmd == MAX_CMD_LIST) {
    resp *respIndexB;

    for (respIndexB = tMonster->resps->respList;
         respIndexB && respIndexB->next != respIndex;
         respIndexB = respIndexB->next);

    if (!respIndexB) {
      ch->sendTo("Something went Seriously wrong in SEdit.  Tell a coder.\n\r");
      return;
    }

    respIndexB->next = respIndex->next;
    ch->sendTo(format("Block {%s} deleted.\n\r") %               seditExtraWords(blockCmd) % tStCommand);
    delete respIndex;
    respIndex = NULL;
    return;
  }

  for (tRespCmd = respIndex->cmds;
       tRespCmd && (tRespCmd->cmd != tCmd ||
                    strcmp(tRespCmd->args, tStCommand.c_str()) != 0);
       tRespCmd = tRespCmd->next);

  if (!tRespCmd) {
    ch->sendTo("That was not found in that block.  Sorry.");
    return;
  }

  command *tRespCmdB;

  for (tRespCmdB = respIndex->cmds;
       tRespCmdB && tRespCmdB->next != tRespCmd;
       tRespCmdB = tRespCmdB->next);

  if (!tRespCmdB) {
    ch->sendTo("Something seriously wrong in SEdit.  Tell a coder.\n\r");
    return;
  }

  ch->sendTo(format("\t%s%s%s;\n\rDeleted from Trigger {%s%s%s}\n\r") %

             seditExtraWords(tRespCmd->cmd) %
             (tRespCmd->args ? " " : "") %
             (tRespCmd->args ? tRespCmd->args : "") %
             seditExtraWords(respIndex->cmd) %
             (respIndex->args ? " " : "") %
             (respIndex->args ? respIndex->args : ""));

  tRespCmdB->next = tRespCmd->next;
  delete tRespCmd;
  tRespCmd = NULL;
}

void seditReadUntil(sstring & tStOrig, sstring & tStStore, char tChar)
{
  char tString[256];
  int  tMarker;

  for (tMarker = 0; tStOrig[tMarker] &&
                    tStOrig[tMarker] != tChar; tMarker++)
    tString[tMarker] = tStOrig[tMarker];

  tString[tMarker] = tStOrig[tMarker];
  tMarker++;
  tString[tMarker] = '\0';

  tStOrig.replace(tStOrig.find(tString), strlen(tString), "");

  tStStore = tString;
}

// Makes sure the tree exists
FILE * seditVerifyDirTree(TBeing *ch, char *tArg,
                          bool tWrite, bool isSilent)
{
  FILE *tFile;
  sstring tPath;

  tPath = format("immortals/%s/mobs") % ch->getNameNOC(ch);

  if (!(tFile = fopen(tPath.c_str(), "r"))) {
    if (mkdir(tPath.c_str(), 0770)) {
      if (!isSilent)
        ch->sendTo("Unable to open/create a mobile directory for you.\n\r");

      return (FILE *)NULL;
    } else if (!isSilent)
      ch->sendTo("Mobile directory created.\n\r");
  }

  if (tFile)
    fclose(tFile);

  tPath+="/scripts";

  if (!(tFile = fopen(tPath.c_str(), "r"))) {
    if (mkdir(tPath.c_str(), 0770)) {
      if (!isSilent)
        ch->sendTo("Unable to open/create a script directory for you.\n\r");

      return (FILE *)NULL;
    } else if (!isSilent)
      ch->sendTo("Scripts directory created.\n\r");
  }

  if (tFile)
    fclose(tFile);

  if (tArg && *tArg) {
    tPath+="/";
    tPath+=tArg;

    if ((tFile = fopen(tPath.c_str(), (tWrite ? "w" : "r"))))
      return tFile;
  }

  return (FILE *)NULL;
}

void seditSave(TBeing *ch, TMonster *tMonster, bool isSilent)
{
  FILE *tFile;
  char  tValue[256];

  if (seditCanModify(ch, tMonster) != 1) {
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    return;
  }

  sprintf(tValue, "%d", tMonster->getSnum());
  tFile = seditVerifyDirTree(ch, tValue, true, isSilent);

  if (!tMonster->resps) {
    ch->sendTo("Monster lacks triggers, thus nothing to save.\n\r");

    if (tFile)
      fclose(tFile);

    return;
  }

  if (!tFile) {
    if (!isSilent)
      ch->sendTo("Could not open file for saving, try again later.\n\r");
  } else {
    time_t tTime = time(0);

    sprintf(tValue, "# %s\n", tMonster->getNameNOC(ch).cap().c_str());
    fputs(tValue, tFile);
    fputs("# Generic Quest\n", tFile);
    sprintf(tValue, "# Created by %s\n", ch->getNameNOC(ch).cap().c_str());
    fputs(tValue, tFile);

    sprintf(tValue, "# Copyright %d, SneezyMUD Dev Team.  All rights reserved.\n\n",
            localtime(&tTime)->tm_year + 1900);
    fputs(tValue, tFile);

    for (resp *respIndex = tMonster->resps->respList; respIndex; respIndex = respIndex->next)
      if (respIndex->cmd < MAX_CMD_LIST || respIndex->cmd == CMD_RESP_ROOM_ENTER) {
        sprintf(tValue, "%s { \"%s\";\n",
                (respIndex->cmd == CMD_RESP_ROOM_ENTER ? "roomenter"
                 : commandArray[respIndex->cmd]->name),
                (respIndex->args ? respIndex->args : ""));
        fputs(tValue, tFile);

        for (command *tCmd = respIndex->cmds; tCmd; tCmd = tCmd->next) {
          sprintf(tValue, "\t%s %s;\n", seditExtraWords(tCmd->cmd).c_str(), tCmd->args);
          fputs(tValue, tFile);
        }

        fputs("\t}\n\n", tFile);
      }

    fclose(tFile);
  }
}

void seditLoad(TBeing *ch, TMonster *tMonster, sstring tStArg, bool isSilent)
{
  FILE *tFile;
  char  tValue[256];
  //  resp *respIndex = NULL;

  if (seditCanModify(ch, tMonster) != 1) {
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    return;
  }

  stSpaceOut(tStArg);

  if (!tStArg.empty() && is_abbrev(tStArg, "core")) {
    sprintf(tValue, "mobdata/responses/%d", tMonster->getSnum());
    tFile = fopen(tValue, "r");
  } else {
    sprintf(tValue, "%d", tMonster->getSnum());
    tFile = seditVerifyDirTree(ch, tValue, false, isSilent);
  }

  if (!tFile) {
    if (!isSilent)
      ch->sendTo("Could not open file for loading, try again later.\n\r");
  } else {
    if (tMonster->resps) {
      delete tMonster->resps;
      tMonster->resps = NULL;
    }

    tMonster->resps = new Responses();

#if 0
    while ((respIndex = tMonster->readCommand(tFile))) {
      if (respIndex->cmd == CMD_RESP_ROOM_ENTER &&
          respIndex->args && *respIndex->args == '\"')
        *respIndex->args = '\0';

      respIndex->next = tMonster->resps->respList;
      tMonster->resps->respList = respIndex;
    }
#endif

    fclose(tFile);
  }
}

cmdTypeT seditCmdFromText(sstring tStString, bool checkMini)
{
  if (is_abbrev(tStString, "roomenter"))
    return CMD_RESP_ROOM_ENTER;

  if (checkMini)
    return searchForCommandNum(tStString);

  if (is_abbrev(tStString, "toggle"))
    return CMD_RESP_TOGGLE;

  if (is_abbrev(tStString, "untoggle"))
    return CMD_RESP_UNTOGGLE;

  if (is_abbrev(tStString, "checktoggle"))
    return CMD_RESP_CHECKTOG;

  if (is_abbrev(tStString, "personalize"))
    return CMD_RESP_PERSONALIZE;

  if (is_abbrev(tStString, "unflag"))
    return CMD_RESP_UNFLAG;

  if (is_abbrev(tStString, "toroom"))
    return CMD_RESP_TOROOM;

  if (is_abbrev(tStString, "tovict"))
    return CMD_RESP_TOVICT;

  if (is_abbrev(tStString, "tonotvict"))
    return CMD_RESP_TONOTVICT;

  if (is_abbrev(tStString, "checkuntoggle"))
    return CMD_RESP_CHECKUNTOG;

  if (is_abbrev(tStString, "checkmax"))
    return CMD_RESP_CHECKMAX;

  if (is_abbrev(tStString, "link"))
    return CMD_RESP_LINK;

  if (is_abbrev(tStString, "special"))
    return CMD_RESP_CODE_SEGMENT;

  if (is_abbrev(tStString, "resize"))
    return CMD_RESP_RESIZE;

  if (is_abbrev(tStString, "checkload"))
    return CMD_RESP_CHECKLOAD;

  if (is_abbrev(tStString, "loadmob") && tStString.length() > 4)
    return CMD_RESP_LOADMOB;

  if (is_abbrev(tStString, "checkclass"))
    return CMD_RESP_CHECKCLASS;

  if (is_abbrev(tStString, "checknotclass"))
    return CMD_RESP_CHECKNOTCLASS;

  return searchForCommandNum(tStString);
}

sstring seditExtraWords(cmdTypeT tCmd)
{
  if (tCmd < MAX_CMD_LIST)
    return commandArray[tCmd]->name;
  else
    switch (tCmd) {
      case CMD_RESP_TOGGLE:
        return "toggle";
        break;
      case CMD_RESP_UNTOGGLE:
        return "untoggle";
        break;
      case CMD_RESP_CHECKTOG:
        return "checktoggle";
        break;
      case CMD_RESP_PERSONALIZE:
        return "personalize";
        break;
      case CMD_RESP_ROOM_ENTER:
        return "roomenter";
        break;
      case CMD_RESP_UNFLAG:
        return "unflag";
        break;
      case CMD_RESP_TOROOM:
        return "toroom";
        break;
      case CMD_RESP_TOVICT:
        return "tovict";
        break;
      case CMD_RESP_TONOTVICT:
        return "tonotvict";
        break;
      case CMD_RESP_CHECKUNTOG:
        return "checkuntoggle";
        break;
      case CMD_RESP_CHECKMAX:
        return "checkmax";
        break;
      case CMD_RESP_LINK:
        return "link";
        break;
      case CMD_RESP_CODE_SEGMENT:
        return "special";
        break;
      case CMD_RESP_RESIZE:
        return "resize";
        break;
      case CMD_RESP_CHECKLOAD:
        return "checkload";
        break;
      case CMD_RESP_LOADMOB:
        return "loadmob";
        break;
      case CMD_RESP_CHECKCLASS:
        return "checkclass";
        break;
      case CMD_RESP_CHECKNOTCLASS:
        return "checknotclass";
        break;
      default:
        return "Unknown";
        break;
    }

  return "Nasty Error.";
}

// if tForm ==
//   true  : {name} {conditions}
//   false : {name} {conditions} {contents}
void seditDisplayResponse(TBeing *ch, resp *respIndex,
                          bool tForm, int tValue,
                          bool isSilent, sstring *tStString)
{
  char tString[256] = "",
       tBuffer[256];

  // This is bad.  Should never happen.
  // But i'm paranoid.
  if (isSilent && !tStString)
    return;

  if (tValue) {
    sprintf(tBuffer, "[%2d] ", tValue);

    if (isSilent)
      *tStString += tBuffer;
    else
      ch->sendTo(format(tBuffer) % tValue);
  }

  if (respIndex->cmd == CMD_GIVE && ch->hasWizPower(POWER_SEDIT_IMP_POWER) &&
      respIndex->args && is_number(respIndex->args)) {
    int tObjNum = real_object(convertTo<int>(respIndex->args));

    if (tObjNum < 0 || tObjNum > (signed int) obj_index.size())
      strcpy(tString, " [Unknown]");
    else
      sprintf(tString, " [%s]", obj_index[tObjNum].name);
  }

  if (respIndex->cmd < MAX_CMD_LIST) {
    sprintf(tBuffer, "{%s} = {%s}%s\n\r",
            commandArray[respIndex->cmd]->name, respIndex->args, tString);

    if (isSilent)
      *tStString += tBuffer;
    else
      ch->sendTo(COLOR_COMM, tBuffer);
  } else if (respIndex->cmd == CMD_RESP_ROOM_ENTER) {
    sprintf(tBuffer, "{roomenter} {%s}\n\r", respIndex->args);

    if (isSilent)
      *tStString += tBuffer;
    else
      ch->sendTo(COLOR_COMM, tBuffer);
  } else {
    sprintf(tBuffer, "{%d} {%s}\n\r", respIndex->cmd, respIndex->args);

    if (isSilent)
      *tStString += tBuffer;
    else
      ch->sendTo(COLOR_COMM, tBuffer);
  }

  if (tForm) {
    command *tCmd;

    if (isSilent)
      *tStString += "{\n\r";
    else
      ch->sendTo("{\n\r");

    if (!respIndex->cmds) {
      if (isSilent)
        *tStString += "\tEmpty\n\r";
      else
        ch->sendTo("\tEmpty\n\r");
    } else for (tCmd = respIndex->cmds; tCmd; tCmd = tCmd->next) {
      tString[0] = '\0';

      if ((tCmd->cmd == CMD_LOAD ||
           tCmd->cmd == CMD_RESP_CHECKLOAD) &&
          is_number(tCmd->args) && ch->hasWizPower(POWER_SEDIT_IMP_POWER)) {
        int tObjNum = real_object(convertTo<int>(tCmd->args));

        if (tObjNum < 0 || tObjNum > (signed int) obj_index.size())
          strcpy(tString, " [Unknown]");
        else
          sprintf(tString, " [%s]", obj_index[tObjNum].name);
      }

      sprintf(tBuffer, "\t%s %s;%s\n\r",
              seditExtraWords(tCmd->cmd).c_str(), tCmd->args, tString);

      if (isSilent)
        *tStString += tBuffer;
      else {
        sstring tStTemporary(tBuffer);
        sstring::size_type tSize;

        // Turn all those %n into %%n so they become %n in the lower sendTo.
        if ((tSize = tStTemporary.find("%")) != sstring::npos)
          do
            tStTemporary.replace(tSize, 1, "%%");
          while ((tSize = tStTemporary.find("%", tSize += 2)) != sstring::npos);

        ch->sendTo(COLOR_COMM, tStTemporary);
      }
    }

    if (isSilent)
      *tStString += "\t}\n\r";
    else
      ch->sendTo("\t}\n\r");
  }
}

void sedit(TBeing *ch, TMonster *tMonster)
{
  if (!ch->isPc() || !ch->desc)
    return;

  ch->specials.edit   = MAIN_MENU;
  ch->desc->connected = CON_SEDITING;
  act("$n is sucked into a swirling votex.", FALSE, tMonster, NULL, NULL, TO_ROOM);

  tMonster->swapToStrung();

  if (dynamic_cast<TPerson *>(tMonster) && !tMonster->desc) {
    for (Descriptor *descIndex = descriptor_list; descIndex; descIndex = descIndex->next)
      if (dynamic_cast<TMonster *>(descIndex->original) == tMonster)
        descIndex->character->doReturn("", WEAR_NOWHERE, 0);
  }

  if (tMonster->in_room == Room::NOWHERE)
    *(real_roomp(Room::VOID)) += *tMonster;

  while (tMonster->rider)
    tMonster->rider->dismount(POSITION_STANDING);

  if (tMonster->followers || tMonster->master)
    tMonster->dieFollower();

  if (tMonster->fight())
    tMonster->stopFighting();

  for (TBeing *combatIndex = gCombatList, *nextCombat = NULL;
       combatIndex; combatIndex = nextCombat) {
    nextCombat = combatIndex->next_fighting;

    if (combatIndex->fight() == tMonster)
      combatIndex->stopFighting();
  }

  --(*tMonster);

  if (tMonster == character_list)
    character_list = tMonster->next;
  else {
    TBeing *beingIndex = NULL;

    for (beingIndex = character_list;
         beingIndex && beingIndex->next != tMonster;
         beingIndex = beingIndex->next);

    if (beingIndex)
      beingIndex->next = tMonster->next;
    else {
      vlogf(LOG_EDIT, "Trying to remove ?? from character_list.");
      abort();
    }
  }

  if (tMonster->desc) {
    if (tMonster->desc->original)
      tMonster->doReturn("", WEAR_NOWHERE, 0);

    tMonster->saveChar(Room::NOWHERE);
  }

  ch->desc->mob = tMonster;
  act("$n just went into script edit mode.", FALSE, ch, NULL, NULL, TO_ROOM);
  update_sedit_menu(ch, ch->desc->mob);
}

long int seditCountTriggers(TMonster *tMonster, int tCmd = -1)
{
  long int tCount = 0;

  for (resp *respIndex = tMonster->resps->respList; respIndex; respIndex = respIndex->next)
    if (tCmd == -1 || (tCmd != -2 && tCmd == respIndex->cmd) ||
        (tCmd == -2 &&
         respIndex->cmd != CMD_SAY &&
         respIndex->cmd != CMD_GIVE &&
         respIndex->cmd != CMD_RESP_ROOM_ENTER))
      tCount++;

  return tCount;
}

void update_sedit_menu(TBeing *ch, TMonster *tMonster, bool useBar)
{
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(format("%sMobile Name:%s %s") %             ch->cyan() % ch->norm() % tMonster->name);
  ch->sendTo(format(VT_CURSPOS) % 2 % 1);
  ch->sendTo(format("%sTotal Triggers:%s %d") %              ch->cyan() % ch->norm() % seditCountTriggers(tMonster));
  ch->sendTo(format(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Editing Menu:\n\r");
  send_sedit_menu(ch);

  if (useBar)
    ch->sendTo("\n\r\n\r---------------------------------------------------\n\r");
  else
    ch->sendTo("\n\r\n\r--> ");
}

void seditClear(TBeing *ch, TMonster *tMonster)
{
  char  tString[256];
  FILE *tFile;

  if (seditCanModify(ch, tMonster) != 1) {
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    return;
  }

  sprintf(tString, "%d", tMonster->getSnum());

  if (tMonster->resps) {
    delete tMonster->resps;
    tMonster->resps = NULL;
    ch->sendTo("Triggers on monster removed.\n\r");
  }

  if (!(tFile = seditVerifyDirTree(ch, tString)))
    ch->sendTo("One doesn't exist for this mob, thus it cannot be removed.\n\r");
  else {
    fclose(tFile);
    sprintf(tString, "immortals/%s/mobs/scripts/%d",
            ch->getNameNOC(ch).c_str(), tMonster->getSnum());
    unlink(tString);
    ch->sendTo("Response Quest Script file removed.\n\r");
  }
}

void seditList(TBeing *ch)
{
  FILE           *tFile;
  struct dirent  *tDir;
  DIR            *tDirInfo;
  char            tString[256],
                  tBuffer[256];
  sstring          tStString(""),
                  tStBuffer("");
  unsigned int    tCount = 0;
  std::vector<sstring>  sortStr(0);

  if ((tFile = seditVerifyDirTree(ch, NULL)))
    fclose(tFile);

  if (!safe_to_be_in_system(ch->getName()))
    return;

  sprintf(tString, "immortals/%s/mobs/scripts", ch->getNameNOC(ch).c_str());

  if (!(tDirInfo = opendir(tString))) {
    vlogf(LOG_EDIT, format("Unable to dirwalk directory %s") %  tString);
    return;
  }

  while ((tDir = readdir(tDirInfo))) {
    if (!strcmp(tDir->d_name, ".") || !strcmp(tDir->d_name, ".."))
      continue;

    tStString += tDir->d_name;
    tStBuffer += tString;
    tStBuffer += "/";
    tStBuffer += tDir->d_name;

    if (!(tFile = fopen(tStBuffer.c_str(), "r")))
      tStString += " Unknown...\n\r";
    else {
      fgets(tBuffer, 256, tFile);
      tStString += " ";
      tStString += tBuffer;
      tStString += "\r";
      fclose(tFile);
    }

    sortStr.push_back(tStString);
  }

  sort(sortStr.begin(), sortStr.end(), dirlistSort());

  tStString = "";

  for (tCount = 0; tCount < sortStr.size(); tCount++)
    tStString += sortStr[tCount];

  if (tStString.empty())
    tStString += "Nothing found.\n\r";

  closedir(tDirInfo);
  ch->desc->page_string(tStString);
}

void seditPurge(TBeing *ch)
{
  FILE           *tFile;
  struct dirent  *tDir;
  DIR            *tDirInfo;
  char            tString[256];
  sstring          tStBuffer;

  if ((tFile = seditVerifyDirTree(ch, NULL)))
    fclose(tFile);

  sprintf(tString, "immortals/%s/mobs/scripts", ch->getNameNOC(ch).c_str());

  if (!(tDirInfo = opendir(tString))) {
    vlogf(LOG_EDIT, format("Unable to dirwalk directory %s") %  tString);
    return;
  }

  while ((tDir = readdir(tDirInfo))) {
    if (!strcmp(tDir->d_name, ".") || !strcmp(tDir->d_name, ".."))
      continue;

    tStBuffer += tString;
    tStBuffer += "/";
    tStBuffer += tDir->d_name;

    unlink(tStBuffer.c_str());
  }

  closedir(tDirInfo);

  ch->sendTo("Scripts Directory Purged of *ALL* Scripts.\n\r");
}

void seditCore(TBeing *ch, const char *tArg)
{
  if (!ch->desc) {
    ch->desc->connected = CON_PLYNG;
    return;
  }

  if (ch->desc->showstr_head) {
    ch->desc->show_string(tArg, SHOWNOW_YES, ALLOWREP_YES);

    if (!ch->desc->showstr_head &&
        ch->specials.edit == SEDIT_DISPLAY &&
        ch->specials.editFriend > 0)
      ch->sendTo("\n\r\n\r[Enter to Goto Main Menu/0 to see menu again]--> ");
    return;
  }

  switch (ch->specials.edit) {
    case MAIN_MENU:
      switch (*tArg) {
        case '1': // add
          ch->specials.edit = SEDIT_ADD;
          ch->specials.editFriend = 0;
          seditAddMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '2': // delete
          ch->specials.edit = SEDIT_DELETE;
          ch->specials.editFriend = 0;
          seditDeleteMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '3': // modify
          ch->specials.edit = SEDIT_MODIFY;
          ch->specials.editFriend = 0;
          seditModifyMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '4': // display
          ch->specials.edit        = SEDIT_DISPLAY;
          ch->specials.editFriend = 0;
          seditDisplayMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '5': // save
          seditSaveMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '6': // load
          seditLoadMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        case '7': // clear
          seditClearMenu(ch, ch->desc->mob, "", ENTER_CHECK);
          return;
        default:
          if (!tArg || !*tArg || *tArg == '\n') {
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from script editing.",
                TRUE, ch, NULL, NULL, TO_ROOM);

            if (ch->desc->mob) {
              ch->desc->mob->next = character_list;
              character_list = ch->desc->mob;
              *ch->roomp += *ch->desc->mob;
              ch->desc->mob = NULL;
            } else
              vlogf(LOG_EDIT, "seditCore returning to main game with no mobile in queue.");

            if (ch->vt100() || ch->ansi())
              ch->doCls(false);

            return;
          }

          update_sedit_menu(ch, ch->desc->mob);
          break;
      }
      break;
    case SEDIT_ADD: // add
      seditAddMenu(ch, ch->desc->mob, tArg, 0);
      break;
    case SEDIT_DELETE: // delete
      seditDeleteMenu(ch, ch->desc->mob, tArg, 0);
      break;
    case SEDIT_MODIFY: // modify
      seditModifyMenu(ch, ch->desc->mob, tArg, 0);
      break;
    case SEDIT_DISPLAY: // display
      seditDisplayMenu(ch, ch->desc->mob, tArg, 0);
      break;
    default:
      vlogf(LOG_EDIT, format("Got to bad place in seditCore() [%d]") %  ch->specials.edit);
      break;
  }
}

void seditAddMenu(TBeing *ch, TMonster *tMonster, const char *, int tType)
{
  if (seditCanModify(ch, tMonster) != 1) {
    ch->specials.edit = MAIN_MENU;
    update_sedit_menu(ch, tMonster, true);
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    ch->sendTo("\n\r--> ");
    return;
  }

  ch->specials.edit = MAIN_MENU;
  update_sedit_menu(ch, tMonster);
}

void seditDeleteMenu(TBeing *ch, TMonster *tMonster, const char *, int tType)
{
  if (seditCanModify(ch, tMonster) != 1) {
    ch->specials.edit = MAIN_MENU;
    update_sedit_menu(ch, tMonster, true);
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    ch->sendTo("\n\r--> ");
    return;
  }

  ch->specials.edit = MAIN_MENU;
  update_sedit_menu(ch, tMonster);
}

void seditModifyMenu(TBeing *ch, TMonster *tMonster, const char *, int tType)
{
  if (seditCanModify(ch, tMonster) != 1) {
    ch->specials.edit = MAIN_MENU;
    update_sedit_menu(ch, tMonster, true);
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    ch->sendTo("\n\r--> ");
    return;
  }

  ch->specials.edit = MAIN_MENU;
  update_sedit_menu(ch, tMonster);
}

const char * sedit_display_menu =
"%s1)%s [%2d] say\n\r"
"%s2)%s [%2d] give\n\r"
"%s3)%s [%2d] roomenter\n\r"
"%s4)%s [%2d] other\n\r";

resp * seditFindResponse(resp *tResp, sstring tStArg, bool *tForm, int tCount)
{
  exactTypeT tExact = exactTypeT(*tForm);
  sstring tStString(tStArg);

  if (tExact)
    tStString += '.';

  bool foundMatch = false;
  *tForm = false;

  for (resp *respIndex = tResp; respIndex; respIndex = respIndex->next) {
    foundMatch = false;

    // Find any
    if (tStArg.empty()) {
      foundMatch = true;
      *tForm = false;

    // Find command {command} {args} match
    } else if (tStArg[0] == '-') {
      if ((respIndex->cmd < MAX_CMD_LIST &&
           isname(&(tStString.c_str())[1], commandArray[respIndex->cmd]->name))) {
        foundMatch = true;
        *tForm = false;
      }

      if ((respIndex->cmd == CMD_RESP_ROOM_ENTER &&
           is_abbrev(&(tStArg.c_str())[1], "roomenter", MULTIPLE_NO, tExact))) {
        foundMatch = true;
        *tForm = true;
      }

    // Find special 'other'
    } else if ((tStArg.c_str())[0] == '+' && !strncmp(&(tStArg.c_str())[1], "other", 5)) {
      if (respIndex->cmd != CMD_SAY &&
          respIndex->cmd != CMD_GIVE &&
          respIndex->cmd != CMD_RESP_ROOM_ENTER) {
        foundMatch = true;
        *tForm = false;
      }

    // Find args {command} {args} match
    } else {
      if (isname(tStString, respIndex->args)) {
        foundMatch = true;
        *tForm = true;
      }
    }

    if (foundMatch)
      if (tCount == -1 || --tCount == 0)
        return respIndex;
  }

  return NULL;
}

void seditDisplayMenuFull(TBeing *ch, TMonster *tMonster, const char *tArg, int tType)
{
  resp   *respIndex = (tMonster->resps ? tMonster->resps->respList : NULL);
  bool    tForm = false;
  int     tCount = 0;
  sstring  tStString(""),
          tStOutput("");
  char    tString[256],
          tCommand[256];

  if (tType != ENTER_CHECK) {
    if (!tArg || !*tArg || *tArg == '\n') {
      ch->specials.edit = MAIN_MENU;
      update_sedit_menu(ch, tMonster);
      return;
    }

    tStString = tArg;
    stSpaceOut(tStString);
    strcpy(tString, tStString.c_str());

    if (is_number(tString) && tString[0] != '0') {
      if (ch->specials.editFriend == 0) {
        if (tString[0] == '1') {
          ch->specials.editFriend = 1;
          strcpy(tCommand, "-say");
        } else if (tString[0] == '2') {
          ch->specials.editFriend = 2;
          strcpy(tCommand, "-give");
        } else if (tString[0] == '3') {
          ch->specials.editFriend = 3;
          strcpy(tCommand, "-roomenter");
        } else {
          ch->specials.editFriend = 4;
          strcpy(tCommand, "+other");
        }

        if ((respIndex = seditFindResponse(respIndex, tCommand, &tForm, -1))) {
          ch->sendTo(VT_HOMECLR);

          do {
            seditDisplayResponse(ch, respIndex, false, ++tCount, true, &tStOutput);
            tForm = false;
          } while ((respIndex = seditFindResponse(respIndex->next, tCommand, &tForm, -1)));

          ch->desc->page_string(tStOutput);
        } else {
          ch->sendTo("No Matches.\n\r");
          ch->specials.editFriend = 0;
        }

        return;
      } else {
        tCount = convertTo<int>(tString);

        if (ch->specials.editFriend == 1)
          strcpy(tCommand, "-say");
        else if (ch->specials.editFriend == 2)
          strcpy(tCommand, "-give");
        else if (ch->specials.editFriend == 3)
          strcpy(tCommand, "-roomenter");
        else
          strcpy(tCommand, "+other");

        if ((respIndex = seditFindResponse(respIndex, tCommand, &tForm, tCount))) {
          ch->sendTo(VT_HOMECLR);
          seditDisplayResponse(ch, respIndex, true, 0);
        } else
          ch->sendTo("No Matches.\n\r");

        ch->sendTo("\n\r\n\r[Enter to Goto Main Menu/0 to see menu again]--> ");
      }

      return;
    }
  }

  ch->specials.editFriend = 0;
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Options: [More than 20 Triggers present]\n\r\n\r");
  ch->sendTo(format(sedit_display_menu) %
             ch->cyan() % ch->norm() % seditCountTriggers(tMonster, CMD_SAY) %
             ch->cyan() % ch->norm() % seditCountTriggers(tMonster, CMD_GIVE) %
             ch->cyan() % ch->norm() % seditCountTriggers(tMonster, CMD_RESP_ROOM_ENTER) %
             ch->cyan() % ch->norm() % seditCountTriggers(tMonster, -2));
  ch->sendTo("\n\r\n\r--> ");
}

void seditDisplayMenu(TBeing *ch, TMonster *tMonster, const char *tArg, int tType)
{
  resp   *respIndex = (tMonster->resps ? tMonster->resps->respList : NULL);
  bool    tForm = false;
  int     tCount = 0;
  sstring  tStString("");
  char    tString[256];

  if (!seditCanModify(ch, tMonster)) {
    ch->specials.edit = MAIN_MENU;
    update_sedit_menu(ch, tMonster, true);
    ch->sendTo("I'm sorry.  This mob is active so it is forbidden for you to do this.\n\r");
    ch->sendTo("\n\r--> ");
    return;
  }

  if (seditCountTriggers(tMonster) > 20) {
    seditDisplayMenuFull(ch, tMonster, tArg, tType);
    return;
  }

  if (tType != ENTER_CHECK) {
    if (!tArg || !*tArg || *tArg == '\n') {
      ch->specials.edit = MAIN_MENU;
      update_sedit_menu(ch, tMonster);
      return;
    }

    tStString = tArg;
    stSpaceOut(tStString);
    strcpy(tString, tStString.c_str());

    if (is_number(tString) && tString[0] != '0') {
      if ((respIndex = seditFindResponse(respIndex, "", &tForm, convertTo<int>(tString)))) {
        seditDisplayResponse(ch, respIndex, true, 0);
        ch->sendTo("\n\r\n\r[Enter to Goto Main Menu/0 to see menu again]--> ");
      }

      return;
    }
  }

  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Current Triggers:\n\r\n\r");

  for (; respIndex; respIndex = respIndex->next)
    seditDisplayResponse(ch, respIndex, false, ++tCount);

  ch->sendTo("\n\r--> ");
}

void seditSaveMenu(TBeing *ch, TMonster *tMonster, const char *, int)
{
  update_sedit_menu(ch, tMonster, true);
  seditSave(ch, tMonster);
  ch->sendTo("\n\r--> ");
}

void seditLoadMenu(TBeing *ch, TMonster *tMonster, const char *, int)
{
  update_sedit_menu(ch, tMonster, true);
  seditLoad(ch, tMonster, "");
  ch->sendTo("\n\r--> ");
}

void seditClearMenu(TBeing *ch, TMonster *tMonster, const char *, int)
{
  update_sedit_menu(ch, tMonster, true);
  seditClear(ch, tMonster);
  ch->sendTo("\n\r--> ");
}
