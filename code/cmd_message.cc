//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_message.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_message.cc"
  All functions and routines related to the various message modifiers.

  Created 6/ 1/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include <unistd.h>
#include "stdsneezy.h"

messageTypeT mapMessageFromFile(const char tString);
string mapMessageToFile(TMessages *tMsgStore, messageTypeT tType);

messageTypeT & operator++ (messageTypeT &c, int)
{
  return c = (c == MSG_TYPE_MAX) ? MSG_MIN : messageTypeT(c + 1);
}

const char * messageCommandFormat =
"Syntax: message <field> <message>  Where field is one of:
\r\t        title -  - Immortal Title, special [] text.
\r\t        purge -n - When you do a general purge.
\r\t purge-target -nN- When purge a specific being.
\r\t        rload -  - Message people get when you rload a room there in.
\r\t  load-object -nN- When you load an object.
\r\t  load-mobile -nN- When you load a mobile.
\r\t        medit -nN- When you medit load a mobile.
\r\t        oedit -nN- When you oedit load an object.
\r\tswitch-target -nN- When you use the 'switch load <mobile>' syntax.
\r\t      move-in -d - When you move in: north, southeast, up, down
\r\t         ***What people see when you Leave a room by walking.
\r\t     move-out -d - When you move out: the north, the southeast, above, below
\r\t         ***What people see when you Enter a room by walking.
\r\t         slay -nN- When you slay a creature. (what others see)
\r\t  slay-target -n - When you slay a creature. (what They see)
\r\t        force -na- What a person sees when you force them to do something.
\r\tmessage <field> <default>   -   will reset the type to the standard.
\r\tmessage <field>  -  will display that fields current setting.
\r\t---------------^^- Required Items:
\r\t<n> = Your name.  (can be substituted with such)
\r\t<N> = Object/Mobile/Player name.
\r\t<a> = Arguments associated with the command in question.
\r\t<d> = Direction of movement.
\r\t<m> = Your sex. (him/her/it)
\r\t<M> = Targets sex. (him/her/it)
\r\t<e> = Your sex. (he/she/it)
\r\t<E> = Targets sex. (he/she/it)
\r\t<s> = Your sex. (his/her/its)
\r\t<S> = Targets sex. (his/her/its)
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
  "\n"
};

void TBeing::doMessage(const char *tArg)
{
  string  tStString(tArg),
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
  else {
    bisect_arg(tStCommand.c_str(), &tValue, tString, messageCommandTypes);

    if (tValue < 1 || tValue > 14)
      sendTo("Incorrect message type.\n\r");
    else if (messageCommandSwitches[tValue][2] &&
             !hasWizPower(wizPowerT(messageCommandSwitches[tValue][2])))
      sendTo("You do not have the power to change that, sorry.\n\r");
    else if (tStString.length() > 250)
      sendTo("All strings have a hard limit of 250 characters, please use less than you did.\n\r");
    else {
      if (tStString.empty()) {
        sendTo(COLOR_BASIC, "Message Type: %s set to:\n\r%s\n\r",
               messageCommandTypes[(tValue - 1)],
               msgVariables(messageTypeT(tValue), (TThing *)NULL, (const char *)NULL, false).c_str());
        return;
      }

      if (is_abbrev(tStString.c_str(), "default")) {
        msgVariables(messageTypeT(tValue), "");
        sendTo("Message Type: %s set to default.\n\r",
               messageCommandTypes[(tValue - 1)]);
        msgVariables.savedown();
        return;
      }

      bool isNamed = (colorString(this, desc, getName(), NULL, COLOR_NONE, TRUE).find(getNameNOC(this)) != string::npos);

      if (colorString(this, desc, tStString.c_str(), NULL, COLOR_NONE, TRUE).length() >
          messageCommandSwitches[tValue][0]) {
        sendTo("String length, for this field, is limited to %d characters in total.\n\r",
               messageCommandSwitches[tValue][0]);
        return;
      }

      if (tValue == MSG_IMM_TITLE &&
          (tStString.find("~R") != string::npos)) {
        sendTo("You Can NOT use newlines in the god title, Bad Bad.\n\r");
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_GNAME) &&
          !isNamed && (tStString.find("<n>") == string::npos)) {
        sendTo("This type requires your name.  Either use %s or <n>\n\r",
               getNameNOC(this).c_str());
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_ONAME) &&
          (tStString.find("<N>") == string::npos)) {
        sendTo("This type requires <N> in it.\n\r");
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_STRING) &&
          (tStString.find("<a>") == string::npos)) {
        sendTo("This type requires <a> in it.\n\r");
        return;
      }

      if ((messageCommandSwitches[tValue][1] & MSG_REQ_DIR) &&
          (tStString.find("<d>") == string::npos)) {
        sendTo("This type requires <d> in it.\n\r");
        return;
      }

      msgVariables(messageTypeT(tValue), tStString);

      sendTo(COLOR_BASIC, "Message %s set to:\n\r%s\n\r",
              messageCommandTypes[(tValue - 1)],
              msgVariables(messageTypeT(tValue), (TThing *)NULL, (const char *)NULL, false).c_str());
      msgVariables.savedown();

      if (tStString.find("$") != string::npos)
        sendTo("You used $ in your string, this will be replaced with -, sorry.\n\r");
    }
  }
}

string TMessages::getImmortalTitles(TBeing *tChar)
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

string TMessages::getDefaultMessage(messageTypeT tValue, TBeing *tChar)
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
      return "<n> quickly jets <d>.";
      break;
    case MSG_MOVE_OUT: // Move Out
      return "<n> jets in from <d>.";
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

TMessages & TMessages::operator()(messageTypeT tValue, string tStString)
{
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
    default:
      vlogf(7, "TMessages::operator()(int, string) got invalid tValue.  [%d]",
            tValue);
  }
}

void findAndReplace(string & tStOrig, string tStArg, string tStNew)
{
  while ((tStOrig.find(tStArg.c_str()) != string::npos))
    tStOrig.replace(tStOrig.find(tStArg.c_str()), tStArg.length(), tStNew.c_str());
}

string TMessages::operator()(messageTypeT tValue,
                             TThing *tThing = NULL,
                             const char * tString = NULL,
                             bool sendFiltered = true)
{
  string  tMessage("");
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
    {"her", "him", "it"}
  };

  tMessage = (*this)[tValue];

  if (tMessage.empty() || !tPlayer || !tPlayer->isImmortal())
    tMessage = getDefaultMessage(tValue, tPlayer);

  if (sendFiltered) {
    if (tPlayer)
      findAndReplace(tMessage, "<n>", (tPlayer->getName() ? tPlayer->getName() : "ERROR"));
    else
      findAndReplace(tMessage, "<n>", "Someone");

    findAndReplace(tMessage, "<m>", sexTypes[0][tSexP]);
    findAndReplace(tMessage, "<e>", sexTypes[1][tSexP]);
    findAndReplace(tMessage, "<s>", sexTypes[2][tSexP]);

    if (tThing)
      findAndReplace(tMessage, "<N>", (tThing->getName() ? tThing->getName() : "ERROR"));
    else
      findAndReplace(tMessage, "<N>", "Someone");

    findAndReplace(tMessage, "<M>", sexTypes[0][tSexM]);
    findAndReplace(tMessage, "<E>", sexTypes[1][tSexM]);
    findAndReplace(tMessage, "<S>", sexTypes[2][tSexM]);

    if (tString && *tString) {
      findAndReplace(tMessage, "<d>", tString);
      findAndReplace(tMessage, "<a>", tString);
    } else {
      findAndReplace(tMessage, "<d>", "somewhere");
      findAndReplace(tMessage, "<a>", "to do something");
    }

    findAndReplace(tMessage, "~R", "\n\r");

    // These are potential bombs, don't allow them.
    findAndReplace(tMessage, "$", "");
    // All color codes are accepted so we just blot out those that are not
    // color codes and are not used above.
    findAndReplace(tMessage, "<A>", "");
    findAndReplace(tMessage, "<h>", "");
    findAndReplace(tMessage, "<H>", "");
  } else {
    findAndReplace(tMessage, "<d>", "<-d>");
    findAndReplace(tMessage, "<a>", "<-a>");
  }

  return tMessage;
}

string TMessages::operator[](messageTypeT tValue) const
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
    default:
      vlogf(7, "TMessages::operator[](int) got invalid tValue.  [%d]",
            tValue);
  }

  return "ERROR";
}

string fread_tilTilde(FILE *tFile)
{
  string tStString("");
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
    vlogf(7, "TMessages::initialize() called by Invalid player.");
    return;
  }

  char  tString[256],
        tBuffer[MAX_STRING_LENGTH],
        tChar;
  FILE *tFile = NULL;

  sprintf(tString, "player/%c/%s.strings",
          LOWER(tPlayer->name[0]),
          lower(tPlayer->name).c_str());

  // They don't have a strings file, so just return.
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
    vlogf(7, "TMessages::savedown() called by Invalid player.");
    return;
  }

  FILE   *tFile = NULL;
  char    tString[256];
  string  tStString("");
  bool    didWrite = false;

  sprintf(tString, "player/%c/%s.strings",
          LOWER(tPlayer->name[0]),
          lower(tPlayer->name).c_str());

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

  // if we didn't write even 1 string, just delete the file.
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

string mapMessageToFile(TMessages *tMsgStore, messageTypeT tType)
{
  char tString[256];

  sprintf(tString, "%c%s~", tType, (*tMsgStore)[tType].c_str());
  return tString;
}
