/////////////////////////////////////////
//
//  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.
//
//  "cmd_message.cc"
//  All functions and routines related to the various message modifiers.
//
/////////////////////////////////////////

#include <unistd.h>
#include "stdsneezy.h"

messageTypeT mapMessageFromFile(const char tString);
sstring mapMessageToFile(TMessages *tMsgStore, messageTypeT tType);

messageTypeT & operator++ (messageTypeT &c, int)
{
  return c = (c == MSG_TYPE_MAX) ? MSG_MIN : messageTypeT(c + 1);
}

const char * messageCommandFormat =
"Syntax: message <field> <message>\n\r\
\tmessage <field> default   -  resets the type to the standard.\n\r\
\tmessage <field>           -  displays that field's current setting.\n\r\
\tmessage list              -  lists fields and their current settings.\n\r\
";

const unsigned short int messageCommandSwitches[][3] =
{
  // Formate: {Max-String-Length, MSG_Type Flags, WizPower Required}
  // if WizPower == 0 then no wizpower check is made.
  {  0, (0                             ), 0},
  { 14, (0                             ), POWER_WIZARD},
  {200, (MSG_REQ_GNAME                 ), POWER_PURGE},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_PURGE},
  {200, (0                             ), POWER_RLOAD},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_LOAD},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_LOAD},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_MEDIT},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_OEDIT},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_SWITCH},
  { 79, (MSG_REQ_DIR                   ), 0},
  { 79, (MSG_REQ_DIR                   ), 0},
  {200, (MSG_REQ_GNAME | MSG_REQ_ONAME ), POWER_WIZARD},
  {200, (MSG_REQ_GNAME                 ), POWER_WIZARD},
  {200, (MSG_REQ_GNAME | MSG_REQ_STRING), POWER_FORCE},
  {200, (0                             ), POWER_GOTO},
  {200, (0                             ), POWER_GOTO},
  {200, (0                             ), POWER_LONGDESC},
};

const char * messageCommandTypes[] =
{
  "title",         //  1
  "purge",         //  2
  "purge-target",  //  3
  "rload",         //  4
  "load-object",   //  5
  "load-mobile",   //  6
  "medit",         //  7
  "oedit",         //  8
  "switch-target", //  9
  "move-in",       // 10
  "move-out",      // 11
  "slay",          // 12
  "slay-target",   // 13
  "force",         // 14
  "bamfin",        // 15
  "bamfout",       // 16
  "longdescr",     // 17
  "\n"
};

void TBeing::doMessage(const char *tArg)
{
  sstring  tStString(tArg),
          tStCommand("");
  char    tString[256],
         *tMark = NULL;
  int     tValue = -1;

  tStString = one_argument(tStString, tStCommand);
  strcpy(tString, tStString.c_str());
  tMark = tString;

  if (isspace(*tMark))
    tMark++;

  tStString = tMark;

  if (tStCommand.empty())
    sendTo(messageCommandFormat);
  else if (tStCommand.word(0).lower() == "list") {
    for (tValue = MSG_MIN; tValue < MSG_TYPE_MAX; tValue++ ) {
      if (messageCommandSwitches[tValue][2] &&
          hasWizPower(wizPowerT(messageCommandSwitches[tValue][2])))
        sendTo(fmt("%-15s:  %s\n\r") % messageCommandTypes[tValue-1] %
            msgVariables(messageTypeT(tValue), 
              (TThing *)NULL, (const char *) NULL, false));
    }
    return;
  } else {
    bisect_arg(tStCommand.c_str(), &tValue, tString, messageCommandTypes);

    if (tValue < 1 || tValue >= MSG_TYPE_MAX)
      sendTo("Incorrect message type.\n\r");
    else if (messageCommandSwitches[tValue][2] &&
             !hasWizPower(wizPowerT(messageCommandSwitches[tValue][2])))
      sendTo("You do not have the power to change that, sorry.\n\r");
    else if (tStString.length() > 250)
      sendTo("All sstrings have a hard limit of 250 characters, please use less than you did.\n\r");
    else {
      if (tStString.empty()) {
        sendTo(COLOR_BASIC, fmt("Message Type: %s set to:\n\r%s\n\r") %
               messageCommandTypes[(tValue - 1)] %
               msgVariables(messageTypeT(tValue), (TThing *)NULL, (const char *)NULL, false));
        return;
      }

      if (is_abbrev(tStString, "default")) {
        msgVariables(messageTypeT(tValue), "");
        sendTo(fmt("Message Type: %s set to default.\n\r") %
               messageCommandTypes[(tValue - 1)]);
        msgVariables.savedown();
        return;
      }

      bool isNamed = (colorString(this, desc, getName(), NULL, COLOR_NONE, TRUE).find(getNameNOC(this)) != sstring::npos);

      if (colorString(this, desc, tStString, NULL, COLOR_NONE, TRUE).length() >
          messageCommandSwitches[tValue][0]) {
        sendTo(fmt("String length, for this field, is limited to %d characters in total.\n\r") %
               messageCommandSwitches[tValue][0]);
        return;
      }

      if (tValue == MSG_IMM_TITLE &&
          (tStString.find("~R") != sstring::npos)) {
        sendTo("You Can NOT use newlines in the god title, Bad Bad.\n\r");
        return;
      }

      // sstring has the extra \n\r from the input attached, so strip that off
      while (tStString.find("\n") != sstring::npos)
        tStString.replace(tStString.find("\n"), 1, "");
      while (tStString.find("\r") != sstring::npos)
        tStString.replace(tStString.find("\r"), 1, "");

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_GNAME) &&
          !isNamed && (tStString.find("<n>") == sstring::npos)) {
        sendTo(fmt("This type requires your name.  Either use %s or <n>\n\r") %
               getNameNOC(this));
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_ONAME) &&
          (tStString.find("<N>") == sstring::npos)) {
        sendTo("This type requires <N> in it.\n\r");
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_STRING) &&
          (tStString.find("<a>") == sstring::npos)) {
        sendTo("This type requires <a> in it.\n\r");
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_DIR) &&
          (tStString.find("<d>") == sstring::npos)) {
        sendTo("This type requires <d> in it.\n\r");
        return;
      }

      msgVariables(messageTypeT(tValue), tStString);

      sendTo(COLOR_BASIC, fmt("Message %s set to:\n\r%s\n\r") %
              messageCommandTypes[(tValue - 1)] %
              msgVariables(messageTypeT(tValue), (TThing *)NULL, (const char *)NULL, false));
      msgVariables.savedown();

      if (tStString.find("$") != sstring::npos)
        sendTo("You used $ in your sstring, this will be replaced with -, sorry.\n\r");
    }
  }
}

sstring TMessages::getImmortalTitles(TBeing *tChar)
{
  int tLevel = (tChar ? (tChar->GetMaxLevel() - 51) : -1);
  const char * levelMessages[] =
  {
    "Area Designer ",
    "--------------", // Heroine/Hero caught below.
    "     Saint    ",
    "--------------", // Demi-Goddess/Demi-God caught below.
    "--------------", // Goddess/God caught below.
    "    Eternal   ",
    " Junior Lord  ",
    " Senior Lord  ",
    " Grand Wizard ",
    " Implementor  "
  };

  if (tLevel < 0 || tLevel > 9)
    return "    Unknown   ";

  if (tLevel == 1 || tLevel == 3 || tLevel == 4)
    if (tChar->getSex() == SEX_FEMALE) {
      if (tLevel == 1)
        return "    Heroine   ";
      else if (tLevel == 3)
        return " Demi-Goddess ";
      else
        return "    Goddess   ";
    } else {
      if (tLevel == 1)
        return "     Hero     ";
      else if (tLevel == 3)
        return "   Demi-God   ";
      else
        return "      God     ";
    }

  return levelMessages[tLevel];
}

sstring TMessages::getDefaultMessage(messageTypeT tValue, TBeing *tChar)
{
  switch (tValue)
  {
    case MSG_IMM_TITLE: // Immortal Title
      return getImmortalTitles(tChar);
      break;
    case MSG_PURGE: // Purge
      return "<n> gestures... You are surrounded by thousands of tiny scrubbing bubbles!";
      break;
    case MSG_PURGE_TARG: // Purge Target
      return "<n> disintegrates <N>.";
      break;
    case MSG_RLOAD: // RLoad
      return "<n> reaches down and scrambles reality.";
      break;
    case MSG_LOAD_OBJ: // Load Obj
      return "<n> has created <N>!";
      break;
    case MSG_LOAD_MOB: // Load Mob
      return "<n> has summoned <N> from the ether!";
      break;
    case MSG_MEDIT: // MEdit
      return "<n> just summoned a creature from <s> saved mobiles.";
      break;
    case MSG_OEDIT: // OEdit
      return "<n> just summoned an object from <s> saved objects.";
      break;
    case MSG_SWITCH_TARG: // Switch Targ
      return "<n> calls up <N> then quickly melds with <M>";
      break;
    case MSG_MOVE_IN: // Move In
      return "<n> quickly struts <d>.";
      break;
    case MSG_MOVE_OUT: // Move Out
      return "<n> struts in from <d>.";
      break;
    case MSG_SLAY: // Slay
      return "<n> brutally slays <N>!";
      break;
    case MSG_SLAY_TARG: // Slay Target
      return "<n> chops you to pieces!";
      break;
    case MSG_FORCE: // Force
      return "<n> has forced you to '<a>'.";
      break;
    case MSG_BAMFIN: // goto <room>
      return "<n> appears with an explosion of rose-petals";
      break;
    case MSG_BAMFOUT: // goto <room> when leaving a room
      return "<n> disappears in a cloud of mushrooms.";
      break;
    case MSG_LONGDESCR: // Long Description
      return "<n> is here.";
      break;
    default:
      return "ERROR";
      break;
  }
}

bool TMessages::operator==(messageTypeT tValue)
{
  if (!tPlayer || (*this)[tValue].empty() ||
      (messageCommandSwitches[tValue][2] &&
      !tPlayer->hasWizPower(wizPowerT(messageCommandSwitches[tValue][2]))))
    return false;

  return true;
}

// TMessages & TMessages::operator()(messageTypeT tValue, sstring tStString)
void TMessages::operator()(messageTypeT tValue, sstring tStString)
{
  // look for "~R" and replace with newlines
  while (tStString.find("~R") != sstring::npos)
    tStString.replace(tStString.find("~R"), 2, "\n\r");
  
  switch (tValue)
  {
    case MSG_IMM_TITLE: // Immortal Title
      delete [] tMessages.msgImmTitle;
      tMessages.msgImmTitle = NULL;
      tMessages.msgImmTitle = new char [tStString.length() + 1];
      strcpy(tMessages.msgImmTitle, tStString.c_str());
      break;
    case MSG_PURGE: // purge
      delete [] tMessages.msgPurge;
      tMessages.msgPurge = NULL;
      tMessages.msgPurge = new char[tStString.length() + 1];
      strcpy(tMessages.msgPurge, tStString.c_str());
      break;
    case MSG_PURGE_TARG: // purge-target
      delete [] tMessages.msgPurgeTarg;
      tMessages.msgPurgeTarg = NULL;
      tMessages.msgPurgeTarg = new char[tStString.length() + 1];
      strcpy(tMessages.msgPurgeTarg, tStString.c_str());
      break;
    case MSG_RLOAD: // rload
      delete [] tMessages.msgRLoad;
      tMessages.msgRLoad = NULL;
      tMessages.msgRLoad = new char[tStString.length() + 1];
      strcpy(tMessages.msgRLoad, tStString.c_str());
      break;
    case MSG_LOAD_OBJ: // load-object
      delete [] tMessages.msgLoadObj;
      tMessages.msgLoadObj = NULL;
      tMessages.msgLoadObj = new char[tStString.length() + 1];
      strcpy(tMessages.msgLoadObj, tStString.c_str());
      break;
    case MSG_LOAD_MOB: // load-mobile
      delete [] tMessages.msgLoadMob;
      tMessages.msgLoadMob = NULL;
      tMessages.msgLoadMob = new char[tStString.length() + 1];
      strcpy(tMessages.msgLoadMob, tStString.c_str());
      break;
    case MSG_MEDIT: // medit
      delete [] tMessages.msgMEdit;
      tMessages.msgMEdit = NULL;
      tMessages.msgMEdit = new char[tStString.length() + 1];
      strcpy(tMessages.msgMEdit, tStString.c_str());
      break;
    case MSG_OEDIT: // oedit
      delete [] tMessages.msgOEdit;
      tMessages.msgOEdit = NULL;
      tMessages.msgOEdit = new char[tStString.length() + 1];
      strcpy(tMessages.msgOEdit, tStString.c_str());
      break;
    case MSG_SWITCH_TARG: // switch-target
      delete [] tMessages.msgSwitchTarg;
      tMessages.msgSwitchTarg = NULL;
      tMessages.msgSwitchTarg = new char[tStString.length() + 1];
      strcpy(tMessages.msgSwitchTarg, tStString.c_str());
      break;
    case MSG_MOVE_IN: // move in
      delete [] tMessages.msgMoveIn;
      tMessages.msgMoveIn = NULL;
      tMessages.msgMoveIn = new char[tStString.length() + 1];
      strcpy(tMessages.msgMoveIn, tStString.c_str());
      break;
    case MSG_MOVE_OUT: // move out
      delete [] tMessages.msgMoveOut;
      tMessages.msgMoveOut = NULL;
      tMessages.msgMoveOut = new char[tStString.length() + 1];
      strcpy(tMessages.msgMoveOut, tStString.c_str());
      break;
    case MSG_SLAY: // slay
      delete [] tMessages.msgSlay;
      tMessages.msgSlay = NULL;
      tMessages.msgSlay = new char[tStString.length() + 1];
      strcpy(tMessages.msgSlay, tStString.c_str());
      break;
    case MSG_SLAY_TARG: // slay target
      delete [] tMessages.msgSlayTarg;
      tMessages.msgSlayTarg = NULL;
      tMessages.msgSlayTarg = new char[tStString.length() + 1];
      strcpy(tMessages.msgSlayTarg, tStString.c_str());
      break;
    case MSG_FORCE: // force
      delete [] tMessages.msgForce;
      tMessages.msgForce = NULL;
      tMessages.msgForce = new char[tStString.length() + 1];
      strcpy(tMessages.msgForce, tStString.c_str());
      break;
    case MSG_BAMFIN: // bamfin
      delete [] tMessages.msgBamfin;
      tMessages.msgBamfin = NULL;
      tMessages.msgBamfin = new char[tStString.length() + 1];
      strcpy(tMessages.msgBamfin, tStString.c_str());
      break;
    case MSG_BAMFOUT: // bamfout
      delete [] tMessages.msgBamfout;
      tMessages.msgBamfout = NULL;
      tMessages.msgBamfout = new char[tStString.length() + 1];
      strcpy(tMessages.msgBamfout, tStString.c_str());
      break;
    case MSG_LONGDESCR: // Long Description
      delete [] tMessages.msgLongDescr;
      tMessages.msgLongDescr = NULL;
      tMessages.msgLongDescr = new char[tStString.length() + 1];
      strcpy(tMessages.msgLongDescr, tStString.c_str());
      break;
    case MSG_ERROR:
    case MSG_MAX:
      vlogf(LOG_BUG, fmt("TMessages::operator()(int, sstring) got invalid tValue.  [%d]") % 
            tValue);
  }
}

void findAndReplace(sstring & tStOrig, sstring tStArg, sstring tStNew)
{
  while ((tStOrig.find(tStArg.c_str()) != sstring::npos))
    tStOrig.replace(tStOrig.find(tStArg.c_str()), tStArg.length(), tStNew.c_str());
}

sstring TMessages::operator()(messageTypeT tValue,
                             TThing *tThing,
                             const char * tString,
                             bool sendFiltered)
{
  sstring  tMessage("");
  TBeing *tBeing = dynamic_cast<TBeing *>(tThing);
  int     tSexP = (!tPlayer ? 2 :
                   (tPlayer->getSex() == SEX_FEMALE ? 0 :
                    (tPlayer->getSex() == SEX_MALE ? 1 : 2)));
  int     tSexM = (!tBeing ? 2 :
                   (tBeing->getSex() == SEX_FEMALE ? 0 :
                    (tBeing->getSex() == SEX_MALE ? 1 : 2)));

  const char * sexTypes[][3] =
  {
    {"her", "him", "it"},
    {"she", "he" , "it"},
    {"her", "his", "its"}
  };

  tMessage = (*this)[tValue];

  if (tMessage.empty() || !tPlayer || !tPlayer->isImmortal())
    tMessage = getDefaultMessage(tValue, tPlayer);

  if (sendFiltered) {
    if (tString && *tString) {
      findAndReplace(tMessage, "<d>", tString);
      findAndReplace(tMessage, "<a>", tString);
    } else {
      findAndReplace(tMessage, "<d>", "somewhere");
      findAndReplace(tMessage, "<a>", "to do something");
    }

    findAndReplace(tMessage, "<m>", sexTypes[0][tSexP]);
    findAndReplace(tMessage, "<e>", sexTypes[1][tSexP]);
    findAndReplace(tMessage, "<s>", sexTypes[2][tSexP]);

    findAndReplace(tMessage, "<M>", sexTypes[0][tSexM]);
    findAndReplace(tMessage, "<E>", sexTypes[1][tSexM]);
    findAndReplace(tMessage, "<S>", sexTypes[2][tSexM]);

    findAndReplace(tMessage, "~R", "\n\r");

    // These are potential bombs, don't allow them.
    findAndReplace(tMessage, "$", "");
    // All color codes are accepted so we just blot out those that are not
    // color codes and are not used above.
    findAndReplace(tMessage, "<A>", "");
    findAndReplace(tMessage, "<h>", "");
    findAndReplace(tMessage, "<H>", "");

    if (tPlayer)
      findAndReplace(tMessage, "<n>", (tPlayer->getName() ? tPlayer->getName() : "ERROR"));
    else
      findAndReplace(tMessage, "<n>", "Someone");

    if (tThing)
      findAndReplace(tMessage, "<N>", (tThing->getName() ? tThing->getName() : "ERROR"));
    else
      findAndReplace(tMessage, "<N>", "Someone");
  } else {
    findAndReplace(tMessage, "<d>", "<-d>");
    findAndReplace(tMessage, "<a>", "<-a>");
  }

  return tMessage;
}

sstring TMessages::operator[](messageTypeT tValue) const
{
  switch (tValue)
  {
    case MSG_IMM_TITLE: // Immortal Title
      return tMessages.msgImmTitle;
      break;
    case MSG_PURGE: // purge
      return tMessages.msgPurge;
      break;
    case MSG_PURGE_TARG: // purge-target
      return tMessages.msgPurgeTarg;
      break;
    case MSG_RLOAD: // rload
      return tMessages.msgRLoad;
      break;
    case MSG_LOAD_OBJ: // load-object
      return tMessages.msgLoadObj;
      break;
    case MSG_LOAD_MOB: // load-mobile
      return tMessages.msgLoadMob;
      break;
    case MSG_MEDIT: // medit
      return tMessages.msgMEdit;
      break;
    case MSG_OEDIT: // oedit
      return tMessages.msgOEdit;
      break;
    case MSG_SWITCH_TARG: // switch-target
      return tMessages.msgSwitchTarg;
      break;
    case MSG_MOVE_IN: // move in
      return tMessages.msgMoveIn;
      break;
    case MSG_MOVE_OUT: // move out
      return tMessages.msgMoveOut;
      break;
    case MSG_SLAY: // slay
      return tMessages.msgSlay;
      break;
    case MSG_SLAY_TARG: // slay target
      return tMessages.msgSlayTarg;
      break;
    case MSG_FORCE: // force
      return tMessages.msgForce;
      break;
    case MSG_BAMFIN: // bamfin
      return tMessages.msgBamfin;
      break;
    case MSG_BAMFOUT: // bamfout
      return tMessages.msgBamfout;
      break;
    case MSG_LONGDESCR: // Long Description
      return tMessages.msgLongDescr;
      break;
    default:
      vlogf(LOG_BUG, fmt("TMessages::operator[](int) got invalid tValue.  [%d]") % 
            tValue);
  }

  return "ERROR";
}

sstring fread_tilTilde(FILE *tFile)
{
  sstring tStString("");
  char   tChar = '\0';

  if (tFile && !feof(tFile))
    while (1) {
      tChar = fgetc(tFile);

      if (tChar == '~')
        return tStString;

      tStString += tChar;

      if (tChar == '\n')
        tStString += '\r';

      if (feof(tFile)) {
        tStString += '\r';
        return tStString;
      }
    }

  return tStString;
}

void TMessages::initialize()
{
  if (!tPlayer || !tPlayer->name) {
    vlogf(LOG_BUG, "TMessages::initialize() called by Invalid player.");
    return;
  }

  char  tString[256],
        tBuffer[MAX_STRING_LENGTH],
        tChar;
  FILE *tFile = NULL;

  sprintf(tString, "player/%c/%s.strings",
          LOWER(tPlayer->name[0]),
          sstring(tPlayer->name).lower().c_str());

  // They don't have a sstrings file, so just return.
  // Common for mortals.
  if (!(tFile = fopen(tString, "r")))
    return;

  while (1) {
    tChar   = fgetc(tFile);
    strcpy(tBuffer, fread_tilTilde(tFile).c_str());

    if (tBuffer[0])
      (*this)(messageTypeT(mapMessageFromFile(tChar)), tBuffer);

    if (feof(tFile)) {
      fclose(tFile);
      return;
    }
  }

  fclose(tFile);
}

void TMessages::savedown()
{
  // Various safty checks to force a return.
  if (!tPlayer || !tPlayer->desc || !tPlayer->isPc() ||
      tPlayer->isLinkdead() ||
      tPlayer->desc->connected != CON_PLYNG)
    return;

  if (!tPlayer->name) {
    vlogf(LOG_BUG, "TMessages::savedown() called by Invalid player.");
    return;
  }

  FILE   *tFile = NULL;
  char    tString[256];
  sstring  tStString("");
  bool    didWrite = false;

  sprintf(tString, "player/%c/%s.strings",
          LOWER(tPlayer->name[0]),
          sstring(tPlayer->name).lower().c_str());

  if (!(tFile = fopen(tString, "w")))
    return;

  for (messageTypeT cMsg = MSG_MIN; cMsg < MSG_MAX; cMsg++) {
    tStString = mapMessageToFile(this, cMsg);

    if ((tStString.c_str())[1] != '~') {
      fputs(tStString.c_str(), tFile);
      didWrite = true;
    }
  }

  fclose(tFile);

  // if we didn't write even 1 sstring, just delete the file.
  if (!didWrite)
    unlink(tString);
}

TMessages::TMessages() :
  tPlayer(NULL)
{
  // Null out the messages, this is a quick way of doing it.
  memset(&tMessages, 0, sizeof(tMessages));

  // Set them to the default empty.
  for (messageTypeT cMsg = MSG_MIN; cMsg < MSG_MAX; cMsg++)
    (*this)(cMsg, "");
}

TMessages::TMessages(const TMessages &a) :
  tPlayer(a.tPlayer)
{
  for (messageTypeT cMsg = MSG_MIN; cMsg < MSG_MAX; cMsg++)
    (*this)(cMsg, a[cMsg]);
}

TMessages & TMessages::operator==(const TMessages &a)
{
  tPlayer = a.tPlayer;

  for (messageTypeT cMsg = MSG_MIN; cMsg < MSG_MAX; cMsg++)
    (*this)(cMsg, a[cMsg]);

  return (*this);
}

TMessages::~TMessages()
{
  tPlayer = NULL;
}

messageTypeT mapMessageFromFile(const char tString)
{
  int tType = tString;

  return messageTypeT(tType);
}

sstring mapMessageToFile(TMessages *tMsgStore, messageTypeT tType)
{
  char tString[256];

  sprintf(tString, "%c%s~", tType, (*tMsgStore)[tType].c_str());
  return tString;
}



