///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//      "parse.cc" - All functions and routines related to command parsing
//      
//
///////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <arpa/telnet.h>
}

#include "stdsneezy.h"
#include "mail.h"

sstring lockmess;
commandInfo *commandArray[MAX_CMD_LIST];
bool WizLock;

int search_block(const sstring &arg, const char * const *list, bool exact)
{
  register int i, l;

  l = arg.length();

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcasecmp(arg.c_str(), *(list + i)))
        return (i);
  } else {
    if (!l)
      l = 1;

    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncasecmp(arg.c_str(), *(list + i), l))
        return (i);
  }
  return (-1);
}


int old_search_block(const char *argument, int bgin, int length, const char * const * list, bool mode)
{
  int guess, search;
  bool found;

  // If the word contain 0 letters, then a match is already found 
  found = (length < 1);

  guess = 0;

  if (mode)
    while (!found && *(list[guess]) != '\n') {
      found = (length == (int) strlen(list[guess]));
      for (search = 0; (search < length && found); search++)
        found = (*(argument + bgin + search) == *(list[guess] + search));
      guess++;
  } else {
    while (!found && *(list[guess]) != '\n') {
      found = 1;
      for (search = 0; (search < length && found); search++)
        found = (*(argument + bgin + search) == *(list[guess] + search));
      guess++;
    }
  }
  return (found ? guess : -1);
}

cmdTypeT searchForCommandNum(const sstring &argument)
{
  cmdTypeT i;

  for (i = MIN_CMD; i < MAX_CMD_LIST; i++) {
    if (!commandArray[i])
      continue;
    if (is_abbrev(argument, commandArray[i]->name))
      return i;
  }

  return MAX_CMD_LIST;
}

void TBeing::incorrectCommand() const
{
  sendTo(fmt("%sIncorrect%s command. Please see help files if you need assistance!\n\r") % red() % norm());
}

bool willBreakHide(cmdTypeT tCmd, bool isPre)
{
  switch (tCmd) {
    case CMD_BACKSTAB:
      return (isPre ? false : true);
    case CMD_SLIT:
      return (isPre ? false : true);

    case CMD_LOOK:
    case CMD_SCORE:
    case CMD_TROPHY:
    case CMD_INVENTORY:
    case CMD_HELP:
    case CMD_WHO:
    case CMD_NEWS:
    case CMD_EQUIPMENT:
    case CMD_WEATHER:
    case CMD_SAVE:
    case CMD_EXITS:
    case CMD_TIME:
    case CMD_HIDE:
    case CMD_SNEAK:
    case CMD_QUEST:
    case CMD_LEVELS:
    case CMD_WIZLIST:
    case CMD_CONSIDER:
    case CMD_CREDITS:
    case CMD_TITLE:
    case CMD_ATTRIBUTE:
    case CMD_WORLD:
    case CMD_SPY:
    case CMD_CLS:
    case CMD_PROMPT:
    case CMD_ALIAS:
    case CMD_CLEAR:
    case CMD_MOTD:
    case CMD_PRACTICE:
    case CMD_HISTORY:
    case CMD_EVALUATE:
    case CMD_DISGUISE:
    case CMD_EMAIL:
    case CMD_AFK:
    case CMD_SPELLS:
    case CMD_COMPARE:
    case CMD_ZONES:
    case MAX_CMD_LIST:
      return false;

    default:
      return true;
  }

  return true;
}

extern int handleMobileResponse(TBeing *, cmdTypeT, const sstring &);

// returns DELETE_THIS if this should be nuked
// returns DELETE_VICT if vict should be nuked
// otherwise returns FALSE
int TBeing::doCommand(cmdTypeT cmd, const sstring &argument, TThing *vict, bool typedIn)
{
  int rc = 0;
  TBeing *ch;
  bool isPoly = FALSE;
  sstring newarg = "";
  sstring tStNewArg = "";
  sstring buf, bufname;
  size_t tVar = 0;
 
  // sendrpf(COLOR_NONE, roomp, "doCommand:argument=[%s]\n\r", argument.c_str());
  newarg=stripColorCodes(argument);
  // sendrpf(COLOR_NONE, roomp, "doCommand:newarg=[%s]\n\r", newarg.c_str());
  int i = 0;
  tStNewArg += newarg.word(i++);
  while (true) {
    sstring arg_word = newarg.word(i++);
    if (arg_word.empty()) {
      break;
    }

    tStNewArg += " ";
    tStNewArg += arg_word;
  }
  tStNewArg = stripColorCodes(tStNewArg);
  // sendrpf(COLOR_NONE, roomp, "doCommand:tStNewArg=[%s]\n\r", tStNewArg.c_str());

  // The pray code is extremely messed up so this is really
  // better put here until pray can get fixed.
  //
  // This way is much like the old way but it verifies that the
  // "self"/"me"/"tank" word has a leading space and is the Last
  // word in the line.  This way ->me<-rcees doesn't trigger.
  if (cmd == CMD_PRAY || cmd == CMD_RECITE) {
    if ((tVar = tStNewArg.find(" self")) != sstring::npos &&
        !(tStNewArg.size() - tVar - 5))
      tStNewArg.replace(tStNewArg.find("self"), 4, getName());

    else if ((tVar = tStNewArg.find(" me")) != sstring::npos &&
             !(tStNewArg.size() - tVar - 3))
      tStNewArg.replace(tStNewArg.find("me"), 2, getName());

    else if ((tVar = tStNewArg.find(" tank")) != sstring::npos && fight() &&
             !(tStNewArg.size() - tVar - 5) &&
             isAffected(AFF_GROUP) && fight()->fight()) {
      tStNewArg.replace(tStNewArg.find("tank"), 4, persfname(fight()->fight()).c_str());
    } else if ((tVar = tStNewArg.find(" tank")) != sstring::npos &&
               !(tStNewArg.size() - tVar - 5) &&
               cmd == CMD_PRAY && isAffected(AFF_GROUP)) {
      if ((master && master->followers) || followers) {
        followData * tF = (master ? master->followers : followers);

        for (; tF; tF = tF->next)
          if ((ch = tF->follower->specials.fighting) &&
              ch->specials.fighting == tF->follower) {
            tStNewArg.replace(tStNewArg.find("tank"), 4, persfname(tF->follower).c_str());
            break;
          }

        if (!tF)
          tStNewArg.replace(tStNewArg.find("tank"), 4, getName());
      }
    }

    newarg = tStNewArg;
  } else if (cmd == CMD_CAST || cmd == CMD_SAY || cmd == CMD_SAY2 ||
             cmd == CMD_SIGN || cmd == CMD_TELL || cmd == CMD_SHOUT ||
             cmd == CMD_EMOTE || cmd == CMD_EMOTE2 || cmd == CMD_EMOTE3 ||
             cmd == CMD_WHISPER || cmd == CMD_PTELL || cmd == CMD_PSAY ||
             cmd == CMD_PSHOUT || cmd == CMD_ECHO || cmd == CMD_SYSTEM ||
             cmd == CMD_GT || cmd == CMD_ASK || cmd == CMD_TITLE ||
             cmd == CMD_MESSAGE || cmd == CMD_WIZNET || cmd == CMD_GROUP) {
    newarg = argument;
  } else if (tStNewArg.lower() == "self" || tStNewArg.lower() == "me") {
    newarg = getNameNOC(this);
  } else {
    newarg = tStNewArg;
  }

  if (typedIn && desc && dynamic_cast<TMonster *>(this)) 
    isPoly = TRUE;
  
  if (GetMaxLevel() < commandArray[cmd]->minLevel &&
      ((cmd != CMD_WIZNET) || !desc || !desc->original ||
       desc->original->GetMaxLevel() < commandArray[cmd]->minLevel)) {
    incorrectCommand();
    return FALSE;
  }
  if (isAffected(AFF_PARALYSIS) &&
      commandArray[cmd]->minPosition > POSITION_STUNNED) {
    sendTo("You are paralyzed, you can't do much of anything!\n\r");
    return FALSE;
  }
  if (isAffected(AFF_STUNNED) &&
      commandArray[cmd]->minPosition > POSITION_STUNNED) {
    sendTo("You are stunned, you can't do much of anything!\n\r");
    return FALSE;
  }

  if (hasClass(CLASS_SHAMAN)) {
    if (isPc()) {
      if (-10 > getHit()) {
	vlogf(LOG_MISC, fmt("Half-tick force updated for %s (Shaman).") %  getName());
	sendTo("The loa are disappointed in your state of life.\n\r");
	updateHalfTickStuff();
	doSave(SILENT_YES);
      }
    }
  }

  // ADDED THIS TO MAKE SURE POSITIONS ARE UPDATED ON SHAMAN

  if ((commandArray[cmd]->minPosition >= POSITION_CRAWLING) && fight()){
    sendTo("You can't concentrate enough while fighting!\n\r");
    return FALSE;
  } else if (getPosition() < commandArray[cmd]->minPosition) {
    switch (getPosition()) {
      case POSITION_DEAD:
	sendTo("You cannot do that while dead!\n\r");
	break;
      case POSITION_INCAP:
      case POSITION_MORTALLYW:
	sendTo("You cannot do that while mortally wounded!\n\r");
	break;
      case POSITION_STUNNED:
	sendTo("You cannot do that while stunned!!\n\r");
	break;
      case POSITION_SLEEPING:
	sendTo("You cannot do that while sleeping!\n\r");
	break;
      case POSITION_RESTING:
	sendTo("You cannot do that while resting!\n\r");
	break;
      case POSITION_SITTING:
	sendTo("You cannot do that while sitting!?\n\r");
	break;
      case POSITION_CRAWLING:
	sendTo("You cannot do that while crawling!\n\r");
	break;
      case POSITION_ENGAGED:
	sendTo("You cannot do that while engaged!\n\r");
	break;
      case POSITION_FIGHTING:
	sendTo("You cannot do that while fighting!\n\r");
	break;
      case POSITION_STANDING:
	sendTo("You cannot do that while standing!\n\r");
	break;
      case POSITION_MOUNTED:
	sendTo("You cannot do that while mounted!\n\r");
	break;
      case POSITION_FLYING:
	sendTo("You cannot do that while flying!\n\r");
	break;
    }
  } else {
    ch = ((desc && desc->original) ? desc->original : this);
    if (should_be_logged(ch) && ch->isPc()){
      TPerson * tPerson = dynamic_cast<TPerson *>(ch);

      if (ch == this) {
	vlogf(LOG_SILENT, fmt("%s (%i):%s %s") %  name % in_room % commandArray[cmd]->name % newarg);

	if (tPerson)
	  tPerson->logf("%s:%s %s", name, commandArray[cmd]->name, newarg.c_str());
      } else {
	vlogf(LOG_SILENT, fmt("%s (%s) (%i):%s %s") %  name % desc->original->name % 
	      in_room % commandArray[cmd]->name % newarg);

	if (tPerson)
	  tPerson->logf("%s:%s %s", name,
			commandArray[cmd]->name, newarg.c_str());
      }
    } else if (ch->isPc() && ch->isPlayerAction(PLR_LOGGED))
      vlogf(LOG_SILENT, fmt("%s %s%s") %  name % commandArray[cmd]->name % newarg);
    else if (numberLogHosts && desc) {
      for (int a = 0; a < numberLogHosts; a++) {
        if (desc->host.lower() == sstring(hostLogList[a]).lower())
	  vlogf(LOG_SILENT, fmt("%s %s%s") %  name % commandArray[cmd]->name % newarg);
      }
    }

    if (IS_SET(specials.affectedBy, AFF_HIDE) && willBreakHide(cmd, true))
      REMOVE_BIT(specials.affectedBy, AFF_HIDE);

    rc = triggerSpecial(NULL, cmd, newarg.c_str());
    if (IS_SET_DELETE(rc, DELETE_THIS)) 
      return DELETE_THIS;
    else if (rc) 
      return FALSE;

    switch(cmd) {
      case CMD_UNSADDLE:
	doUnsaddle(newarg);
	break;
      case CMD_SPRINGLEAP:
	doSpringleap(newarg, true, dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SADDLE:
	doSaddle(newarg);
	break;
      case CMD_CONCEAL:
	doConceal(newarg);
	break;
      case CMD_RESTRING:
	doRestring(newarg);
	break;
      case CMD_RESET:
	doReset(newarg);
	addToLifeforce(1);
	break;
      case CMD_BOOT:
	doBoot(newarg);
	break;
      case CMD_RELEASE:
	if (!hasWizPower(POWER_WIZARD)) {
	  sendTo("Prototype command.  You need to be a developer to use this.\n\r");
	  break;
	}	  
	doRelease(newarg);
	break;
      case CMD_CRIT:
	rc = doCrit(newarg);
	break;
      case CMD_CLIENTS:
	doClients();
	addToLifeforce(1);
	break;
      case CMD_CAPTURE:
	if (!hasWizPower(POWER_WIZARD)) {
	  sendTo("Prototype command.  You need to be a developer to use this.\n\r");
	  break;
	}
	doCapture(newarg);
	addToLifeforce(1);
	break;
      case CMD_HEAVEN:
	doHeaven(newarg);
	break;
      case CMD_REFUEL:
	doRefuel(newarg);
	addToLifeforce(1);
	break;
      case CMD_REPLY:
	doReply(newarg);
	addToLifeforce(1);
	break;
      case CMD_USE:
	rc = doUse(newarg);
	addToLifeforce(1);
	break;
      case CMD_DRAG:
	doDrag(newarg);
	break;
      case CMD_MOVE:
	doRoll(newarg);
	break;
      case CMD_DISSECT:
	rc = doDissect(newarg);
	addToLifeforce(1);
	break;
      case CMD_DISARM:
	rc = doDisarm(newarg, vict);
	break;
      case CMD_EXEC:
	rc = doExec();
	break;
      case CMD_QUAFF:
	rc = doQuaff(newarg);
	addToLifeforce(1);
	break;
      case CMD_GUARD:
      case CMD_PROTECT:
	doGuard(newarg);
	addToLifeforce(1);
	break;
      case CMD_ORDER:
	rc = doOrder(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_HISTORY:
	doHistory();
	addToLifeforce(1);
	break;
      case CMD_PEEK:
	doPeek();
	addToLifeforce(1);
	break;
      case CMD_BUG:
	doBug(newarg);
	addToLifeforce(1);
	break;
      case CMD_IDEA:
	doIdea(newarg);
	addToLifeforce(1);
	break;
      case CMD_TYPO:
	doTypo(newarg);
	addToLifeforce(1);
	break;
      case CMD_NORTH:
      case CMD_SOUTH:
      case CMD_WEST:
      case CMD_EAST:
      case CMD_UP:
      case CMD_DOWN:
      case CMD_NE:
      case CMD_SW:
      case CMD_SE:
      case CMD_NW:
	rc = doMove(cmd);
	break;
      case CMD_TRACEROUTE:
	doSysTraceroute(newarg);
	break;
      case CMD_MID:
	doSysMid();
	break;
      case CMD_VIEWOUTPUT:
	doSysViewoutput();
	break;
      case CMD_TASKS:
	doSysTasks(newarg);
	break;
      case CMD_SAY:
      case CMD_SAY2:
	rc = doSay(newarg);
	addToLifeforce(1);
	break;
      case CMD_LOOK:
	doLook(newarg, cmd);
	addToLifeforce(1);
	break;
      case CMD_ADJUST:
	doAdjust(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_FACTIONS:
	doFactions(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_LIST:
	if ((rc = handleMobileResponse(this, cmd, newarg)))
	  break;

	doList(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_RENT:
	rc = doRent(newarg);
	addToLifeforce(1);
	break;
      case CMD_BUY:
	if ((rc = handleMobileResponse(this, cmd, newarg)))
	  addToLifeforce(1);
	break;
      case CMD_TWIST:
      case CMD_PRESS:
      case CMD_PUSH:
      case CMD_SELL:
      case CMD_VALUE:
      case CMD_OFFER:
      case CMD_BALANCE:
      case CMD_WITHDRAW:
      case CMD_DEPOSIT:
      case CMD_CHECK:
      case CMD_RECEIVE:
      case CMD_MAIL:
      case CMD_PULL:
      case CMD_POST:
      case CMD_BREAK:
      case CMD_GAIN:
      case CMD_CLIMB:
      case CMD_DESCEND:
      case CMD_CHIP:
      case CMD_DIG:
      case CMD_COVER:
      case CMD_OPERATE:
      case CMD_ABORT:
	doNotHere();
	addToLifeforce(1);
	break;
      case CMD_MEND_LIMB:
	rc = doMendLimb(newarg);
	break;
      case CMD_TITHE:
	rc = doTithe();
	break;
      case CMD_ACCOUNT:
	doAccount(newarg);
	addToLifeforce(1);
	break;
      case CMD_FILL:
	doFill(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_BOUNCE:
      case CMD_DANCE:
      case CMD_SMILE:
      case CMD_CACKLE:
      case CMD_LAUGH:
      case CMD_GIGGLE:
      case CMD_SHAKE:
      case CMD_PUKE:
      case CMD_GROWL:
      case CMD_SCREAM:
      case CMD_COMFORT:
      case CMD_NOD:
      case CMD_SIGH:
      case CMD_SULK:
      case CMD_HUG:
      case CMD_SNUGGLE:
      case CMD_CUDDLE:
      case CMD_NUZZLE:
      case CMD_CRY:
      case CMD_ACCUSE:
      case CMD_GRIN:
      case CMD_BOW:
      case CMD_APPLAUD:
      case CMD_BLUSH:
      case CMD_BURP:
      case CMD_CHUCKLE:
      case CMD_CLAP:
      case CMD_COUGH:
      case CMD_CURTSEY:
      case CMD_FART:
      case CMD_FLIP:
      case CMD_FONDLE:
      case CMD_FROWN:
      case CMD_GASP:
      case CMD_GLARE:
      case CMD_GROAN:
      case CMD_GROPE:
      case CMD_HICCUP:
      case CMD_LICK:
      case CMD_LOVE:
      case CMD_MOAN:
      case CMD_NIBBLE:
      case CMD_POUT:
      case CMD_PURR:
      case CMD_RUFFLE:
      case CMD_SHIVER:
      case CMD_SHRUG:
      case CMD_SING:
      case CMD_SLAP:
      case CMD_SMIRK:
      case CMD_SNAP:
      case CMD_SNEEZE:
      case CMD_SNICKER:
      case CMD_SNIFF:
      case CMD_SNORE:
      case CMD_SPIT:
      case CMD_SQUEEZE:
      case CMD_STARE:
      case CMD_STRUT:
      case CMD_THANK:
      case CMD_TWIDDLE:
      case CMD_WAVE:
      case CMD_WHISTLE:
      case CMD_WIGGLE:
      case CMD_WINK:
      case CMD_YAWN:
      case CMD_SNOWBALL:
      case CMD_FRENCH:
      case CMD_COMB:
      case CMD_MASSAGE:
      case CMD_TICKLE:
      case CMD_PAT:
      case CMD_CURSE:
      case CMD_BEG:
      case CMD_BLEED:
      case CMD_CRINGE:
      case CMD_DAYDREAM:
      case CMD_FUME:
      case CMD_GROVEL:
      case CMD_HOP:
      case CMD_NUDGE:
      case CMD_PEER:
      case CMD_PONDER:
      case CMD_PUNCH:
      case CMD_SNARL:
      case CMD_SPANK:
      case CMD_STEAM:
      case CMD_TACKLE:
      case CMD_TAUNT:
      case CMD_WHINE:
      case CMD_WORSHIP:
      case CMD_YODEL:
      case CMD_THINK:
      case CMD_WHAP:
      case CMD_BEAM:
      case CMD_CHORTLE:
      case CMD_BONK:
      case CMD_SCOLD:
      case CMD_DROOL:
      case CMD_RIP:
      case CMD_STRETCH:
      case CMD_PIMP:
      case CMD_BELITTLE:
      case CMD_TAP:
      case CMD_PILEDRIVE:
      case CMD_FLIPOFF:
      case CMD_MOON:
      case CMD_PINCH:
      case CMD_KISS:
      case CMD_CHEER:
      case CMD_WOO:
      case CMD_GRUMBLE:
      case CMD_APOLOGIZE:
      case CMD_AGREE:
      case CMD_DISAGREE:
      case CMD_SPAM:
      case CMD_ARCH:
      case CMD_ROLL:
      case CMD_BLINK:
      case CMD_FAINT:
      case CMD_GREET:
      case CMD_BOP:
      case CMD_WHIMPER:
      case CMD_SNEER:
      case CMD_MOO:
      case CMD_BOGGLE:
      case CMD_SNORT:
      case CMD_TANGO:
      case CMD_ROAR:
      case CMD_FLEX:
      case CMD_TUG:
      case CMD_CROSS:
      case CMD_HOWL:
      case CMD_GRUNT:
      case CMD_WEDGIE:
      case CMD_SCUFF:
      case CMD_NOOGIE:
      case CMD_BRANDISH:
      case CMD_DUCK:
      case CMD_BECKON:
      case CMD_WINCE:
      case CMD_HUM:
      case CMD_RAZZ:
      case CMD_GAG:
      case CMD_AVERT:
      case CMD_SALUTE:
      case CMD_PET:
      case CMD_GRIMACE:
	rc = doAction(newarg, cmd);
	addToLifeforce(1);
	break;
      case CMD_JUMP:
	rc = doJump(newarg);
	addToLifeforce(1);
	break;
      case CMD_TIP:
	doTip(newarg);
	break;
      case CMD_POKE:
	doPoke(newarg);
	addToLifeforce(1);
	break;
      case CMD_POINT:
	doPoint(newarg);
	addToLifeforce(1);
	break;
      case CMD_BITE:
	rc = doBite(newarg);
	addToLifeforce(1);
	break;
      case CMD_AS:
	rc = doAs(newarg.c_str());
	break;
      case CMD_AT:
	rc = doAt(newarg.c_str(), false);
	break;
      case CMD_GIVE:
	rc = doGive(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TAKE:
      case CMD_GET:
	rc = doGet(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_DROP:
	rc = doDrop(newarg.c_str(), vict);
	addToLifeforce(1);
	break;
      case CMD_SAVE:
	doSave(SILENT_NO, newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_DEATHCHECK:
	doDeathcheck(newarg);
	break;
      case CMD_CHECKLOG:
	doSysChecklog(newarg);
	break;
      case CMD_RECITE:
	rc = doRecite(newarg.c_str());
	break;
      case CMD_RESTORE:
	doRestore(newarg.c_str());
	break;
      case CMD_EMOTE:
      case CMD_EMOTE2:
      case CMD_EMOTE3:
	rc = doEmote(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_ECHO: 
	doEcho(newarg.c_str());
	break;
      case CMD_SHOW:
	doShow(newarg.c_str());
	break;
      case CMD_HIGHFIVE:
	doHighfive(newarg);
	addToLifeforce(1);
	break;
      case CMD_TOGGLE:
	doToggle(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WIZLOCK:
	doWizlock(newarg.c_str());
	break;
      case CMD_FLAG:
	doFlag(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SYSTEM:
	doSystem(newarg);
	addToLifeforce(1);
	break;
      case CMD_TRANSFER:
	doTrans(newarg.c_str());
	break;
      case CMD_SWITCH:
	doSwitch(newarg.c_str());
	break;
      case CMD_CUTLINK:
	doCutlink(newarg.c_str());
	break;
      case CMD_WIZNEWS:
	doWiznews();
	break;
      case CMD_NOSHOUT:
	doNoshout(argument.c_str());
	addToLifeforce(1);
	break;
      case CMD_STEAL:
	rc = doSteal(newarg, dynamic_cast<TBeing *>(vict));
	break;
      case CMD_INVISIBLE:
	doInvis(newarg.c_str());
	break;
      case CMD_VISIBLE:
	doVisible(newarg.c_str(), false);
	addToLifeforce(1);
	break;
      case CMD_LOGLIST:
	doSysLoglist();
	break;
      case CMD_LEAVE:
	rc = doLeave(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_EXITS:
	rc = doExits(newarg.c_str(), cmd);
	addToLifeforce(1);
	break;
      case CMD_WIPE:
	doWipe(newarg.c_str());
	break;
      case CMD_ACCESS:
	doAccess(newarg.c_str());
	break;
      case CMD_CLONE:
	doClone(newarg.c_str());
	break;
      case CMD_OFFICE:
	doOffice(newarg.c_str());
	break;
      case CMD_REPLACE:
	doReplace(newarg.c_str());
	break;
      case CMD_SETSEV:
	doSetsev(newarg.c_str());
	break;
      case CMD_INFO:
	doInfo(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TIMESHIFT:
	doTimeshift(newarg.c_str());
	break;
      case CMD_LOG:
	doLog(newarg.c_str());
	break;
      case CMD_HOSTLOG:
	doHostlog(newarg.c_str());
	break;
      case CMD_ASSIST:
	rc = doAssist(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	addToLifeforce(1);
	break;
      case CMD_WHO:
	doWho(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_BRUTTEST:
	doBruttest(newarg.c_str());
	break;
      case CMD_PEELPK:
	doPeelPk(newarg.c_str());
	break;
      case CMD_SHUTDOW:
	doShutdow();
	break;
      case CMD_SHUTDOWN:
	doShutdown(newarg.c_str());
	break;
      case CMD_LOAD:
	doLoad(newarg.c_str());
	break;
      case CMD_GOTO:
	rc = doGoto(newarg);
	addToLifeforce(1);
	break;
      case CMD_SHOUT:
	doShout(argument.c_str());
	addToLifeforce(1);
	break;
      case CMD_CLIENTMESSAGE:
	doClientMessage(argument.c_str());
	addToLifeforce(1);
	break;
      case CMD_GT:
	doGrouptell(argument.c_str());
	addToLifeforce(1);
	break;
      case CMD_SIGN:
	doSign(newarg);
	addToLifeforce(1);
	break;
      case CMD_TELL:
	buf=newarg;
	buf=one_argument(buf, bufname);
	  
	rc = doTell(bufname, buf);
	addToLifeforce(1);
	break;
      case CMD_WHISPER:
	rc = doWhisper(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_ASK:
	rc = doAsk(newarg);
	addToLifeforce(1);
	break;
      case CMD_WRITE:
	doWrite(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WHOZONE:
	doWhozone();
	addToLifeforce(1);
	break;
      case CMD_EXAMINE:
	doExamine(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SCORE:
	doScore();
	addToLifeforce(1);
	break;
      case CMD_WIZHELP:
	doWizhelp();
	break;
      case CMD_WIZLIST:
	doWizlist();
	break;
      case CMD_INVENTORY:
	doInventory(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_STAND:
	doStand();
	addToLifeforce(1);
	break;
      case CMD_SIT:
	doSit(newarg);
	addToLifeforce(1);
	break;
      case CMD_RAISE:
      case CMD_LIFT:
	rc = doRaise(newarg.c_str(), cmd);
	addToLifeforce(1);
	break;
      case CMD_OPEN:
	rc = doOpen(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_REST:
	doRest(newarg);
	addToLifeforce(1);
	break;
      case CMD_LOWER:
	rc = doLower(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CLOSE:
	doClose(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_LOCK:
	doLock(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_UNLOCK:
	doUnlock(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SLEEP:
	doSleep(newarg);
	break;
      case CMD_WAKE:
	doWake(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TIME:
	doTime(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WEATHER:
	doWeather(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_USERS:
	doUsers(newarg.c_str());
	break;
      case CMD_EQUIPMENT:
	doEquipment(newarg);
	addToLifeforce(1);
	break;
      case CMD_QUIT:
	doQuit();
	addToLifeforce(1);
	break;
      case CMD_QUIT2:
	rc = doQuit2();
	addToLifeforce(1);
	break;
      case CMD_CREDITS:
	doCredits();
	addToLifeforce(1);
	break;
      case CMD_NEWS:
	doNews(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WHERE:
	doWhere(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_LEVELS:
	doLevels(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CONSIDER:
	doConsider(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WORLD:
	doWorld();
	addToLifeforce(1);
	break;
      case CMD_ATTRIBUTE:
	doAttribute(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CLEAR:
	doClear(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_ALIAS:
	doAlias(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_GLANCE:
	doGlance(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_MOTD:
	doMotd(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_LIMBS:
	doLimbs(newarg);
	addToLifeforce(1);
	break;
      case CMD_BREATH:
	doBreath(newarg.c_str());
	break;
      case CMD_CHANGE:
	doChange(newarg.c_str());
	break;
      case CMD_PROMPT:
	doPrompt(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_REMOVE:
	rc = doRemove(newarg.c_str(), vict);
	addToLifeforce(1);
	break;
      case CMD_WEAR:
	doWear(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_STAT:
	doStat(newarg.c_str());
	break;
      case CMD_PURGE:
	doPurge(newarg.c_str());
	break;
      case CMD_SET:
	doSet(newarg.c_str());
	break;
      case CMD_COMMAND:
	doCommand(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_WIZNET:
	doCommune(newarg);
	break;
      case CMD_WIELD:
	doWield(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_GRAB:
      case CMD_HOLD:
	doGrab(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_PUT:
	rc = doPut(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_KILL:
      case CMD_SLAY:
	rc = doKill(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_HIT:
	rc = doHit(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_ENGAGE:
	rc = doEngage(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_DISENGAGE:
	rc = doDisengage();
	break;
      case CMD_QUEST:
	doQuest(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TESTCODE:
	doTestCode(newarg.c_str());
	break;
      case CMD_FOLLOW:
	doFollow(newarg.c_str());
	break;
      case CMD_RETURN:
	doReturn(newarg.c_str(), WEAR_NOWHERE, true);
	break;
      case CMD_REPORT:
	doReport(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_PRAY:
	rc = doPray(newarg.c_str());
	break;
      case CMD_CAST:
	rc = doCast(newarg.c_str());
	break;
      case CMD_CONTINUE:
	doContinue(newarg.c_str());
	break;
      case CMD_READ:
	doRead(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_EAT:
	doEat(newarg.c_str());
	break;
      case CMD_DRINK:
	rc = doDrink(newarg.c_str());
	break;
      case CMD_POUR:
	rc = doPour(newarg.c_str());
	break;
      case CMD_SIP:
	doSip(newarg.c_str());
	break;
      case CMD_TASTE:
	doTaste(newarg.c_str());
	break;
      case CMD_BERSERK:
	rc = doBerserk();
	break;
      case CMD_SHOVE:
	rc = doShove(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_GRAPPLE:
	rc = doGrapple(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_RESCUE:
	rc = doRescue(newarg.c_str());
	break;
      case CMD_DEATHSTROKE:
	rc = doDeathstroke(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_BODYSLAM:
	rc = doBodyslam(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SPIN:
	rc = doSpin(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_STOMP:
	rc = doStomp(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_EMAIL:
	doEmail(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_HEADBUTT:
	rc = doHeadbutt(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_KNEESTRIKE:
	rc = doKneestrike(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_DOORBASH:
	rc = doDoorbash(newarg);
	break;
      case CMD_TRANCE_OF_BLADES:
	doTranceOfBlades(newarg.c_str());
	break;
      case CMD_ATTUNE:
	doAttune(newarg.c_str());
	break;
      case CMD_AFK:
	doAfk();
	addToLifeforce(1);
	break;
      case CMD_SHARPEN:
	doSharpen(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_DULL:
	doDull(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_MEND:     // just aliasing this to repair
      case CMD_REPAIR:
	doRepair(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SACRIFICE:
	doSacrifice(newarg.c_str());
	break;
      case CMD_BANDAGE:
	doBandage(newarg.c_str());
	break;
      case CMD_SET_TRAP:
	rc = doSetTraps(newarg.c_str());
	break;
      case CMD_PICK:
	rc = doPick(newarg.c_str());
	break;
      case CMD_SEARCH:
	rc = doSearch(newarg.c_str());
	break;
      case CMD_SPY:
	rc = doSpy();
	break;
      case CMD_PARRY:
	rc = doParry();
	break;
      case CMD_DODGE:
	rc = doDodge();
	break;
      case CMD_INSULT:
	doInsult(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SCRATCH:
	doScratch(newarg.c_str());
	break;
      case CMD_PEE:
	doPee(newarg.c_str());
	break;
      case CMD_POOP:
	doPoop();
	break;
      case CMD_COMBINE:
	doCombine(newarg.c_str());
	break;
      case CMD_EDIT:
	doEdit(newarg.c_str());
	break;
      case CMD_FADD:
	add_faction(newarg.c_str());
	break;
      case CMD_FEDIT:
	edit_faction(newarg.c_str());
	break;
      case CMD_JOIN:
	doJoin(newarg.c_str());
	break;
      case CMD_DEFECT:
	doDefect(newarg.c_str());
	break;
      case CMD_RECRUIT:
	doRecruit(newarg.c_str());
	break;
      case CMD_RLOAD:
	doRload(newarg.c_str());
	break;
      case CMD_RSAVE:
	doRsave(newarg.c_str());
	break;
      case CMD_REDIT:
	doRedit(newarg.c_str());
	break;
      case CMD_GROUP:
	doGroup(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_FLEE:
	rc = doFlee(newarg.c_str());
	break;
      case CMD_BREW:
	doBrew(newarg.c_str());
	break;
      case CMD_SCRIBE:
	doScribe(newarg.c_str());
	break;
      case CMD_TURN:
	doTurn(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_YOGINSA:
	doYoginsa();
	break;
      case CMD_MEDITATE:
	doMeditate();
	break;
      case CMD_PENANCE:
	doPenance();
	break;
      case CMD_PRACTICE:
	doPractice(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_BLOAD:
	doBload(newarg.c_str());
	break;
      case CMD_GLOAD:
	doGload(newarg);
	break;
      case CMD_THROW:
	doThrow(newarg.c_str());
	break;
      case CMD_SHOOT:
	rc = doShoot(newarg.c_str());
	break;
      case CMD_TRACK:
	doTrack(newarg.c_str());
	break;
      case CMD_SEEKWATER:
	doSeekwater();
	break;
      case CMD_MEDIT:
	doMedit(newarg.c_str());
	break;
      case CMD_SEDIT:
	doSEdit(newarg.c_str());
	break;
      case CMD_LAYHANDS:
	rc = doLayHands(newarg.c_str());
	break;
      case CMD_STAY:
	doStay();
	addToLifeforce(1);
	break;
      case CMD_PASS:
	doPass(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_DEAL:
	doDeal(newarg.c_str());
	break;
      case CMD_ATTACK:
	doAttack(newarg.c_str());
	break;
      case CMD_BET:
	doBet(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CALL:
	doCall(newarg);
	break;
      case CMD_FOLD:
	doFold(newarg);
	break;
      case CMD_OEDIT:
	doOEdit(newarg.c_str());
	break;
      case CMD_MAKELEADER:
	doMakeLeader(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_NEWMEMBER:
	doNewMember(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SEND:
	doSend(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_RMEMBER:
	doRMember(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_HELP:
	doHelp(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_PLAY:
	doPlay(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_SORT:
	doSort(newarg.c_str());
	break;
      case CMD_GAMESTATS:
	doGamestats(newarg.c_str());
	break;
      case CMD_SCAN:
	doScan(newarg.c_str());
	break;
      case CMD_FEIGNDEATH:
	rc = doFeignDeath();
	break;
      case CMD_JUNK:
	rc = doJunk(newarg.c_str(), dynamic_cast<TObj *>(vict));
	break;
      case CMD_NOJUNK:
	rc = doNoJunk(newarg.c_str(), dynamic_cast<TObj *>(vict));
	break;
      case CMD_KICK:
	rc = doKick(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SHOULDER_THROW:
	rc = doShoulderThrow(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_CHOP:
	rc = doChop(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_HURL:
	rc = doHurl(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_CHI:
	rc = doChi(newarg.c_str(), vict);
	break;
      case CMD_LEAP:
	rc = doLeap(newarg);
	break;
      case CMD_VOTE:
	rc = doVote(newarg);
	break;
      case CMD_EVALUATE:
	doEvaluate(newarg.c_str());
	break;
      case CMD_TITLE:
	doTitle(newarg.c_str());
	break;
      case CMD_SNOOP:
	doSnoop(newarg.c_str());
	break;
      case CMD_QUIVPALM:
	rc = doQuiveringPalm(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_RIDE:
      case CMD_MOUNT:
      case CMD_DISMOUNT:
	rc = doMount(newarg.c_str(), cmd, dynamic_cast<TBeing *>(vict));
	break;
      case CMD_FORCE:
	doForce(newarg.c_str());
	break;
      case CMD_COLOR:
	doColor(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CLS:
	doCls(true);
	addToLifeforce(1);
	break;
      case CMD_LIGHT:
	doLight(newarg);
	addToLifeforce(1);
	break;
      case CMD_FISH:
	doFish(newarg);
	addToLifeforce(1);
	break;
      case CMD_LOW:
	doLow(newarg);
	break;
      case CMD_ENTER:
	rc = doEnter(newarg.c_str(), NULL);
	addToLifeforce(1);
	break;
      case CMD_RESIZE:
	doResize(newarg.c_str());
	break;
      case CMD_DISBAND:
	doDisband();
	addToLifeforce(1);
	break;
      case CMD_EXTINGUISH:
	doExtinguish(newarg);
	addToLifeforce(1);
	break;
      case CMD_BASH:
	rc = doBash(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_BACKSTAB:
	rc = doBackstab(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SLIT:
	rc = doThroatSlit(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_HIDE:
	rc = doHide();
	break;
      case CMD_SNEAK:
	rc = doSneak(newarg.c_str());
	break;
      case CMD_CRAWL:
	doCrawl();
	break;
      case CMD_SUBTERFUGE:
	rc = doSubterfuge(newarg.c_str());
	break;
      case CMD_RENAME:
	doNameChange(newarg.c_str());
	break;
      case CMD_MARGINS:
	doResetMargins();
	addToLifeforce(1);
	break;
      case CMD_DISGUISE:
	rc = doDisguise(newarg.c_str());
	break;
      case CMD_DESCRIPTION:
	addToLifeforce(1);
	doDescription();
	break;
      case CMD_POISON_WEAPON:
	rc = doPoisonWeapon(newarg.c_str());
	break;
      case CMD_GARROTTE:
	rc = doGarrotte(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_STAB:
	rc = doStab(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_CUDGEL:
	rc = doCudgel(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_CHARGE:
	rc = doCharge(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SPLIT:
	doSplit(newarg.c_str(), true);
	addToLifeforce(1);
	break;
      case CMD_TRIP:
	rc = doTrip(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_SMITE:
	rc = doSmite(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	break;
      case CMD_FORAGE:
	doForage();
	break;
      case CMD_APPLY_HERBS:
	rc = doApplyHerbs(newarg.c_str());
	break;
      case CMD_BUTCHER:
	doButcher(newarg.c_str());
	break;
      case CMD_SKIN:
	doSkin(newarg.c_str());
	break;
      case CMD_TAN:
	doTan();
	break;
      case CMD_PLANT:
	doPlant(newarg);
	break;
      case CMD_COOK:
	doCook(newarg);
	break;
      case CMD_DRIVE:
	doDrive(newarg);
	break;
      case CMD_WHITTLE:
	doWhittle(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_MESSAGE:
	doMessage(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_FINDEMAIL:
	doFindEmail(newarg.c_str());
	break;
      case CMD_COMMENT:
	doComment(newarg.c_str());
	break;
      case CMD_CAMP:
	rc = doEncamp();
	break;
      case CMD_SOOTH:
	rc = doSoothBeast(newarg.c_str());
	break;
      case CMD_SUMMON:
	rc = doSummonBeast(newarg.c_str());
	break;
      case CMD_CHARM:
	rc = doCharmBeast(newarg.c_str());
	break;
      case CMD_RETRAIN:
	rc = doRetrainPet(newarg.c_str(), dynamic_cast<TBeing *>(vict));
	addToLifeforce(1);
	break;
      case CMD_BEFRIEND:
	rc = doBefriendBeast(newarg.c_str());
	break;
      case CMD_TRANSFIX:
	rc = doTransfix(newarg.c_str());
	break;
      case CMD_BARKSKIN:
	rc = doBarkskin(newarg.c_str());
	break;
      case CMD_FERAL_WRATH:
	rc = doFeralWrath(newarg.c_str());
	break;
      case CMD_SKY_SPIRIT:
	rc = doSkySpirit(newarg.c_str());
	break;
      case CMD_EARTHMAW:
	rc = doEarthmaw(newarg.c_str());
	break;
      case CMD_FLY:
	doFly();
	break;
      case CMD_LAND:
	doLand();
	break;
      case CMD_DIVINE:
	doDivine(newarg.c_str());
	break;
      case CMD_OUTFIT:
	doOutfit(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TRANSFORM:
	rc = doTransform(newarg.c_str());
	break;
      case CMD_EGOTRIP:
	doEgoTrip(newarg.c_str());
	break;
      case CMD_SPELLS:
	doSpells(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_RITUALS:
	doRituals(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_COMPARE:
	doCompare(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_TEST_FIGHT:
	doTestFight(newarg.c_str());
	break;
      case CMD_DONATE:
	doDonate(newarg.c_str());
	break;
      case CMD_ZONES:
	doZones(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_CREATE:
	rc = doCreate(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_POWERS:
	doPowers(newarg.c_str());
	break;
      case CMD_SMOKE:
	doSmoke(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_STOP:
	doStop(newarg);
	addToLifeforce(1);
	break;
      case CMD_TRIGGER:
	doTrigger(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_STORE:
	doStore(newarg.c_str());
	break;
      case CMD_ZONEFILE:
	doZonefile(newarg);
	break;
      case CMD_LOOT:
	doLoot(newarg);
	addToLifeforce(1);
	break;
      case CMD_TROPHY:
	doTrophy(newarg.c_str());
	addToLifeforce(1);
	break;
      case CMD_PTELL:
	doPTell(argument.c_str(), TRUE);
	break;
      case CMD_PSAY:
	doPSay(argument.c_str());
	break;
      case CMD_PSHOUT:
	doPShout(argument.c_str());
	break;
      case CMD_TELEVISION:
	doTelevision(newarg.c_str());
	break;
      case CMD_MINDFOCUS:
	doMindfocus(newarg.c_str());
	break;
      case CMD_PSIBLAST:
	rc = doPsiblast(newarg.c_str());
	break;
      case CMD_MINDTHRUST:
	rc = doMindthrust(newarg.c_str());
	break;
      case CMD_PSYCRUSH:
	rc = doPsycrush(newarg.c_str());
	break;
      case CMD_KWAVE:
	rc = doKwave(newarg.c_str());
	break;
      case CMD_PSIDRAIN:
	rc = doPsidrain(newarg.c_str());
	break;
      case MAX_CMD_LIST:
      case CMD_RESP_TOGGLE:
      case CMD_RESP_UNTOGGLE:
      case CMD_RESP_CHECKTOG:
      case CMD_RESP_PERSONALIZE:
      case CMD_RESP_ROOM_ENTER:
      case CMD_RESP_UNFLAG:
      case CMD_RESP_TOROOM:
      case CMD_RESP_TOVICT:
      case CMD_RESP_TONOTVICT:
      case CMD_RESP_CHECKUNTOG:
      case CMD_RESP_CHECKMAX:
      case CMD_RESP_LINK:
      case CMD_RESP_CODE_SEGMENT:
      case CMD_RESP_RESIZE:
      case CMD_RESP_CHECKLOAD:
      case CMD_RESP_LOADMOB:
      case CMD_RESP_PACKAGE:
      case CMD_RESP_PULSE:
      case CMD_RESP_CHECKROOM:
      case CMD_RESP_CHECKNROOM:
      case CMD_RESP_CHECKZONE:
      case CMD_RESP_CHECKNOTZONE:
      case CMD_RESP_MOVETO:
      case CMD_RESP_DESTINATION:
      case CMD_RESP_CHECKPERSON:
      case CMD_RESP_RANDOM:
      case CMD_RESP_RANDOPTION:
      case CMD_RESP_DONERAND:
      case CMD_GENERIC_PULSE:
      case CMD_GENERIC_QUICK_PULSE:
      case CMD_GENERIC_CREATED:
      case CMD_GENERIC_RESET:
      case CMD_GENERIC_INIT:
      case CMD_GENERIC_DESTROYED:
      case CMD_ROOM_ENTERED:
      case CMD_ROOM_ATTEMPTED_EXIT:
      case CMD_OBJ_HITTING:
      case CMD_OBJ_HIT:
      case CMD_OBJ_MISS:
      case CMD_OBJ_BEEN_HIT:
      case CMD_OBJ_THROWN:
      case CMD_OBJ_PUT_INSIDE_SOMETHING:
      case CMD_OBJ_HAVING_SOMETHING_PUT_INTO:
      case CMD_OBJ_STUCK_IN:
      case CMD_OBJ_PULLED_OUT:
      case CMD_OBJ_USED:
      case CMD_OBJ_TOLD_TO_PLAYER:
      case CMD_OBJ_GOTTEN:
      case CMD_OBJ_WEATHER_TIME:
      case CMD_OBJ_WAGON_INIT:
      case CMD_OBJ_WAGON_UNINIT:
      case CMD_OBJ_MOVEMENT:
      case CMD_OBJ_MOVE_IN:
      case CMD_OBJ_SATON:
      case CMD_OBJ_EXPELLED:
      case CMD_OBJ_START_TO_FALL:
      case CMD_OBJ_OPENED:
      case CMD_ARROW_GLANCE:
      case CMD_ARROW_MISSED:
      case CMD_ARROW_EMBED:
      case CMD_ARROW_RIPPED:
      case CMD_ARROW_DODGED:
      case CMD_ARROW_HIT_OBJ:
      case CMD_ARROW_INTO_ROOM:
      case CMD_ARROW_SHOT:
      case CMD_MOB_GIVEN_ITEM:
      case CMD_MOB_GIVEN_COINS:
      case CMD_MOB_ALIGN_PULSE:
      case CMD_MOB_KILLED_NEARBY:
      case CMD_MOB_MOVED_INTO_ROOM:
      case CMD_MOB_VIOLENCE_PEACEFUL:
      case CMD_MOB_COMBAT:
      case CMD_TASK_FIGHTING:
      case CMD_TASK_CONTINUE:
      case CMD_OBJ_OWNER_HIT:
        sendTo(fmt("doCommand:incorrectCommand: [%d]\n\r") % cmd);
	incorrectCommand();
	return FALSE;
    }
  }

  if (IS_SET(specials.affectedBy, AFF_HIDE) && willBreakHide(cmd, false))
    REMOVE_BIT(specials.affectedBy, AFF_HIDE);

  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    // switch it to vict
    ADD_DELETE(rc, DELETE_VICT);
    REM_DELETE(rc, DELETE_ITEM);
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    if (isPoly && !desc) {
      delete this;
    }
    return rc;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return rc;
  }

  return FALSE;
}


// call this if command should be executed right now (no lag)
// otherwise use addToCommandQue()
// return DELETE_THIS if tbeing has been killed
int TBeing::parseCommand(const sstring &orig_arg, bool typedIn)
{
  int i;
  unsigned int pos;
  sstring argument, aliasbuf, arg1, arg2;
  sstring whitespace=" \f\n\r\t\v";

  argument=orig_arg;

  arg2=one_argument(orig_arg, arg1);

  if(orig_arg.substr(0,3) == "at "){
    arg1="at";
    arg2=arg1+arg2;
  }


  if (arg1.empty())
    return FALSE;

  if (riding) {
    if (!sameRoom(*riding))
      dismount(POSITION_STANDING);
  }

  // handle aliases
  if (desc) {
    for(i=0;i<=16;++i){
      if(arg1==desc->alias[i].word)
	break;
    }

    if (i < 16) {
      if (!arg2.empty())
	aliasbuf=fmt("%s %s") % desc->alias[i].command % arg2;
      else
        aliasbuf=desc->alias[i].command;

      argument=aliasbuf;
      arg2=one_argument(aliasbuf, arg1);
    }
  }


  // Let people use say and emote shortcuts with no spaces - Russ
  if ((argument[0] == '\'') || (argument[0] == ':') || (argument[0] == ',')) {
    arg1 = argument.substr(0,1);
    argument.erase(0,1); // remove first character
  } else {
    if (!argument.substr(0,3).compare("low") && !isImmortal()){
      // KLUDGE - for low and lower command
      // l and lo == look, so we need not check for them
      arg1="lower";
    } else if (!argument.substr(0,4).compare("repl") && !isImmortal()){
      // KLUDGE - for reply and replace command
      // rep == report, so we need not check for shorter
      arg1="reply";
    } else if (!argument.substr(0,3).compare("med") && !isImmortal()){
      // KLUDGE - for meditate and medit command
      // me == mend limb, so we need not check for shorter
      arg1="meditate";
    } else if (!(argument.lower().substr(0,6).compare("southe"))){
      arg1="se";
    } else if (!(argument.lower().substr(0,6).compare("northw"))){
      arg1="nw";
    } else if (!(argument.lower().substr(0,6).compare("southw"))){
      arg1="sw";
    } else if (!(argument.lower().substr(0,6).compare("northe"))){
      arg1="ne";
    }

    // strip out first word
    pos=argument.find_first_not_of(whitespace,0);
    argument.erase(0, argument.find_first_of(whitespace, pos));
  }


  cmdTypeT cmd = searchForCommandNum(arg1);
  if (cmd >= MAX_CMD_LIST) {
    // sendrpf(COLOR_NONE, roomp, "parseCommand:incorrectCommand=[%s]\n\r", arg1.c_str());
    arg1=stripColorCodes(arg1);
    // sendrpf(COLOR_NONE, roomp, "parseCommand:incorrectCommand=[%s]\n\r", arg1.c_str());
    incorrectCommand();
    return FALSE;
  }

  if (IS_SET(specials.affectedBy, AFF_HIDE) && cmd != CMD_BACKSTAB)
    REMOVE_BIT(specials.affectedBy,AFF_HIDE);
  if (IS_SET(specials.affectedBy, AFF_HIDE) && cmd != CMD_SLIT)
    REMOVE_BIT(specials.affectedBy,AFF_HIDE);

  if (getCaptiveOf()) {
    switch (cmd) {
      case CMD_NORTH:
      case CMD_EAST:
      case CMD_SOUTH:
      case CMD_WEST:
      case CMD_UP:
      case CMD_DOWN:
      case CMD_NE:
      case CMD_NW:
      case CMD_SE:
      case CMD_SW:
      case CMD_FLEE:
        sendTo("You've been captured.  You aren't going anywhere until you get away.\n\r");
        return FALSE;
      case CMD_SAY:
      case CMD_LOOK:
      case CMD_LAUGH:
      case CMD_WHO:
      case CMD_HELP:
      case CMD_BUG:
      case CMD_IDEA:
      case CMD_TYPO:
      case CMD_NOD:
      case CMD_SAY2:
      case CMD_SIGH:
      case CMD_CRY:
      case CMD_GIVE:
      case CMD_REMOVE:
      case CMD_DROP:
      case CMD_RETURN:
        break;
      default:
        sendTo("You are unable to do that while a captive.\n\r");
        return FALSE;
    }
  }
  // LIFEFORCE DRAIN ON EVERY DAMN TICK
  if (hasClass(CLASS_SHAMAN)) {
    if (0 >= getLifeforce()) {
      setLifeforce(0);
      updatePos();
      if (GetMaxLevel() > 5) {
	addToHit(-1);
      }
    } else {
      addToLifeforce(-1);
      updatePos();
    }
  }
  // END LIFEFORCE

  // strip leading whitespace, if any
  if(argument.find_first_not_of(whitespace) != sstring::npos)
    argument=argument.substr(argument.find_first_not_of(whitespace));

  return (doCommand(cmd, argument, NULL, typedIn));
}

// I tried it this way, but had a memory leak reported by insure from it
// bat 2/19/99
// static bool fill_word(sstring & argument)
static bool fill_word(const char * argument)
{
  const char *filler_word[] =
  {
    "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
  };

  return (search_block(argument, filler_word, TRUE) >= 0);
}

void argument_interpreter(const char *argument, char *first_arg, char *second_arg)
{
  try {
    sstring tf1, tf2;
    argument_interpreter(argument, tf1, tf2);
    strcpy(first_arg, tf1.c_str());
    strcpy(second_arg, tf2.c_str());
  } catch (...) {
    mud_assert(0, "Failure in argument_interpreter");
  }
}

void argument_interpreter(sstring argument, sstring &first_arg, sstring &second_arg)
{
  sstring st = one_argument(argument, first_arg);
  one_argument(st, second_arg);
}

bool is_number(const sstring &str)
{
  int look_at;

  if (str.empty())
    return (0);

  for (look_at = 0; *(str.c_str() + look_at) != '\0'; look_at++) {
    if ((*(str.c_str() + look_at) < '0') || (*(str.c_str() + look_at) > '9'))
      return (0);
  }
  return (1);
}

const char *one_argument(const char *argument, char *first_arg)
{
  char * temp;
  sstring s;
  sstring tmp_fa;
  try {
    s = one_argument(argument, tmp_fa);
    strcpy(first_arg, tmp_fa.c_str());
  
    // we should return a pointer into argument equivalent to s.c_str
    if (s.empty())
      return &argument[strlen(argument)];  // return pointer to the NULL
    else {
#if 0
      // has problems with " 50 5"
      return strstr(argument, s.c_str());
#else
      // start looking at the spot denoted by "first_arg", for "s"
      temp = strstr(argument, first_arg);
      return strstr(temp, s.c_str());
//    return strstr(strstr(argument, first_arg), s.c_str());
//  COSMO STRING FIX 2/9/01
#endif
    }
  } catch (...) {
    mud_assert(0, "Bat's expirimental code don't work - exception caught");
    return NULL;
  }
// COSMO STRING FIX
  return NULL;
}

sstring one_argument(sstring argument, sstring & first_arg)
{
  size_t bgin, look_at;
  sstring a2;
  sstring whitespace = " \n\r\t";
  bgin = 0;

  do {
    bgin = argument.find_first_not_of(whitespace);
    look_at = argument.find_first_of(whitespace, bgin);

    if (look_at != sstring::npos) {
      // normal, return the part between
      first_arg = argument.substr(bgin, look_at - bgin);
      a2 = argument.substr(look_at);
      argument = a2;
    } else if (bgin != sstring::npos) {
      // sstring had no terminator
      first_arg = argument.substr(bgin);
      argument = "";
    } else {
      // whole sstring was whitespace
      first_arg = "";
      argument = "";
    }
  } while (fill_word(first_arg.c_str()));


  // strip leading whitespace from argument
  if((bgin = argument.find_first_not_of(whitespace))!= string::npos){
    a2 = argument.substr(bgin);
    argument = a2;
  }

  return argument;
}


bool is_abbrev(const char *arg1, const char *arg2, multipleTypeT multiple, exactTypeT exact)
{
  const sstring str1 = arg1;
  const sstring str2 = arg2;
  return is_abbrev(str1, str2, multiple, exact);
}

// determine if a given sstring is an abbreviation of another 
// multiple word functionality FALSE by c++ default - Russ
// Must be explicitly passed TRUE otherwise defaults to FALSE

bool is_abbrev(const sstring &arg1, const sstring &arg2, multipleTypeT multiple, exactTypeT exact)
{
  int spaces1 = 0;
  int spaces2 = 0;
  const sstring whitespace = " \n\r\t";

  // This functionality was added 01/03/98 by me - Russ
  if (multiple) {
    // Do we wanna check for multi word stuff?
    sstring carg1 = arg1;
    trimString(carg1);
    sstring::size_type pos = carg1.find_last_not_of(whitespace);
    if (pos != sstring::npos)
      carg1.erase(pos+1);

    pos = 0;
    do {
      pos++;
      pos = carg1.find_first_of(whitespace, pos);
      if (pos != sstring::npos)
        spaces1++;
    } while (pos != sstring::npos);

    // Now we have number of spaces in arg1, lets see if arg2
    // 1) Has that many words
    // 2) passes is_abbrev for all words
    sstring carg2 = arg2;
    pos = 0;
    do {
      ++pos;
      pos = carg2.find_first_of(whitespace, pos);
      if (pos != sstring::npos)
        spaces2++;
    } while (pos != sstring::npos);

    if (exact) {
      if (spaces1 != spaces2)
        return false;
    } else {
      if (spaces1 > spaces2)
        return false;
    }

    // may have converted the following code incorrectly
    // I wasn't entirely certain what it was doing - peel
    vector <sstring> buf1, buf2;

    split_string(carg1, " ", buf1);
    split_string(carg2, " ", buf2);

    for(unsigned int i=0;i<buf1.size();++i){
      if(buf1[i].lower() != buf2[i].lower().substr(0,buf1[i].size()))
	return false;
    }

    return true;
  }
  // Even if multiple, if we got here, just try to look for
  // something that matches

  if (arg1.length() > arg2.length() || arg1.empty())
    return false;

  // do case insenitive matching
  // we create carg2 "short" so that the compare will work properly
  sstring carg1 = arg1.lower();
  sstring carg2 = arg2.lower();

  // check for just garbage whitespace
  trimString(carg1);
  if (carg1.empty())
    return false;

  if (!sstringncmp(carg1, carg2, carg1.length()))
    return true;
  return false;
}

// return first 'word' plus trailing subsstring of input sstring 
void half_chop(const char *sstring, char *arg1, char *arg2)
{
  for (; isspace(*sstring); sstring++);
  for (; *sstring && !isspace(*arg1 = *sstring); sstring++, arg1++);
  *arg1 = '\0';
  for (; isspace(*sstring); sstring++);
  for (; (*arg2 = *sstring); sstring++, arg2++);
}

sstring add_bars(const sstring &s){
  sstring whitespace=" \f\n\r\t\v";
  sstring stmp=s;

  for(unsigned int pos=stmp.find_first_of(whitespace);
      pos != sstring::npos;
      pos=stmp.find_first_of(whitespace, pos)){
    // replace any contiguous string of white space with a single -
    stmp.replace(pos, stmp.find_first_not_of(whitespace, pos)-pos, "-");
  }

  return stmp;
}


// returns DELETE_THIS, DELETE_VICT, TRUE or FALSE
int TBeing::triggerSpecialOnPerson(TThing *ch, cmdTypeT cmd, const char *arg)
{
  wearSlotT j;
  int rc;
  TThing *t2, *t;

  // special in equipment list?
  for (j = MIN_WEAR; j < MAX_WEAR; j++) {
    if ((t = equipment[j])) {
      rc = t->checkSpec(this, cmd, arg, ch);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        return DELETE_VICT;
      else if (rc)
        return TRUE;
    }
    // special on imbedded item
    if ((t = getStuckIn(j))) {
      rc = t->checkSpec(this, cmd, arg, ch);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        return DELETE_VICT;
      else if (rc)
        return TRUE;
    }
  }
  // special in inventory?
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    if (t->spec) {
      rc = t->checkSpec(this, cmd, arg, ch);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_ONLY(rc, DELETE_VICT))
        return DELETE_THIS;
      if (IS_SET_ONLY(rc, DELETE_ITEM))
        return DELETE_VICT;
      else if (rc)
        return TRUE;
    }
  }
  return FALSE;
}

// this function is a generic trigger for any/all special procs that
// "this" might trigger by doing cmd.  ch is here for future implementation
// if we want to add the capability to trigger with 2ndary parameters.  
// ch is not used now
// return DELETE_THIS will cause this to be destroyed
// return DELETE_VICT will cause ch to be destroyed
int TBeing::triggerSpecial(TThing *ch, cmdTypeT cmd, const char *arg)
{
  int rc;
  TThing *t, *t2;

  // is the player busy doing something else? 
  if (task && ((*(tasks[task->task].taskf)) 
            (this, cmd, arg, 0, roomp, task->obj)))
    return TRUE;

  // is the player busy doing something else?
  if (spelltask && cast_spell(this, cmd, 0))
    return TRUE;

  // special in room? 
  if (roomp) {
    rc = roomp->checkSpec(this, cmd, arg, NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      // delete room?
      vlogf(LOG_BUG, fmt("checkSpec indicated delete room (%d)") %  in_room);
    }
    if (IS_SET_ONLY(rc, DELETE_VICT))
      return DELETE_THIS;
    if (rc)
      return TRUE;
  }

  rc = triggerSpecialOnPerson(ch, cmd, arg);
  if (IS_SET_ONLY(rc, DELETE_THIS))
    return DELETE_THIS;
  if (IS_SET_ONLY(rc, DELETE_VICT))
    return DELETE_VICT;
  else if (rc)
    return TRUE;

  if (roomp) {
    // special in mobile/object present? 
    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;

      // note this is virtual function call
      rc = t->checkSpec(this, cmd, arg, ch);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete t;
        t = NULL;
      }
      if (IS_SET_ONLY(rc, DELETE_VICT))
        return DELETE_THIS;
      if (IS_SET_ONLY(rc, DELETE_ITEM))
        return DELETE_VICT;
      if (rc)
        return TRUE;
    } 
  } 
  return FALSE;
}

void buildCommandArray(void)
{
  commandArray[CMD_NORTH] = new commandInfo("north", POSITION_CRAWLING, 0);
  commandArray[CMD_EAST] = new commandInfo("east", POSITION_CRAWLING, 0);
  commandArray[CMD_SOUTH] = new commandInfo("south", POSITION_CRAWLING, 0);
  commandArray[CMD_WEST] = new commandInfo("west", POSITION_CRAWLING, 0);
  commandArray[CMD_UP] = new commandInfo("up", POSITION_CRAWLING, 0);
  commandArray[CMD_DOWN] = new commandInfo("down", POSITION_CRAWLING, 0);
  commandArray[CMD_NE] = new commandInfo("ne", POSITION_CRAWLING, 0);
  commandArray[CMD_NW] = new commandInfo("nw", POSITION_CRAWLING, 0);
  commandArray[CMD_SE] = new commandInfo("se", POSITION_CRAWLING, 0);
  commandArray[CMD_SW] = new commandInfo("sw", POSITION_CRAWLING, 0);
  commandArray[CMD_DRINK] = new commandInfo("drink", POSITION_RESTING, 0);
  commandArray[CMD_EAT] = new commandInfo("eat", POSITION_RESTING, 0);
  commandArray[CMD_WEAR] = new commandInfo("wear", POSITION_RESTING, 0);
  commandArray[CMD_WIELD] = new commandInfo("wield", POSITION_RESTING, 0);
  commandArray[CMD_LOOK] = new commandInfo("look", POSITION_RESTING, 0);
  commandArray[CMD_SCORE] = new commandInfo("score", POSITION_DEAD, 0);
  commandArray[CMD_TROPHY] = new commandInfo("trophy", POSITION_DEAD, 0);
  commandArray[CMD_CACKLE] = new commandInfo("cackle", POSITION_RESTING, 0);
  commandArray[CMD_SHOUT] = new commandInfo("shout", POSITION_RESTING, 0);
  commandArray[CMD_TELL] = new commandInfo("tell",POSITION_RESTING, 0);
  commandArray[CMD_INVENTORY]=new commandInfo("inventory", POSITION_RESTING, 0);
  commandArray[CMD_GET] = new commandInfo("get", POSITION_RESTING, 0);
  commandArray[CMD_SAY] = new commandInfo("say", POSITION_RESTING, 0);
  commandArray[CMD_SMILE] = new commandInfo("smile", POSITION_RESTING, 0);
  commandArray[CMD_DANCE] = new commandInfo("dance", POSITION_STANDING, 0);
  commandArray[CMD_KILL] = new commandInfo("kill", POSITION_FIGHTING, 0);
  commandArray[CMD_CRAWL] = new commandInfo("crawl", POSITION_RESTING, 0);
  commandArray[CMD_LAUGH] = new commandInfo("laugh", POSITION_RESTING, 0);
  commandArray[CMD_GROUP] = new commandInfo("group", POSITION_RESTING, 0);
  commandArray[CMD_SHAKE] = new commandInfo("shake", POSITION_RESTING, 0);
  commandArray[CMD_PUKE] = new commandInfo("puke", POSITION_RESTING, 0);
  commandArray[CMD_GROWL] = new commandInfo("growl", POSITION_RESTING, 0);
  commandArray[CMD_SCREAM] = new commandInfo("scream", POSITION_RESTING, 0);
  commandArray[CMD_INSULT] = new commandInfo("insult", POSITION_RESTING, 0);
  commandArray[CMD_COMFORT] = new commandInfo("comfort", POSITION_RESTING, 0);
  commandArray[CMD_NOD] = new commandInfo("nod", POSITION_RESTING, 0);
  commandArray[CMD_SIGH] = new commandInfo("sigh", POSITION_RESTING, 0);
  commandArray[CMD_SULK] = new commandInfo("sulk", POSITION_RESTING, 0);
  commandArray[CMD_HELP] = new commandInfo("help", POSITION_DEAD, 0);
  commandArray[CMD_WHO] = new commandInfo("who", POSITION_DEAD, 0);
  commandArray[CMD_EMOTE] = new commandInfo("emote", POSITION_RESTING, 0);
  commandArray[CMD_ECHO]=new commandInfo("echo", POSITION_SLEEPING, GOD_LEVEL1);
  commandArray[CMD_STAND] = new commandInfo("stand", POSITION_RESTING, 0);
  commandArray[CMD_SIT] = new commandInfo("sit", POSITION_RESTING, 0);
  commandArray[CMD_REST] = new commandInfo("rest", POSITION_RESTING, 0);
  commandArray[CMD_SLEEP] = new commandInfo("sleep", POSITION_SLEEPING, 0);
  commandArray[CMD_WAKE] = new commandInfo("wake", POSITION_SLEEPING, 0);
  commandArray[CMD_FORCE]=new commandInfo("force",POSITION_SLEEPING,GOD_LEVEL1);
  commandArray[CMD_TRANSFER]=new commandInfo("transfer",POSITION_SLEEPING,GOD_LEVEL1);
  commandArray[CMD_HUG] = new commandInfo("hug", POSITION_RESTING, 0);
  commandArray[CMD_SNUGGLE] = new commandInfo("snuggle", POSITION_RESTING, 0);
  commandArray[CMD_CUDDLE] = new commandInfo("cuddle", POSITION_RESTING, 0);
  commandArray[CMD_NUZZLE] = new commandInfo("nuzzle", POSITION_RESTING, 0);
  commandArray[CMD_CRY] = new commandInfo("cry", POSITION_RESTING, 0);
  commandArray[CMD_NEWS] = new commandInfo("news", POSITION_DEAD, 0);
  commandArray[CMD_EQUIPMENT]=new commandInfo("equipment", POSITION_RESTING, 0);
  commandArray[CMD_BUY] = new commandInfo("buy", POSITION_SITTING, 0);
  commandArray[CMD_SELL] = new commandInfo("sell", POSITION_SITTING, 0);
  commandArray[CMD_VALUE] = new commandInfo("value", POSITION_SITTING, 0);
  commandArray[CMD_LIST] = new commandInfo("list", POSITION_SLEEPING, 0);
  commandArray[CMD_DROP] = new commandInfo("drop", POSITION_RESTING, 0);
  commandArray[CMD_GOTO] = new commandInfo("goto", POSITION_SLEEPING, 0);
  commandArray[CMD_WEATHER] = new commandInfo("weather", POSITION_RESTING, 0);
  commandArray[CMD_READ] = new commandInfo("read", POSITION_RESTING, 0);
  commandArray[CMD_POUR] = new commandInfo("pour", POSITION_SITTING, 0);
  commandArray[CMD_GRAB] = new commandInfo("grab", POSITION_RESTING, 0);
  commandArray[CMD_REMOVE] = new commandInfo("remove", POSITION_RESTING, 0);
  commandArray[CMD_PUT] = new commandInfo("put", POSITION_RESTING, 0);
  commandArray[CMD_SHUTDOW]=new commandInfo("shutdow",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_SAVE] = new commandInfo("save", POSITION_SLEEPING, 0);
  commandArray[CMD_HIT] = new commandInfo("hit", POSITION_SITTING, 0);
  commandArray[CMD_EXITS] = new commandInfo("exits", POSITION_RESTING, 0);
  commandArray[CMD_GIVE] = new commandInfo("give", POSITION_RESTING, 0);
  commandArray[CMD_QUIT] = new commandInfo("quit", POSITION_DEAD, 0);
  commandArray[CMD_STAT] = new commandInfo("stat", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_GUARD] = new commandInfo("guard", POSITION_STANDING, 0);
  commandArray[CMD_TIME] = new commandInfo("time", POSITION_DEAD, 0);
  commandArray[CMD_LOAD] = new commandInfo("load", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_PURGE] = new commandInfo("purge", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_SHUTDOWN] = new commandInfo("shutdown", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_IDEA] = new commandInfo("ideas", POSITION_DEAD, 0);
  commandArray[CMD_TYPO] = new commandInfo("typos", POSITION_DEAD, 0);
  commandArray[CMD_BUG] = new commandInfo("bugs", POSITION_DEAD, 0);
  commandArray[CMD_WHISPER] = new commandInfo("whisper", POSITION_RESTING, 0);
  commandArray[CMD_CAST] = new commandInfo("cast", POSITION_SITTING, 0);
  commandArray[CMD_AT] = new commandInfo("at", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_AS] = new commandInfo("as", POSITION_DEAD, 0);  // needs to be 0 so any mob can do it
  commandArray[CMD_ORDER] = new commandInfo("order", POSITION_RESTING, 0);
  commandArray[CMD_SIP] = new commandInfo("sip", POSITION_RESTING, 0);
  commandArray[CMD_TASTE] = new commandInfo("taste", POSITION_RESTING, 0);
  commandArray[CMD_SNOOP] = new commandInfo("snoop", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_FOLLOW] = new commandInfo("follow", POSITION_RESTING, 0);
  commandArray[CMD_RENT] = new commandInfo("rent", POSITION_RESTING, 0);
  commandArray[CMD_OFFER] = new commandInfo("offer", POSITION_RESTING, 0);
  commandArray[CMD_POKE] = new commandInfo("poke", POSITION_RESTING, 0);
  commandArray[CMD_ACCUSE] = new commandInfo("accuse", POSITION_SITTING, 0);
  commandArray[CMD_GRIN] = new commandInfo("grin", POSITION_RESTING, 0);
  commandArray[CMD_BOW] = new commandInfo("bow", POSITION_STANDING, 0);
  commandArray[CMD_OPEN] = new commandInfo("open", POSITION_RESTING, 0);
  commandArray[CMD_CLOSE] = new commandInfo("close", POSITION_RESTING, 0);
  commandArray[CMD_LOCK] = new commandInfo("lock", POSITION_CRAWLING, 0);
  commandArray[CMD_UNLOCK] = new commandInfo("unlock", POSITION_FIGHTING, 0);
  commandArray[CMD_LEAVE] = new commandInfo("leave", POSITION_CRAWLING, 0);
  commandArray[CMD_APPLAUD] = new commandInfo("applaud", POSITION_RESTING, 0);
  commandArray[CMD_BLUSH] = new commandInfo("blush", POSITION_RESTING, 0);
  commandArray[CMD_BURP] = new commandInfo("burp", POSITION_RESTING, 0);
  commandArray[CMD_CHUCKLE] = new commandInfo("chuckle", POSITION_RESTING, 0);
  commandArray[CMD_CLAP] = new commandInfo("clap", POSITION_RESTING, 0);
  commandArray[CMD_COUGH] = new commandInfo("cough", POSITION_RESTING, 0);
  commandArray[CMD_CURTSEY] = new commandInfo("curtsey", POSITION_STANDING, 0);
  commandArray[CMD_FART] = new commandInfo("fart", POSITION_RESTING, 0);
  commandArray[CMD_FLEE] = new commandInfo("flee", POSITION_RESTING, 0);
  commandArray[CMD_FONDLE] = new commandInfo("fondle", POSITION_RESTING, 0);
  commandArray[CMD_FROWN] = new commandInfo("frown", POSITION_RESTING, 0);
  commandArray[CMD_GASP] = new commandInfo("gasp", POSITION_RESTING, 0);
  commandArray[CMD_GLARE] = new commandInfo("glare", POSITION_RESTING, 0);
  commandArray[CMD_GROAN] = new commandInfo("groan", POSITION_RESTING, 0);
  commandArray[CMD_GROPE] = new commandInfo("grope", POSITION_RESTING, 0);
  commandArray[CMD_HICCUP] = new commandInfo("hiccup", POSITION_RESTING, 0);
  commandArray[CMD_LICK] = new commandInfo("lick", POSITION_RESTING, 0);
  commandArray[CMD_LOVE] = new commandInfo("love", POSITION_RESTING, 0);
  commandArray[CMD_MOAN] = new commandInfo("moan", POSITION_RESTING, 0);
  commandArray[CMD_NIBBLE] = new commandInfo("nibble", POSITION_RESTING, 0);
  commandArray[CMD_POUT] = new commandInfo("pout", POSITION_RESTING, 0);
  commandArray[CMD_PURR] = new commandInfo("purr", POSITION_RESTING, 0);
  commandArray[CMD_RUFFLE] = new commandInfo("ruffle", POSITION_CRAWLING, 0);
  commandArray[CMD_SHIVER] = new commandInfo("shiver", POSITION_RESTING, 0);
  commandArray[CMD_SHRUG] = new commandInfo("shrug", POSITION_RESTING, 0);
  commandArray[CMD_SING] = new commandInfo("sing", POSITION_RESTING, 0);
  commandArray[CMD_SLAP] = new commandInfo("slap", POSITION_RESTING, 0);
  commandArray[CMD_SMIRK] = new commandInfo("smirk", POSITION_RESTING, 0);
  commandArray[CMD_SNAP] = new commandInfo("snap", POSITION_RESTING, 0);
  commandArray[CMD_SNEEZE] = new commandInfo("sneeze", POSITION_RESTING, 0);
  commandArray[CMD_SNICKER] = new commandInfo("snicker", POSITION_RESTING, 0);
  commandArray[CMD_SNIFF] = new commandInfo("sniff", POSITION_RESTING, 0);
  commandArray[CMD_SNORE] = new commandInfo("snore", POSITION_SLEEPING, 0);
  commandArray[CMD_SPIT] = new commandInfo("spit", POSITION_RESTING, 0);
  commandArray[CMD_SQUEEZE] = new commandInfo("squeeze", POSITION_RESTING, 0);
  commandArray[CMD_STARE] = new commandInfo("stare", POSITION_RESTING, 0);
  commandArray[CMD_STRUT] = new commandInfo("strut", POSITION_STANDING, 0);
  commandArray[CMD_THANK] = new commandInfo("thank", POSITION_RESTING, 0);
  commandArray[CMD_TWIDDLE] = new commandInfo("twiddle", POSITION_RESTING, 0);
  commandArray[CMD_WAVE] = new commandInfo("wave", POSITION_RESTING, 0);
  commandArray[CMD_WHISTLE] = new commandInfo("whistle", POSITION_RESTING, 0);
  commandArray[CMD_WIGGLE] = new commandInfo("wiggle", POSITION_CRAWLING, 0);
  commandArray[CMD_WINK] = new commandInfo("wink", POSITION_RESTING, 0);
  commandArray[CMD_YAWN] = new commandInfo("yawn", POSITION_RESTING, 0);
  commandArray[CMD_SNOWBALL] = new commandInfo("snowball", POSITION_CRAWLING, GOD_LEVEL1);
  commandArray[CMD_WRITE] = new commandInfo("write", POSITION_RESTING, 0);
  commandArray[CMD_HOLD] = new commandInfo("hold", POSITION_RESTING, 0);
  commandArray[CMD_FLIP] = new commandInfo("flip", POSITION_STANDING, 0);
  commandArray[CMD_SNEAK] = new commandInfo("sneak", POSITION_CRAWLING, 0);
  commandArray[CMD_HIDE] = new commandInfo("hide", POSITION_STANDING, 0);
  commandArray[CMD_BACKSTAB]=new commandInfo("backstab", POSITION_STANDING, 0);
  commandArray[CMD_SLIT]=new commandInfo("slit", POSITION_STANDING, 0);
  commandArray[CMD_PICK] = new commandInfo("pick", POSITION_SITTING, 0);
  commandArray[CMD_STEAL] = new commandInfo("steal", POSITION_STANDING, 0);
  commandArray[CMD_BASH] = new commandInfo("bash", POSITION_FIGHTING, 0);
  commandArray[CMD_RESCUE] = new commandInfo("rescue", POSITION_FIGHTING, 0);
  commandArray[CMD_KICK] = new commandInfo("kick", POSITION_FIGHTING, 0);
  commandArray[CMD_FRENCH] = new commandInfo("french", POSITION_RESTING, 0);
  commandArray[CMD_COMB] = new commandInfo("comb", POSITION_RESTING, 0);
  commandArray[CMD_MASSAGE] = new commandInfo("massage", POSITION_RESTING, 0);
  commandArray[CMD_TICKLE] = new commandInfo("tickle", POSITION_RESTING, 0);
  commandArray[CMD_PRAY] = new commandInfo("pray", POSITION_DEAD, 0);
  commandArray[CMD_PAT] = new commandInfo("pat", POSITION_RESTING, 0);
  commandArray[CMD_QUIT2] = new commandInfo("quit!", POSITION_DEAD, 0);
  commandArray[CMD_TAKE] = new commandInfo("take", POSITION_RESTING, 0);
  commandArray[CMD_INFO]=new commandInfo("info", POSITION_SLEEPING, GOD_LEVEL1);
  commandArray[CMD_SAY2] = new commandInfo("'", POSITION_RESTING, 0);
  commandArray[CMD_QUEST]=new commandInfo("quest",POSITION_SLEEPING, 0);
  commandArray[CMD_CURSE] = new commandInfo("curse", POSITION_RESTING, 0);
  commandArray[CMD_USE] = new commandInfo("use", POSITION_SITTING, 0);
  commandArray[CMD_WHERE] = new commandInfo("where", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_LEVELS] = new commandInfo("levels", POSITION_DEAD, 0);
  commandArray[CMD_PEE] = new commandInfo("pee", POSITION_STANDING, 0);
  commandArray[CMD_EMOTE3] = new commandInfo(",", POSITION_RESTING, 0);
  commandArray[CMD_BEG] = new commandInfo("beg", POSITION_RESTING, 0);
  commandArray[CMD_BLEED] = new commandInfo("bleed", POSITION_RESTING, 0);
  commandArray[CMD_CRINGE] = new commandInfo("cringe", POSITION_RESTING, 0);
  commandArray[CMD_DAYDREAM]= new commandInfo("daydream", POSITION_SLEEPING, 0);
  commandArray[CMD_FUME] = new commandInfo("fume", POSITION_RESTING, 0);
  commandArray[CMD_GROVEL] = new commandInfo("grovel", POSITION_RESTING, 0);
  commandArray[CMD_HOP] = new commandInfo("hop", POSITION_STANDING, 0);
  commandArray[CMD_NUDGE] = new commandInfo("nudge", POSITION_RESTING, 0);
  commandArray[CMD_PEER] = new commandInfo("peer", POSITION_RESTING, 0);
  commandArray[CMD_POINT] = new commandInfo("point", POSITION_RESTING, 0);
  commandArray[CMD_PONDER] = new commandInfo("ponder", POSITION_RESTING, 0);
  commandArray[CMD_PUNCH] = new commandInfo("punch", POSITION_RESTING, 0);
  commandArray[CMD_SNARL] = new commandInfo("snarl", POSITION_RESTING, 0);
  commandArray[CMD_SPANK] = new commandInfo("spank", POSITION_RESTING, 0);
  commandArray[CMD_STEAM] = new commandInfo("steam", POSITION_RESTING, 0);
  commandArray[CMD_TACKLE] = new commandInfo("tackle", POSITION_RESTING, 0);
  commandArray[CMD_TAUNT] = new commandInfo("taunt", POSITION_RESTING, 0);
  commandArray[CMD_WIZNET] = new commandInfo("wiznet", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_WHINE] = new commandInfo("whine", POSITION_RESTING, 0);
  commandArray[CMD_WORSHIP] = new commandInfo("worship", POSITION_RESTING, 0);
  commandArray[CMD_YODEL] = new commandInfo("yodel", POSITION_RESTING, 0);
  commandArray[CMD_WIZLIST] = new commandInfo("wizlist", POSITION_DEAD, 0);
  commandArray[CMD_CONSIDER] = new commandInfo("consider", POSITION_RESTING, 0);
  commandArray[CMD_GIGGLE] = new commandInfo("giggle", POSITION_RESTING, 0);
  commandArray[CMD_RESTORE]=new commandInfo("restore",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_RETURN] = new commandInfo("return", POSITION_DEAD, 0);
  commandArray[CMD_SWITCH]=new commandInfo("switch", POSITION_DEAD, 0);
  commandArray[CMD_QUAFF] = new commandInfo("quaff", POSITION_RESTING, 0);
  commandArray[CMD_RECITE] = new commandInfo("recite", POSITION_FIGHTING, 0);
  commandArray[CMD_USERS] = new commandInfo("users", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_PROTECT] = new commandInfo("protect", POSITION_CRAWLING, 0);
  commandArray[CMD_NOSHOUT] = new commandInfo("noshout", POSITION_SLEEPING, 2);
  commandArray[CMD_WIZHELP] = new commandInfo("wizhelp", POSITION_SLEEPING, GOD_LEVEL1);
  commandArray[CMD_CREDITS] = new commandInfo("credits", POSITION_DEAD, 0);
  commandArray[CMD_EMOTE2] = new commandInfo(":", POSITION_RESTING, 0);
  commandArray[CMD_EXTINGUISH]=new commandInfo("extinguish",POSITION_RESTING,0);
  commandArray[CMD_SLAY] = new commandInfo("slay", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_JUNK] = new commandInfo("junk", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_NOJUNK] = new commandInfo("nojunk", POSITION_RESTING, 0);
  commandArray[CMD_DEPOSIT] = new commandInfo("deposit", POSITION_RESTING, 0);
  commandArray[CMD_WITHDRAW] = new commandInfo("withdraw", POSITION_RESTING, 0);
  commandArray[CMD_BALANCE] = new commandInfo("balance", POSITION_RESTING, 0);
  commandArray[CMD_SYSTEM]=new commandInfo("system", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_PULL] = new commandInfo("pull", POSITION_CRAWLING, 0);
  commandArray[CMD_EDIT] = new commandInfo("edit", POSITION_DEAD, 0);
  commandArray[CMD_SET] = new commandInfo("@set", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_RSAVE] = new commandInfo("rsave", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_RLOAD] = new commandInfo("rload", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_TRACK] = new commandInfo("track", POSITION_CRAWLING, 0);
  commandArray[CMD_WIZLOCK]=new commandInfo("wizlock",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_HIGHFIVE] = new commandInfo("highfive", POSITION_RESTING, 0);
  commandArray[CMD_TITLE] = new commandInfo("title", POSITION_DEAD, 0);
  commandArray[CMD_WHOZONE]=new commandInfo("whozone",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_ASSIST] = new commandInfo("assist", POSITION_FIGHTING, 0);
  commandArray[CMD_ATTRIBUTE] = new commandInfo("attribute", POSITION_DEAD, 0);
  commandArray[CMD_WORLD] = new commandInfo("world", POSITION_DEAD, 0);
  commandArray[CMD_BREAK] = new commandInfo("break", POSITION_CRAWLING, 0);
  commandArray[CMD_REFUEL] = new commandInfo("refuel", POSITION_RESTING, 0);
  commandArray[CMD_SHOW] = new commandInfo("show", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_BODYSLAM] =new commandInfo("bodyslam", POSITION_FIGHTING, 0);
  commandArray[CMD_SPIN] =new commandInfo("spin", POSITION_FIGHTING, 0);
  commandArray[CMD_TRANCE_OF_BLADES] = new commandInfo("trance", POSITION_SITTING, 0);
  commandArray[CMD_INVISIBLE] = new commandInfo("invisible", POSITION_DEAD, 0);
  commandArray[CMD_GAIN] = new commandInfo("gain", POSITION_CRAWLING, 0);
  commandArray[CMD_TIMESHIFT] = new commandInfo("timeshift", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_DISARM] = new commandInfo("disarm", POSITION_FIGHTING, 0);
  commandArray[CMD_THINK] = new commandInfo("think", POSITION_RESTING, 0);
  commandArray[CMD_ENTER] = new commandInfo("enter", POSITION_SITTING, 0);
  commandArray[CMD_FILL] = new commandInfo("fill", POSITION_RESTING, 0);
  commandArray[CMD_SHOVE] = new commandInfo("shove", POSITION_STANDING, 0);
  commandArray[CMD_SCAN] = new commandInfo("scan", POSITION_CRAWLING, 0);
  commandArray[CMD_TOGGLE] = new commandInfo("toggle", POSITION_DEAD, 0);
  commandArray[CMD_BREATH] = new commandInfo("breathe", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_GT] = new commandInfo("gtell", POSITION_RESTING, 0);
  commandArray[CMD_WHAP] = new commandInfo("whap", POSITION_RESTING, 0);
  commandArray[CMD_LOG] = new commandInfo("log", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_BEAM] = new commandInfo("beam", POSITION_SLEEPING, 0);
  commandArray[CMD_CHORTLE] = new commandInfo("chortle", POSITION_RESTING, 0);
  commandArray[CMD_REPORT] = new commandInfo("report", POSITION_RESTING, 0);
  commandArray[CMD_WIPE] = new commandInfo("wipe", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_STOP] = new commandInfo("stop", POSITION_DEAD, 0);
  commandArray[CMD_BONK] = new commandInfo("bonk", POSITION_RESTING, 0);
  commandArray[CMD_SCOLD] = new commandInfo("scold", POSITION_RESTING, 0);
  commandArray[CMD_DROOL] = new commandInfo("drool", POSITION_SLEEPING, 0);
  commandArray[CMD_RIP] = new commandInfo("rip", POSITION_RESTING, 0);
  commandArray[CMD_STRETCH] = new commandInfo("stretch", POSITION_RESTING, 0);
  commandArray[CMD_SPLIT] = new commandInfo("split", POSITION_RESTING, 0);
  commandArray[CMD_COMMAND] = new commandInfo("commands", POSITION_SLEEPING, 0);
  commandArray[CMD_DEATHSTROKE] = new commandInfo("deathstroke", POSITION_FIGHTING, 0);
  commandArray[CMD_PIMP] = new commandInfo("pimp", POSITION_STANDING, 0);
  commandArray[CMD_LIGHT] = new commandInfo("light", POSITION_RESTING, 0);
  commandArray[CMD_FISH] = new commandInfo("fish", POSITION_RESTING, 0);
  commandArray[CMD_BELITTLE] = new commandInfo("belittle", POSITION_RESTING, 0);
  commandArray[CMD_PILEDRIVE]=new commandInfo("piledrive",POSITION_STANDING, 0);
  commandArray[CMD_TAP] = new commandInfo("tap", POSITION_CRAWLING, 0);
  commandArray[CMD_BET] = new commandInfo("bet", POSITION_RESTING, 0);
  commandArray[CMD_CALL] = new commandInfo("call", POSITION_RESTING, 0);
  commandArray[CMD_FOLD] = new commandInfo("fold", POSITION_RESTING, 0);
  commandArray[CMD_STAY] = new commandInfo("stay", POSITION_RESTING, 0);
  commandArray[CMD_PEEK] = new commandInfo("peek", POSITION_RESTING, 0);
  commandArray[CMD_COLOR] = new commandInfo("color", POSITION_SLEEPING, 0);
  commandArray[CMD_HEADBUTT]=new commandInfo("headbutt", POSITION_FIGHTING, 0);
  commandArray[CMD_KNEESTRIKE]=new commandInfo("kneestrike",POSITION_FIGHTING, 0);
  commandArray[CMD_SUBTERFUGE] = new commandInfo("subterfuge", POSITION_CRAWLING, 0);
  commandArray[CMD_THROW] = new commandInfo("throw", POSITION_FIGHTING, 0);
  commandArray[CMD_EXAMINE] = new commandInfo("examine", POSITION_RESTING, 0);
  commandArray[CMD_SCRIBE]=new commandInfo("scribe", POSITION_DEAD, 0);
  commandArray[CMD_BREW] = new commandInfo("brew", POSITION_STANDING, 0);
  commandArray[CMD_GRAPPLE] = new commandInfo("grapple", POSITION_FIGHTING, 0);
  commandArray[CMD_FLIPOFF] = new commandInfo("flipoff", POSITION_SITTING, 0);
  commandArray[CMD_MOO] = new commandInfo("moo", POSITION_RESTING, 0);
  commandArray[CMD_PINCH] = new commandInfo("pinch", POSITION_SITTING, 0);
  commandArray[CMD_BITE] = new commandInfo("bite", POSITION_FIGHTING, 0);
  commandArray[CMD_SEARCH] = new commandInfo("search", POSITION_CRAWLING, 0);
  commandArray[CMD_SPY] = new commandInfo("spy", POSITION_CRAWLING, 0);
  commandArray[CMD_DOORBASH]= new commandInfo("doorbash", POSITION_STANDING, 0);
  commandArray[CMD_PLAY] = new commandInfo("play", POSITION_DEAD, 0);
  commandArray[CMD_FLAG] = new commandInfo("flag", POSITION_SLEEPING, GOD_LEVEL1);
  commandArray[CMD_QUIVPALM] = new commandInfo("quivering palm", POSITION_FIGHTING, 0);
  commandArray[CMD_FEIGNDEATH] = new commandInfo("feign death", POSITION_RESTING, 0);
  commandArray[CMD_SPRINGLEAP] = new commandInfo("springleap", POSITION_RESTING, 0);
  commandArray[CMD_MEND_LIMB]=new commandInfo("mend limb", POSITION_RESTING, 0);
  commandArray[CMD_ABORT] = new commandInfo("abort", POSITION_DEAD, 0);
  commandArray[CMD_SIGN] = new commandInfo("sign", POSITION_RESTING, 0);
  commandArray[CMD_CUTLINK]=new commandInfo("cutlink",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_LAYHANDS]=new commandInfo("lay-hands", POSITION_RESTING, 0);
  commandArray[CMD_WIZNEWS]=new commandInfo("wiznews",POSITION_DEAD, 0);
  commandArray[CMD_MAIL] = new commandInfo("mail", POSITION_CRAWLING, 0);
  commandArray[CMD_CHECK] = new commandInfo("check", POSITION_CRAWLING, 0);
  commandArray[CMD_RECEIVE] = new commandInfo("receive", POSITION_CRAWLING, 0);
  commandArray[CMD_CLS] = new commandInfo("cls", POSITION_DEAD, 0);
  commandArray[CMD_REPAIR] = new commandInfo("repair", POSITION_CRAWLING, 0);
  commandArray[CMD_MEND] = new commandInfo("mend", POSITION_CRAWLING, 0);
  commandArray[CMD_SACRIFICE] = new commandInfo("sacrifice", POSITION_CRAWLING, 0);
  commandArray[CMD_PROMPT] = new commandInfo("prompt", POSITION_DEAD, 0);
  commandArray[CMD_GLANCE] = new commandInfo("glance", POSITION_RESTING, 0);
  commandArray[CMD_CHECKLOG] = new commandInfo("checklog", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_LOGLIST]=new commandInfo("loglist",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_DEATHCHECK] = new commandInfo("deathcheck", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_SET_TRAP] = new commandInfo("trap", POSITION_RESTING, 0);
  commandArray[CMD_CHANGE] = new commandInfo("change", POSITION_RESTING, 0);
  commandArray[CMD_REDIT]=new commandInfo("redit", POSITION_DEAD, GOD_LEVEL1); 
  commandArray[CMD_OEDIT] = new commandInfo("oedit", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_FEDIT] = new commandInfo("fedit", POSITION_DEAD, 0);
  commandArray[CMD_FADD]  = new commandInfo("fadd", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_JOIN] = new commandInfo("join", POSITION_RESTING, 0);
  commandArray[CMD_DEFECT] = new commandInfo("defect", POSITION_RESTING, 0);
  commandArray[CMD_RECRUIT] = new commandInfo("recruit", POSITION_RESTING, 0);
  commandArray[CMD_MEDIT] = new commandInfo("medit", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_DODGE] = new commandInfo("dodge", POSITION_FIGHTING, 0);
  commandArray[CMD_PARRY] = new commandInfo("parry", POSITION_FIGHTING, 0);
  commandArray[CMD_ALIAS] = new commandInfo("alias", POSITION_DEAD, 0);
  commandArray[CMD_CLEAR] = new commandInfo("clear", POSITION_DEAD, 0);
  commandArray[CMD_SHOOT] = new commandInfo("shoot", POSITION_CRAWLING, 0);
  commandArray[CMD_BLOAD] = new commandInfo("bload", POSITION_CRAWLING, 0);
  commandArray[CMD_GLOAD] = new commandInfo("gload", POSITION_RESTING, 0);
  commandArray[CMD_MOUNT] = new commandInfo("mount", POSITION_ENGAGED, 0);
  commandArray[CMD_DISMOUNT]= new commandInfo("dismount", POSITION_ENGAGED, 0);
  commandArray[CMD_RIDE] = new commandInfo("ride", POSITION_ENGAGED, 0);
  commandArray[CMD_POST] = new commandInfo("post", POSITION_CRAWLING, 0);
  commandArray[CMD_ASK] = new commandInfo("ask", POSITION_RESTING, 0);
  commandArray[CMD_ATTACK] = new commandInfo("attack", POSITION_SLEEPING, 0);
  commandArray[CMD_SHARPEN] = new commandInfo("sharpen", POSITION_SITTING, 0);
  commandArray[CMD_KISS] = new commandInfo("kiss", POSITION_RESTING, 0);
  commandArray[CMD_ACCESS] = new commandInfo("access", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_OFFICE] = new commandInfo("office", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_CLONE] = new commandInfo("clone", POSITION_RESTING, GOD_LEVEL1);
  commandArray[CMD_MOTD] = new commandInfo("motd", POSITION_DEAD, 0);
  commandArray[CMD_REPLACE] = new commandInfo("replace", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_LIMBS] = new commandInfo("limbs", POSITION_SLEEPING, 0);
  commandArray[CMD_PRACTICE] = new commandInfo("practice", POSITION_DEAD, 0);
  commandArray[CMD_GAMESTATS] = new commandInfo("gamestats", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_BANDAGE] = new commandInfo("bandage", POSITION_SITTING, 0);
  commandArray[CMD_SETSEV]=new commandInfo("setsev", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_TURN]=new commandInfo("turn", POSITION_CRAWLING, 0);
  commandArray[CMD_DEAL] = new commandInfo("deal", POSITION_SITTING, 0);
  commandArray[CMD_PASS] = new commandInfo("pass", POSITION_SITTING, 0);
  commandArray[CMD_MAKELEADER]=new commandInfo("makeleader", POSITION_DEAD, 0);
  commandArray[CMD_NEWMEMBER]=new commandInfo("newmember",POSITION_RESTING,  0);
  commandArray[CMD_RMEMBER] = new commandInfo("rmember", POSITION_DEAD, 0);
  commandArray[CMD_HISTORY] = new commandInfo("history", POSITION_SLEEPING, 0);
  commandArray[CMD_DRAG] = new commandInfo("drag", POSITION_STANDING, 0);
  commandArray[CMD_MOVE] = new commandInfo("move", POSITION_STANDING, 0);
  commandArray[CMD_MEDITATE] = new commandInfo("meditate", POSITION_RESTING, 0);
  commandArray[CMD_SCRATCH] = new commandInfo("scratch", POSITION_RESTING, 0);
  commandArray[CMD_CHEER] = new commandInfo("cheer", POSITION_RESTING, 0);
  commandArray[CMD_WOO] = new commandInfo("woo", POSITION_RESTING, 0);
  commandArray[CMD_GRUMBLE] = new commandInfo("grumble", POSITION_RESTING, 0);
  commandArray[CMD_APOLOGIZE]=new commandInfo("apologize", POSITION_RESTING, 0);
  commandArray[CMD_SEND] = new commandInfo("send", POSITION_RESTING, 0);
  commandArray[CMD_AGREE] = new commandInfo("agree", POSITION_RESTING, 0);
  commandArray[CMD_DISAGREE] = new commandInfo("disagree", POSITION_RESTING, 0);
  commandArray[CMD_BERSERK] = new commandInfo("berserk", POSITION_FIGHTING, 0);
  commandArray[CMD_TESTCODE] = new commandInfo("testcode", POSITION_SLEEPING, GOD_LEVEL1);
  commandArray[CMD_SPAM] = new commandInfo("spam", POSITION_RESTING,0);
  commandArray[CMD_RAISE] = new commandInfo("raise", POSITION_RESTING,0);
  commandArray[CMD_ROLL] = new commandInfo("roll", POSITION_RESTING,0);
  commandArray[CMD_BLINK] = new commandInfo("blink", POSITION_RESTING,0);
  commandArray[CMD_BRUTTEST]=new commandInfo("bruttest",POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_HOSTLOG]=new commandInfo("hostlog",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_PRESS] = new commandInfo("press",POSITION_SITTING,0);
  commandArray[CMD_TWIST] = new commandInfo("twist",POSITION_SITTING,0);
  commandArray[CMD_MID] = new commandInfo("mid",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_TRACEROUTE] = new commandInfo("traceroute", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_TASKS] = new commandInfo("tasks", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_VIEWOUTPUT] = new commandInfo("viewoutput", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_EVALUATE] = new commandInfo("evaluate", POSITION_RESTING, 0);
  commandArray[CMD_EXEC] = new commandInfo("exec", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_LOW] = new commandInfo("low", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_PUSH] = new commandInfo("push",POSITION_RESTING,0);
  commandArray[CMD_RESIZE] = new commandInfo("resize",POSITION_RESTING,GOD_LEVEL1);
  commandArray[CMD_DISBAND] = new commandInfo("disband",POSITION_DEAD,0);
  commandArray[CMD_LIFT] = new commandInfo("lift",POSITION_CRAWLING,0);
  commandArray[CMD_ARCH] = new commandInfo("arch",POSITION_RESTING,0);
  commandArray[CMD_BOUNCE] = new commandInfo("bounce",POSITION_STANDING,0);
  commandArray[CMD_DISGUISE] = new commandInfo("disguise", POSITION_STANDING, 0); 
  commandArray[CMD_RENAME] = new commandInfo("rename", POSITION_DEAD, 0);
  commandArray[CMD_MARGINS] = new commandInfo("margins", POSITION_DEAD,0);
  commandArray[CMD_DESCRIPTION]=new commandInfo("description",POSITION_DEAD,0);
  commandArray[CMD_POISON_WEAPON] = new commandInfo("poison-weapon", POSITION_STANDING, 0); 
  commandArray[CMD_GARROTTE]=new commandInfo("garrotte", POSITION_STANDING, 0); 
  commandArray[CMD_STAB] = new commandInfo("stab", POSITION_FIGHTING, 0); 
  commandArray[CMD_CUDGEL] = new commandInfo("cudgel", POSITION_STANDING, 0); 
  commandArray[CMD_PENANCE] = new commandInfo("penance", POSITION_RESTING, 0);
  commandArray[CMD_SMITE] = new commandInfo("smite", POSITION_SITTING, 0);
  commandArray[CMD_CHARGE] = new commandInfo("charge", POSITION_SITTING, 0);
  commandArray[CMD_LOWER] = new commandInfo("lower", POSITION_STANDING, 0);
  commandArray[CMD_REPLY] = new commandInfo("reply", POSITION_RESTING, 0);
  commandArray[CMD_HEAVEN]=new commandInfo("heaven",POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_CAPTURE] = new commandInfo("capture", POSITION_CRAWLING, GOD_LEVEL1);
  commandArray[CMD_ACCOUNT]=new commandInfo("account",POSITION_DEAD,GOD_LEVEL1);
  commandArray[CMD_RELEASE] = new commandInfo("release", POSITION_CRAWLING, GOD_LEVEL1);
  commandArray[CMD_FAINT] = new commandInfo("faint", POSITION_RESTING, 0);
  commandArray[CMD_GREET] = new commandInfo("greet", POSITION_RESTING, 0);
  commandArray[CMD_TIP] = new commandInfo("tip", POSITION_RESTING, 0);
  commandArray[CMD_BOP] = new commandInfo("bop", POSITION_RESTING, 0);
  commandArray[CMD_JUMP] = new commandInfo("jump", POSITION_STANDING, 0);
  commandArray[CMD_WHIMPER] = new commandInfo("whimper", POSITION_RESTING, 0);
  commandArray[CMD_SNEER] = new commandInfo("sneer", POSITION_RESTING, 0);
  commandArray[CMD_MOON] = new commandInfo("moon", POSITION_CRAWLING, 0);
  commandArray[CMD_BOGGLE] = new commandInfo("boggle", POSITION_RESTING, 0);
  commandArray[CMD_SNORT] = new commandInfo("snort", POSITION_RESTING, 0);
  commandArray[CMD_TANGO] = new commandInfo("tango", POSITION_STANDING, 0);
  commandArray[CMD_ROAR] = new commandInfo("roar", POSITION_SITTING, 0);
  commandArray[CMD_FLEX] = new commandInfo("flex", POSITION_STANDING, 0);
  commandArray[CMD_TUG] = new commandInfo("tug", POSITION_SITTING, 0);
  commandArray[CMD_CROSS] = new commandInfo("cross", POSITION_RESTING, 0);
  commandArray[CMD_HOWL] = new commandInfo("howl", POSITION_SITTING, 0);
  commandArray[CMD_GRUNT] = new commandInfo("grunt", POSITION_RESTING, 0);
  commandArray[CMD_WEDGIE] = new commandInfo("wedgie", POSITION_STANDING, 0);
  commandArray[CMD_SCUFF] = new commandInfo("scuff", POSITION_STANDING, 0);
  commandArray[CMD_NOOGIE] = new commandInfo("noogie", POSITION_STANDING, 0);
  commandArray[CMD_BRANDISH] = new commandInfo("brandish",POSITION_STANDING, 0);
  commandArray[CMD_DUCK] = new commandInfo("duck", POSITION_RESTING, 0);
  commandArray[CMD_BECKON] = new commandInfo("beckon", POSITION_RESTING, 0);
  commandArray[CMD_WINCE] = new commandInfo("wince", POSITION_RESTING, 0);
  commandArray[CMD_HUM] = new commandInfo("hum", POSITION_RESTING, 0);
  commandArray[CMD_RAZZ] = new commandInfo("razz", POSITION_RESTING, 0);
  commandArray[CMD_GAG] = new commandInfo("gag", POSITION_RESTING, 0);
  commandArray[CMD_AVERT] = new commandInfo("avert", POSITION_RESTING, 0);
  commandArray[CMD_SALUTE] = new commandInfo("salute", POSITION_STANDING, 0);
  commandArray[CMD_PET] = new commandInfo("pet", POSITION_RESTING, 0);
  commandArray[CMD_GRIMACE] = new commandInfo("grimace", POSITION_RESTING, 0);
  commandArray[CMD_SEEKWATER]=new commandInfo("seekwater",POSITION_CRAWLING, 0);
  commandArray[CMD_CRIT] = new commandInfo("crit", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_FORAGE] = new commandInfo("forage", POSITION_CRAWLING, 0);
  commandArray[CMD_APPLY_HERBS] = new commandInfo("apply-herbs", POSITION_CRAWLING, 0);
  commandArray[CMD_RESET] = new commandInfo("reset", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_BOOT] = new commandInfo("boot", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_STOMP] = new commandInfo("stomp", POSITION_FIGHTING, 0);
  commandArray[CMD_EMAIL] = new commandInfo("email", POSITION_DEAD, 0);
  commandArray[CMD_CLIMB] = new commandInfo("climb", POSITION_STANDING, 0);
  commandArray[CMD_DESCEND] = new commandInfo("descend", POSITION_STANDING, 0);
  commandArray[CMD_SORT] = new commandInfo("sort", POSITION_SITTING, 0);
  commandArray[CMD_SADDLE] = new commandInfo("saddle", POSITION_STANDING, 0);
  commandArray[CMD_UNSADDLE]=new commandInfo("unsaddle", POSITION_STANDING, 0);
  commandArray[CMD_SHOULDER_THROW] = new commandInfo("shoulder throw", POSITION_FIGHTING, 0);
  commandArray[CMD_CHOP] = new commandInfo("chop", POSITION_FIGHTING, 0);
  commandArray[CMD_HURL] = new commandInfo("hurl", POSITION_FIGHTING, 0);
  commandArray[CMD_CHI] = new commandInfo("chi", POSITION_FIGHTING, 0);
  commandArray[CMD_LEAP] = new commandInfo("leap", POSITION_STANDING, 0);
  commandArray[CMD_VOTE] = new commandInfo("vote", POSITION_STANDING, 0);
  commandArray[CMD_DIVINE] = new commandInfo("divine", POSITION_STANDING, 0);
  commandArray[CMD_OUTFIT] = new commandInfo("outfit", POSITION_STANDING, 0);
  commandArray[CMD_CLIENTS] = new commandInfo("clients", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_DULL] = new commandInfo("smooth", POSITION_SITTING, 0);
  commandArray[CMD_ADJUST] = new commandInfo("adjust", POSITION_SLEEPING, 0);
  commandArray[CMD_BUTCHER] = new commandInfo("butcher", POSITION_STANDING, 0);
  commandArray[CMD_PLANT] = new commandInfo("plant", POSITION_STANDING, 0);
  commandArray[CMD_COOK] = new commandInfo("cook", POSITION_STANDING, 0);
  commandArray[CMD_DRIVE] = new commandInfo("drive", POSITION_RESTING, 0);
  commandArray[CMD_SKIN] = new commandInfo("skin", POSITION_STANDING, 0);
  commandArray[CMD_TAN] = new commandInfo("tan", POSITION_STANDING, 0);
  commandArray[CMD_TITHE] = new commandInfo("tithe", POSITION_STANDING, 0);
  commandArray[CMD_DISSECT] = new commandInfo("dissection",POSITION_SITTING, 0);
  commandArray[CMD_FINDEMAIL] = new commandInfo("findemail", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_ENGAGE] = new commandInfo("engage", POSITION_FIGHTING, 0);
  commandArray[CMD_DISENGAGE]=new commandInfo("disengage",POSITION_FIGHTING, 0);
  commandArray[CMD_RESTRING]= new commandInfo("restring", POSITION_FIGHTING, 0);
  commandArray[CMD_CONCEAL] = new commandInfo("conceal", POSITION_STANDING, 0);
  commandArray[CMD_COMMENT] = new commandInfo("comment", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_CAMP] = new commandInfo("camp", POSITION_CRAWLING, 0);
  commandArray[CMD_YOGINSA] = new commandInfo("yoginsa", POSITION_RESTING, 0);
  commandArray[CMD_FLY] = new commandInfo("fly", POSITION_FIGHTING, 0);
  commandArray[CMD_LAND] = new commandInfo("land", POSITION_FIGHTING, 0);
  commandArray[CMD_ATTUNE] = new commandInfo("attune", POSITION_RESTING, 0);
  commandArray[CMD_AFK] = new commandInfo("afk", POSITION_DEAD, 0);
  commandArray[CMD_CONTINUE] = new commandInfo("continue", POSITION_DEAD, 0);
  commandArray[CMD_PEELPK] = new commandInfo("peelpk", POSITION_DEAD, 0);
  commandArray[CMD_SOOTH] = new commandInfo("sooth", POSITION_STANDING, 0);
  commandArray[CMD_SUMMON] = new commandInfo("summon", POSITION_STANDING, 0);
  commandArray[CMD_CHARM] = new commandInfo("charm", POSITION_STANDING, 0);
  commandArray[CMD_BEFRIEND] = new commandInfo("befriend", POSITION_STANDING, 0);
  commandArray[CMD_TRANSFIX] = new commandInfo("transfix", POSITION_STANDING, 0);
  commandArray[CMD_BARKSKIN] = new commandInfo("barkskin", POSITION_STANDING, 0);
  commandArray[CMD_FERAL_WRATH] = new commandInfo("feral", POSITION_STANDING, 0);
  commandArray[CMD_SKY_SPIRIT] = new commandInfo("sky", POSITION_FIGHTING, 0);
  commandArray[CMD_EARTHMAW] = new commandInfo("earthmaw", POSITION_FIGHTING, 0);
  commandArray[CMD_TRANSFORM] = new commandInfo("transform", POSITION_STANDING, 0);
  commandArray[CMD_EGOTRIP] = new commandInfo("egotrip", POSITION_STANDING, GOD_LEVEL1);
  commandArray[CMD_CHIP] = new commandInfo("chip", POSITION_STANDING, 0);
  commandArray[CMD_DIG] = new commandInfo("dig", POSITION_STANDING, 0);
  commandArray[CMD_COVER] = new commandInfo("cover", POSITION_STANDING, 0);
  commandArray[CMD_OPERATE] = new commandInfo("operate", POSITION_STANDING, 0);
  commandArray[CMD_SPELLS] = new commandInfo("spells", POSITION_DEAD, 0);
  commandArray[CMD_RITUALS] = new commandInfo("rituals", POSITION_DEAD, 0);
  commandArray[CMD_COMPARE] = new commandInfo("compare", POSITION_DEAD, 0);
  commandArray[CMD_TEST_FIGHT] = new commandInfo("testfight", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_DONATE] = new commandInfo("donate", POSITION_RESTING, 0);
  commandArray[CMD_ZONES] = new commandInfo("zones", POSITION_SLEEPING, 0);
  commandArray[CMD_FACTIONS] = new commandInfo("factions", POSITION_SLEEPING, 0);
  commandArray[CMD_CREATE] = new commandInfo("create", POSITION_STANDING, GOD_LEVEL1);
  commandArray[CMD_POWERS] = new commandInfo("powers", POSITION_STANDING, GOD_LEVEL1);
  commandArray[CMD_WHITTLE] = new commandInfo("whittle", POSITION_STANDING, 0);
  commandArray[CMD_MESSAGE] = new commandInfo("message", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_SMOKE] = new commandInfo("smoke", POSITION_RESTING, 0);
  commandArray[CMD_CLIENTMESSAGE] = new commandInfo("clientmessage", POSITION_RESTING, 60);
  commandArray[CMD_SEDIT] = new commandInfo("sedit", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_RETRAIN] = new commandInfo("retrain", POSITION_STANDING, 0);
  commandArray[CMD_VISIBLE] = new commandInfo("visible", POSITION_STANDING, 0);
  commandArray[CMD_TRIGGER] = new commandInfo("trigger", POSITION_STANDING, GOD_LEVEL1);
  commandArray[CMD_STORE] = new commandInfo("store", POSITION_STANDING, GOD_LEVEL1);
  commandArray[CMD_ZONEFILE] = new commandInfo("zonefile", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_LOOT] = new commandInfo("loot", POSITION_DEAD, GOD_LEVEL1);
  commandArray[CMD_PSAY] = new commandInfo("psay", POSITION_DEAD, 0);
  commandArray[CMD_PTELL] = new commandInfo("ptell", POSITION_DEAD, 0);
  commandArray[CMD_PSHOUT] = new commandInfo("pshout", POSITION_DEAD, 0);
  commandArray[CMD_TELEVISION] = new commandInfo("television", POSITION_RESTING, 0);
  commandArray[CMD_MINDFOCUS] = new commandInfo("mindfocus", POSITION_RESTING, 0);
  commandArray[CMD_PSIBLAST] = new commandInfo("psiblast", POSITION_RESTING, 0);
  commandArray[CMD_MINDTHRUST] = new commandInfo("mindthrust", POSITION_RESTING, 0);
  commandArray[CMD_PSYCRUSH] = new commandInfo("psycrush", POSITION_RESTING, 0);
  commandArray[CMD_KWAVE] = new commandInfo("kwave", POSITION_RESTING, 0);
  commandArray[CMD_PSIDRAIN] = new commandInfo("psidrain", POSITION_RESTING, 0);
  commandArray[CMD_TRIP] = new commandInfo("trip", POSITION_FIGHTING, 0);
  commandArray[CMD_POOP] = new commandInfo("poop", POSITION_STANDING, 0);
  commandArray[CMD_COMBINE] = new commandInfo("combine", POSITION_RESTING, 0);
}

bool _parse_name(const char *arg, char *name)
{
  char buf[80];
  unsigned int i;

  for (; isspace(*arg); arg++);

  if (strlen(arg) < 3)
    return TRUE;

  for (i = 0; *illegalnames[i] != '\n'; i++) {
    if (*illegalnames[i] == '*') {
      if (strstr(sstring(arg).lower().c_str(), illegalnames[i] + 1))
        return TRUE;
    } else {
      if (!strcasecmp(illegalnames[i], arg))
        return TRUE;
    }
  }
  if (!AllowPcMobs) {
    for (i= 0; i < mob_index.size(); i++) {
      sprintf(buf, fname(mob_index[i].name).c_str());
      if (!strcasecmp(buf, arg))
        return TRUE;
    }
  }
#if 0
  for (i= 0; i < obj_index.size(); i++) {
    sprintf(buf, fname(obj_index[i].name));
    if (!strcasecmp(buf, arg))
      return TRUE;
  }
#endif    
  for (i = 0; (*name = *arg); arg++, i++, name++)
    if ((*arg < 0) || !isalpha(*arg) || i > 15)
      return TRUE;

  if (!i)
    return TRUE;

  return FALSE;
}

#if 0
int min_stat(race_t race, statTypeT iStat)
{
  // 1=str 2=dex 3=int 4=wis 5=con 6=chr 
  if (iStat == 1) {                                                              
    if (race == RACE_DWARF) 
      return (8);            
    else if (race == RACE_GNOME)                                                 
      return (6);                                                              
    else if (race == RACE_OGRE)                                                 
      return (7);                                                              
    else                                                                        
      return (3);                                                              
  } else if (iStat == 2) {                                                       
    if (race == RACE_HOBBIT)
      return (8);                                                              
    else if (race == RACE_OGRE)
      return (5); 
    else if (race == RACE_ELVEN)
      return (6);
    else
      return (3);
  } else if (iStat == 3) {                                                       
    if ((race == RACE_GNOME) || (race == RACE_ELVEN))                           
      return (7);                                                               
    else                                                                        
      return (3);
  } else if (iStat == 4) {                                                       
    if ((race == RACE_GNOME) || (race == RACE_ELVEN)) 
      return (7);                                                              
    else                                                                        
      return (3);                                                              
  } else if (iStat == 5) {                                                       
    if ((race == RACE_DWARF) || (race == RACE_HOBBIT))
      return (8); 
    else if (race == RACE_OGRE)                                                
      return (7);                                                              
    else                                                                        
      return (3);                                                              
  } else if (iStat == 6) {                                                       
    if (race == RACE_ELVEN)                                                     
      return (7);                                                              
    else if (race == RACE_HOBBIT)
      return (6);
    else                                                                        
      return (3);                                                              
  }                                                                             
  return (3);                                                                  
} 


int max_stat(race_t race, statTypeT iStat)
{
  // 1=str 2=dex 3=int 4=wis 5=con 6=chr 

  if (iStat == 1) {
    if ((race == RACE_HOBBIT))
      return (14);
    else if (race == RACE_ELVEN)
      return (14);
    else if (race == RACE_GNOME) 
      return (14);
    else if (race == RACE_OGRE)
      return (19);
    else
      return (18);
  } else if (iStat == 2) {
    if (race == RACE_DWARF)
      return (14);
    else if (race == RACE_HOBBIT)
      return (19);
    else if (race == RACE_OGRE)
      return (14);
    else if (race == RACE_GNOME)
      return (16);
    else
      return (18);
  } else if (iStat == 3) {
    if (race == RACE_OGRE)
      return (15);
    else if (race == RACE_ELVEN)
      return (19);
    else if (race == RACE_HOBBIT)
      return (14);
    else if (race == RACE_DWARF)
       return 14;
    else
      return (18);
  } else if (iStat == 4) {
    if (race == RACE_GNOME)
      return (19);
    else if (race == RACE_OGRE)
      return (15);
    else if (race == RACE_HOBBIT)
      return (14);
    else if (race == RACE_DWARF)
      return (14);
    else
      return (18);
  } else if (iStat == 5) {
    if (race == RACE_DWARF)
      return (19);
    else if (race == RACE_GNOME)
      return (14);
    else if (race == RACE_ELVEN)
      return (12);
    else
      return (18);
  } else if (iStat == 6) {
    if (race == RACE_DWARF)
      return (15);
    else if ((race == RACE_OGRE))
      return (12);
    else if (race == RACE_GNOME)
      return (14);
    else
      return (18);
  }
  return (18);
}
#endif

// This will put a command into player's command que.
// for mobs, it causes instanteous execution (no que)
// will return DELETE_THIS if this should be deleted
int TBeing::addCommandToQue(const sstring &msg)
{
  int rc;

  if (isPc() && desc){
    if (!isPlayerAction(PLR_MAILING) && 
        desc->connected != CON_WRITING) 
    desc->input.putInQ(msg);
  } else {
    rc = parseCommand(msg, TRUE);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}

sstring sprintbit(unsigned long vektor, const char * const names[])
{
  long nr;
  sstring result;

  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(vektor, (unsigned long) 1L))
      if (*names[nr]) {
	result += names[nr];
	result += " ";
      }
    if (*names[nr] != '\n')
      nr++;
  }

  if (result.empty())
    result="NOBITS";

  return result;
}

void sprinttype(int type, const sstring names[], char *result)
{
  int nr;

  for (nr = 0; (names[nr] != "\n"); nr++);

  if (type < nr)
    strcpy(result, names[type].c_str());
  else
    strcpy(result, "UNDEFINED");
}

#if (!defined SUN && !defined LINUX && !defined(SOLARIS))
char *strstr(const char *s1, const char *s2)
{
  if (!*s2)
    return s1;  // conformance with strstr

  int j = strlen(s1) - strlen(s2);
  if (j < 0)
    return NULL;  // conformance with strstr
  int i, k = strlen(s2);
  for (i = 0; i <= j && strncmp(&s1[i], s2, k) != 0; i++);

  return (i > j) ? NULL : &s1[i];
}
#endif

// I redid this function to make it more flexible. It has a new argument  
// for the array, making it useful with any array.  - Russ                
void bisect_arg(const char *arg, int *field, char *sstring, const char * const array[])
{
  char buf[MAX_INPUT_LENGTH];

  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), array, 0)))
    return;

  for (; isspace(*arg); arg++);
  for (; (*sstring = *arg); arg++, sstring++);

  return;
}



char *fold(char *line)
{
  const unsigned int FOLDAT = 78;

  int i, j = 0, folded;

  if (strlen(line) > FOLDAT)
    for (i = FOLDAT; i < (int) strlen(line); i += FOLDAT) {
      folded = FALSE;
      for (j = i; !folded; j--)
        if (line[j] == ' ') {
          line[j] = '\n';
          folded = TRUE;
          i = j;
        }
    }
  return line;
}

int ctoi(char c)
{
  char buf[5];

  sprintf(buf, "%c", c);
  return convertTo<int>(buf);
}

// essentially, strips out multiple ' ' truncating to a single space
void cleanCharBuf(char *buf)
{
  char *from, *to;
  char p =' ';

  from = to = buf;

  while(*from) {
    if (*from != ' ' || (*from == ' ' && p != ' ')) 
      *to++ = *from;
    p = *from++;
  }

  if (p == ' ' && to != buf) 
    *(to-1) = '\000';
  else 
    *to = '\000';
}

void str_shiftleft(char *str, int n)
{
  char *cp;

  for (cp = str + n; (*(cp - n) = *cp); cp++);
}

sstring nextToken(char delim, unsigned int maxSize, char *str)
{
  char retbuf[256];
  char *cp;

  for (cp = str; *cp && (*cp != delim); cp++);
  if ((cp - str) > (int) maxSize) {
    strncpy(retbuf, str, maxSize);
    retbuf[maxSize] = '\0';
  } else {
    strncpy(retbuf, str, (int) (cp - str));
    retbuf[(int) (cp - str)] = '\0';
  }
  if (!*cp)
    *str = '\0';
  else
    str_shiftleft(str, (int) (cp - str + 1));
 
  return retbuf;
}

char *mud_str_dup(const char *buf)
{
  if (!buf)
    return NULL;
  
  return mud_str_dup((sstring) buf);
}


char * mud_str_dup(const sstring &buf)
{
  char *tmp = NULL;

  try {
    tmp = new char[buf.length() + 1];
  } catch (...) {
    mud_assert(0, "exception caught in mud_str_dup");
  }
  strcpy(tmp, buf.c_str());
  return tmp;
}

// copy n bytes of src to dest
// if src is bigger than n, copy as much as possible to dest and null terminate
// then generate an error log
char *mud_str_copy(char *dest, const sstring &src, size_t n)
{
  strncpy(dest, src.c_str(), n);

  if(src.length() > n){
    dest[n-1]='\0';
    vlogf(LOG_BUG, fmt("mud_str_copy: source sstring too long.  Truncated to: %s") %  dest);
  }

  return dest;
}



void trimString(sstring &arg)
{
  if (arg.empty())
    return;
  size_t iter = arg.find_first_not_of(" \n\r");
  if (iter == sstring::npos)
    return;
  arg.erase(0, iter);  // erase the leading whitespace
}

int sstringncmp(const sstring str1, const sstring str2, unsigned int len)
{
  // ANSI doesn't provide a strncmp for sstrings
  // I've hacked this together cuz I think it's needed

  // trunc down to length and compare
  return string(str1, 0, len).compare(string(str2, 0, len));
}

commandInfo::~commandInfo()
{
}

// take what is in the output buffer, cat into single sstring, and page it
void TBeing::makeOutputPaged()
{
  if (!desc)
    return;

  sstring str;
  char buf[MAX_STRING_LENGTH];

  memset(buf, '\0', sizeof(buf));
  while (desc->output.takeFromQ(buf, sizeof(buf))) {
    str += buf;
    memset(buf, '\0', sizeof(buf));
  }

  desc->page_string(str);
}
