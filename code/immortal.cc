//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "immortal.cc" - All commands reserved for wizards
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

extern "C" {
#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/socket.h>
}

#include "disease.h"
#include "statistics.h"
#include "combat.h"
#include "mail.h"
#include "games.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_armadillo.h"
#include "account.h"
#include "systemtask.h"
#include "socket.h"
#include "shop.h"
#include "cmd_trophy.h"
#include "obj_note.h"
#include "obj_portal.h"
#include "obj_base_clothing.h"
#include "database.h"
#include "rent.h"

bool Silence = FALSE;
bool Sleep = TRUE;

// please document what each testcode does if you use it!!!!!
bool TestCode1 = false;
// code1 vlogf's main loop pulse times
bool TestCode2 = false;       // unfinished code 
  // code2 in use, lets players see level on items
bool TestCode3 = true;       // unfinished code 
  // code3 in use, hiding new spell
bool TestCode4 = true;       // unfinished code 
  // code4 in use, hiding new spell
bool TestCode5 = false;
  // code5 is to disable/enable certain aspects of the new faction code - dash 6/24/01
bool TestCode6 = false;
  // not in use

bool NewbiePK = FALSE;
bool QuestCode = false;       // spec-procs for quests 
bool Gravity = TRUE;         // Do we allow gravity, fallen objects etc.
bool QuestCode2 = FALSE;     // spec-procs for quests 
bool QuestCode3 = FALSE;     // spec-procs for quests
bool QuestCode4 = FALSE;     // spec-procs for quests

bool Clients = true;         // Do we allow clients? 
bool WizBuild = true;         // can builders hear wiznet
bool WizInvis = FALSE;
bool WizShout = FALSE;
bool WizGoto = FALSE;
bool AllowPcMobs = TRUE;    // PCs with same name as mob allowed?
bool Twink = FALSE; // combat twink fun stuff
bool timeQueries = false; // store db query speeds THIS IS SLOW
bool gameLoopTiming = false; // spit out game loop info, very spammy

int QuestVar1 = 0; // varibles for changing constants in the code in-game
int QuestVar2 = 0;
int QuestVar3 = 0;
int QuestVar4 = 0;

void TBeing::doChange(const char *argument)
{
  change_hands(this, argument);
  return;
}

void TPerson::doChange(const char *argument)
{
  int new_lev;
  char buf[200], buf2[200];

  half_chop(argument, buf, buf2);

  if (!isImmortal() || !*buf) {
    change_hands(this, argument);
    return;
  }
  



  if (is_abbrev(argument, "questvar1")) {
    argument = one_argument(argument, buf);
    argument = one_argument(argument, buf);
    int val = convertTo<int>(buf);
    QuestVar1 = val;
    sendTo(fmt("You have changed QuestVar1 to %d.") % val);
    return;
  } else if (is_abbrev(argument, "questvar2")) {
    argument = one_argument(argument, buf);
    argument = one_argument(argument, buf);
    int val = convertTo<int>(buf);
    QuestVar2 = val;
    sendTo(fmt("You have changed QuestVar2 to %d.") % val);
    return;
  } else if (is_abbrev(argument, "questvar3")) {
    argument = one_argument(argument, buf);
    argument = one_argument(argument, buf);
    int val = convertTo<int>(buf);
    QuestVar3 = val;
    sendTo(fmt("You have changed QuestVar3 to %d.") % val);
    return;
  } else if (is_abbrev(argument, "questvar4")) {
    argument = one_argument(argument, buf);
    argument = one_argument(argument, buf);
    int val = convertTo<int>(buf);
    QuestVar4 = val;
    sendTo(fmt("You have changed QuestVar4 to %d.") % val);
    return;
  }




  if (*buf && *buf2 && hasWizPower(POWER_CHANGE)) {
    if (isdigit(*buf2)) {
      if ((new_lev = convertTo<int>(buf2)) > MAX_IMMORT) {
        sendTo(fmt("The new level can't be greater than %d.\n\r") % MAX_IMMORT);
        return;
      }
    } else {
      sendTo("Syntax : change <command name> <new level>\n\r");
      return;
    }
    cmdTypeT cmd = searchForCommandNum(buf);
    if (cmd >= MAX_CMD_LIST) {
      sendTo(fmt("(%s) no such command.\n\r") % buf);
      return;
    }
    if (cmd >= 0 && GetMaxLevel() < commandArray[cmd]->minLevel) {
      if (strcmp(getName(), "Batopr")) {
        sendTo("That command is too high for you to change.\n\r");
        return;
      } else 
        sendTo(fmt("It was set to level %d.\n\r") %  commandArray[cmd]->minLevel);
    }
    if (cmd >= 0) {
      commandArray[cmd]->minLevel = new_lev;
      sendTo(fmt("You just changed the %s command's min level to %d.\n\r") % commandArray[cmd]->name % new_lev);
    }
  } else if (!*buf2) {
    cmdTypeT cmd = searchForCommandNum(buf);
    if (cmd >= MAX_CMD_LIST) {
      sendTo("Syntax : change <command name> <new level>.\n\r");
      return;
    }
    sendTo(fmt("\n\r%s : Min Level : %d\n\r") % commandArray[cmd]->name % commandArray[cmd]->minLevel);
  } else {
    sendTo("Syntax : change <command name> <new level>\n\r");
    return;
  }
}

char * dsearch(const char *sstring)
{
  char *c, buf[256], buf2[256];
  int j;
  char *tmp;

  tmp = mud_str_dup(sstring);
  while (1) {
    if (!strchr(tmp, '~')) {
      return tmp;
    } else {
      c = strchr(tmp, '~');
      j = c - tmp;
      if (j+1 >= (int) strlen(tmp)) {
        // This prevents someone from ending with a ~
        return tmp;
      }
      switch (tmp[j + 1]) {
        case 'N':
          strcpy(buf2, "$n");
          break;
        case 'H':
          strcpy(buf2, "$s");
          break;
        case 'R':
          strcpy(buf2, "\n\r");
          break;
        default:
          strcpy(buf2, "");
          break;
      }
      strcpy(buf, tmp);
      strcpy(&buf[j], buf2);
      strcat(buf, (tmp + j + 2));
      delete [] tmp;
      tmp = mud_str_dup(buf);
    }
  }
}

void TBeing::doHighfive(const sstring &argument)
{
  sstring mess;
  TBeing *tch;

  if (!argument.empty()) {
    if ((tch = get_char_room_vis(this, argument)) != 0) {
      if (tch->isImmortal() && isImmortal()) {
        switch(::number(1,3)) {
          case 1:
            mess = fmt("Time stops for a moment as %s and %s high five.\n\r") %
	      name % tch->name;
            break;
          case 2:
            mess = fmt("Thunder booms and lightning streaks across the heavens as %s and %s high five.\n\r") %
	      name % tch->name;
	    break;
          case 3:
            mess = fmt("The world shakes as %s and %s high five.\n\r") %
	      name % tch->name;
            break;
          default:
            mess = fmt("Time stops for a moment as %s and %s high five.\n\r") %
	      name % tch->name;
            break;
        }
        descriptor_list->worldSend(mess, this);
      } else {
        act("$n gives you a high five.", TRUE, this, 0, tch, TO_VICT);
        act("You give a hearty high five to $N.", TRUE, this, 0, tch, TO_CHAR);
        act("$n and $N do a high five.", TRUE, this, 0, tch, TO_NOTVICT);
      }
    } else
      sendTo("I don't see anyone here like that.\n\r");
  }
}



void TBeing::doWizlock(const char *argument)
{
  int a, length, b;
  char buf[256];
  TObj *note;

  if (powerCheck(POWER_WIZLOCK))
    return;

  if (!isImmortal()) {
    sendTo("You cannot WizLock.\n\r");
    return;
  }
  argument = one_argument(argument, buf);

  if (!*buf) {
    sendTo("Wizlock {all | off | add <host> | rem <host> | list  | message}\n\r");
    sendTo(fmt("Global wizlock is presently %s.\n\r") % (WizLock ? "ON" : "OFF"));
    sendTo("The wizlock message is currently:\n\r");
    sendTo(lockmess);
    return;
  }
  if (!strcmp(buf, "all")) {
    if (WizLock)
      sendTo("It's already on!\n\r");
    else {
      sendTo("WizLock is now on.\n\r");
      vlogf(LOG_MISC, fmt("WizLock was turned on by %s.") %  getName());
      WizLock = TRUE;
    }
  } else if (!strcmp(buf, "off")) {
    if (!WizLock)
      sendTo("It's already off!\n\r");
    else {
      sendTo("WizLock is now off.\n\r");
      vlogf(LOG_MISC, fmt("WizLock was turned off by %s.") %  getName());
      WizLock = FALSE;
    }
  } else if (!strcmp(buf, "add")) {
    argument = one_argument(argument, buf);
    if (!*buf) {
      sendTo("Wizlock add <host_name>\n\r");
      return;
    }
    length = (int) strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      sendTo("Host is too long or short, please try again.\n\r");
      return;
    }
    for (a = 0; a <= numberhosts - 1; a++) {
      if (!strncmp(hostlist[a], buf, length)) {
    sendTo("Host is already in database.\n\r");
    return;
      }
    }
    strcpy(hostlist[numberhosts], buf);
    vlogf(LOG_MISC, fmt("%s has added host %s to the access denied list.") %  getName() % hostlist[numberhosts]);
    numberhosts++;
    return;
  } else if (!strcmp(buf, "rem")) {
    if (numberhosts <= 0) {
      sendTo("Host list is empty.\n\r");
      return;
    }
    argument = one_argument(argument, buf);

    if (!*buf) {
      sendTo("Wizlock rem <host_name>\n\r");
      return;
    }
    length = (int) strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      sendTo("Host length is bad, please try again.\n\r");
      return;
    }
    for (a = 0; a <= numberhosts - 1; a++) {
      if (!strncmp(hostlist[a], buf, length)) {
    for (b = a; b <= numberhosts; b++)
      strcpy(hostlist[b], hostlist[b + 1]);
    vlogf(LOG_MISC, fmt("%s has removed host %s from the access denied list.") %  getName() % buf);
    numberhosts--;
    return;
      }
    }
    sendTo("Host is not in database.\n\r");
    return;
  } else if (!strcmp(buf, "list")) {
    sendTo(fmt("Global wizlock is presently %s.\n\r") % (WizLock ? "ON" : "OFF"));
    if (numberhosts <= 0) {
      sendTo("Host list is empty.\n\r");
      return;
    }
    for (a = 0; a <= numberhosts - 1; a++)
      sendTo(fmt("Host: %s\n\r") % hostlist[a]);

    return;
  } else if (is_abbrev(buf, "message")) {
    TThing *t_note = searchLinkedListVis(this, "note", getStuff());
    note = dynamic_cast<TObj *>(t_note);
    if (note) {
      if (!note->action_description) {
        sendTo("Your note has no message for the new lockmess!\n\r");
        return;
      } else {
        lockmess = note->action_description;
        vlogf(LOG_MISC, fmt("%s added a wizlock message.") %  getName());
        sendTo("The wizlock message is now:\n\r");
        sendTo(lockmess);
        return;
      }
    } else {
      sendTo("You need a note with what you want the message to be in you inventory.\n\r");
      return;
    }
  } else {
    sendTo("Wizlock {all | add <host> | rem <host> | list}\n\r");
    sendTo(fmt("Global wizlock is presently %s.\n\r") % (WizLock ? "ON" : "OFF"));
    sendTo("The wizlock message is currently:\n\r");
    sendTo(lockmess);
    return;
  }
  return;
}

// returns DELETE_THIS if this should go
int TBeing::doEmote(const sstring &argument)
{
  sstring buf, tmpbuf;
  TThing *t, *t2;

  if (checkSoundproof())
    return FALSE;

  if (!awake() && !isPc())
    return FALSE;

  if (isPlayerAction(PLR_GODNOSHOUT)) {
    sendTo("You have been sanctioned by the gods and can't emote!!\n\r");
    return FALSE;
  }
  if (getCond(DRUNK) > plotStat(STAT_CURRENT, STAT_CON, 0, 9, 6)) {
    act("You're way too drunk to attempt that.", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }

  if (argument.empty())
    sendTo("Yes.. But what?\n\r");
  else {
    buf = fmt("$n %s<z>") % argument;
    tmpbuf = fmt("%s") % nameColorString(this, desc, buf, NULL, COLOR_BASIC, FALSE);
    act(tmpbuf, TRUE, this, 0, 0, TO_CHAR);
    for (t = roomp->getStuff(); t ; t = t2) {
      t2 = t->nextThing;
      TBeing *ch = dynamic_cast<TBeing *>(t);
      if (!ch || ch == this)
        continue;
      if (roomp && ch->desc && 
                      (ch->canSee(this)) && ch->awake() && 
                      (ch->desc->connected <= 20) && 
                      !(ch->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {
        tmpbuf = fmt("%s") % nameColorString(ch, ch->desc, buf, NULL, COLOR_COMM, FALSE);
        act(tmpbuf, TRUE, this, 0, ch, TO_VICT);
      }
#if 0
// Commented out..cosmo..we dont break it for say we shouldnt for emote
// cept the caster shouldnt be able to
      disturbMeditation(ch);
#endif

#if 0
// commented out since polymorph isn't overused/abused
      TMonster *tmons = dynamic_cast<TMonster *>(ch);
      if (tmons && IS_SET(specials.act, ACT_POLYSELF)) {
        if ((tmons->getPosition() <= POSITION_SLEEPING) || !tmons->canSee(this))
          continue;
        if (!::number(0, tmons->plotStat(STAT_CURRENT, STAT_PER, 18, 2, 8))) {
          if (::number(0,1)) {
            act("$n realizes $N is not a mob.",TRUE, tmons,0,this,TO_NOTVICT);
            act("$n realizes you are not a mob.  You are forced to return.",
                TRUE,tmons,0,this,TO_VICT);
            doReturn("", WEAR_NOWHERE, 1);
            break;
          }
        }
        tmons->aiSay(this, 0);
      }
#endif
    }
  }
  return TRUE;
}

void TBeing::doFlag(const char *argument)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  TBeing *victim;

  // sanity check
  if (!hasWizPower(POWER_FLAG) || !dynamic_cast<TPerson *>(this)) {
    incorrectCommand();
    return;
  }
  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sendTo("Flag whom? Flag what?\n\r");
    return;
  }
  if (!(victim = get_pc_world(this, buf, EXACT_YES)) && 
      !(victim = get_pc_world(this, buf, EXACT_NO)))
    sendTo("No one by that name on!\n\r");
  else if (victim->GetMaxLevel() > GetMaxLevel()) {
    sendTo("Sorry you can't flag someone higher than you.\n\r");
    return;
  } else if (!victim->isPc()) {
    sendTo("Flag an NPC?\n\r");
    return;
  } else if (is_abbrev(buf2, "newbiehelper") ||
             is_abbrev(buf2, "helper")) {
    if (victim->isPlayerAction(PLR_NEWBIEHELP)) {
      victim->remPlayerAction(PLR_NEWBIEHELP);
      act("You just removed $N's newbie-helper flag",
                 FALSE, this, 0, victim, TO_CHAR);
    } else {
      victim->addPlayerAction(PLR_NEWBIEHELP);
      act("You just set $N's newbie-helper flag.",
              FALSE, this, 0, victim, TO_CHAR);
    }
  } else if (is_abbrev(buf2, "killable")) {
    if (!victim->isPlayerAction(PLR_KILLABLE)) {
      victim->addPlayerAction(PLR_KILLABLE);
      act("$N is now killable even though a newbie.", FALSE, this, 0, victim, TO_CHAR);
    } else {
      victim->remPlayerAction(PLR_KILLABLE);
      act("$N is no longer killable.", FALSE, this, 0, victim,
TO_CHAR);
    }
  } else if (is_abbrev(buf2, "banished")) {
    if (victim->isPlayerAction(PLR_BANISHED)) {
      victim->remPlayerAction(PLR_BANISHED);
      act("You just removed $N's banished flag", FALSE, this, 0, victim, TO_CHAR);
      victim->sendTo("Your banishment flag has been removed.\n\r");
      vlogf(LOG_MISC, fmt("%s removed %s's banish flag.") %  getName() % victim->getName());
    } else {
      victim->addPlayerAction(PLR_BANISHED);
      act("You just set $N's banished flag.", FALSE, this, 0, victim, TO_CHAR);
      vlogf(LOG_MISC, fmt("%s banished %s.") %  getName() % victim->getName());
      victim->sendTo("Your banishment flag has just been activated!\n\r");
    }
  } else if (is_abbrev(buf2, "solo")) {
    // check for quest stuff first
    if ((victim->hasQuestBit(TOG_AVENGER_HUNTING) ||
         victim->hasQuestBit(TOG_AVENGER_SOLO) ||
         victim->hasQuestBit(TOG_AVENGER_CHEAT) ||
         victim->hasQuestBit(TOG_AVENGER_PENANCED) ||
         victim->hasQuestBit(TOG_VINDICATOR_HUNTING_1) ||
         victim->hasQuestBit(TOG_VINDICATOR_SOLO_1) ||
         victim->hasQuestBit(TOG_VINDICATOR_CHEAT) ||
         victim->hasQuestBit(TOG_VINDICATOR_SEEK_PHOENIX) ||
         victim->hasQuestBit(TOG_VINDICATOR_HUNTING_2) ||
         victim->hasQuestBit(TOG_VINDICATOR_SOLO_2) ||
         victim->hasQuestBit(TOG_VINDICATOR_CHEAT_2) ||
         victim->hasQuestBit(TOG_VINDICATOR_DISHONOR) ||
         victim->hasQuestBit(TOG_VINDICATOR_ON_PENANCE) ||
         victim->hasQuestBit(TOG_VINDICATOR_GOT_BOOK) ||
         victim->hasQuestBit(TOG_VINDICATOR_PURIFIED) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_STARTED) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED1) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED2) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED3) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_LEPER_KILLED4) ||
         victim->hasQuestBit(TOG_MONK_PURPLE_FINISHED) 
        ) &&
           victim->isPlayerAction(PLR_SOLOQUEST)) {
      sendTo("NOTICE: that person is currently involved in a quest.\n\r");
      sendTo("Doing this will allow them to get help on the quest and is probably a bad idea.\n\r");
#if 0
      sendTo("Flag them again if you want to undo what you just did.\n\r");
#else
      sendTo("Flagging aborted.\n\r");
      return;
#endif
    } else if (victim->isPlayerAction(PLR_SOLOQUEST)) {
      victim->remPlayerAction(PLR_SOLOQUEST);
      act("You just removed $N's solo quest flag.", FALSE, this, 0, victim, TO_CHAR);
      act("$n just removed your solo quest flag.", FALSE, this, NULL, victim, TO_VICT);
    } else {
      victim->addPlayerAction(PLR_SOLOQUEST);
      act("You just set $N's solo quest flag.", FALSE, this, 0, victim, TO_CHAR);
      act("$n just set your solo quest flag.", FALSE, this, 0, victim, TO_VICT);
    }
  } else if (is_abbrev(buf2, "group")) {
    if (victim->isPlayerAction(PLR_GRPQUEST)) {
      victim->remPlayerAction(PLR_GRPQUEST);
      act("You just removed $N's group quest flag", FALSE, this, 0, victim, TO_CHAR);
      act("$n just removed your group quest flag.", FALSE, this, NULL, victim, TO_VICT);
    } else {
      victim->addPlayerAction(PLR_GRPQUEST);
      act("You just set $N's group quest flag.", FALSE, this, 0, victim, TO_CHAR);
      act("$n just set your group quest flag.", FALSE, this, 0, victim, TO_VICT);
      victim->dieFollower();
    }
  } else if (is_abbrev(buf2, "nosnoop")) {
    if (powerCheck(POWER_FLAG_IMP_POWER))
      return;

    if (victim->isPlayerAction(PLR_NOSNOOP)) {
      victim->remPlayerAction(PLR_NOSNOOP);
      act("$N is now set snoopable.", FALSE, this, 0, victim, TO_CHAR);
    } else {
      victim->addPlayerAction(PLR_NOSNOOP);
      act("$N can no longer be snooped.", FALSE, this, 0, victim, TO_CHAR);
    }
  } else if (is_abbrev(buf2, "faction")) {
    if (powerCheck(POWER_FLAG_IMP_POWER))
      return;

    if (!victim->desc) {
      sendTo("You cannot flag them for this.\n\r");
      return;
    }
    if (victim->hasQuestBit(TOG_FACTIONS_ELIGIBLE)) {
      if (victim->isUnaff()) {
        victim->remQuestBit(TOG_FACTIONS_ELIGIBLE);
        act("$N can no longer join a faction.",
            FALSE, this, 0, victim, TO_CHAR);
      } else
        act("$N is already of a faction, you cannot pull there faction ability.",
            FALSE, this, 0, victim, TO_CHAR);
    } else {
      if (victim->isUnaff()) {
        victim->setQuestBit(TOG_FACTIONS_ELIGIBLE);
        act("You grant $N the ability to join factions.",
            FALSE, this, 0, victim, TO_CHAR);
      } else
        act("$N is already of a faction, you cannot let them join another.",
            FALSE, this, 0, victim, TO_CHAR);
    }
  } else if (is_abbrev(buf2, "immortal")) {
    if (powerCheck(POWER_FLAG_IMP_POWER))
      return;

    if (!victim->desc) {
      sendTo("You can not flag them for this.\n\r");
      return;
    }

    if (victim->GetMaxLevel() > MAX_MORT) {
      sendTo("That person is an immortal...Are you crazy??");
      return;
    }

    if (IS_SET(victim->desc->account->flags, ACCOUNT_IMMORTAL)) {
      REMOVE_BIT(victim->desc->account->flags, ACCOUNT_IMMORTAL);
      act("$N is no longer flagged as an immortal.", false, this, 0, victim, TO_CHAR);
      victim->doSave(SILENT_YES);
    } else
      sendTo("This flag is setup Automatically.  You Can Not simply add it.\n\r");
  } else {
    sendTo("Syntax: flag <player> <\"banished\" | \"killable\" | \"solo\" | \"group\" | \"newbiehelper\" | \"nosnoop\" | \"double\" | \"triple\" | \"faction\">\n\r");
    return;
  }
}

void TBeing::doEcho(const char *argument)
{
  int i;
  char buf[256];

  if (powerCheck(POWER_ECHO))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (isImmortal()) {
    sprintf(buf, "%s<z>\n\r", argument + i);
    sendTo("Ok.\n\r");
    sendToRoom(COLOR_COMM, buf, in_room);
  }
}


int getSockReceiveBuffer(int s)
{
  int buf = 0;

#if defined(SOLARIS) || defined(SUN)
  int size;
  size = sizeof(buf);

  if (getsockopt((int) s, (int) SOL_SOCKET, (int) SO_RCVBUF, (char *) &buf, 
          (int *) &size))
    perror("getsockopt 1");
#elif defined(LINUX)
  unsigned int size;
  size = sizeof(buf);
  // JESUS
  if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &buf, (unsigned *) &size))
    perror("getsockopt 2");
#else
  int size;
  size = sizeof(buf);

  if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &buf, &size))
    perror("getsockopt 3");
#endif

  return buf;
}


int getSockSendBuffer(int s)
{
  int buf = 0;
#if defined(SOLARIS) || defined(SUN)
  int size;
  size = sizeof(buf);
  if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &buf, &size))
    perror("getsockopt 4");
#elif defined(LINUX)
  unsigned int size;
  size = sizeof(buf);
  if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &buf, &size))
    perror("getsockopt 5");
#else
  int size;
  size = sizeof(buf);
  if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &buf, &size))
    perror("getsockopt 6");
#endif
  return buf;
}

const char *getSockOptString(int s, int opt)
{
  int size;
  struct linger ld;
  int result;

  if (opt == SO_LINGER) {
    size = sizeof(ld);
    ld.l_onoff = -1;        // serious error checking 

#if defined(SOLARIS) || defined(SUN)
    if (getsockopt(s, SOL_SOCKET, opt, (char *) &ld, &size) == -1) {
#elif defined(LINUX)
    if (getsockopt(s, SOL_SOCKET, opt, &ld, (unsigned *) &size) == -1) {
#else
    if (getsockopt(s, SOL_SOCKET, opt, &ld, &size) == -1) {
#endif
      perror("getsockopt 5");
      return "Test Failed";
    }
    switch (ld.l_onoff) {
      case TRUE:
    return "On";
      case FALSE:
    return "Off";
      default:
    return "Bad result";
    }
  }
  size = sizeof(result);
#if defined(SOLARIS) || defined(SUN)
  if (getsockopt(s, SOL_SOCKET, opt, (char *) &result, &size) == -1) {
#elif defined(LINUX)
  if (getsockopt(s, SOL_SOCKET, opt, (char *) &result, (unsigned *) &size) == -1) {
#else
  if (getsockopt(s, SOL_SOCKET, opt, &result, &size) == -1) {
#endif
    perror("getsockopt 6");
    return "Test Failed";
  }

  switch (opt) {
    case SO_DEBUG:
    case SO_DONTROUTE:
      if (result)
    return "On";
      else
    return "Off";
    case SO_BROADCAST:
    case SO_REUSEADDR:
    case SO_KEEPALIVE:
    case SO_OOBINLINE:
      if (result)
    return "Yes";
      else
    return "No";
    default:
      return "Bad option";
  }
}


void TBeing::doSystem(const sstring &argument)
{
  sstring buf;

  if (powerCheck(POWER_SYSTEM))
    return;

  if(argument.empty()){
    sendTo("You usually system something.\n\r");
    return;
  } else if (!hasWizPower(POWER_WIZARD)) {
    buf=fmt("The following is an official message from %s:\n\r   %s\n\r") %
      getName() % argument;
    descriptor_list->worldSend(buf, this);
  } else {
    buf=fmt("%s\n\r") % argument;
    descriptor_list->worldSend(buf, this);
  } 
}

void TBeing::doTrans(const char *)
{
  sendTo("Mobs can't trans.\n\r");
}

void TPerson::doTrans(const char *argument)
{
  Descriptor *i;
  TBeing *victim;
  char buf[100];
  TThing *t;

  if (powerCheck(POWER_TRANSFER))
    return;

  // this guy is needed, roomp is not guaranteed valid if my victim (or riding)
  // causes me to be one of the trans victims.
  TRoom *rp = roomp;

  strcpy(buf, argument);
  if (!*buf)
    sendTo("Whom do you wish to transfer?\n\r");
  else if (strcmp("all", buf)) {
    if (!(victim = get_pc_world(this, buf, EXACT_YES)) &&
        !(victim = get_pc_world(this, buf, EXACT_NO)) &&
        !(victim = get_char_vis_world(this, buf, NULL, EXACT_YES)) &&
        !(victim = get_char_vis_world(this, buf, NULL, EXACT_NO))) {
      sendTo("No-one by that name around.\n\r");
    } else {
      if (victim->desc && (victim->desc->connected >= MAX_CON_STATUS)) {
        sendTo("You cannot transfer a god who is editing!\n\r");
        return;
      }
      if (victim->hasWizPower(POWER_WIZARD) && victim->isPc()) {
        sendTo("You can't trans someone that powerful!\n\r");
        victim->sendTo(COLOR_MOBS, fmt("%s just tried to transfer you!\n\r") % getName());
        return;
      }
      if (!victim->isPc() && victim->number == -1 && 
          !hasWizPower(POWER_MEDIT_LOAD_ANYWHERE)) {
        sendTo("You lack the power to load anywhere, therefore you can't transfer MEdit Mobs.\n\r");
        return;
      }
      if(!limitPowerCheck(CMD_TRANSFER,(victim->isPc()) ? -1 : victim->number)) {
	sendTo("You are not allowed to transfer that being.\n\r");
	return;
      }
      // used to track down: XXX trans'd me outta inn and got me killed!
      vlogf(LOG_SILENT, fmt("%s transferring %s from %d to %d.") %  getName() % victim->getName() % victim->inRoom() % inRoom());

      act("$n disappears in a cloud of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      --(*victim);
      if (victim->riding) {
        --(*(victim->riding));
        *rp += *(victim->riding);
      }
      for (t = victim->rider; t; t = t->nextRider) {
        --(*t);
        *rp += *t;
      }
      *rp += *victim;
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, this, 0, victim, TO_VICT);
      victim->doLook("", CMD_LOOK);
      sendTo("Ok.\n\r");
    }
  } else {            // Trans All 
    for (i = descriptor_list; i; i = i->next) {
      if (i->character != this && !i->connected) {
    victim = i->character;
    act("$n disappears in a cloud of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    --(*victim);
        *rp += *victim;
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has transferred you!", FALSE, this, 0, victim, TO_VICT);
    victim->doLook("", CMD_LOOK);
      }
    }
    sendTo("Ok.\n\r");
  }
}

int TBeing::doAt(const char *, bool)
{
  sendTo("Mob's can't use this command.\n\r");
  return FALSE;
}

// Add rooms here that should require a forceful entry.
// Either by spam or some other reason.
static bool isSpammyRoom(int tRoom)
{
  switch (tRoom) {
    case ROOM_NOCTURNAL_STORAGE:
      return TRUE;
  }

  return FALSE;
}

bool isInt(const sstring &s)
{
  int x;
  istringstream is(s);
  if(!(is >> x)){
    return false;
  } else {
    return true;
  }
}


// returns DELETE_THIS if this dies
int TPerson::doAt(const char *argument, bool isFarlook)
{
  char com_buf[256], loc[256];
  int loc_nr, location, original_loc;
  TBeing *mob;
  TObj *obj;
  int rc;

  if (!isFarlook && powerCheck(POWER_AT))
    return FALSE;

  half_chop(argument, loc, com_buf);
  if (!*loc) {
    sendTo("You must supply a room number or a name.\n\r");
    return FALSE;
  }
  if ((isInt(loc) && !strchr(loc, '.')) ||
      *loc == '0') {
    // this latter case is for "at 0 look"
    loc_nr = convertTo<int>(loc);
    if (!real_roomp(loc_nr)) {
      sendTo("No room exists with that number.\n\r");
      return FALSE;
    }
    location = loc_nr;
  } else if ((mob = get_pc_world(this, loc, EXACT_YES)) ||
             (mob = get_pc_world(this, loc, EXACT_NO)) ||
             (mob = get_char_vis_world(this, loc, NULL, EXACT_YES)) ||
             (mob = get_char_vis_world(this, loc, NULL, EXACT_NO))) {
    location = mob->in_room;
  } else if ((obj = get_obj_vis_world(this, loc, NULL, EXACT_YES)) ||
             (obj = get_obj_vis_world(this, loc, NULL, EXACT_NO))) {
    if (obj->in_room != ROOM_NOWHERE)
      location = obj->in_room;
    else {
      sendTo("The object is not available.\n\r");
      return FALSE;
    }
  } else {
    sendTo("No such creature or object around.\n\r");
    return FALSE;
  }

  // if they are in a casino now, prevent
  if ((checkBlackjack(true) && gBj.index(this) >= 0) ||
      (gGin.check(this) && gGin.index(this) >= 0) || 
      (checkCrazyEights() && gEights.index(this) >= 0) ||
      (checkDrawPoker() && gDrawPoker.index(this) >= 0) ||
      (checkHearts(true) && gHearts.index(this) >= 0)) {
    sendTo("You can't do that while in a casino game.\n\r");
    return FALSE;
  }

  if (isSpammyRoom(location)) {
    sstring tStArgument(com_buf),
           tStString(""),
           tStBuffer("");

    tStString=tStArgument.word(0);
    tStBuffer=tStArgument.word(1);

    if (!is_abbrev(tStString, "yes")) {
      sendTo("That room, or the creature's room you chose, is a particular room.\n\r");
      sendTo(fmt("To do this, do this: at %s yes %s %s %s\n\r") % 
             loc % tStString % tStBuffer % tStArgument);
      return FALSE;
    }

    strcpy(com_buf, tStBuffer.c_str());
    strcat(com_buf, tStArgument.c_str());
  }

  if (location == ROOM_STORAGE && !hasWizPower(POWER_GOTO_IMP_POWER)) {
    sendTo("You are forbidden to do this.  Sorry.\n\r");
    return FALSE;
  }

  if (hasWizPower(POWER_IDLED) && real_roomp(location) &&
      !real_roomp(location)->inImperia()) {
    sendTo("I'm afraid you can not do this right now.\n\r");
    return FALSE;
  }

  original_loc = in_room;
  --(*this);
  if (riding) {
    --(*riding);
     thing_to_room(riding, location);
  }
  thing_to_room(this, location);
  rc = parseCommand(com_buf, FALSE);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // It's possible the at put them into a game, so take them out if so
  removeAllCasinoGames();

  --(*this);
  if (riding) {
    --(*riding);
     thing_to_room(riding, original_loc);
  }
  thing_to_room(this, original_loc);
  return FALSE;
}

// returns DELETE_THIS if died
int TBeing::doGoto(const sstring & argument)
{
  followData *k, *n;
  int loc_nr, was_in = inRoom(), location, i;
  TBeing *target_mob;//, *v = NULL;
  TThing *t;
  TObj *target_obj;
  TRoom *rp2;
  int rc;

  if (!roomp) {
    vlogf(LOG_MISC, "Character in invalid room in doGoto!");
    return FALSE;
  }

  if (!isImmortal()) 
    return doMortalGoto(argument);

  if (powerCheck(POWER_GOTO))
    return FALSE;

  sstring buf,
         tStString;

  buf=argument.word(0);
  tStString=argument.word(1);

  if (buf.empty()) {
    char tString[10];

    sprintf(tString, "%d", (desc ? desc->office : ROOM_IMPERIA));
    buf = tString;
  } else if (buf == "help" && desc) {
    sendTo("Useful rooms:\n\r____________________\n\r");
    sendTo(fmt("         %6d: Your Office\n\r") % desc->office);

    if (desc->blockastart)
      sendTo(fmt("%6d - %6d: Your Primary Room Block\n\r") % desc->blockastart % desc->blockaend);

    if (desc->blockbstart)
      sendTo(fmt("%6d - %6d: Your Secondary Room Block\n\r") % desc->blockbstart % desc->blockbend);

    sendTo(fmt("         %6d: Wizard Board\n\r") % ROOM_IMPERIA);
    sendTo("              9: Lab\n\r");
    sendTo("              2: Builder's Lounge/Board\n\r");
    sendTo("              8: Reimbursement/Enforcement Board\n\r");
    sendTo(fmt("         %6d: Center Square\n\r") % ROOM_CS);
    sendTo("             77: Slap on the Wrist (First Offense)\n\r");
    sendTo("             70: Hell\n\r");
    sendTo("             81: Conference Room\n\r");
    sendTo("            557: Roaring Lion Inn\n\r");

    return FALSE;
  }

  if (isdigit(*buf.c_str()) && buf.find('.') == sstring::npos) {
    loc_nr = convertTo<int>(buf);
    if (NULL == real_roomp(loc_nr)) {
      if (loc_nr < 0) {
        sendTo("No room exists with that number.\n\r");
        return FALSE;
      } else {
	if (!limitPowerCheck(CMD_GOTO, loc_nr)) {
	  sendTo("You are currently forbidden from going there.\n\r");
	  return FALSE;
	} else if (loc_nr < WORLD_SIZE) {
          sendTo("You form order out of chaos.\n\r");
          CreateOneRoom(loc_nr);
        } else {
          sendTo("Sorry, that room # is too large.\n\r");
          return FALSE;
        }
      }
    }
    location = loc_nr;
  } else if ((target_mob = get_pc_world(this, buf, EXACT_NO)) != NULL) {
    location = target_mob->in_room;
  } else if ((target_mob = get_char_vis_world(this, buf, NULL, EXACT_NO)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis_world(this, buf.c_str(), NULL, EXACT_YES)) ||
           (target_obj = get_obj_vis_world(this, buf.c_str(), NULL, EXACT_NO))) {
    if (target_obj->in_room != ROOM_NOWHERE)
      location = target_obj->in_room;
    else {
      sendTo("The object is not available.\n\r");
      sendTo("Try where #.object to nail its room number.\n\r");
      return FALSE;
    }
  } else {
    sendTo("No such creature or object around.\n\r");
    return FALSE;
  }
  // a location has been found.
  if (!limitPowerCheck(CMD_GOTO, location)) {
    sendTo("You are currently forbidden from going there.\n\r");
    return FALSE;
  }

  if (isSpammyRoom(location) && !is_abbrev(tStString, "yes")) {
    sendTo(fmt("To enter this particular room you must do: goto %d yes\n\r") % location);
    return FALSE;
  }

  if (location == ROOM_STORAGE && !hasWizPower(POWER_GOTO_IMP_POWER)) {
    sendTo("You are forbidden to do this.  Sorry.\n\r");
    return FALSE;
  }

  if (!(rp2 = real_roomp(location))) {
    vlogf(LOG_MISC, "Invalid room in doGoto!");
    return FALSE;
  }
  if (!hasWizPower(POWER_GOTO_IMP_POWER) &&
      real_roomp(location)->isRoomFlag(ROOM_PRIVATE)) {
    for (i = 0, t = real_roomp(location)->getStuff(); t; t = t->nextThing)
      if (dynamic_cast<TBeing *>(t))
        i++;

    if (i > 1) {
      sendTo("There's a private conversation going on in that room.\n\r");
      return FALSE;
    }
  }

  bool hasStealth = (desc ? isPlayerAction(PLR_STEALTH) : false);

  if (msgVariables(MSG_BAMFOUT)[0] != '!') {
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(t);

      if (tbt && this != tbt && (!hasStealth || tbt->GetMaxLevel() > MAX_MORT)) {
        sstring s = nameColorString(this, tbt->desc, msgVariables(MSG_BAMFOUT, (TThing *)NULL, (const char *)NULL, false).c_str(), NULL, COLOR_BASIC, false);
        act(s, TRUE, this, 0, tbt, TO_VICT);
      }
    }
  } else if ((rc = parseCommand((msgVariables(MSG_BAMFOUT).c_str()) + 1, FALSE)) == DELETE_THIS)
    return DELETE_THIS;

#if 0
  if (!desc || !desc->poof.poofout)
    act("$n disappears in a cloud of mushrooms.",
        TRUE, this, NULL, NULL, TO_ROOM, NULL,  (hasStealth ? MAX_MORT : 0));
  else if (*desc->poof.poofout != '!') {
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(t);

      if (tbt && this != tbt && (!hasStealth || tbt->GetMaxLevel() > MAX_MORT)) {
        sstring s = nameColorString(tbt, tbt->desc, desc->poof.poofout, NULL, COLOR_BASIC, TRUE);
        act(s, TRUE, this, 0, tbt, TO_VICT);
      }
    }
  } else {
    if ((rc = parseCommand((desc->poof.poofout + 1), FALSE)) == DELETE_THIS)
      return DELETE_THIS;
  }
#endif

  if (fight())
    stopFighting();

  // this is mostly here for blackjack, but what the heck...
  removeAllCasinoGames();

  --(*this);

  if (riding) {
    --(*riding);
    *rp2 += *riding;
  }
  *rp2 += *this;

  if (!riding && !isFlying())
    setPosition(POSITION_STANDING);

  hasStealth = (desc ? isPlayerAction(PLR_STEALTH) : false);

  if (msgVariables(MSG_BAMFIN)[0] != '!') {
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(t);

      if (tbt && this != tbt && (!hasStealth || tbt->GetMaxLevel() > MAX_MORT)) {
        sstring s = nameColorString(this, tbt->desc, msgVariables(MSG_BAMFIN, (TThing *)NULL, (const char *)NULL, false).c_str(), NULL, COLOR_BASIC, false);
        act(s, TRUE, this, 0, tbt, TO_VICT);
      }
    }
  } else if ((rc = parseCommand((msgVariables(MSG_BAMFIN).c_str()) + 1, FALSE)) == DELETE_THIS)
    return DELETE_THIS;

#if 0
  if (!desc || !desc->poof.poofin) {
    act("$n appears with an explosion of rose-petals.",
        TRUE, this, 0, v, TO_ROOM, NULL, (hasStealth ? MAX_MORT : 0));
    *roomp += *read_object(OBJ_ROSEPETAL, VIRTUAL);
  } else if (*desc->poof.poofin != '!') {
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      TBeing *tbt = dynamic_cast<TBeing *>(t);

      if (tbt && this != tbt && (!hasStealth || tbt->GetMaxLevel() > MAX_MORT)) {
        sstring s = nameColorString(tbt, tbt->desc, desc->poof.poofin, NULL, COLOR_BASIC, TRUE);
        act(s, TRUE, this, 0, tbt, TO_VICT);
      }
    }
  } else {
    if ((rc = parseCommand((desc->poof.poofin + 1), FALSE)) == DELETE_THIS)
      return DELETE_THIS;
  }
#endif

  doLook("", CMD_LOOK);
  if (riding) {
    rc = riding->genericMovedIntoRoom(rp2, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete riding;
      riding = NULL;
    }
  } else {
    rc = genericMovedIntoRoom(rp2, was_in);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  for (k = followers; k; k = n) {
    n = k->next;
    if (k->follower->isImmortal() && k->follower->inRoom() == was_in) {
      act("You follow $N.", FALSE, k->follower, 0, this, TO_CHAR);
      rc = k->follower->doGoto(fname(name));
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete k->follower;
        k->follower = NULL;
      }
    }
  }
  return FALSE;
}

void TBeing::doShutdow()
{
  sendTo("Mobs shouldn't be shutting down.\n\r");
}

void TPerson::doShutdow()
{
  sendTo("If you want to shut something down - say so!\n\r");
}

void TBeing::doShutdown(const char *)
{
  sendTo("Mobs shouldn't be shutting down.\n\r");
}

sstring shutdown_or_reboot()
{
  int rc = system("ps aux | grep reboot | grep -v grep");
 
  // if there is an entry, this will return 0
  if (rc == 0)
    return "Reboot";
  else
    return "Shutdown";
}
 
void TPerson::doShutdown(const char *argument)
{
  char buf[1000], arg[256];
  int num;

  if (powerCheck(POWER_SHUTDOWN))
    return;

  argument = one_argument(argument, arg);

  if (!*arg) {
    if (gamePort == PROD_GAMEPORT || gamePort == BUILDER_GAMEPORT) {
      // oops, did we type shutdown in the wrong window again???
      sendTo(fmt("Running on game port (%d) or builder port (%d).\n\r") 
          % PROD_GAMEPORT % BUILDER_GAMEPORT);
      sendTo("Please do a timed shutdown to avoid complaints.\n\r");
      return;
    }
    sprintf(buf, "<r>%s by %s.<z>\n\r", shutdown_or_reboot().c_str(), getName());
    descriptor_list->worldSend(buf, this);
    Shutdown = 1;
  } else {
    if (isdigit(*arg)) {
      num = convertTo<int>(arg);
      if (num <= 0) {
        sendTo("Illegal number of minutes.\n\r");
        sendTo("Syntax : shutdown <minutes until shutdown>\n\r");
        return;
      } 
      if (!timeTill)
        timeTill = time(0) + (num * SECS_PER_REAL_MIN);
      else if (timeTill < (time(0) + (num * SECS_PER_REAL_MIN))) {
        sendTo(fmt("A shutdown has already been scheduled for %d minutes.\n\r") %
               ((timeTill - time(0))/SECS_PER_REAL_MIN));
        return;
      } else {
        timeTill = time(0) + (num * SECS_PER_REAL_MIN);
      }
      sprintf(buf, "<r>******* SYSTEM MESSAGE *******\n\r%s in %d minute%s by %s.<z>\n\r<c>Use the TIME command at any point to see time until %s.<z>\n\r", 
       shutdown_or_reboot().c_str(), num, (num == 1 ? "" : "s"),getName(),
       shutdown_or_reboot().c_str());
      descriptor_list->worldSend(buf, this); 
    } else if (is_abbrev(arg, "abort")) {
      if (!timeTill) {
        sendTo("No shutdown has been scheduled.\n\r");
        return;
      }
      sprintf(buf, "<r>System %s aborted by %s.<z>\n\r", shutdown_or_reboot().c_str(), getName());
      descriptor_list->worldSend(buf, this);
      timeTill = 0L;
    } else {
      sendTo("Syntax : shutdown <minutes until shutdown>\n\r");
      return;
    }
  } 
}

void TBeing::doSnoop(const char *)
{
  sendTo("You're a mob.  No snooping.\n\r");
}

void TPerson::doSnoop(const char *argument)
{
  char arg[256];
  TBeing *victim;

  if (!desc)
    return;

  if (powerCheck(POWER_SNOOP))
    return;

  strcpy(arg, argument);

  if (!*arg) {
    sendTo("Snoop whom?\n\r");
    return;
  }

  // we use get_char here rather than get_pc so we can snoop switched imm's
  if (!(victim = get_pc_world(this, arg, EXACT_YES)) &&
      !(victim = get_pc_world(this, arg, EXACT_NO)) && 
      !(victim = get_char_vis_world(this, arg, NULL, EXACT_YES)) &&
      !(victim = get_char_vis_world(this, arg, NULL, EXACT_NO))) {
    sendTo("No such person around.\n\r");
    return;
  }
  if (!victim->desc) {
    sendTo("There's no link.. nothing to snoop.\n\r");
    return;
  }
  if (victim->isPlayerAction(PLR_NOSNOOP) && strcmp(getName(), victim->getName())) {
    sendTo("That person's nosnoop flag is set.\n\r");
    return;
  }
  if (victim == this) {
    sendTo("Ok, you just snoop yourself.\n\r");
    if (desc->snoop.snooping) {
      if (desc->snoop.snooping->desc)
    desc->snoop.snooping->desc->snoop.snoop_by = 0;
      else
    vlogf(LOG_MISC, fmt("Caught %s snooping %s who didn't have a descriptor!") %  name % desc->snoop.snooping->name);

      desc->snoop.snooping = 0;
    }
    return;
  }
  if (victim->desc->snoop.snoop_by) {
    sendTo("Busy already. \n\r");
    return;
  }
  if (victim->GetMaxLevel() >= GetMaxLevel()) {
    sendTo("You failed.\n\r");
    return;
  }
  if (victim->desc->original) {
    sendTo("Not good to snoop a switched person.\n\r");
    return;
  }
  sendTo("Ok. \n\r");

  if (desc->snoop.snooping)
    if (desc->snoop.snooping->desc)
      desc->snoop.snooping->desc->snoop.snoop_by = 0;

  desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = this;
  return;
}

void TBeing::doSwitch(const char *)
{
  sendTo("You're already a mob.  You'll have to return before you can switch.\n\r");
}

void TPerson::doSwitch(const char *argument)
{
  sstring  tStMobile(""),
          tStBuffer(""),
          tStArg(argument);
  TBeing *tBeing;
  bool    doLoadCmd = false;
  int     mobileIndex;

  TThing *switchProcObj = equipment[WEAR_NECK];
  bool hasSwiO = switchProcObj && switchProcObj->spec == 139;
  // see spec_objs.cc
  
  if ((!isImmortal() && !hasSwiO) || 
      (isImmortal()) && powerCheck(POWER_SWITCH))
  {
    sendTo(fmt("%sIncorrect%s command. Please see help files if you need assistance!\n\r") % red() % norm());
    return;
  }
  
  tStMobile=tStArg.word(0);
  tStBuffer=tStArg.word(1);

  doLoadCmd = is_abbrev(tStMobile, "load");

  if (tStMobile.empty() || (doLoadCmd && tStBuffer.empty())) {
    sendTo("Switch with whom?\n\r");
    return;
  }

  if (doLoadCmd) {
    for (mobileIndex = 0; mobileIndex < (signed int) mob_index.size(); mobileIndex++)
      if (isname(tStBuffer, mob_index[mobileIndex].name))
        break;

    if (mobileIndex >= (signed int) mob_index.size()) {
      sendTo("Could not find that mobile.  Sorry.\n\r");
      return;
    }

    if (mob_index[mobileIndex].spec == SPEC_SHOPKEEPER) {
      sendTo("You cannot load a shopkeeper this way.\n\r");
      return;
    }

    if (mob_index[mobileIndex].spec == SPEC_NEWBIE_EQUIPPER) {
      sendTo("You cannot load a newbieHelper this way.\n\r");
      return;
    }

    if (!(tBeing = read_mobile(mobileIndex, REAL))) {
      sendTo("Well, um, that doesn't seem to exist.  Sorry.\n\r");
      return;
    }

    if((isImmortal() && !limitPowerCheck(CMD_SWITCH, tBeing->number)) ||
        (!isImmortal() && !hasSwiO)) {
      sendTo("You're not allowed to switch/load that mobile.\n\r");
      return;
    }

    *roomp += *tBeing;
    (dynamic_cast<TMonster *>(tBeing))->oldRoom = inRoom();
    (dynamic_cast<TMonster *>(tBeing))->createWealth();

    tStMobile = tStBuffer;
  }

  if (!(tBeing = get_char_room(tStMobile, in_room))) {
    sendTo("No one in room with that name......searching world.\n\r");
    if (!(tBeing = get_char(tStMobile.c_str(), EXACT_YES)) &&
        !(tBeing = get_char(tStMobile.c_str(), EXACT_NO))) {
      sendTo("No one with that name found in world, sorry.\n\r");
      return;
    }
  }
  if((isImmortal() && !limitPowerCheck(CMD_SWITCH, tBeing->number)) ||
      (!isImmortal() && !hasSwiO)) {
    sendTo("You're not allowed to switch into that mobile.\n\r");
    return;
  }


  if (this == tBeing) {
    sendTo("Heh heh heh...we are jolly funny today, aren't we?\n\r");
    return;
  }
  if (!desc || desc->snoop.snoop_by || desc->snoop.snooping) {
    sendTo("Mixing snoop & switch is bad for your health.\n\r");
    return;
  }
  if (tBeing->desc || dynamic_cast<TPerson *>(tBeing)) {
    sendTo("You can't do that; the body is already in use!\n\r");
    return;
  }
  if (desc->original) {
    // implies they are switching, while already switched (as x switch)
    sendTo("You already seem to be switched.\n\r");
    return;
  }

  if (doLoadCmd)
    act(msgVariables(MSG_SWITCH_TARG, tBeing),
        FALSE, this, NULL, NULL, TO_ROOM);

  sendTo("Ok.\n\r");

  polyed = POLY_TYPE_SWITCH;

  desc->character = tBeing;
  desc->original = this;

  tBeing->desc = desc;
  desc = NULL;
}

// This function will set all limbs to affect and call an equipment drop functin

void TBeing::makeLimbTransformed(TBeing * victim, wearSlotT limb, bool paired)
{
  wearSlotT otherLimb;

  if (paired) {
    switch (limb) {
      case WEAR_ARM_R:
      case WEAR_ARM_L: 
        otherLimb = WEAR_ARM_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_ARM_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_HAND_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_HAND_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FINGER_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FINGER_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = HOLD_RIGHT;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = HOLD_LEFT;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_WRIST_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_WRIST_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_HAND_R:
      case WEAR_HAND_L:
        otherLimb = WEAR_HAND_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_HAND_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FINGER_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FINGER_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = HOLD_RIGHT;
          doTransformDrop(otherLimb);
        otherLimb = HOLD_RIGHT;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = HOLD_LEFT;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_LEGS_R:
      case WEAR_LEGS_L:
        otherLimb = WEAR_LEGS_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_LEGS_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FOOT_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FOOT_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_FOOT_R:
      case WEAR_FOOT_L:
        otherLimb = WEAR_FOOT_R;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_FOOT_L;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_HEAD:
        otherLimb = WEAR_HEAD;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        otherLimb = WEAR_NECK;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_NECK:
        otherLimb = WEAR_NECK;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      case WEAR_WAISTE:
        otherLimb = WEAR_WAISTE;
          doTransformDrop(otherLimb);
          victim->addToLimbFlags(otherLimb, PART_TRANSFORMED);
        return;
      default:
        sendTo("That limb is currently not supported for paired transformation.\n\r");
        vlogf(LOG_MISC, "doTransformLimb called with bad case");
        return;
    }
  } else {
        sendTo("Single limb transformations are not currently supported.\n\r");
        vlogf(LOG_MISC, "doTransformLimb called a single limb transformation");
        return;
  }
}

void TBeing::doTransformDrop(wearSlotT slot)
{
  int dam;
  TObj *tmp = NULL;

  if (equipment[slot]) {
    tmp = dynamic_cast<TObj *>(equipment[slot]);
  }

  if (tmp) {
    if (slot == HOLD_RIGHT || slot == HOLD_LEFT) {
      act("You hear a small grinding sound coming from your new limb as something drops to the $g.", TRUE,this,0,0,TO_CHAR);
      act("You hear a grinding sound coming from $n and something drops to the $g.", TRUE,this,0,0,TO_ROOM);
      unequip(slot);
      *roomp += *tmp;
      return;
    } else {
      switch (::number(1,10)) {
        case 1:
          act("You hear a loud wrenching sound coming from your limbs.",
               TRUE,this,0,0,TO_CHAR);
          act("You hear a loud wrending sound coming from $n's limbs.",
               TRUE,this,0,0,TO_ROOM);
          tmp->makeScraps();
          delete tmp;
          tmp = NULL;
          return;
        default:
          dam = ::number(1,4);
          (tmp)->obj_flags.struct_points -= dam;
          if (tmp->obj_flags.struct_points <= 0) {
            act("You hear a loud wrenching sound coming from your limbs.",
                 TRUE,this,0,0,TO_CHAR);
            act("You hear a loud wrending sound coming from $n's limbs.",
                 TRUE,this,0,0,TO_ROOM);
            tmp->makeScraps();
            delete tmp;
            tmp = NULL;
          } else {
            act("You hear a small grinding sound coming from your new limb as something drops to the $g.", TRUE,this,0,0,TO_CHAR);
            act("You hear a grinding sound coming from $n and something drops to the $g.", TRUE,this,0,0,TO_ROOM);
	    unequip(slot);
            *roomp += *tmp;
          }
          return;
      }
    }
  } else {
  return;
  }
}


void TBeing::transformLimbsBack(const char * buffer, wearSlotT limb, bool cmd)
{
  bool found = FALSE;
  int X = LAST_TRANSFORM_LIMB;
  spellNumT spell = TYPE_UNDEFINED;
  wearSlotT slot;
  char argument[256];

  strcpy(argument, buffer);

// only do if a limb isn't specificed but one wants to read an armgument
  if (!limb) {
    while (!found) {
      if (is_abbrev(argument,TransformLimbList[X].name)) {
        limb = TransformLimbList[X].limb;
        spell = TransformLimbList[X].spell;
        found = TRUE;
        break;
      } else {
        X--;
        if (X < 0)
          break;
        continue;
      }
    }
  } else {
    if (limb == WEAR_HAND_R)
      spell = AFFECT_TRANSFORMED_HANDS;
    if (limb == WEAR_ARM_R)
      spell = AFFECT_TRANSFORMED_ARMS;
    if (limb == WEAR_LEGS_R)
      spell = AFFECT_TRANSFORMED_LEGS;
    if (limb == WEAR_HEAD)
      spell = AFFECT_TRANSFORMED_HEAD;
    if (limb == WEAR_NECK)
      spell = AFFECT_TRANSFORMED_NECK;
  }

  if (!hasTransformedLimb() && affectedBySpell(spell)) {
    sendTo("Please bug what you just did.\n\r");
    vlogf(LOG_MISC,"Somehow transformlimbback got sent a null limb");
  }

  if (!limb) {
    sendTo("Please bug what you just did.\n\r");
    vlogf(LOG_MISC,"Somehow transformlimbback got sent a null limb");
    return;
  } else if (limb == MAX_WEAR) {
    if (!hasTransformedLimb() &&
        !affectedBySpell(AFFECT_TRANSFORMED_HANDS) &&
        !affectedBySpell(AFFECT_TRANSFORMED_ARMS) &&
        !affectedBySpell(AFFECT_TRANSFORMED_LEGS) &&
        !affectedBySpell(AFFECT_TRANSFORMED_HEAD) &&
        !affectedBySpell(AFFECT_TRANSFORMED_NECK)) {
      if (cmd)
        sendTo("Your limbs are not transformed.\n\r");
      return;
    }
  } else {
    if (!isLimbFlags(limb, PART_TRANSFORMED) && 
        !affectedBySpell(spell)) {
      sendTo("Those limbs don't appear to be transformed.\n\r");
      return;
    }
  }

  found = 0;

  if (cmd) {
  }
  switch (limb) {
    case (WEAR_HAND_R):
      if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED) ||
          isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED) ||
          affectedBySpell(AFFECT_TRANSFORMED_ARMS)) {
        sendTo("You must use another keyworld to transform your limbs.\n\r");
        return;
      }
      slot = WEAR_FINGER_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_FINGER_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = HOLD_RIGHT;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = HOLD_LEFT;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_HAND_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_HAND_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      if (affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_HANDS);
      }
      break;
    case (WEAR_ARM_R):
      slot = WEAR_FINGER_R;
      if (isLimbFlags(slot, PART_TRANSFORMED)) 
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_FINGER_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = HOLD_RIGHT;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = HOLD_LEFT;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_ARM_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_ARM_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_WRIST_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_WRIST_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_HAND_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_HAND_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      if (affectedBySpell(AFFECT_TRANSFORMED_ARMS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_ARMS);
      }
      break;
    case (WEAR_LEGS_R):
      slot = WEAR_LEGS_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_LEGS_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_FOOT_R;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_FOOT_L;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      if (affectedBySpell(AFFECT_TRANSFORMED_LEGS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_LEGS);
      }
      break;
    case (WEAR_NECK):
      if (isLimbFlags(WEAR_HEAD, PART_TRANSFORMED) || affectedBySpell(AFFECT_TRANSFORMED_HEAD)) {
        sendTo("You must use another keyworld to transform your limbs\n\r");
        return;
      }
      slot = WEAR_NECK;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      if (affectedBySpell(AFFECT_TRANSFORMED_NECK)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_NECK);
      }
      break;
    case WEAR_HEAD:
      slot = WEAR_HEAD;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      slot = WEAR_NECK;
      if (isLimbFlags(slot, PART_TRANSFORMED))
        remLimbFlags(slot, PART_TRANSFORMED);
      if (affectedBySpell(AFFECT_TRANSFORMED_HEAD)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_HEAD);
      }
      break;
    case MAX_WEAR:
      for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
        if (!slotChance(slot))
          continue;
        if (isLimbFlags(slot, PART_TRANSFORMED)) {
          found = TRUE;
          if (isLimbFlags(slot, PART_MISSING))
           vlogf(LOG_MISC, fmt("%s has both a missing and a transformed limb") %  getName());
        remLimbFlags(slot, PART_TRANSFORMED);
        }
      }
      if (affectedBySpell(AFFECT_TRANSFORMED_HEAD)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_HEAD);
          if (affectedBySpell(AFFECT_TRANSFORMED_HEAD)) 
            affectFrom(AFFECT_TRANSFORMED_HEAD);
      }
      if (affectedBySpell(AFFECT_TRANSFORMED_NECK)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_NECK);
        if (affectedBySpell(AFFECT_TRANSFORMED_NECK))
          affectFrom(AFFECT_TRANSFORMED_NECK);
      }
      if (affectedBySpell(AFFECT_TRANSFORMED_ARMS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_ARMS);
        if (affectedBySpell(AFFECT_TRANSFORMED_ARMS))
          affectFrom(AFFECT_TRANSFORMED_ARMS);
      }
      if (affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_HANDS);
        if (affectedBySpell(AFFECT_TRANSFORMED_HANDS))
          affectFrom(AFFECT_TRANSFORMED_HANDS);
      }
      if (affectedBySpell(AFFECT_TRANSFORMED_LEGS)) {
        found = TRUE;
        affectFrom(AFFECT_TRANSFORMED_LEGS);
        if (affectedBySpell(AFFECT_TRANSFORMED_LEGS))
          affectFrom(AFFECT_TRANSFORMED_LEGS);
      }
      break;
    default:
      vlogf(LOG_BUG,"Bad limb number sent to transformLimbsBack");
      sendTo("Please bug what you just did or tell a god.\n\r");
      return;
  }
  if (found) {
    sendTo ("Your magic limbs transform back into their true form!\n\r");
    act("$n's magic limbs transform back into their true form.",
        TRUE, this, 0, 0, TO_ROOM);
  }
  return;
}

void TBeing::doReturn(const char * buffer, wearSlotT limb, bool tell, bool deleteMob)
{
  TBeing *mob = NULL, *per;
  int X = LAST_TRANSFORM_LIMB, found = FALSE;
  char argument[80];
  TRoom *rp = NULL;

  if(hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)){
    sendTo("You can't do that of your own free will!\n\r");
    return;
  }

  strcpy(argument, buffer);

  if (!limb && argument) {
    if (hasTransformedLimb()) {
      while (!found) {
        if (is_abbrev(argument,TransformLimbList[X].name)) {
          limb = TransformLimbList[X].limb;
          found = TRUE;
          break;
        } else {
          X--;
          if (X < 0)
            break;
          continue;
        }
      }
    }
  }


  if (limb) {
    transformLimbsBack("", limb, TRUE);
    return;
  }

  if (!desc || !desc->original) {
    sendTo("What are you trying to return from?!\n\r");
    return;
  } else {
    sendTo("You return to your original body.\n\r");

    per = desc->original;
    if ((specials.act & ACT_POLYSELF) && tell) {
      mob = this;

      act("$n turns liquid, and reforms as $N.", TRUE, mob, 0, per, TO_ROOM);

      --(*per);
      if (mob->roomp)
        *mob->roomp += *per;
      else {
        rp = real_roomp(ROOM_CS);
        *rp += *per;
      }

      SwitchStuff(mob, per);
      per->affectFrom(SPELL_POLYMORPH);
      per->affectFrom(SKILL_DISGUISE);
      per->affectFrom(SPELL_SHAPESHIFT);
    }

    desc->character = desc->original;
    desc->original = NULL;

    desc->character->desc = desc;
    desc = NULL;
    
    if (IS_SET(specials.act, ACT_POLYSELF) && tell && deleteMob ) {
      delete mob;
      mob = NULL;
    } else if (IS_SET(specials.act, ACT_POLYSELF)) { // move mob to room 72
      --(*mob);
      rp = real_roomp(ROOM_POLY_STORAGE);
      *rp += *mob;
    }
      
    per->polyed = POLY_TYPE_NONE;

    // POLYSELF idetifies a TMonster as an isPc
    // It would be bad to have the "desc = NULL" and still have isPc
    REMOVE_BIT(specials.act, ACT_POLYSELF);
  }
}


void TBeing::doForce(const char *)
{
  sendTo("Mobs can't force.\n\r");
}

void TPerson::doForce(const char *argument)
{
  Descriptor *i;
  TBeing *vict;
  char name_buf[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  int rc;
  TThing *t, *t2;

  if (powerCheck(POWER_FORCE))
    return;

  half_chop(argument, name_buf, to_force);

  if (!*name_buf || !*to_force)
    sendTo("Whom do you wish to force to do what?\n\r");
  else if (strcmp("all", name_buf) && 
           strcmp("mobs", name_buf) &&
           strcmp("room", name_buf)) {
    // force specific critter
    if (!(vict = get_char_room(name_buf, in_room))) {
      sendTo("No one in room with that name......searching world.\n\r");
      if (!(vict = get_char(name_buf, EXACT_YES)) &&
          !(vict = get_char(name_buf, EXACT_NO))) {
    sendTo("No one with that name found in world, sorry.\n\r");
    return;
      }
    }
    if (!limitPowerCheck(CMD_FORCE, (vict->isPc()) ? -1 : vict->number)) {
      sendTo("You are not allowed to force that being.\n\r");
      return;
    }
    if ((GetMaxLevel() <= vict->GetMaxLevel()) && dynamic_cast<TPerson *>(vict))
      sendTo("Oh no you don't!!\n\r");
    else {
      if (vict->canSee(this))
        vict->sendTo(COLOR_MOBS, fmt("%s\n\r") %
                     msgVariables(MSG_FORCE, (TThing *)NULL, to_force));
      else
        vict->sendTo(COLOR_MOBS, fmt("%s has forced you to '%s'.\n\r") %
                     vict->pers(this) % to_force);

      sendTo(COLOR_MOBS, fmt("You force %s to '%s'.\n\r") % vict->getName() %to_force);

      if ((rc = vict->parseCommand(to_force, FALSE)) == DELETE_THIS) {
        delete vict;
        vict = NULL;
      }
    }
  } else if (!strcmp("all", name_buf)) {
    // force all 
    vlogf(LOG_MISC, fmt("%s just forced all to '%s'") %  getName() % to_force);
    for (i = descriptor_list; i; i = i->next) {
      if ((vict = i->character) && (i->character != this) && !i->connected) {
        if ((GetMaxLevel() <= vict->GetMaxLevel()) && dynamic_cast<TPerson *>(vict))
          continue;
        else {
          sprintf(buf, "$n has forced you to '%s'.\n\r", to_force);
          act(buf, FALSE, this, 0, vict, TO_VICT);
          if ((rc = vict->parseCommand(to_force, FALSE)) == DELETE_THIS) {
            delete vict;
            vict = NULL;
          }
        }
      }
    }
    sendTo("Ok.\n\r");
  } else if (!strcmp(name_buf, "room")) {
    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt) {
        if ((GetMaxLevel() <= tbt->GetMaxLevel()) && dynamic_cast<TPerson *>(tbt))
          continue;
        else {
          sprintf(buf, "$n has forced you to '%s'.\n\r", to_force);
          act(buf, FALSE, this, 0, tbt, TO_VICT);

          if ((rc = tbt->parseCommand(to_force, FALSE)) == DELETE_THIS) {
            delete tbt;
            tbt = NULL;
          }
        }
      }
    }
    sendTo("Ok.\n\r");
  } else if (!strcmp(name_buf, "mobs")) {
    for (t = roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TMonster *tbt = dynamic_cast<TMonster *>(t);
      if (tbt) {
        sprintf(buf, "$n has forced you to '%s'.\n\r", to_force);
        act(buf, FALSE, this, 0, tbt, TO_VICT);

        if ((rc = tbt->parseCommand(to_force, FALSE)) == DELETE_THIS) {
          delete tbt;
          tbt = NULL;
        }
      }
    }
    sendTo("Ok.\n\r");
  }
}

void TBeing::doLoad(const char *)
{
  sendTo("You're a mob.  Go away.\n\r");
}

void TPerson::doLoad(const char *argument)
{
  TMonster *mob;
  TObj *obj;
  char type[100], num[100], newarg[100];
  int numx, count=0;

  if (powerCheck(POWER_LOAD))
    return;

  argument = one_argument(argument, type);

  strcpy(num, argument);

  if ((count = getabunch(num, newarg)))
    strcpy(num, newarg);
  if (!count)
    count = 1;

  if (isdigit(*num))
    numx = convertTo<int>(num);
  else
    numx = -1;

  if (is_abbrev(type, "mobile")) {
    if (numx < 0) {
      for (numx = 0; numx < (signed int) mob_index.size(); numx++)
        if (isname(num, mob_index[numx].name))
          break;
      if (numx >= (signed int) mob_index.size())
        numx = -1;
    } else 
      numx = real_mobile(numx);
    
    if (numx < 0 || numx >= (signed int) mob_index.size()) {
      sendTo("There is no such monster.\n\r");
      return;
    }
    if (mob_index[numx].spec == SPEC_SHOPKEEPER) {
      if (!hasWizPower(POWER_LOAD_IMP_POWER)) {
        sendTo("Loading shopkeepers is a restricted function.\n\r");
        return;
      }
      if (mob_index[numx].getNumber() > 0) {
        sendTo("Trying to create that mob when one already exists could create problems.\n\r");
        return;
      }
    }
    if (mob_index[numx].spec == SPEC_NEWBIE_EQUIPPER) {
      if (!hasWizPower(POWER_LOAD_IMP_POWER)) {
        sendTo("Loading newbieHelpers is a restricted function.\n\r");
        return;
      }
    }

    while(count--){
      if (!(mob = read_mobile(numx, REAL))) {
	sendTo("You suck.  You can't load that!\n\r");
	return;
      }
      *roomp += *mob;
      mob->oldRoom = inRoom();
      mob->createWealth();

      act("$n makes a quaint, magical gesture with one hand.", TRUE, this, 0, 0, TO_ROOM);
      act(msgVariables(MSG_LOAD_MOB, mob), TRUE, this, 0, mob, TO_ROOM);
      act("You bring forth $N from the cosmic ether.", TRUE, this, 0, mob, TO_CHAR);
    }
  } else if (is_abbrev(type, "object")) {
    if (numx < 0) {
      for (numx = 0; numx < (signed int) obj_index.size(); numx++) {
        if (isname(num, obj_index[numx].name))
          break;
      }
      if (numx >= (signed int) obj_index.size())
        numx = -1;
    } else {
      numx = real_object(numx);
    }
    if (numx < 0 || numx >= (signed int) obj_index.size()) {
      sendTo("There is no such object.\n\r");
      return;
    }
    if (!hasWizPower(POWER_LOAD_IMP_POWER)) {
      switch (obj_index[numx].virt) {
        case DEITY_TOKEN:
          sendTo("Tokens can't be reimbursed.\n\r");
          return;
        case YOUTH_POTION:
          sendTo("Youth potions are unloadable.\n\r");
          return;
        case STATS_POTION:
          sendTo("Stat potions are unloadable.\n\r");
          return;
	case LEARNING_POTION:
	  sendTo("Learning potions are unloadable.\n\r");
	  return;
	case MYSTERY_POTION:
	  sendTo("Mystery potions are unloadable.\n\r");
	  return;
        case CRAPS_DICE:
          sendTo("These dice are unloadable by anyone but Brutius. More than one pair in the game could wreck havok!\n\r");
          return;
      }
    }

    while(count--){
      if ((obj_index[numx].getNumber() >= obj_index[numx].max_exist) &&
          !hasWizPower(POWER_LOAD_LIMITED)) {
	sendTo("Sorry, all of those items are presently in use in the game.\n\r");
	sendTo("You are unable to load extraneous artifacts at your level.\n\r");
	return;
      }
      if (!(obj = read_object(numx, REAL))) {
	vlogf(LOG_BUG, "Error finding object.");
	return;
      }
      if (obj_index[numx].getNumber() > obj_index[numx].max_exist) {
	sendTo("WARNING!  This item is in excess of the defined max_exist.\n\r");
	sendTo("Please don't let this item fall into mortal hands.\n\r");
      }
      act("$n makes a strange magical gesture.", TRUE, this, 0, 0, TO_ROOM);
      act(msgVariables(MSG_LOAD_OBJ, obj), TRUE, this, obj, 0, TO_ROOM);
      act("You now have $p.", TRUE, this, obj, 0, TO_CHAR);
      *this += *obj;
      logItem(obj, CMD_LOAD);

      // let builder load a real obj, but make it proto
      if (!hasWizPower(POWER_LOAD_NOPROTOS)) {
	if (!obj->isObjStat(ITEM_PROTOTYPE)) {
	  obj->addObjStat(ITEM_PROTOTYPE);
	  sendTo("Changing the object to a prototype.\n\r");
	}
      }
    }
  } else if (is_abbrev(type, "room")) {
    // a backdoor into rload?
    int start, end;

    if (!hasWizPower(POWER_RLOAD))
      return;

    switch (sscanf(num, "%d %d", &start, &end)) {
      case 2:            // we got both numbers 
        RoomLoad(this, start, end, false);
        break;
      case 1:            // we only got one, load it 
        RoomLoad(this, start, start, false);
        break;
      default:
        sendTo("Load? Fine!  Load we must, But what?\n\r");
        break;
    }
  } else if (is_abbrev(type, "set") || is_abbrev(type, "suit")) {
    if (!hasWizPower(POWER_LOAD_SET)) {
      sendTo("Sorry, you are not high enough to do this yet.\n\r");
      return;
    }
    loadSetEquipment((is_number(num) ? convertTo<int>(num) : -1), num, 101);
  } else
    sendTo("Usage: load (object|mobile|set) (number|name)\n\r       load room start [end]\n\r");
}

static void purge_one_room(int rnum, TRoom *rp, int *range)
{
  TThing *t;

  if (!rnum || (rnum < range[0]) || (rnum > range[1]))
    return;

  for (t = rp->getStuff(); t; t = rp->getStuff()) {
    --(*t);
    thing_to_room(t, ROOM_VOID);

    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt) {
      tbt->sendTo(fmt("A god strikes the heavens making the %s around you erupt into a\n\r") % rp->describeGround());
      tbt->sendTo("fluid fountain boiling into the ether.  All that's left is the Void.\n\r");
      tbt->doLook("", CMD_LOOK);
    }
  }
  delete rp;
  roomCount--;
}

void TBeing::doCutlink(const char *)
{
  return;
}

void TPerson::doCutlink(const char *argument)
{
  Descriptor *d;
  char name_buf[100];

  if (powerCheck(POWER_CUTLINK))
    return;

  argument = one_argument(argument, name_buf);

  if (!*name_buf) {
    for (d = descriptor_list; d; d = d->next) {
      if (!d->character || !d->character->name) {
        sendTo(fmt("You cut a link from host %s\n\r") %
               (!(d->host.empty()) ? d->host : "Host Unknown"));

        delete d;
      }
    }
  } else {
    for (d = descriptor_list; d; d = d->next) {
      if (d->character) {
        if (d->character->name && !(sstring(d->character->name).lower()).compare(name_buf)) {
          if (d->character == this) {
            sendTo("You can't cut your own link, sorry.\n\r");
            return;
          }
          if (d->character->GetMaxLevel() >= GetMaxLevel()) {
            sendTo("You can only cut the link of players lower level than you.\n\r");
            return;
          }
          if (d->character->roomp)  // necessary check due to canSeeMe
            act("You cut $S link.", TRUE, this, 0, d->character, TO_CHAR);
          else 
            act("You cut someone's link.", TRUE, this, 0, 0, TO_CHAR);

          TPerson *p = dynamic_cast<TPerson *>(d->character);
          if (p)
            p->fixClientPlayerLists(TRUE);
          delete d;
          return;
        }
      }
    }
    sendTo("No one by that name logged in!\n\r");
    return;
  }
}

void TMonster::purgeMe(TBeing *ch)
{
  if (desc || IS_SET(specials.act, ACT_POLYSELF)) {
    if (ch)
      ch->sendTo("You can not purge a polyed or switched mob.\n\r");
    return;
  }
  if (isShopkeeper() || spec == SPEC_SHARPENER || spec == SPEC_REPAIRMAN || spec == SPEC_ATTUNER) {
    if (ch)
      ch->sendTo("Boy that was close.\n\rBe glad Brutius put in a catch so you didn't purge the shopkeeper.\n\r");
    return;
  }
  delete this;
}

void TObj::purgeMe(TBeing *ch)
{
  if (!isObjStat(ITEM_NOPURGE)) {
    if (ch)
      ch->logItem(this, CMD_PURGE);
    delete this;
  }
}

void TThing::purgeMe(TBeing *)
{
  // default is not to do anything
}

void nukeLdead(TBeing *vict)
{
  wearSlotT ij;
  TThing *t, *t2;

  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    TObj *obj = dynamic_cast<TObj *>(vict->equipment[ij]);
    if (obj) {
      vict->unequip(ij);

      for (t = obj->getStuff(); t; t = t2) {
        t2 = t->nextThing;

        vict->logItem(t, CMD_NE);  // purge ldead

        TObj *o2 = dynamic_cast<TObj *>(t);
        if (o2 && o2->number >= 0)
          obj_index[o2->getItemIndex()].addToNumber(1);

        delete t;
      }
  
      vict->logItem(obj, CMD_NE);  // purge ldead

      // since the item is technically still in rent, it is accounted
      // for.  Deleting it here is going to drop the number by 1 which
      // would be bad.  Artifically bump it up, so that things stay
      // in synch.
      if ((obj->number >= 0))
        obj_index[obj->getItemIndex()].addToNumber(1);

      delete obj;
      obj = NULL;
    }
  }
  for (t = vict->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    TThing *t3, *t4;
    for (t3 = obj->getStuff(); t3; t3 = t4) {
      t4 = t3->nextThing;

      vict->logItem(t3, CMD_NE);  // purge ldead

      TObj *o2 = dynamic_cast<TObj *>(t3);
      if (o2 && o2->number >= 0)
        obj_index[o2->getItemIndex()].addToNumber(1);

      delete t3;
    }

    vict->logItem(obj, CMD_NE);  // purge ldead

    // since the item is technically still in rent, it is accounted
    // for.  Deleting it here is going to drop the number by 1 which
    // would be bad.  Artifically bump it up, so that things stay
    // in synch.
    if ((obj->number >= 0))
      obj_index[obj->getItemIndex()].addToNumber(1);

    delete obj;
    obj = NULL;
  }
}

void genericPurgeLdead(TBeing *ch)
{
  TBeing *vict, *temp;

  for (vict = character_list; vict; vict = temp) {
    temp = vict->next;
    if (vict->isLinkdead()) {
      if (ch)
        ch->sendTo(COLOR_MOBS, fmt("Purging %s.\n\r") % vict->getName());
      nukeLdead(vict);
      delete vict;
      vict = NULL;
    }
  }

  Descriptor *d, *d2;
  // also do a cutlink
  for (d = descriptor_list; d; d = d2) {
    d2 = d->next;
    if (!d->character || !d->character->name) {
      delete d;
    }
  }
}

// clean a room of all mobiles and objects 
void TBeing::doPurge(const char *)
{
  return;
}

void TPerson::doPurge(const char *argument)
{
  TBeing *vict;
  TObj *obj;
  Descriptor *d, *dn;

  char name_buf[100];

  if (powerCheck(POWER_PURGE))
    return;

  argument = one_argument(argument, name_buf);

  if (*name_buf) {    // argument supplied. destroy single object or char 
    if (!strcmp(name_buf, "links") && hasWizPower(POWER_PURGE_LINKS)) {
      for (d = descriptor_list; d; d = dn) {
        dn = d->next;
        if (d->character) {
          if (d->character->GetMaxLevel() <= MAX_MORT ) {
            d->writeToQ("Link severed by admin.\n\r");
            delete d;
          }
        } else {
          d->writeToQ("Link severed by admin.\n\r");
          delete d;
        }
      }
      return;
    } else if (!strcmp(name_buf, "ldead")) {
      sendTo("Purge ldead has some bad side effects (pet loss, etc).\n\r");
      sendTo("If you are doing this out of habit, stop!\n\r");
      sendTo("If there REALLY is a reason to kick linkdeads out (e.g wizlock),\n\r");
      sendTo("purge ldead-real\n\r");
      return;
    } else if (!strcmp(name_buf, "ldead-real")) {
      genericPurgeLdead(this);
      return;
    } else if (is_abbrev(name_buf, "zone")) {
      unsigned int zone = 0;

      for (; isspace(*argument); argument++);

      if (is_abbrev(argument, "all")) {
        int oldnum = mobCount;
        for (zone = 1; zone < zone_table.size(); zone++) {
          if (zone_table[zone].zone_value > 0 && 
	      zone_table[zone].isEmpty()) {
            zone_table[zone].nukeMobs();
            zone_table[zone].zone_value = 0;
          }
        }
        sendTo(fmt("All excess mobs nuked.  %d mobs destroyed, %d creatures now.\n\r") % (oldnum-mobCount) % mobCount);
        return;
      }

      zone = convertTo<int>(argument);

      if (zone <= 0 || zone >= zone_table.size()) {
        sendTo("Syntax: purge zone <zone #>\n\r");
        return;
      }

      zone_table[zone].nukeMobs();
      sendTo("Mobs nuked.\n\r");
      zone_table[zone].zone_value = 0;
      return;
    }
    TThing *t_obj;
    if ((vict = get_char_room_vis(this, name_buf))) {
      if (vict->isShopkeeper()) {    // shopkeeper 
        sendTo("Be glad Brutius put this catch in for shopkeepers.\n\r");
        vlogf(LOG_MISC, fmt("%s just tried to purge a shopkeeper.") %  getName());
        return;
      }
      if (vict->isPc() && 
            (!hasWizPower(POWER_PURGE_PC) ||  // prevent until this level
             vict->GetMaxLevel() >= GetMaxLevel())) { // prevent over my lev
       sendTo("I can't let you do that.\n\r");
       return;
      }
                            
      if (dynamic_cast<TMonster *>(vict) && 
        (vict->desc || IS_SET(vict->specials.act, ACT_POLYSELF))) {
        sendTo("Sorry, you can't directly purge a poly'ed mob, you made them return.\n\r");
         vict->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
         vict->doReturn("", WEAR_NOWHERE, CMD_RETURN);
         return;
        // delete vict;
        // vict = NULL;
      }
      act(msgVariables(MSG_PURGE_TARG, vict), TRUE, this, 0, vict, TO_NOTVICT);
      act("You disintegrate $N.", TRUE, this, 0, vict, TO_CHAR);
      if (vict->isLinkdead()) {
        // linkdead PC will have eq stored from last save.
        // without this, eq would be dropped on ground (duplicated)
        wearSlotT ij;
        for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
          if (vict->equipment[ij]) {
            t_obj = vict->unequip(ij);
            obj = dynamic_cast<TObj *>(t_obj);

            // since the item is technically still in rent, it is accounted
            // for.  Deleting it here is going to drop the number by 1 which
            // would be bad.  Artifically bump it up, so that things stay
            // in synch.
            if ((obj->number >= 0))
              obj_index[obj->getItemIndex()].addToNumber(1);

            delete obj;
            obj = NULL;
          }
        }
        TThing *t, *t2;
        for (t = vict->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          obj = dynamic_cast<TObj *>(t);

          // since the item is technically still in rent, it is accounted
          // for.  Deleting it here is going to drop the number by 1 which
          // would be bad.  Artifically bump it up, so that things stay
          // in synch.
          if ((obj->number >= 0))
            obj_index[obj->getItemIndex()].addToNumber(1);

          delete obj;
          obj = NULL;
        }
      }

      if (vict->desc) {
        delete vict->desc;
        vict->desc = NULL;
      }
      delete vict;
      vict = NULL;
    } else if ((t_obj = searchLinkedListVis(this, name_buf, roomp->getStuff()))) {
      // since we already did a get_char loop above, this is really just doing
      // objs, despite the fact that it is a TThing
      act("$n destroys $p.", TRUE, this, t_obj, 0, TO_ROOM);
      TPerson *vict = dynamic_cast<TPerson *>(t_obj);
      if (vict && vict->isLinkdead()) {
        // linkdead PC will have eq stored from last save.
        // without this, eq would be dropped on ground (duplicated)
        wearSlotT ij;
        for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
          if (vict->equipment[ij]) {
            t_obj = vict->unequip(ij);
            obj = dynamic_cast<TObj *>(t_obj);

            // since the item is technically still in rent, it is accounted
            // for.  Deleting it here is going to drop the number by 1 which
            // would be bad.  Artifically bump it up, so that things stay
            // in synch.
            if ((obj->number >= 0))
              obj_index[obj->getItemIndex()].addToNumber(1);

            delete obj;
            obj = NULL;
          }
        }
        TThing *t, *t2;
        for (t = vict->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          obj = dynamic_cast<TObj *>(t);

          // since the item is technically still in rent, it is accounted
          // for.  Deleting it here is going to drop the number by 1 which
          // would be bad.  Artifically bump it up, so that things stay
          // in synch.
          if ((obj->number >= 0))
            obj_index[obj->getItemIndex()].addToNumber(1);

          delete obj;
          obj = NULL;
        }
      } else if (vict) 
        vict->dropItemsToRoom(SAFE_YES, DROP_IN_ROOM);

      delete t_obj;
      t_obj = NULL;
    } else {
      if (!strcmp("room", name_buf)) {
        int range[2];
        register int regi;
        TRoom *rp;
        if (powerCheck(POWER_PURGE_ROOM))
          return;

        argument = one_argument(argument, name_buf);
        if (!isdigit(*name_buf)) {
          sendTo("purge room start [end]\n\r");
          return;
        }
        range[0] = convertTo<int>(name_buf);
        argument = one_argument(argument, name_buf);
        if (isdigit(*name_buf))
          range[1] = convertTo<int>(name_buf);
        else
          range[1] = range[0];
    
        if (!*range || !range[1]) {
          sendTo("usage: purge room start [end]\n\r");
          return;
        }
        if (range[0] >= WORLD_SIZE || range[1] >= WORLD_SIZE) {
          sendTo("only purging to WORLD_SIZE\n\r");
          return;
        }
        for (regi = range[0]; regi <= range[1]; regi++) {
          if ((rp = real_roomp(regi)) != 0)
            purge_one_room(regi, rp, range);
        }
      } else {
        sendTo("I don't see that here.\n\r");
        return;
      }
    }
    sendTo("Ok.\n\r");
  } else {            // no argument. clean out the room 
    act(msgVariables(MSG_PURGE, (TThing *)NULL), TRUE, this, 0, 0, TO_ROOM);
    sendToRoom("The World seems a little cleaner.\n\r", in_room);

    TThing *t, *n;
    for (t = roomp->getStuff(); t; t = n) {
      n = t->nextThing;
      t->purgeMe(this);
      // t may be invalid here
    }
  }
}

static void welcomeNewPlayer(const TPerson *ch)
{
  // let's find out if they are a true newbie, or just someone's new char
  // basically, check how many chars they have in their account
  char buf[128];
  DIR *dfd;
  struct dirent *dp;
  unsigned int count = 0;

  sprintf(buf, "account/%c/%s", LOWER(ch->name[0]), sstring(ch->name).lower().c_str());
  if (!(dfd = opendir(buf))) {
    return;
  }
  while ((dp = readdir(dfd)) != NULL) {
    if (!strcmp(dp->d_name, "account") || !strcmp(dp->d_name, "comment") ||
        !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;

    count++;
  }
  closedir(dfd);


  Descriptor *d;
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected != CON_PLYNG)
      continue;
    TBeing *tbt = d->character;
    if (!tbt)
      continue;
    if (tbt->isPlayerAction(PLR_NEWBIEHELP)) {
      tbt->sendTo(COLOR_BASIC, fmt("<c>Attention Newbie-Helpers: Please welcome %s to <h>!!!<1>\n\r") % ch->getName());
      if (count == 1)
        tbt->sendTo(COLOR_BASIC, fmt("<c>%s is the first character in %s account!<1>\n\r") % ch->getName() % ch->hshr());
    }
  }
}

// this function is ONLY called on first login
void TPerson::doStart()
{
  int r_num;
  TObj *obj;
  char buf[256];

  if (desc->account->term == TERM_ANSI) {
    doToggle("term none");
    doCls(false);
    // I'm leaving rooms off this list intentionally - bat
    // I'm using = here (rather than set_bit) since I want to know
    // that PLR_COLOR_CODES and PLR_COLOR_LOGS are off
    desc->plr_color = PLR_COLOR_BASIC | PLR_COLOR_ALL | 
       PLR_COLOR_BASIC | PLR_COLOR_COMM | PLR_COLOR_OBJECTS | PLR_COLOR_MOBS |
       PLR_COLOR_ROOM_NAME | PLR_COLOR_SHOUTS | PLR_COLOR_SPELLS;
    addPlayerAction(PLR_COLOR);
    SET_BIT(desc->prompt_d.type, PROMPT_COLOR);
  }
  if (!desc->m_bIsClient)
    sendTo(COLOR_BASIC, "<R>Initializing your Character<1> ...\n\r");

  if (!discs)
    assignDisciplinesClass();

  // requires disciplines be set before calling
  startLevels();

  setHeight(race->generateHeight(getSex()));
  setWeight(race->generateWeight(getSex()));
  setBaseAge(race->generateAge());

  if (desc->account->term == TERM_VT100) 
    doToggle("term vt100");
  
  if (desc && !desc->m_bIsClient && !ansi() && !vt100()) 
    doPrompt("all");

  desc->autobits = 0;
  SET_BIT(desc->autobits, (unsigned int) (AUTO_EAT | AUTO_AFK |
          AUTO_KILL | AUTO_LOOT_NOTMONEY | AUTO_SPLIT |
          AUTO_DISSECT | AUTO_NOHARM | AUTO_TIPS));

  if ((GetMaxLevel() > MAX_MORT) && !isPlayerAction(PLR_IMMORTAL)) {
    addPlayerAction(PLR_IMMORTAL);
    SET_BIT(desc->autobits, AUTO_SUCCESS);
    if (!desc->m_bIsClient)
      sendTo("Setting various autobits with recommended immortal configuration.\n\r");
    setMoney(100000);
    sprintf(buf, "%s full", getName());
    doRestore(buf);
  } else {
    if (!desc->m_bIsClient) {
      sendTo("Setting various autobits with recommended newbie configuration.\n\r");
      if (IS_SET(desc->plr_color, PLR_COLOR_BASIC)) 
        sendTo("Setting color to all available color options.\n\r");
    }
  }
  setExp(1);
  setTitle(true);

  classIndT Class;
  for (Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
    
    if(!getLevel(Class))
      continue;
    
    float pracs = pracsPerLevel(Class, false);
    int prac_gain = (int)pracs;
    if (::number(0,100) < (int)(((float)pracs - prac_gain) * 100))
      prac_gain++;
    
    addPracs(prac_gain, Class);
    // we need to do first level pracs this way even with the new system - dash
  }

  doNewbieEqLoad(RACE_NORACE, 0, true);

  if (hasClass(CLASS_CLERIC))
    personalize_object(NULL, this, 500, -1);

  if (hasClass(CLASS_MAGE))
    personalize_object(NULL, this, 321, -1);

  if (hasClass(CLASS_SHAMAN)) {
    personalize_object(NULL, this, 31317, -1);
    personalize_object(NULL, this, 31395, -1);
  }

  if (hasClass(CLASS_DEIKHAN))
    personalize_object(NULL, this, 500, -1);

  if ((r_num = real_object(1458)) >= 0) {
    obj = read_object(r_num, REAL);
    *this += *obj;    // newbie book 
  }
#ifdef NOGUIDE
  // Damescena wants this to NOT load
  if ((r_num = real_object(1459)) >= 0) {
    obj = read_object(r_num, REAL);
    *this += *obj;    // conversion book 
  }
#endif
  // give a weapon
  if (hasClass(CLASS_WARRIOR) || hasClass(CLASS_RANGER) || hasClass(CLASS_DEIKHAN)) {
    if ((r_num = real_object(WEAPON_T_SWORD)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;    // newbie sword
    }
  } else if (!hasClass(CLASS_CLERIC) && !hasClass(CLASS_MONK) && !hasClass(CLASS_SHAMAN)) {
    if ((r_num = real_object(WEAPON_T_DAGGER)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;    // newbie dagger
    }
  } else {
    if ((r_num = real_object(WEAPON_T_STAFF)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;    // newbie staff 
    }
  }

  if (!desc->m_bIsClient) {
    sendTo("You have been given appropriate newbie equipment.\n\r");
    sendTo(fmt("--> See %sHELP WEAR%s for more details.\n\r") % cyan() % norm());
    sendTo(fmt("Be sure to read your %snewbie guide%s...\n\r") % green() % norm());
    sendTo("If you are feeling lost, read HELP GOTO for some quick directions.\n\r");
  }

  if (hasClass(CLASS_SHAMAN)) {
    sendTo(fmt("\n\r%s**%s\a\t*+*+*+*+*+*+*+*+*+*+*+ %sWARNING%s +*+*+*+*+*+*+*+*+*+*+*+\n\r\n\r") % red() % norm() % red() % norm());
    sendTo(fmt("%s**%s\tThe class you are about to play has been designed with\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tthe most advanced of players in mind. The class is not\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tan easy one to play in the least. Please be sure, if \n\r") % red() % norm());
    sendTo(fmt("%s**%s\tyou decide to dedicate time to a Shaman character, that\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tyou can deal with playing an advanced class that would\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tbe by most considered impossible or unplayable. Please\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tfamiliarize yourself with the help files pertaining to\n\r") % red() % norm());
    sendTo(fmt("%s**%s\tthe Shaman class before undertaking this challenge.\n\r%s**%s\n\r") % red() % norm() % red() % norm());
    sendTo(fmt("%s**%s\tSee %sHELP LIFEFORCE%s for more details.\n\r") % red() % norm() % cyan() % norm());
    sendTo(fmt("%s**%s\tBe sure to read your %snewbie guide%s...\n\r") % red() % norm() % green() % norm());
    sendTo(fmt("%s**%s\tIf you have serious problems, by all means,\n\r") % red() % norm());
    sendTo(fmt("%s**%s\task a player with the %sNEWBIE HELPER%s flag.\n\r") % red() % norm() % red() % norm());
    sendTo("***********************************************************************\n\r\n\r");
  }

  setMana(manaLimit());
  setPiety(pietyLimit());
  setMove(moveLimit());
  setLifeforce(50);

  // Ripped from the level gain functions to give classes a variant 
  // from the standard 25 hp based on con etc. Brutius 06/18/1999
  classIndT cit;
  for (cit = MIN_CLASS_IND; cit < MAX_CLASSES; cit++) {
    if (getLevel(cit))
      doHPGainForLev(cit);
  }
  setHit(21);

  wearSlotT iw;
  for (iw = MIN_WEAR; iw < MAX_WEAR; iw++) {
    setLimbFlags(iw, 0);
    setStuckIn(iw, NULL);
    setCurLimbHealth(iw, getMaxLimbHealth(iw));
  }

  setFaction(FACT_NONE);
#if FACTIONS_IN_USE
  setPerc(5.0);
  factionTypeT ij;
  for (ij = MIN_FACTION; ij < MAX_FACTIONS; ij++)
    setPercX(5.0, ij);
#endif

  condTypeT ic;
  for (ic = MIN_COND; ic < MAX_COND_TYPE; ++ic)
    setCond(ic, (GetMaxLevel() > MAX_MORT ? -1 : 24));
  setCond(DRUNK, min((sbyte) 0, getCond(DRUNK)));

  welcomeNewPlayer(this);

  // this is needed to prevent double loading of newbie eq
  player.time.birth = time(0);

  doSave(SILENT_NO);

  if (desc->m_bIsClient) {
    addPlayerAction(PLR_COMPACT);
    SET_BIT(desc->plr_color, PLR_COLOR_BASIC);
    addPlayerAction(PLR_COLOR);
    SET_BIT(desc->prompt_d.type, PROMPT_COLOR);
    SET_BIT(desc->plr_color, PLR_COLOR_ALL);
    SET_BIT(desc->plr_color, PLR_COLOR_BASIC);
    SET_BIT(desc->plr_color, PLR_COLOR_COMM);
    SET_BIT(desc->plr_color, PLR_COLOR_OBJECTS);
    SET_BIT(desc->plr_color, PLR_COLOR_MOBS);
    SET_BIT(desc->plr_color, PLR_COLOR_ROOMS);
    SET_BIT(desc->plr_color, PLR_COLOR_ROOM_NAME);
    SET_BIT(desc->plr_color, PLR_COLOR_SHOUTS);
    SET_BIT(desc->plr_color, PLR_COLOR_SPELLS);

    // codes works in reverse
    REMOVE_BIT(desc->plr_color, PLR_COLOR_CODES);
  }
}

void TBeing::doWiznews()
{
  if (!desc || desc->connected)
    return;

  if (isImmortal() || IS_SET(desc->account->flags, ACCOUNT_IMMORTAL)) {
    wiznews_used_num++;
    desc->start_page_file(WIZNEWS_FILE, "No news for the immorts!\n\r");
  } else 
    sendTo("This command is for immortals only.\n\r");
}

// returns DELETE_THIS
int TBeing::genericRestore(restoreTypeT partial)
{
  setCond(FULL, 24);
  setCond(THIRST, 24);
  setPiety(pietyLimit());
  setMana(manaLimit());
  setHit(hitLimit());
  setMove(moveLimit());
  if (getPosition() <= POSITION_STUNNED)
    setPosition(POSITION_SITTING);

  if (GetMaxLevel() > MAX_MORT) {
    setCond(DRUNK, -1);
    setCond(FULL, -1);
    setCond(THIRST, -1);
  }
  if (partial == RESTORE_FULL) {
    // heal all limbs and cure all diseases - Russ 
    wearSlotT ij;
    for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
      setCurLimbHealth(ij, getMaxLimbHealth(ij));
      setLimbFlags(ij, 0);
    }

    affectFrom(AFFECT_NEWBIE);
    affectFrom(AFFECT_SKILL_ATTEMPT);

    affectedData *aff, *aff2;
    for (aff = affected; aff; aff = aff2) {
      aff2 = aff->next;
      if (aff->type == AFFECT_DISEASE) {
        diseaseStop(aff);
        affectRemove(aff);
      } else if ((aff->type == SKILL_QUIV_PALM ||
                  aff->type == SKILL_SMITE)) {
        // spellWearOff is potentially lethal, but since we know the type
        // we know these types are not lethal if they decay
        int rc = spellWearOff(aff->type);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        affectRemove(aff);
      } else {
	affectRemove(aff);
      }
    }
  }
  return 0;
}

void TBeing::doRestore(const char *argument)
{
  TBeing *victim;
  char buf[256], arg1[100], arg2[100];
  restoreTypeT partial = RESTORE_FULL;
  statTypeT statx;
  int rc;
  bool found = FALSE;

  if (powerCheck(POWER_RESTORE))
    return;

  half_chop(argument, arg1, arg2);
  if (!*arg1) {
    sendTo("Whom do you wish to restore?\n\r");
    return;
  }
  if (!(victim = get_char(arg1, EXACT_YES)) &&
      !(victim = get_char(arg1, EXACT_NO))) {
    sendTo("Syntax : restore <character> <partial | full | pracs>\n\r");
    return;
  }
  if (!(victim->GetMaxLevel() > MAX_MORT) && !hasWizPower(POWER_RESTORE_MORTAL)) {
    sendTo("You can only restore another immortal.\n\r");
    return;
  }

  if (!*arg2) {
    sendTo("Syntax : restore <character> <partial | full | pracs>\n\r");
    return;
  }
  if (is_abbrev(arg2, "partial"))
    partial = RESTORE_PARTIAL;
  else if (is_abbrev(arg2, "full"))
    partial = RESTORE_FULL;
  else if (is_abbrev(arg2, "pracs")) {
    int pracs,pracs2;
    pracs =  pracs2  = 0;
    char name2[60],name[60];
    char buf[60];
    
    FILE *fp, *fp2;
    
    if (!(fp = fopen("reimburse.list","r"))) {
      vlogf(LOG_FILE, "Couldn't open reimbursement list for restore pracs.");
      return;
    }
    if (!(fp2 = fopen("reimburse.new","w+"))) {
      vlogf(LOG_FILE, "Couldn't open reimbursement list for restore pracs.");
      return;
    }
    if (fgets(buf,60,fp) == NULL && !strcmp("START\n",buf)) {
      vlogf(LOG_FILE,"ERROR: bad format of reimbursement list");
      fclose(fp);
      return;
    }
    fprintf(fp2,buf);
    
    while(strcmp(buf, "END")) {
      if (fgets(buf,60,fp) == NULL) {
	vlogf(LOG_FILE,"ERROR: bad format of reimbursement list");
	fclose(fp);
	return;
      }
      if (!strcmp(buf, "END")) //end of list
	break;
      if (sscanf(buf,"%d %s\n", &pracs, name) != 2) {
        vlogf(LOG_FILE,"ERROR: bad format of reimbursement list");
        fclose(fp);
	return;
      }
      if (!strcmp(name, victim->getName())) {
	found = TRUE;
	strncpy(name2, name, sizeof(name));
	pracs2 = pracs;
      } else {
	fprintf(fp2,buf);
      }
    }
    strncpy(name, name2, sizeof(name2));
    pracs = pracs2;
    fprintf(fp2,buf);
    fclose(fp);
    fclose(fp2);
    char buf2[256];
    strcpy(buf2,"cp reimburse.new reimburse.list");
    vsystem(buf2);
    if (!found) {// not in list
      sendTo("They are not currently supposed to be reimbursed.\n\r");
      return;
    }
    //ok we found them in the list, num holds how many practices they need
    pracs = min(pracs,(int)victim->getLevel(victim->bestClass())); // cap at 1/lev
    victim->practices.prac[victim->bestClass()]+=pracs;

    victim->sendTo(fmt("Congratulations, you've been reimbursed %d practices!\n\r") %pracs);
    sendTo(fmt("Reimbursing %s %d practices.\n\r") %victim->getName() %pracs);

    
  } else {
    sendTo("Syntax : restore <character> <partial | full | pracs>\n\r");
    return;
  }

  rc = victim->genericRestore(partial);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete victim;
    victim = NULL;
    return;
  }
    

  if (partial == RESTORE_FULL) {
    rc = generic_dispel_magic(this, victim, GetMaxLevel(), isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete victim;
      victim = NULL;
      return;
    }
  }
  if (partial == RESTORE_FULL) {
    rc = genericChaseSpirits(this, victim, GetMaxLevel(), isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      delete victim;
      victim = NULL;
      return;
    }
  }

  if (dynamic_cast<TMonster *>(victim)) {
    sendTo("You restore the pitiful creature.\n\r");
    return;
  }
  if (victim->GetMaxLevel() > MAX_MORT) {
    spellNumT i;
    for (i = MIN_SPELL; i < MAX_SKILL; i++) {
      if (hideThisSpell(i))
        continue;
      victim->setSkillValue(i,100);
    }
    discNumT dnt;
    for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
      CDiscipline *cd = victim->getDiscipline(dnt);
      if (cd)
        cd->setLearnedness(100);
    }
    if (victim->hasWizPower(POWER_GOD)) {
      for(statx=MIN_STAT;statx<MAX_STATS;statx++) {
        victim->setStat(STAT_CURRENT, statx, 250);
      }
    }
    checkForStr(SILENT_NO);
  }
  victim->updatePos();
  sendTo("Done.\n\rIf Shaman lifeforce needs to be changed please do so with \"@set\".\n\r");
  if (partial == RESTORE_PARTIAL)
    act("You have been partially healed by $N!", FALSE, victim, 0, this, TO_CHAR);
  else {
    sprintf(buf, "You have been %sFULLY%s healed by $N!", cyan(), norm());
    act(buf, FALSE, victim, 0, this, TO_CHAR);
  }
  victim->doSave(SILENT_NO);
}


void TBeing::doNoshout(const sstring &argument)
{
  TBeing *vict;
  TObj *dummy;

  if (argument.empty()) {
    if (desc) {
      if (IS_SET(desc->autobits, AUTO_NOSHOUT)) {
        sendTo("You can now hear shouts again.\n\r");
        REMOVE_BIT(desc->autobits, AUTO_NOSHOUT);
      } else {
        sendTo("From now on, you won't hear shouts.\n\r");
        SET_BIT(desc->autobits, AUTO_NOSHOUT);
      }
    } else {
      sendTo("Go away.  Bad mob.\n\r");
    }
  } else if (isImmortal()) {
    if (!generic_find(argument.c_str(), FIND_CHAR_WORLD, this, &vict, &dummy))
      sendTo("Couldn't find any such creature.\n\r");
    else if (dynamic_cast<TMonster *>(vict))
      sendTo("Can't do that to a beast.\n\r");
    else if (vict->GetMaxLevel() >= GetMaxLevel())
      act("$E might object to that.. better not.", 0, this, 0, vict, TO_CHAR);
    else if (vict->isPlayerAction(PLR_GODNOSHOUT) && hasWizPower(POWER_NOSHOUT)) {
      vict->sendTo("You can shout again.\n\r");
      sendTo("NOSHOUT removed.\n\r");
      vict->remPlayerAction(PLR_GODNOSHOUT);
      vlogf(LOG_MISC,fmt("%s had noshout removed by %s") % vict->getName() % getName());
    } else if (hasWizPower(POWER_NOSHOUT)) {
      vict->sendTo("The gods take away your ability to shout!\n\r");
      sendTo("NOSHOUT set.\n\r");
      vict->addPlayerAction(PLR_GODNOSHOUT);
      vlogf(LOG_MISC,fmt("%s had noshout set by %s") % vict->getName() % getName());
    } else
      sendTo("Sorry, you can't do that.\n\r");
  } else {
    sendTo("Mortal noshout syntax : noshout (no argument)\n\r");
    return;
  }
}

void TBeing::doDeathcheck(const sstring &arg)
{
  char file[256], playerx[256], buf[256];
  char *p;

  if (powerCheck(POWER_DEATHCHECK))
    return;

  if (!isImmortal())
    return;

  if (sscanf(arg.c_str(), "%s %s", playerx, file) == EOF) {
    sendTo("Syntax:  deathcheck playername logfile\n\r");
    return;
  }
  for (p = playerx; *p; p++) {
    if (!isalpha(*p)) {
      sendTo("Please use a valid playername.\n\r");
      return;
    }
  }
  sprintf(buf, "\"%s.*killed.*by\" %s", playerx, file);
  doSysChecklog(buf);
}

// returns DELETE_THIS
int TBeing::doExits(const char *argument, cmdTypeT cmd)
{
  bool found = FALSE;
  dirTypeT door;
  char buf[1024];
  roomDirData *exitdata;
  int darkhere = FALSE;
  TThing *t;
  TRoom *rp;
  char nameBuf[256];

  *buf = '\0';
  memset(nameBuf, '\0', sizeof(nameBuf));

  one_argument(argument, buf);
  if (buf && *buf ) {
    int rc = portalLeaveCheck(buf, cmd);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)  
      return TRUE;
  }
  *buf = '\0';

  sendTo("Obvious exits:\n\r");
  if (roomp) {
    if (!isImmortal() && isAffected(AFF_BLIND) &&
        !isAffected(AFF_TRUE_SIGHT)) {
      sendTo("Blindness makes it impossible to tell.\n\r");
      return FALSE;
    }
    if (!isImmortal() && roomp->pitchBlackDark() &&
        !roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
        (visionBonus <= 0) &&
        !isAffected(AFF_INFRAVISION) &&
        !isAffected(AFF_SCRYING) &&  // room name shows on look x, duplicate
        !isAffected(AFF_TRUE_SIGHT)) {
      darkhere = TRUE;
    }
  }
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if ((exitdata = exitDir(door)) != NULL) {
      if (!(rp = real_roomp(exitdata->to_room))) {
        if (isImmortal()) {
          sendTo(fmt("%s - swirling chaos of #%d\n\r") % exits[door] % exitdata->to_room);
          found = TRUE;
        }  
      } else if (exitdata->to_room != ROOM_NOWHERE &&
                     (canSeeThruDoor(exitdata) || isImmortal())) {
        if (rp->pitchBlackDark() &&
             !rp->isRoomFlag(ROOM_ALWAYS_LIT) &&
             !isImmortal() && !isAffected(AFF_TRUE_SIGHT)) {
          if (!darkhere) {
              sendTo(fmt("%s - Too dark to tell\n\r") % exits[door]);
              found = TRUE;
          }
        } else {
          char slopedData[256] = "\0";

          if (door != DIR_UP && door != DIR_DOWN)
            if ((exitdata->condition & EX_SLOPED_UP))
              strcpy(slopedData, " (ascending)");
            else if ((exitdata->condition & EX_SLOPED_DOWN))
              strcpy(slopedData, " (descending)");

          if (IS_SET(desc->plr_color, PLR_COLOR_ROOM_NAME)) {
            if (hasColorStrings(NULL, rp->getName(), 2)) {
              if (isImmortal()) {
                sendTo(COLOR_ROOM_NAME, fmt("%s - %s%s (%d)%s\n\r") % exits[door] % dynColorRoom(rp, 1, TRUE) % norm() % rp->number % slopedData);
               found = TRUE;
              } else {
                sendTo(COLOR_ROOM_NAME, fmt("%s - %s%s%s\n\r") % exits[door] % dynColorRoom(rp, 1, TRUE) % norm() % slopedData);
                found = TRUE;
              }
            } else {
              if (isImmortal()) {
                sendTo(COLOR_ROOM_NAME, fmt("%s - %s%s%s (%d)%s\n\r") % exits[door] % addColorRoom(rp, 1) % rp->name % norm() % rp->number % slopedData);
                found = TRUE;
              } else {
                sendTo(COLOR_ROOM_NAME, fmt("%s - %s%s%s%s\n\r") % exits[door] % addColorRoom(rp, 1) % rp->name % norm() % slopedData);
                found = TRUE;
              }        
            }
          } else {
            if (isImmortal()) {
              sendTo(COLOR_BASIC, fmt("%s - %s%s%s (%d)%s\n\r") % exits[door] % purple() %
                      rp->getNameNOC(this) % norm() % rp->number % slopedData);
            } else {
              sendTo(COLOR_BASIC, fmt("%s - %s%s%s%s\n\r") % exits[door] % purple() %
                      rp->getNameNOC(this) % norm() % slopedData);
            }
            found = TRUE;
          }
        }
      } else if (exitdata->to_room != ROOM_NOWHERE &&
            !IS_SET(exitdata->condition, EX_SECRET)) {
        if (darkhere) {
        } else {
          sendTo(COLOR_ROOMS, fmt("%s - A closed '<b>%s<1>'\n\r") % exits[door] % fname(exitdata->keyword));
          found = TRUE;
        }
      }
    }
  }

  if (!found) {
    if (!darkhere)
      sendTo("None.\n\r");
    else
      sendTo("You see no obvious exits, it is pitch black dark.\n\r");
  }

// check for portals and add it if one is enterable
  for (t = roomp->getStuff(); t; t = t->nextThing) {
    TPortal *tp = dynamic_cast<TPortal *>(t);  
    if (!tp)
      continue;
    if (tp->isPortalFlag(EX_CLOSED | EX_NOENTER))
      continue;
    act("There is $p you can enter here.", FALSE, this, tp, 0, TO_CHAR);
    continue;
  }
  return FALSE;
}


void TBeing::doWipe(const char *argument)
{
  char namebuf[20], passwd[20], buf[1024];
  TBeing *victim;
  Descriptor *d = NULL;
  charFile st;
  TDatabase db(DB_SNEEZY);

  if (powerCheck(POWER_WIPE))
    return;

  half_chop(argument, namebuf, passwd);

  if (!*namebuf || !*passwd) {
    sendTo("You need to enter a name and the wipe password.\n\r");
    return;
  }
  if (strcmp(passwd, "ole'chicken")) {
    sendTo("Wrong wipe password.\n\r");
    return;
  }
  victim = get_char_vis_world(this, namebuf, NULL, EXACT_YES);
  if (victim) {
    if (!victim->isPc() || victim->GetMaxLevel() > MAX_IMMORT ||
        (victim->GetMaxLevel() >= GetMaxLevel()) || !(d = victim->desc)) {
      sendTo("You can only banish players less than your level.\n\r");
      return;
    }
    sendTo("Ok.\n\r");
    sprintf(buf, "You hear a cry of anguish as %s screams in agony.\n\r", victim->getName());
    sprintf(buf + strlen(buf), "%s cackles in triumph as he utterly annihilates %s.\n\r", getName(), victim->getName());
    descriptor_list->worldSend(buf, this);
    victim->sendTo("We're like closed or something.  Go away.\n\r");
  
    // this handles droping items to ground
    // it use to be in ~TPerson, but do it here since the one in ~TPerson is
    // now an error handler, while this is a known good case.
    TPerson *tper = dynamic_cast<TPerson *>(victim);
    if (tper)
      tper->dropItemsToRoom(SAFE_YES, DROP_IN_ROOM);

    victim->wipeChar(FALSE);
    d->outputProcessing();    // tell them why they suck 
    delete d;
    d = NULL;
    delete victim;
    victim = NULL;
  } else 
    sendTo("Player not online.  Hope you know what you are doing...\n\r");

  if (!load_char(sstring(namebuf).lower(), &st)) {
    sendTo("No such player exists.\n\r");
    return;
  }

  wipePlayerFile(namebuf);
  wipeRentFile(namebuf);
  wipeFollowersFile(namebuf);
  TTrophy *trophy=new TTrophy(namebuf);
  trophy->wipe();
  delete trophy;

  db.query("delete from player where lower(name)=lower('%s')", namebuf);

  sprintf(buf, "account/%c/%s/%s",
         LOWER(st.aname[0]), sstring(st.aname).lower().c_str(), sstring(namebuf).lower().c_str());

  if (unlink(buf) != 0)
    vlogf(LOG_FILE, fmt("error in unlink (7) (%s) %d") %  buf % errno);

  sendTo(fmt("Removing: %s\n\r") % buf);
}

// Command to access, view, and edit player files - Russ

void TBeing::doAccess(const sstring &)
{
  sendTo("Mobs can't access players.\n\r");
  return;
}

void TPerson::doAccess(const sstring &arg)
{
  sstring arg1, arg2, arg3, buf, tmpbuf;
  sstring npasswd;
  char pass[20];
  char filebuf[MAX_STRING_LENGTH];
  char *birth, *tmstr, birth_buf[40];
  charFile st;
  int which, lev, Class, ci;
  time_t ct;
  struct time_info_data playing_time;
  FILE *fp;
  accountFile afp;

  if (powerCheck(POWER_ACCESS))
    return;

  // crypted is used for crypt() function
  // there is apparently a mem leak in that library function
  // to lessen the impact of it, we make it static so we only leak
  // once, rather than once per call.
  static char *crypted;

  static const char *access_args[] = {
    "passwd",
    "name",
    "description",
    "title",
    "level",
    "\n"
  };

  if (!isImmortal())
    return;

  arg1=arg.word(0);
  arg2=arg.word(1);
  arg3=arg.word(2);

  if(arg1.empty()){
    sendTo("Syntax: access <player> (<changes>)\n\r");
    return;
  }
  if (!load_char(arg1, &st)) {
    sendTo("Can't find that player file.\n\r");
    return;
  }
  if(!arg2.empty()){
    for(ci = 0;ci <= RANGER_LEVEL_IND;ci++) {
      if (st.level[ci] > GetMaxLevel()) {
        sendTo("You can't access a player a higher level than you.\n\r");
        return;
      }
    }
  }
  if(!arg2.empty()){
    if (get_pc_world(this, arg1, EXACT_YES)) {
      sendTo("That player is online. Better not mess with the player file.\n\r");
      return;
    }
    if (!hasWizPower(POWER_WIZARD)) {
      sendTo("Only creators can change player files.\n\r");
      sendTo("Low level god syntax : access <player>\n\r");
      return;
    }
    if ((which = old_search_block(arg2.c_str(), 0, arg2.length(), access_args, 0)) == -1) {
      sendTo("Syntax : access <player> <changes/flags>\n\r");
      return;
    }
    switch (which) {
      case 1:
        if (arg3.empty()) {
          sendTo("Syntax : access <player> passwd <newpasswd>\n\r");
          return;
        }
	npasswd = arg3;
        if (npasswd.length() > 10) {
          sendTo("Password must be <= 10 characters.\n\r");
          return;
        }
        crypted = (char *) crypt(npasswd.c_str(), st.aname);
        strncpy(pass, crypted, 10);
        *(pass + 10) = '\0';
        strcpy(st.pwd, pass);
      
        if (!raw_save_char(arg1.lower().c_str(), &st)) {
          vlogf(LOG_MISC, "Ran into problems (#1) saving file in doAccess()");
          return;
        }
        arg1 = fmt("account/%c/%s") % LOWER(st.aname[0]) % sstring(st.aname).lower();
        arg2 = fmt("%s/account") % arg1;
        if (!(fp = fopen(arg2.c_str(), "r+"))) 
          sendTo("Cannot open account for player! Tell a coder!\n\r");
        else {
          fread(&afp, sizeof(afp), 1, fp);
          strcpy(afp.passwd,pass);
          rewind(fp);
          fwrite(&afp, sizeof(accountFile), 1, fp);
          fclose(fp);
        } 
        sendTo("Password changed successfully.\n\r");
        vlogf(LOG_MISC, fmt("%s changed password on %s account") %  getName() % st.aname);
        return;
      case 5:
        if (sscanf(arg.c_str(), "%d %d", &lev, &Class) != 2) {
          sendTo("Syntax : access <player> level <newlevel> <class number>.\n\r");
          return;
        }
        if ((lev < 0) || (lev > GetMaxLevel())) {
          sendTo("You must supply a valid level, >= 0 and < your level.\n\r");
          return;
        }
        if (Class <= RANGER_LEVEL_IND) {
          st.level[Class] = lev;
          sendTo(fmt("Setting level %d in Class %s\n\r") % lev %classInfo[Class].name);
        } else {
          sendTo("Class must be between 0 and 7.\n\r");
          return;
        }
        if (!raw_save_char(arg1.lower().c_str(), &st)) {
          vlogf(LOG_MISC, "Ran into problems (#2) saving file in doAccess()");
          return;
        }
        return;
      default:
        sendTo("Syntax : access <player> (<changes>)\n\r");
        return;
    }
  } else {
    tmpbuf=fmt("Name : <p>%s<1>, Sex : <c>%d<1>, Screensize : <c>%d<1>, Weight <c>%.2f<1>, Height <c>%d<1>\n\r") %
      st.name % st.sex % st.screen % st.weight % st.height;
    buf+=tmpbuf;
    birth = asctime(localtime(&(st.birth)));
    *(birth + strlen(birth) - 1) = '\0';
    strcpy(birth_buf, birth);
    realTimePassed(st.played, 0, &playing_time);
    ct = st.last_logon;
    tmstr = (char *) asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    tmpbuf = fmt("Last login : %s%s%s, Last Host : %s%s%s\n\rFirst Login : %s%s%s\n\r") %
        green() % tmstr % norm() %
        green() % st.lastHost % norm() %
        cyan() % birth_buf % norm();
    buf+=tmpbuf;

    tmpbuf = fmt("Playing time : %d days, %d hours.\n\r") % playing_time.day % playing_time.hours;
    buf+=tmpbuf;

    tmpbuf = fmt("User Levels: M%d C%d W%d T%d A%d D%d K%d R%d") %st.level[0] %st.level[1] %st.level[2] %st.level[3] %st.level[4] %st.level[5] %st.level[6] %st.level[7];
    buf+=tmpbuf;

    tmpbuf = fmt("\tRace: %s\n\r") % RaceNames[st.race];
    buf+=tmpbuf;

    buf+="Stats  :[Str][Bra][Con][Dex][Agi][Int][Wis][Foc][Per][Cha][Kar][Spe]\n\r";

    tmpbuf = fmt("Chosen : %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d\n\r") %
      st.stats[STAT_STR] %
      st.stats[STAT_BRA] %
      st.stats[STAT_CON] %
      st.stats[STAT_DEX] %
      st.stats[STAT_AGI] %
      st.stats[STAT_INT] %
      st.stats[STAT_WIS] %
      st.stats[STAT_FOC] %
      st.stats[STAT_PER] %
      st.stats[STAT_CHA] %
      st.stats[STAT_KAR] %
      st.stats[STAT_SPE];
    buf+=tmpbuf;

    tmpbuf = fmt("Gold:  %d,    Bank:  %d,   Exp:  %.3f\n\r") %
          st.money % st.bankmoney % st.exp;
    buf+=tmpbuf;

    tmpbuf = fmt("Height:  %d,    Weight:  %.1f\n\r") %
          st.height % st.weight;
    buf+=tmpbuf;

    arg1 = fmt("account/%c/%s") % LOWER(st.aname[0]) % sstring(st.aname).lower();
    arg2 = fmt("%s/comment") % arg1;
    if ((fp = fopen(arg2.c_str(), "r"))) {
      while (fgets(filebuf, 255, fp))
	buf+=filebuf;
      fclose(fp);
    }

    arg2 = fmt("%s/account") % arg1;
    if (!(fp = fopen(arg2.c_str(), "r"))) {
      buf+="Cannot open account for player! Tell a coder!\n\r";
    } else {
      fread(&afp, sizeof(afp), 1, fp);
      fclose(fp);
    } 
    if ((afp.flags & ACCOUNT_IMMORTAL) &&
          !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
      buf+="Account name: ***, Account email address : ***\n\r";
      buf+="Account flagged immortal.  Remaining Information Restricted.\n\r";
    } else {
      tmpbuf = fmt("Account name: %s%s%s, Account email address : %s%s%s\n\r") % cyan() % afp.name % norm() % cyan() % afp.email % norm();
      buf+=tmpbuf;

      sstring lStr = "";
      if (IS_SET(afp.flags, ACCOUNT_BANISHED))
        lStr += "<R><f>Account is banished<z>\n\r";
      if (IS_SET(afp.flags, ACCOUNT_EMAIL))
        lStr += "<R><f>Account is email-banished<z>\n\r";

      listAccount(afp.name, lStr);
      buf+=lStr;
    }
    desc->page_string(buf.toCRLF());
    return;
  }
}

void TBeing::doReplace(const sstring &argument)
{
  char buf[256], dir[256], dir2[256];
  sstring arg1, arg2, arg3;
  FILE *fp;
  charFile st;
  bool dontMove = FALSE;

  if (!hasWizPower(POWER_REPLACE)) {
    sendTo("You don't have that power.\n\r");
    return;
  }

  if(argument.empty()) {
    sendTo("Syntax : replace <playername> <player | rent | account> <today | yesterday>\n\r");
    return;
  }
  
  arg1=argument.word(0);
  arg2=argument.word(1);
  arg3=argument.word(2);

  if (arg1.empty() || arg2.empty() || arg3.empty()){
    sendTo("Syntax : replace <playername> <player | rent | account> <today | yesterday>\n\r");
    return;
  }
  if (is_abbrev(arg3, "today"))
    strcpy(dir, "SCCS/bkp.today");
  else if (is_abbrev(arg3, "yesterday"))
    strcpy(dir, "SCCS/bkp.yesterday");
  else {
    sendTo("Syntax : replace <playername | accountname> <player | rent | account> <today | yesterday>\n\r");
    return;
  }
  if (is_abbrev(arg2, "player")) {
    if (powerCheck(POWER_REPLACE_PFILE))
      return;

    strcpy(dir2, "player");
    dontMove = TRUE;
  } else if (is_abbrev(arg2, "account")) 
    strcpy(dir2, "account");
  else if (is_abbrev(arg2, "rent")) 
    strcpy(dir2, "rent");
  else {
    sendTo("Syntax : replace <playername> <player | rent | account> <today | yesterday>\n\r");
    return;
  }
  if (get_pc_world(this, arg1, EXACT_YES)) {
    sendTo("Probably don't want to backup with the person on. Ask them to log off.\n\r");
    return;
  }
  sprintf(buf, "%s/%s/%c/%s", dir, dir2, arg1[0], arg1.c_str());
  if (!(fp = fopen(buf, "r"))) {
    sendTo(fmt("Sorry, can't find file '%s'.\n\r") % buf);
    return;
  } else {
    fclose(fp);

    // log this event so we can see if item duplication (etc.) is caused by it.
    vlogf(LOG_FILE, fmt("%s replacing %s's %s file.") % 
       getName() % arg1 % dir2);

    sprintf(buf, "cp -r %s/%s/%c/%s %s/%c/%s", 
	    dir, dir2, arg1[0], arg1.c_str(), dir2, arg1[0], arg1.c_str());
    vsystem(buf);
    // Make sure that the player file hard link is still intact, if
    // not, saves will be inconsistent - Russ 04-19-96
    if (dontMove) {
      sendTo("Removing and relinking player files...\n\r");
      if (!load_char(arg1, &st)) {
         sendTo("Can't find that player file.\n\r");
         return;
      }
      // Check for account directory here maybe. This won't work
      // if account directory isn't there.

      sprintf(buf, "rm player/%c/%s", arg1[0], arg1.c_str());
      vsystem(buf);
      sprintf(buf, "account/%c/%s/%s", LOWER(st.aname[0]),sstring(st.aname).lower().c_str(),arg1.c_str());
      sprintf(dir2, "player/%c/%s",  arg1[0], arg1.c_str());
      link(buf, dir2); 
      sendTo("Done.\n\r");
    }
    if (!dontMove) {
      sendTo(fmt("Copying backup %s file from %s/%s/%c/%s to %s/%c/%s.\n\r") %
      dir2 % dir % dir2 % arg1[0] % arg1 % dir2 % arg1[0] % arg1);
      return;
    }
  }
}

void TBeing::doSetsev(const char *arg)
{
  int  tMatch;
  char tString[512];
  Descriptor *d;
  const char *tFields[] =
  {
    "miscellaneous", "low"     , "file"  , "bug"    , "procedure",
    "player"       , "immortal", "client", "combat" , "cheat",
    "faction"      , "mobile"  , "mobai" , "mobresp", "object",
    "editor"       , "\n"
  };
  const char *tHelp[] =
  {
    "Anything not found in the others",
    "L.O.W Errors",
    "File I/O Errors",
    "Generic Bugs",
    "Mobile/Object/Room Procedures",
    "Player Login/Logouts",
    "Immortal Info",
    "Client Bugs/Info",
    "Combat Bugs",
    "Cheat Reports/Notifications",
    "Faction Bugs",
    "Generic Mobile Bugs",
    "Mobile AI Bugs",
    "Mobile Response Script Bugs",
    "Generic Object Bugs",
    "Editor Bugs"
  };

  if (!(d = desc))
    return;

  if (powerCheck(POWER_SETSEV))
    return;

  for (; isspace(*arg); arg++);

  bisect_arg(arg, &tMatch, tString, tFields);

  if (!tMatch) {
    sendTo("Leg Severity Options:\n\r______________________________\n\r");

    for (tMatch = 0; tFields[tMatch][0] != '\n'; tMatch++)
      sendTo(fmt("%s: %-15s: %s\n\r") %
             ((d->severity & (1 << tMatch)) ? "On " : "Off") %
             tFields[tMatch] % tHelp[tMatch]);

    return;
  }

  if (tMatch < 1 || tMatch > LOG_MAX) {
    if (is_abbrev(arg, "batopr") && !strcmp(getName(), "Batopr")) {
      if ((d->severity & (1 << LOG_BATOPR)))
        d->severity &= ~(1 << LOG_BATOPR);
      else
        d->severity |= (1 << LOG_BATOPR);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_BATOPR)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "brutius") && !strcmp(getName(), "Brutius")) {
      if ((d->severity & (1 << LOG_BRUTIUS)))
        d->severity &= ~(1 << LOG_BRUTIUS);
      else
        d->severity |= (1 << LOG_BRUTIUS);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_BRUTIUS)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "cosmo") && !strcmp(getName(), "Cosmo")) {
      if ((d->severity & (1 << LOG_COSMO)))
        d->severity &= ~(1 << LOG_COSMO);
      else
        d->severity |= (1 << LOG_COSMO);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_COSMO)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "lapsos") && !strcmp(getName(), "Lapsos")) {
      if ((d->severity & (1 << LOG_LAPSOS)))
        d->severity &= ~(1 << LOG_LAPSOS);
      else
        d->severity |= (1 << LOG_LAPSOS);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_LAPSOS)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "peel") && !strcmp(getName(), "Peel")) {
      if ((d->severity & (1 << LOG_PEEL)))
        d->severity &= ~(1 << LOG_PEEL);
      else
        d->severity |= (1 << LOG_PEEL);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_PEEL)) ? "On" : "Off"));
/*    } else if (is_abbrev(arg, "jesus") && !strcmp(getName(), "Jesus")) {
      if ((d->severity & (1 << LOG_JESUS)))
        d->severity &= ~(1 << LOG_JESUS);
      else
        d->severity |= (1 << LOG_JESUS);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_JESUS)) ? "On" : "Off"));*/
    } else if (is_abbrev(arg, "dash") && !strcmp(getName(), "Dash")) {
      if ((d->severity & (1 << LOG_DASH)))
        d->severity &= ~(1 << LOG_DASH);
      else
        d->severity |= (1 << LOG_DASH);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_DASH)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "maror") && !strcmp(getName(), "Maror")) {
      if ((d->severity & (1 << LOG_MAROR))) d->severity &= ~(1 << LOG_MAROR);
      else d->severity |= (1 << LOG_MAROR);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_MAROR)) ? "On" : "Off"));
    } else if (is_abbrev(arg, "angus") && !strcmp(getName(), "Angus")) {
      if ((d->severity & (1 << LOG_ANGUS))) d->severity &= ~(1 << LOG_ANGUS);
      else d->severity |= (1 << LOG_ANGUS);

      sendTo(fmt("Your Personal Log Messages are now %s\n\r") %
             ((d->severity & (1 << LOG_ANGUS)) ? "On" : "Off"));
    } else
      sendTo("Incorrect Log Type.\n\r");
  } else {
    if ((tMatch - 1) == LOG_LOW && powerCheck(POWER_SETSEV_IMM))
      return;

    if ((d->severity & (1 << (tMatch - 1))))
      d->severity &= ~(1 << (tMatch - 1));
    else
      d->severity |= (1 << (tMatch - 1));

    sendTo(fmt("Log Type %s toggled %s.\n\r") %
           tFields[(tMatch - 1)] %
           ((d->severity & (1 << (tMatch - 1))) ? "On" : "Off"));
  }
}

moneyTypeT & operator++(moneyTypeT &c, int)
{ return (c = ((c == MAX_MONEY_TYPE) ? GOLD_XFER : moneyTypeT(c + 1))); }

void TBeing::doInfo(const char *arg)
{
  Descriptor *i;
  TBeing *ch;
  char buf2[126];
  sstring buf;
  int j, ci;
  char arg1[80];

  if (!hasWizPower(POWER_INFO)) {
    sendTo("You don't have that power.\n\r");
    return;
  }

  if (!isImmortal())
    return;

  arg = one_argument(arg, arg1);
  sstring str = "Information available to you : \n\r";
  str += "commands    : how many times certain commands were used.\n\r";
  str += "objects     : how many objects exist of each time.\n\r";
  str += "unlinked    : how many objects are in the obj list but not in the mud.\n\r";
  str += "tweak       : list or change global parameters.\n\r";
  str += "deaths      : number of player deaths.\n\r";
  str += "piety       : faction piety for everyone online.\n\r";
  str += "descriptors : descriptor numbers for everyone online.\n\r";
  str += "numbers     : some misc stats.\n\r";
  str += "gold        : economy data.\n\r";
  str += "discipline  : info on skill usage in specified disc.\n\r";
  str += "skills      : detailed info on skill usage for specified skill.\n\r";
  str += "mobskills   : same as discipline above, but for mobs.\n\r";
  str += "immskills   : same as discipline above, but for imms.\n\r";

  if (!*arg1) {
    sendTo("What would you like info on?\n\r");
    sendTo(str);
  } else {
    if (is_abbrev(arg1, "commands")) {
      sendTo("Command access information:\n\r");
      sendTo(fmt("  News file accessed %d times.\n\r") % news_used_num);
      sendTo(fmt("  Wiznews file accessed %d times.\n\r") % wiznews_used_num);
      sendTo(fmt("  Wizlist file accessed %d times.\n\r") % wizlist_used_num);
      sendTo(fmt("  Help file(s) accessed %d times since last reboot.\n\r") % help_used_num);
      sendTo(fmt("               accessed %d times total.\n\r") % total_help_number);
      sendTo(fmt("  Typo file accessed %d times.\n\r") % typo_used_num);
      sendTo(fmt("  Bugs file accessed %d times.\n\r") % bug_used_num);
      sendTo(fmt("  Idea file accessed %d times.\n\r") % idea_used_num);
    } else if (is_abbrev(arg1, "objects")) {
      int count[MAX_OBJ_TYPES], i=0, li=0, total=0;
      
      for(i=0;i<MAX_OBJ_TYPES;++i)
	count[i]=0;

      for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter)
	count[(*iter)->itemType()]++;

      // BUBBLESORT IS L33T!!!
      while(1){
	for(i=0;i<MAX_OBJ_TYPES;++i){
	  if(count[i]>count[li])
	    li=i;
	}

	if(count[li]==-1)
	  break;

	buf = fmt("%s[%6i] %-17s\n\r") %
		buf % count[li] % ItemInfo[li]->name;
	total += count[li];
	count[li]=-1;
      }

      buf = fmt("%s[%6i] %-17s\n\r") %
	buf % total % "Total";

      desc->page_string(buf);
    }
    else if(is_abbrev(arg1, "unlinked")){
      TObj *obj;
      int i=1;
      sstring tbuf;

      for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
	obj=*iter;
	if(obj->in_room == ROOM_NOWHERE && !obj->parent &&
	   !obj->equippedBy && !obj->stuckIn && !obj->riding){
	  tbuf=fmt("[%6i] %-17s\n\r") %
		   i++ % obj->getNameNOC(this);
	  buf+=tbuf;
	}
      }
      
      TBeing *tb;

      for(tb=character_list;tb;tb=tb->next){
	if(tb->in_room == ROOM_NOWHERE && !tb->parent &&
	   !tb->equippedBy && !tb->stuckIn && !tb->riding){
	  tbuf =fmt("[%6i] %-17s\n\r") %
	    i++ % tb->getNameNOC(this);
	  buf+=tbuf;
	}	
      }

      desc->page_string(buf);
    }
    else if (is_abbrev(arg1, "tweak")) {
      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You should not attempt to change that.\n\r");
	return;
      }
      char arg2[80];
      arg = one_argument(arg,arg2);
      if (is_abbrev(arg2, "loadrate")) {
        char opt[80];
	arg = one_argument(arg,opt);
	if (is_abbrev(opt, "up"))
	  stats.equip += .05;
	else
	  stats.equip -= .05;
	save_game_stats();
      } else {
	sendTo(fmt("loadrate is %f\n\r") % stats.equip);
      }
    } 
      else if (is_abbrev(arg1, "deaths")) {
      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You cannot access that information.\n\r");
        return;
      }
      sendTo("Player death information:\n\r");
      sendTo(fmt("  Total player deaths : %d.\n\r") % total_deaths);
      sendTo(fmt("  Total player kills  : %d.\n\r") % total_player_kills);
    } else if (is_abbrev(arg1, "piety")) {
      double total = 0.0;
      for (i = descriptor_list; i; i = i->next) {
        if (i->character && i->character->name)  {
          sprintf(buf2, "%20.20s : %10.3f\n\r", i->character->getName(), i->session.perc);
          total += i->session.perc;
          sendTo(buf2);
        }
      }
      sendTo("--------------------\n\r");
      sendTo(fmt("TOTAL: %10.3f\n\r") % total);
    } else if (is_abbrev(arg1, "descriptors")) {
      for (i = descriptor_list; i; i = i->next) {
        if (i->character)  {
          sprintf(buf2,"[%d] ",i->socket->m_sock);
          strcat(buf2,((i->character->name) ? i->character->getName() : "unknown"));
          if (!i->connected)
            strcat(buf2," Connected");
          strcat(buf2,"\n\r");
          sendTo(buf2);
        }
      }
    } else if (is_abbrev(arg1, "numbers")) {
      sendTo("Player number info:\n\r");
      sendTo(fmt("  Current number of players: %u.\n\r") % accStat.player_num);
      sendTo(fmt("  Max number since reboot : %u.\n\r") % accStat.max_player_since_reboot);
      sendTo(fmt("  Max descriptors is presently: %d.\n\r") % maxdesc);
      sendTo(fmt("  Average faction_power is: %.4f\n\r") % avg_faction_power);
      sendTo(fmt("  Number of room-specials: %u\n\r") % roomspec_db.size());

      ci = j = 0;
      for (ch = character_list; ch; ch = ch->next) {
        j++;
        if (ch->discs)
          ci++;
      }
      sendTo(fmt("  Beings with initialized disciplines: %d of %d (%d%c)\n\r") % 
            ci % j %  (ci * 100/ j) % '%');

    } else if (is_abbrev(arg1, "gold")) {
      buf.erase();

#if 0
      float tot_gold = getPosGoldGlobal();
      float tot_gold_shop = getPosGold(GOLD_SHOP);
      float tot_gold_income = getPosGold(GOLD_INCOME);
      float tot_gold_dump = getPosGold(GOLD_DUMP);
      float tot_gold_comm = getPosGold(GOLD_COMM);
      float tot_gold_rent = getPosGold(GOLD_RENT);
      float tot_gold_repair = getPosGold(GOLD_REPAIR);
      float tot_gold_hospital = getPosGold(GOLD_HOSPITAL);
      float tot_gold_shop_sym = getPosGold(GOLD_SHOP_SYMBOL);
      float tot_gold_shop_weap = getPosGold(GOLD_SHOP_WEAPON);
      float tot_gold_shop_arm = getPosGold(GOLD_SHOP_ARMOR);
      float tot_gold_shop_pet = getPosGold(GOLD_SHOP_PET);
      float tot_gold_shop_comp = getPosGold(GOLD_SHOP_COMPONENTS);
      float tot_gold_shop_food = getPosGold(GOLD_SHOP_FOOD);
      float tot_gold_shop_resp = getPosGold(GOLD_SHOP_RESPONSES);
      float tot_gold_gamble = getPosGold(GOLD_GAMBLE);
      float tot_gold_tithe = getPosGold(GOLD_TITHE);
      float tot_gold_allshops = getPosGoldShops();
      float tot_gold_budget = getPosGoldBudget();

      float net_gold = getNetGoldGlobal();
      float net_gold_shop = getNetGold(GOLD_SHOP);
      float net_gold_income = getNetGold(GOLD_INCOME);
      float net_gold_comm = getNetGold(GOLD_COMM);
      float net_gold_rent = getNetGold(GOLD_RENT);
      float net_gold_dump = getNetGold(GOLD_DUMP);
      float net_gold_repair = getNetGold(GOLD_REPAIR);
      float net_gold_hospital = getNetGold(GOLD_HOSPITAL);
      float net_gold_shop_sym = getNetGold(GOLD_SHOP_SYMBOL);
      float net_gold_shop_weap = getNetGold(GOLD_SHOP_WEAPON);
      float net_gold_shop_arm = getNetGold(GOLD_SHOP_ARMOR);
      float net_gold_shop_pet = getNetGold(GOLD_SHOP_PET);
      float net_gold_shop_comp = getNetGold(GOLD_SHOP_COMPONENTS);
      float net_gold_shop_food = getNetGold(GOLD_SHOP_FOOD);
      float net_gold_shop_resp = getNetGold(GOLD_SHOP_RESPONSES);
      float net_gold_gamble = getNetGold(GOLD_GAMBLE);
      float net_gold_tithe = getNetGold(GOLD_TITHE);
      float net_gold_allshops = getNetGoldShops();
      float net_gold_budget = getNetGoldBudget();

      float tot_drain = tot_gold - net_gold;

      sprintf(buf2, "Shop  : modifier: %.2f        (factor  : %.2f%%)\n\r",
           gold_modifier[GOLD_SHOP].getVal(),
           100.0 * (tot_gold_allshops - net_gold_allshops) / tot_gold_allshops);
      buf += buf2;
      sprintf(buf2, "Income: modifier: %.2f        (factor  : %.2f%%)\n\r",
            gold_modifier[GOLD_INCOME].getVal(),
            100.0 * net_gold_budget / tot_gold_budget);
      buf += buf2;
      sprintf(buf2, "Repair: modifier: %.2f        (factor  : %.2f%%)\n\r",
            gold_modifier[GOLD_REPAIR].getVal(),
            100.0 * (tot_gold_budget - net_gold_budget) / tot_drain);
      buf += buf2;
      sprintf(buf2, "Equip: modifier: %.2f        (factor  : %.2f%%)\n\r",
            stats.equip,
            100.0 * (tot_gold_shop_weap + tot_gold_shop_arm) / tot_gold);
      buf += buf2;
      buf += "\n\r";

      sprintf(buf2, "TOTAL ECONOMY:     pos %.2f, net gold = %.2f, drain=%.2f\n\r", tot_gold, net_gold, tot_drain);
      buf += buf2;
      // shops are a little diff from normal
      // want shops to be a slight drain, so compare drain to source
      sprintf(buf2, "SHOP ECONOMY:      pos %.2f, net gold = %.2f, drain=%.2f\n\r", tot_gold_allshops, net_gold_allshops, tot_gold_allshops - net_gold_allshops);
      buf += buf2;
      sprintf(buf2, "BUDGET ECONOMY:    pos %.2f, net gold = %.2f, drain=%.2f\n\r", tot_gold_budget, net_gold_budget, tot_gold_budget - net_gold_budget);
      buf += buf2;

      buf += "\n\r";

      sprintf(buf2, "INCOME ECONOMY:    pos %.2f, net gold = %.2f  (drain: %.2f : %.2f%%)\n\r", tot_gold_income, net_gold_income, tot_gold_income - net_gold_income, 100.0 * (tot_gold_income - net_gold_income) / tot_drain);
      buf += buf2;
      sprintf(buf2, "SHOPS ECONOMY:     pos %.2f, net gold = %.2f  (drain: %.2f : %.2f%%)\n\r", tot_gold_shop, net_gold_shop, tot_gold_shop - net_gold_shop, 100.0 * (tot_gold_shop - net_gold_shop) / tot_drain);
      buf += buf2;

      sprintf(buf2, "FOOD SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_food, net_gold_shop_food, tot_gold_shop_food - net_gold_shop_food, 100.0 * (tot_gold_shop_food - net_gold_shop_food) / tot_drain);
      buf += buf2;

      sprintf(buf2, "COMP SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_comp, net_gold_shop_comp, tot_gold_shop_comp - net_gold_shop_comp, 100.0 * (tot_gold_shop_comp - net_gold_shop_comp) / tot_drain);
      buf += buf2;

      sprintf(buf2, "SYMB SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_sym, net_gold_shop_sym, tot_gold_shop_sym - net_gold_shop_sym, 100.0 * (tot_gold_shop_sym - net_gold_shop_sym) / tot_drain);
      buf += buf2;

      sprintf(buf2, "WEAP SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_weap, net_gold_shop_weap, tot_gold_shop_weap - net_gold_shop_weap, 100.0 * (tot_gold_shop_weap - net_gold_shop_weap) / tot_drain);
      buf += buf2;

      sprintf(buf2, "ARMR SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_arm, net_gold_shop_arm, tot_gold_shop_arm - net_gold_shop_arm, 100.0 * (tot_gold_shop_arm - net_gold_shop_arm) / tot_drain);
      buf += buf2;

      sprintf(buf2, "PETS SHOP ECONOMY: pos %.2f, net gold = %.2f (bad drain: %.2f : %.2f%%)\n\r", tot_gold_shop_pet, net_gold_shop_pet, tot_gold_shop_pet - net_gold_shop_pet, 100.0 * (tot_gold_shop_pet - net_gold_shop_pet) / tot_drain);
      buf += buf2;

      sprintf(buf2, "RESP SHOP ECONOMY: pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_shop_resp, net_gold_shop_resp, tot_gold_shop_resp - net_gold_shop_resp, 100.0 * ((int) tot_gold_shop_resp - net_gold_shop_resp) / (int) tot_drain);
      buf += buf2;

      sprintf(buf2, "COMMODITY ECONOMY: pos %.2f, net gold = %.2f\n\r", tot_gold_comm, net_gold_comm);
      buf += buf2;
      sprintf(buf2, "RENT ECONOMY:      pos %.2f, net gold = %.2f (bad drain: %.2f : %.2f%%)\n\r", tot_gold_rent, net_gold_rent, tot_gold_rent - net_gold_rent, 100.0 * ((int) tot_gold_rent - net_gold_rent) / (int) tot_drain);
      buf += buf2;
      sprintf(buf2, "REPAIR ECONOMY:    pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_repair, net_gold_repair, tot_gold_repair - net_gold_repair, 100.0 * (tot_gold_repair - net_gold_repair) / tot_drain);
      buf += buf2;
      sprintf(buf2, "HOSPITAL ECONOMY:  pos %.2f, net gold = %.2f (drain: %.2f : %.2f%%)\n\r", tot_gold_hospital, net_gold_hospital, tot_gold_hospital - net_gold_hospital, 100.0 * (tot_gold_hospital - net_gold_hospital) / tot_drain);
      buf += buf2;
      sprintf(buf2, "GAMBLE ECONOMY:    pos %.2f, net gold = %.2f\n\r", tot_gold_gamble, net_gold_gamble);
      buf += buf2;
      sprintf(buf2, "TITHE ECONOMY:     pos %.2f, net gold = %.2f\n\r", tot_gold_tithe, net_gold_tithe);
      buf += buf2;
      sprintf(buf2, "DUMP ECONOMY:      pos %.2f, net gold = %.2f\n\r", tot_gold_dump, net_gold_dump);
      buf += buf2;

      desc->page_string(buf);
#elif 0
      buf += "\n\rGold Income/Outlay statistics:\n\r\n\r";
      for (j=0; j < MAX_IMMORT; j++ ) {
        long amount = gold_statistics[GOLD_INCOME][j] + 
                 gold_statistics[GOLD_COMM][j] +
                 gold_statistics[GOLD_GAMBLE][j] +
                 gold_statistics[GOLD_REPAIR][j] +
                 gold_statistics[GOLD_SHOP][j] +
                 gold_statistics[GOLD_DUMP][j] +
                 gold_statistics[GOLD_SHOP_ARMOR][j] +
                 gold_statistics[GOLD_SHOP_WEAPON][j] +
                 gold_statistics[GOLD_SHOP_PET][j] +
                 gold_statistics[GOLD_SHOP_COMPONENTS][j] +
                 gold_statistics[GOLD_SHOP_FOOD][j] +
                 gold_statistics[GOLD_SHOP_RESPONSES][j] +
                 gold_statistics[GOLD_SHOP_SYMBOL][j] +
                 gold_statistics[GOLD_RENT][j] +
                 gold_statistics[GOLD_HOSPITAL][j] +
                 gold_statistics[GOLD_TITHE][j];
        unsigned long pos = gold_positive[GOLD_INCOME][j] + 
                 gold_positive[GOLD_COMM][j] +
                 gold_positive[GOLD_GAMBLE][j] +
                 gold_positive[GOLD_REPAIR][j] +
                 gold_positive[GOLD_SHOP][j] +
                 gold_positive[GOLD_DUMP][j] +
                 gold_positive[GOLD_SHOP_SYMBOL][j] +
                 gold_positive[GOLD_SHOP_ARMOR][j] +
                 gold_positive[GOLD_SHOP_WEAPON][j] +
                 gold_positive[GOLD_SHOP_PET][j] +
                 gold_positive[GOLD_SHOP_RESPONSES][j] +
                 gold_positive[GOLD_SHOP_COMPONENTS][j] +
                 gold_positive[GOLD_SHOP_FOOD][j] +
                 gold_positive[GOLD_RENT][j] +
                 gold_positive[GOLD_HOSPITAL][j] +
                 gold_positive[GOLD_TITHE][j];
        sprintf(buf2, "   %sLevel %2d:%s\n\r",
          cyan(),j+1, norm());
        buf += buf2;
        sprintf(buf2, "         %sPos  : %9ld%s  (%.2f%% of total)\n\r",
          cyan(), pos, norm(),
          100.0 * pos / tot_gold);
        buf += buf2;
        sprintf(buf2, "         %sNet  : %9ld%s  (%.2f%% of total)\n\r",
          cyan(), amount, norm(),
          100.0 * amount / net_gold);
        buf += buf2;
        sprintf(buf2, "         %sDrain: %9ld%s  (%.2f%% of total)\n\r",
          cyan(), pos - amount, norm(),
          100.0 * (pos - amount) / (tot_gold - net_gold));
        buf += buf2;

        sprintf(buf2, "      income     : %8ld, comm       : %8ld, gamble     : %8ld\n\r",
              gold_statistics[GOLD_INCOME][j], 
              gold_statistics[GOLD_COMM][j], 
              gold_statistics[GOLD_GAMBLE][j]);
        buf += buf2;

        sprintf(buf2, "      shop       : %8ld, respon shop: %8ld, repair     : %8ld\n\r",
              gold_statistics[GOLD_SHOP][j], 
              gold_statistics[GOLD_SHOP_RESPONSES][j], 
              gold_statistics[GOLD_REPAIR][j]);
        buf += buf2;

        sprintf(buf2, "      armor shop : %8ld, weapon shop: %8ld, pet shop   : %8ld\n\r",
              gold_statistics[GOLD_SHOP_ARMOR][j], 
              gold_statistics[GOLD_SHOP_WEAPON][j], 
              gold_statistics[GOLD_SHOP_PET][j]);
        buf += buf2;

        sprintf(buf2, "      symbol shop: %8ld, compon shop: %8ld, food shop  : %8ld\n\r",
              gold_statistics[GOLD_SHOP_SYMBOL][j],
              gold_statistics[GOLD_SHOP_COMPONENTS][j],
              gold_statistics[GOLD_SHOP_FOOD][j]);
        buf += buf2;

        sprintf(buf2, "      rent       : %8ld, hospit     : %8ld, tithe      : %8ld\n\r",
              gold_statistics[GOLD_RENT][j], 
              gold_statistics[GOLD_HOSPITAL][j], 
              gold_statistics[GOLD_TITHE][j]);
        buf += buf2;

        sprintf(buf2, "      dump       : %8ld\n\r",
              gold_statistics[GOLD_DUMP][j]);
        buf += buf2;
      }
      desc->page_string(buf);
#elif 1
      unsigned int tTotalGold[MAX_MONEY_TYPE],
                   tTotalGlobal = getPosGoldGlobal(),
                   tTotalShops  = getPosGoldShops(),
                   tTotalRent   = getPosGold(GOLD_RENT),
                   tTotalBudget = getPosGoldBudget();
               int tNetGold[MAX_MONEY_TYPE],
                   tNetGlobal   = getNetGoldGlobal(),
                   tNetShops    = getNetGoldShops(),
                   tNetBudget   = getNetGoldBudget(),
                   tNetRent     = getNetGold(GOLD_RENT);
               int tTotalDrain  = tTotalGlobal - tNetGlobal;

      if (!arg || !*arg) {
        char tNames[MAX_MONEY_TYPE][20] =
        { "X-Fer"         , "Income"  , "Repair"     , "Shop",
          "Commodities"   , "Hospital", "Gamble"     , "Rent",
          "Dump"          , "Tithe"   , "Shop-Symbol", "Shop-Weapon",
          "Shop-Armor"    , "Shop-Pet", "Shop-Food"  , "Shop-Components",
          "Shop-Responses"
        };

        sprintf(buf2, "Modifier: Shop  : %2.2f (Factor: %6.2f%%)\n\r",
                gold_modifier[GOLD_SHOP].getVal(),
                100.0 * (tTotalShops - tNetShops) / tTotalShops);
        buf += buf2;
        sprintf(buf2, "Modifier: Income: %2.2f (Factor: %6.2f%%)\n\r",
                gold_modifier[GOLD_INCOME].getVal(),
                100.0 * tNetBudget / tTotalBudget);
        buf += buf2;
        sprintf(buf2, "Modifier: Repair: %2.2f (Factor: %6.2f%%)\n\r",
                gold_modifier[GOLD_REPAIR].getVal(),
                100.0 * (tTotalBudget - tNetBudget) / tTotalDrain);
        buf += buf2;
        sprintf(buf2, "Modifier: Rent  : %2.2f (Factor: %6.2f%%) (Adjusted: %6.2f%%)\n\r",
                gold_modifier[GOLD_RENT].getVal(),
                100.0 * (tTotalRent - tNetRent) / tTotalDrain,
                100.0 * (tTotalRent - tNetRent) / gold_modifier[GOLD_RENT].getVal() / tTotalDrain);
        buf += buf2;
        sprintf(buf2, "Modifier: Equip : %2.2f (Factor: %6.2f%%)\n\r",
                stats.equip,
                100.0 * (getPosGold(GOLD_SHOP_WEAPON) + getPosGold(GOLD_SHOP_ARMOR)) / tTotalGlobal);
        buf += buf2;
        buf += "\n\r";

        sprintf(buf2, "Economy-Total :   pos = %10u   net gold = %10d   drain = %10d\n\r",
                tTotalGlobal, tNetGlobal, tTotalDrain);
        buf += buf2;

        // Shops are a little Different from normal.
        // Want shops to be a slight drain, so compare drain to source.
        sprintf(buf2, "Economy-Shop  :   pos = %10u   net gold = %10d   drain = %10d\n\r",
                tTotalShops, tNetShops, tTotalShops - tNetShops);
        buf += buf2;
        sprintf(buf2, "Economy-Budget:   pos = %10u   net gold = %10d   drain = %10d\n\r",
                tTotalBudget, tNetBudget, tTotalBudget - tNetBudget);
        buf += buf2;
        buf += "\n\r";

        for (moneyTypeT tMoney = GOLD_XFER; tMoney < MAX_MONEY_TYPE; tMoney++) {
          tTotalGold[tMoney] = getPosGold(tMoney);
          tNetGold[tMoney]   = getNetGold(tMoney);

          sprintf(buf2, "Ecomony-%s:\n\r\tpos = %9u   net gold = %9d (drain: %9d : %6.2f%%)\n\r",
                  tNames[tMoney], tTotalGold[tMoney], tNetGold[tMoney],
                  tTotalGold[tMoney] - tNetGold[tMoney],
                  100.0 * (tTotalGold[tMoney] - tNetGold[tMoney]) / tTotalDrain);
          buf += buf2;
        }
      }

      buf += "\n\rGold Income/Outlay statistics:\n\r\n\r";

      if (arg && *arg && (convertTo<int>(arg) < 1 || convertTo<int>(arg) > MAX_IMMORT))
        buf += "...Invalid Level Specified...\n\r\n\r";

      for (j = ((arg && *arg) ? (convertTo<int>(arg) - 1) : 0); j < MAX_IMMORT; j++) {
        if (j < 0)
          break;

        unsigned long tPosTotal = 0;
                 long tStaTotal = 0;

        for (moneyTypeT tMoney = GOLD_INCOME; tMoney < MAX_MONEY_TYPE; tMoney++) {
          tPosTotal += gold_positive[tMoney][j];
          tStaTotal += gold_statistics[tMoney][j];
        }

        sprintf(buf2, "%sLevel %2d:%s\n\r", cyan(), j + 1, norm());
        buf += buf2;
        sprintf(buf2, "     %sPos  : %9ld%s  (%.2f%% of total)\n\r",
                cyan(), tPosTotal, norm(), 100.0 * tPosTotal / tTotalGlobal);
        buf += buf2;
        sprintf(buf2, "     %sNet  : %9ld%s  (%.2f%% of total)\n\r",
                cyan(), tStaTotal, norm(), 100.0 * tStaTotal / tNetGlobal);
        buf += buf2;
        sprintf(buf2, "     %sDrain: %9ld%s  (%.2f%% of total)\n\r",
                cyan(), tPosTotal - tStaTotal, norm(),
                100.0 * (tPosTotal - tStaTotal) / (tTotalGlobal - tNetGlobal));
        buf += buf2;

        sprintf(buf2, "          Income: %8ld          Comm: %8ld      Gamble: %8ld\n\r",
                gold_statistics[GOLD_INCOME][j],
                gold_statistics[GOLD_COMM][j],
                gold_statistics[GOLD_GAMBLE][j]);
        buf += buf2;
        sprintf(buf2, "            Shop: %8ld     Resp-Shop: %8ld      Repair: %8ld\n\r",
                gold_statistics[GOLD_SHOP][j],
                gold_statistics[GOLD_SHOP_RESPONSES][j],
                gold_statistics[GOLD_REPAIR][j]);
        buf += buf2;
        sprintf(buf2, "      Armor-Shop: %8ld   Weapon-Shop: %8ld    Pet-Shop: %8ld\n\r",
                gold_statistics[GOLD_SHOP_ARMOR][j],
                gold_statistics[GOLD_SHOP_WEAPON][j],
                gold_statistics[GOLD_SHOP_PET][j]);
        buf += buf2;
        sprintf(buf2, "     Symbol-Shop: %8ld     Comp-Shop: %8ld   Good-Shop: %8ld\n\r",
                gold_statistics[GOLD_SHOP_SYMBOL][j],
                gold_statistics[GOLD_SHOP_COMPONENTS][j],
                gold_statistics[GOLD_SHOP_FOOD][j]);
        buf += buf2;
        sprintf(buf2, "            Rent: %8ld      Hospital: %8ld       Tithe: %8ld\n\r",
                gold_statistics[GOLD_RENT][j],
                gold_statistics[GOLD_HOSPITAL][j],
                gold_statistics[GOLD_TITHE][j]);
        buf += buf2;
        sprintf(buf2, "            Dump: %8ld\n\r\n\r",
                gold_statistics[GOLD_DUMP][j]);
        buf += buf2;

        if (arg && *arg)
          break;
      }

      desc->page_string(buf);
#else
      unsigned int tot_gold = getPosGoldGlobal();
      unsigned int tot_gold_shop = getPosGold(GOLD_SHOP);
      unsigned int tot_gold_income = getPosGold(GOLD_INCOME);
      unsigned int tot_gold_dump = getPosGold(GOLD_DUMP);
      unsigned int tot_gold_comm = getPosGold(GOLD_COMM);
      unsigned int tot_gold_rent = getPosGold(GOLD_RENT);
      unsigned int tot_gold_repair = getPosGold(GOLD_REPAIR);
      unsigned int tot_gold_hospital = getPosGold(GOLD_HOSPITAL);
      unsigned int tot_gold_shop_sym = getPosGold(GOLD_SHOP_SYMBOL);
      unsigned int tot_gold_shop_weap = getPosGold(GOLD_SHOP_WEAPON);
      unsigned int tot_gold_shop_arm = getPosGold(GOLD_SHOP_ARMOR);
      unsigned int tot_gold_shop_pet = getPosGold(GOLD_SHOP_PET);
      unsigned int tot_gold_shop_comp = getPosGold(GOLD_SHOP_COMPONENTS);
      unsigned int tot_gold_shop_food = getPosGold(GOLD_SHOP_FOOD);
      unsigned int tot_gold_shop_resp = getPosGold(GOLD_SHOP_RESPONSES);
      unsigned int tot_gold_gamble = getPosGold(GOLD_GAMBLE);
      unsigned int tot_gold_tithe = getPosGold(GOLD_TITHE);
      unsigned int tot_gold_allshops = getPosGoldShops();
      unsigned int tot_gold_budget = getPosGoldBudget();

      int net_gold = getNetGoldGlobal();
      int net_gold_shop = getNetGold(GOLD_SHOP);
      int net_gold_income = getNetGold(GOLD_INCOME);
      int net_gold_comm = getNetGold(GOLD_COMM);
      int net_gold_rent = getNetGold(GOLD_RENT);
      int net_gold_dump = getNetGold(GOLD_DUMP);
      int net_gold_repair = getNetGold(GOLD_REPAIR);
      int net_gold_hospital = getNetGold(GOLD_HOSPITAL);
      int net_gold_shop_sym = getNetGold(GOLD_SHOP_SYMBOL);
      int net_gold_shop_weap = getNetGold(GOLD_SHOP_WEAPON);
      int net_gold_shop_arm = getNetGold(GOLD_SHOP_ARMOR);
      int net_gold_shop_pet = getNetGold(GOLD_SHOP_PET);
      int net_gold_shop_comp = getNetGold(GOLD_SHOP_COMPONENTS);
      int net_gold_shop_food = getNetGold(GOLD_SHOP_FOOD);
      int net_gold_shop_resp = getNetGold(GOLD_SHOP_RESPONSES);
      int net_gold_gamble = getNetGold(GOLD_GAMBLE);
      int net_gold_tithe = getNetGold(GOLD_TITHE);
      int net_gold_allshops = getNetGoldShops();
      int net_gold_budget = getNetGoldBudget();

      int tot_drain = tot_gold - net_gold;

      sprintf(buf2, "Shop  : modifier: %.2f        (factor  : %.2f%%)\n\r",
           gold_modifier[GOLD_SHOP].getVal(),
           100.0 * (tot_gold_allshops - net_gold_allshops) / tot_gold_allshops);
      buf += buf2;
      sprintf(buf2, "Income: modifier: %.2f        (factor  : %.2f%%)\n\r",
            gold_modifier[GOLD_INCOME].getVal(),
            100.0 * net_gold_budget / tot_gold_budget);
      buf += buf2;
      sprintf(buf2, "Repair: modifier: %.2f        (factor  : %.2f%%)\n\r",
            gold_modifier[GOLD_REPAIR].getVal(),
            100.0 * (tot_gold_budget - net_gold_budget) / tot_drain);
      buf += buf2;
      sprintf(buf2, "Equip: modifier: %.2f        (factor  : %.2f%%)\n\r",
            stats.equip,
            100.0 * (tot_gold_shop_weap + tot_gold_shop_arm) / tot_gold);
      buf += buf2;
      buf += "\n\r";

      sprintf(buf2, "TOTAL ECONOMY:     pos %u, net gold = %d, drain=%d\n\r", tot_gold, net_gold, tot_drain);
      buf += buf2;
      // shops are a little diff from normal
      // want shops to be a slight drain, so compare drain to source
      sprintf(buf2, "SHOP ECONOMY:      pos %u, net gold = %d, drain=%d\n\r", tot_gold_allshops, net_gold_allshops, tot_gold_allshops - net_gold_allshops);
      buf += buf2;
      sprintf(buf2, "BUDGET ECONOMY:    pos %u, net gold = %d, drain=%d\n\r", tot_gold_budget, net_gold_budget, tot_gold_budget - net_gold_budget);
      buf += buf2;

      buf += "\n\r";

      sprintf(buf2, "INCOME ECONOMY:    pos %u, net gold = %d  (drain: %d : %.2f%%)\n\r", tot_gold_income, net_gold_income, tot_gold_income - net_gold_income, 100.0 * (tot_gold_income - net_gold_income) / tot_drain);
      buf += buf2;
      sprintf(buf2, "SHOPS ECONOMY:     pos %u, net gold = %d  (drain: %d : %.2f%%)\n\r", tot_gold_shop, net_gold_shop, tot_gold_shop - net_gold_shop, 100.0 * (tot_gold_shop - net_gold_shop) / tot_drain);
      buf += buf2;

      sprintf(buf2, "FOOD SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_food, net_gold_shop_food, tot_gold_shop_food - net_gold_shop_food, 100.0 * (tot_gold_shop_food - net_gold_shop_food) / tot_drain);
      buf += buf2;

      sprintf(buf2, "COMP SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_comp, net_gold_shop_comp, tot_gold_shop_comp - net_gold_shop_comp, 100.0 * (tot_gold_shop_comp - net_gold_shop_comp) / tot_drain);
      buf += buf2;

      sprintf(buf2, "SYMB SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_sym, net_gold_shop_sym, tot_gold_shop_sym - net_gold_shop_sym, 100.0 * (tot_gold_shop_sym - net_gold_shop_sym) / tot_drain);
      buf += buf2;

      sprintf(buf2, "WEAP SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_weap, net_gold_shop_weap, tot_gold_shop_weap - net_gold_shop_weap, 100.0 * (tot_gold_shop_weap - net_gold_shop_weap) / tot_drain);
      buf += buf2;

      sprintf(buf2, "ARMR SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_arm, net_gold_shop_arm, tot_gold_shop_arm - net_gold_shop_arm, 100.0 * (tot_gold_shop_arm - net_gold_shop_arm) / tot_drain);
      buf += buf2;

      sprintf(buf2, "PETS SHOP ECONOMY: pos %u, net gold = %d (bad drain: %d : %.2f%%)\n\r", tot_gold_shop_pet, net_gold_shop_pet, tot_gold_shop_pet - net_gold_shop_pet, 100.0 * (tot_gold_shop_pet - net_gold_shop_pet) / tot_drain);
      buf += buf2;

      sprintf(buf2, "RESP SHOP ECONOMY: pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_shop_resp, net_gold_shop_resp, tot_gold_shop_resp - net_gold_shop_resp, 100.0 * ((int) tot_gold_shop_resp - net_gold_shop_resp) / (int) tot_drain);
      buf += buf2;

      sprintf(buf2, "COMMODITY ECONOMY: pos %u, net gold = %d\n\r", tot_gold_comm, net_gold_comm);
      buf += buf2;
      sprintf(buf2, "RENT ECONOMY:      pos %u, net gold = %d (bad drain: %d : %.2f%%)\n\r", tot_gold_rent, net_gold_rent, tot_gold_rent - net_gold_rent, 100.0 * ((int) tot_gold_rent - net_gold_rent) / (int) tot_drain);
      buf += buf2;
      sprintf(buf2, "REPAIR ECONOMY:    pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_repair, net_gold_repair, tot_gold_repair - net_gold_repair, 100.0 * (tot_gold_repair - net_gold_repair) / tot_drain);
      buf += buf2;
      sprintf(buf2, "HOSPITAL ECONOMY:  pos %u, net gold = %d (drain: %d : %.2f%%)\n\r", tot_gold_hospital, net_gold_hospital, tot_gold_hospital - net_gold_hospital, 100.0 * (tot_gold_hospital - net_gold_hospital) / tot_drain);
      buf += buf2;
      sprintf(buf2, "GAMBLE ECONOMY:    pos %u, net gold = %d\n\r", tot_gold_gamble, net_gold_gamble);
      buf += buf2;
      sprintf(buf2, "TITHE ECONOMY:     pos %u, net gold = %d\n\r", tot_gold_tithe, net_gold_tithe);
      buf += buf2;
      sprintf(buf2, "DUMP ECONOMY:      pos %u, net gold = %d\n\r", tot_gold_dump, net_gold_dump);
      buf += buf2;

      buf += "\n\rGold Income/Outlay statistics:\n\r\n\r";
      for (j=0; j < MAX_IMMORT; j++ ) {
        long amount = gold_statistics[GOLD_INCOME][j] + 
                 gold_statistics[GOLD_COMM][j] +
                 gold_statistics[GOLD_GAMBLE][j] +
                 gold_statistics[GOLD_REPAIR][j] +
                 gold_statistics[GOLD_SHOP][j] +
                 gold_statistics[GOLD_DUMP][j] +
                 gold_statistics[GOLD_SHOP_ARMOR][j] +
                 gold_statistics[GOLD_SHOP_WEAPON][j] +
                 gold_statistics[GOLD_SHOP_PET][j] +
                 gold_statistics[GOLD_SHOP_COMPONENTS][j] +
                 gold_statistics[GOLD_SHOP_FOOD][j] +
                 gold_statistics[GOLD_SHOP_RESPONSES][j] +
                 gold_statistics[GOLD_SHOP_SYMBOL][j] +
                 gold_statistics[GOLD_RENT][j] +
                 gold_statistics[GOLD_HOSPITAL][j] +
                 gold_statistics[GOLD_TITHE][j];
        unsigned long pos = gold_positive[GOLD_INCOME][j] + 
                 gold_positive[GOLD_COMM][j] +
                 gold_positive[GOLD_GAMBLE][j] +
                 gold_positive[GOLD_REPAIR][j] +
                 gold_positive[GOLD_SHOP][j] +
                 gold_positive[GOLD_DUMP][j] +
                 gold_positive[GOLD_SHOP_SYMBOL][j] +
                 gold_positive[GOLD_SHOP_ARMOR][j] +
                 gold_positive[GOLD_SHOP_WEAPON][j] +
                 gold_positive[GOLD_SHOP_PET][j] +
                 gold_positive[GOLD_SHOP_RESPONSES][j] +
                 gold_positive[GOLD_SHOP_COMPONENTS][j] +
                 gold_positive[GOLD_SHOP_FOOD][j] +
                 gold_positive[GOLD_RENT][j] +
                 gold_positive[GOLD_HOSPITAL][j] +
                 gold_positive[GOLD_TITHE][j];
        sprintf(buf2, "   %sLevel %2d:%s\n\r",
          cyan(),j+1, norm());
        buf += buf2;
        sprintf(buf2, "         %sPos  : %9ld%s  (%.2f%% of total)\n\r",
          cyan(), pos, norm(),
          100.0 * pos / tot_gold);
        buf += buf2;
        sprintf(buf2, "         %sNet  : %9ld%s  (%.2f%% of total)\n\r",
          cyan(), amount, norm(),
          100.0 * amount / net_gold);
        buf += buf2;
        sprintf(buf2, "         %sDrain: %9ld%s  (%.2f%% of total)\n\r",
          cyan(), pos - amount, norm(),
          100.0 * (pos - amount) / (tot_gold - net_gold));
        buf += buf2;

        sprintf(buf2, "      income     : %8ld, comm       : %8ld, gamble     : %8ld\n\r",
              gold_statistics[GOLD_INCOME][j], 
              gold_statistics[GOLD_COMM][j], 
              gold_statistics[GOLD_GAMBLE][j]);
        buf += buf2;

        sprintf(buf2, "      shop       : %8ld, respon shop: %8ld, repair     : %8ld\n\r",
              gold_statistics[GOLD_SHOP][j], 
              gold_statistics[GOLD_SHOP_RESPONSES][j], 
              gold_statistics[GOLD_REPAIR][j]);
        buf += buf2;

        sprintf(buf2, "      armor shop : %8ld, weapon shop: %8ld, pet shop   : %8ld\n\r",
              gold_statistics[GOLD_SHOP_ARMOR][j], 
              gold_statistics[GOLD_SHOP_WEAPON][j], 
              gold_statistics[GOLD_SHOP_PET][j]);
        buf += buf2;

        sprintf(buf2, "      symbol shop: %8ld, compon shop: %8ld, food shop  : %8ld\n\r",
              gold_statistics[GOLD_SHOP_SYMBOL][j],
              gold_statistics[GOLD_SHOP_COMPONENTS][j],
              gold_statistics[GOLD_SHOP_FOOD][j]);
        buf += buf2;

        sprintf(buf2, "      rent       : %8ld, hospit     : %8ld, tithe      : %8ld\n\r",
              gold_statistics[GOLD_RENT][j], 
              gold_statistics[GOLD_HOSPITAL][j], 
              gold_statistics[GOLD_TITHE][j]);
        buf += buf2;

        sprintf(buf2, "      dump       : %8ld\n\r",
              gold_statistics[GOLD_DUMP][j]);
        buf += buf2;
      }
      desc->page_string(buf);
#endif
    } else if (is_abbrev(arg1, "discipline")) {
      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You cannot access that information at your level.\n\r");
        return;
      }
      int which = convertTo<int>(arg);
      if (!*arg || which < MIN_DISC || which >= MAX_DISCS) {
        sendTo("Syntax: info discipline <disc #>\n\r");
        return;
      }

      sprintf(buf2,"%-20.20s: %5s %6s %6s %5s %6s %7s %9s %3s\n\r", "SKILL:", " Uses", "   Lev", "Damage", "Victs", "    %", " Crits ", " Suc/Fail", "Sav");
      buf = buf2;
      spellNumT snt;
      for (snt = MIN_SPELL ;snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
        if (!(discArray[snt]->disc == which))
          continue;
        sprintf(buf2,"%-20.20s: %5d %6.2f %6.2f %5d %6.2f %3d/%-3d %4d/%-4d %3d\n\r",
              discArray[snt]->name,
              discArray[snt]->uses,
              (float) (discArray[snt]->uses == 0 ? 0 : (float) discArray[snt]->levels/(float) discArray[snt]->uses),
              (float) (discArray[snt]->victims == 0 ? 0 : (float) discArray[snt]->damage/(float) discArray[snt]->victims),
              discArray[snt]->victims,
              (float) (discArray[snt]->uses == 0 ? 0 : (float) discArray[snt]->learned/(float) discArray[snt]->uses),
              discArray[snt]->crits,discArray[snt]->critf,
              discArray[snt]->success,discArray[snt]->fail,
              discArray[snt]->saves);
        buf += buf2;
      }
      desc->page_string(buf);
    } else if (is_abbrev(arg1, "skills")) {
      arg = one_argument(arg, arg1);

      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You cannot access that information at your level.\n\r");
        return;
      }

      spellNumT which;

      if (is_number(arg1)) {
        which = spellNumT(convertTo<int>(arg1));

        if (which < MIN_SPELL || which >= MAX_SKILL) {
          sendTo(fmt("Syntax: info skills <skill #:%d - %d>\n\r") % MIN_SPELL % (MAX_SKILL + 1));
          return;
        }
      } else {
        for (which = MIN_SPELL; which < MAX_SKILL; which++) {
          if (!discArray[which] || hideThisSpell(which))
            continue;

          if (is_abbrev(arg1, discArray[which]->name))
            break;
        }

        if (which >= MAX_SKILL) {
          sendTo("Unable to find requested skill.\n\r");
          return;
        }
      }

      if (hideThisSpell(which)) {
        sendTo("No such skill exists.\n\r");
        return;
      }

      buf = "";
      sprintf(buf2, "SKILL:      %s\n\r", discArray[which]->name);
      buf += buf2;
      sprintf(buf2, "DIFFICULTY: %s\tLAG: %d rounds\n\r", displayDifficulty(which).c_str(),  discArray[which]->lag);
      buf += buf2;
      sprintf(buf2, "USES:     %d\tMOBUSES: %d\tIMMUSES: %d\n\r", 
           discArray[which]->uses,
           discArray[which]->mobUses,
           discArray[which]->immUses);
      buf += buf2;
      sprintf(buf2, "LEVEL:    %.2f\tMOBLVL:  %.2f\tIMMLVL:  %.2f\n\r", 
           discArray[which]->uses ? (float) discArray[which]->levels / discArray[which]->uses : 0,
           discArray[which]->mobUses ? (float) discArray[which]->mobLevels / discArray[which]->mobUses : 0,
           discArray[which]->immUses ? (float) discArray[which]->immLevels / discArray[which]->immUses : 0);
      buf += buf2;
      sprintf(buf2, "LEARN:    %.2f\tMOBLRN:  %.2f\tIMMLRN:  %.2f\n\r", 
           discArray[which]->uses ? (float) discArray[which]->learned / discArray[which]->uses : 0,
           discArray[which]->mobUses ? (float) discArray[which]->mobLearned / discArray[which]->mobUses : 0,
           discArray[which]->immUses ? (float) discArray[which]->immLearned / discArray[which]->immUses : 0);
      buf += buf2;
      sprintf(buf2, "SUCCESS:  %d\tMOBSUCC: %d\tIMMSUCC: %d\n\r", 
           discArray[which]->success,
           discArray[which]->mobSuccess,
           discArray[which]->immSuccess);
      buf += buf2;
      sprintf(buf2, "FAILS:    %d\tMOBFAIL: %d\tIMMFAIL: %d\n\r", 
           discArray[which]->fail,
           discArray[which]->mobFail,
           discArray[which]->immFail);
      buf += buf2;
      sprintf(buf2, "POTIONS:  %d\tMOB-POT: %d\tIMM-POT: %d\n\r", 
           discArray[which]->potSuccess,
           discArray[which]->potSuccessMob,
           discArray[which]->potSuccessImm);
      buf += buf2;
      // we will keep the potion successes out of this calculation
      // as we basically want to use it to look at if a naturally used skill
      // is succeeding like it ought to
      int tot = discArray[which]->success + discArray[which]->fail;
      int mobtot = discArray[which]->mobSuccess + discArray[which]->mobFail;
      int immtot = discArray[which]->immSuccess + discArray[which]->immFail;
      sprintf(buf2, "RATE :    %.1f%%\tMOBRATE: %.1f%%\tIMMRATE: %.1f%%\n\r", 
           tot ? discArray[which]->success * 100.0 / tot : 0,
           mobtot ? discArray[which]->mobSuccess * 100.0 / mobtot : 0,
           immtot ? discArray[which]->immSuccess * 100.0 / immtot : 0);
      buf += buf2;
      sprintf(buf2, "SAVES:    %d\tMOBSAVE: %d\tIMMSAVE: %d\n\r", 
           discArray[which]->saves,
           discArray[which]->mobSaves,
           discArray[which]->immSaves);
      buf += buf2;
      sprintf(buf2, "CRIT-SUC: %d\tMOBCSUC: %d\tIMMCSUC: %d\n\r", 
           discArray[which]->crits,
           discArray[which]->mobCrits,
           discArray[which]->immCrits);
      buf += buf2;
      sprintf(buf2, "CRIT-FAI: %d\tMOBCFAI: %d\tIMMCFAI: %d\n\r", 
           discArray[which]->critf,
           discArray[which]->mobCritf,
           discArray[which]->immCritf);
      buf += buf2;
      sprintf(buf2, "VICTIMS:  %d\tMOBVICT: %d\tIMMVICT: %d\n\r", 
           discArray[which]->victims,
           discArray[which]->mobVictims,
           discArray[which]->immVictims);
      buf += buf2;
      sprintf(buf2, "DAMAGE:   %.2f\tMOBDAM:  %.2f\tIMMDAM:  %.2f\n\r", 
           discArray[which]->victims ? (float) discArray[which]->damage / discArray[which]->victims : 0,
           discArray[which]->mobVictims ? (float) discArray[which]->mobDamage / discArray[which]->mobVictims : 0,
           discArray[which]->immVictims ? (float) discArray[which]->immDamage / discArray[which]->immVictims : 0);
      buf += buf2;
      sprintf(buf2, "FOCUS:    %.2f\n\r", 
           discArray[which]->uses ? (float) discArray[which]->focusValue / discArray[which]->uses : 0);
      buf += buf2;
      sprintf(buf2, "Potential Victims:  %ld, Potential Damage %.2f, Potential Level %.2f\n\r", 
           discArray[which]->pot_victims,
           discArray[which]->pot_victims ? (float) discArray[which]->pot_damage / discArray[which]->pot_victims : 0,
           discArray[which]->pot_victims ? (float) discArray[which]->pot_level / discArray[which]->pot_victims : 0);
      buf += buf2;

      buf += "\n\rAttempts Breakdown:\n\r";
      sprintf(buf2, "New: %d, low: %d, mid: %d, good: %d, high: %d\n\r",
         discArray[which]->newAttempts,
         discArray[which]->lowAttempts,
         discArray[which]->midAttempts,
         discArray[which]->goodAttempts,
         discArray[which]->highAttempts);
      buf += buf2;

      buf += "Failure Breakdown:\n\r";
      sprintf(buf2, "General: %d, Focus: %d, Engage: %d\n\r",
         discArray[which]->genFail,
         discArray[which]->focFail,
         discArray[which]->focFail);
      buf += buf2;

      buf += "Learning Info:\n\r";
      sprintf(buf2, "Attempts: %ld, Successes: %d, Fails: %d\n\rLearning: %d, Level: %d, Boost: %ld\n\rDisc Success: %d, Adv Success: %d\n\r",
         discArray[which]->learnAttempts,
         discArray[which]->learnSuccess,
         discArray[which]->learnFail,
         discArray[which]->learnLearn,
         discArray[which]->learnLevel,
         discArray[which]->learnBoost,
         discArray[which]->learnDiscSuccess,
         discArray[which]->learnAdvDiscSuccess);
      buf += buf2;

      one_argument(arg, arg1);
      if (*arg1 && is_abbrev(arg1, "note")) {
        TNote *note = createNote(mud_str_dup(buf));
        if (!note) {
          sendTo("No note created in doInfo, tell a God.\n\r");
          return;
        }
        *this += *note;
        sendTo("Note created.\n\r");
      } else
        desc->page_string(buf);
      return;
    } else if (is_abbrev(arg1, "mobskills")) {
      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You cannot access that information at your level.\n\r");
        return;
      }
      int which = convertTo<int>(arg);
      if (which < MIN_DISC || which >= MAX_DISCS) {
        sendTo("Syntax: info mobskills <disc #>\n\r");
        return;
      }

      sprintf(buf2,"%-20.20s: %5s %6s %6s %5s %6s %7s %9s %3s\n\r", "SKILL:", " Uses", "   Lev", "Damage", "Victs", "    %", " Crits ", " Suc/Fail", "Sav");
      buf = buf2;
      spellNumT snt;
      for (snt= MIN_SPELL; snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
        if (!(discArray[snt]->disc == which))
          continue;
        sprintf(buf2,"%-20.20s: %5d %6.2f %6.2f %5d %6.2f %3d/%-3d %4d/%-4d %3d\n\r",
              discArray[snt]->name,
              discArray[snt]->mobUses,
              (float) (discArray[snt]->mobUses == 0 ? 0 : (float) discArray[snt]->mobLevels/(float) discArray[snt]->mobUses),
              (float) (discArray[snt]->mobVictims == 0 ? 0 : (float) discArray[snt]->mobDamage/(float) discArray[snt]->mobVictims),
              discArray[snt]->mobVictims,
              (float) (discArray[snt]->mobUses == 0 ? 0 : (float) discArray[snt]->mobLearned/(float) discArray[snt]->mobUses),
              discArray[snt]->mobCrits, discArray[snt]->mobCritf,
              discArray[snt]->mobSuccess, discArray[snt]->mobFail,
              discArray[snt]->mobSaves);
        buf += buf2;
      }
      desc->page_string(buf);
    } else if (is_abbrev(arg1, "immskills")) {
      if (!hasWizPower(POWER_INFO_TRUSTED)) {
        sendTo("You cannot access that information at your level.\n\r");
        return;
      }
      int which = convertTo<int>(arg);
      if (which < MIN_DISC || which >= MAX_DISCS) {
        sendTo("Syntax: info immskills <disc #>\n\r");
        return;
      }

      sprintf(buf2,"%-20.20s: %5s %6s %6s %5s %6s %7s %9s %3s\n\r", "SKILL:", " Uses", "   Lev", "Damage", "Victs", "    %", " Crits ", " Suc/Fail", "Sav");
      buf = buf2;
      spellNumT snt;
      for (snt= MIN_SPELL; snt < MAX_SKILL; snt++) {
        if (hideThisSpell(snt))
          continue;
        if (!(discArray[snt]->disc == which))
          continue;
        sprintf(buf2,"%-20.20s: %5d %6.2f %6.2f %5d %6.2f %3d/%-3d %4d/%-4d %3d\n\r",
              discArray[snt]->name,
              discArray[snt]->immUses,
              (float) (discArray[snt]->immUses == 0 ? 0 : (float) discArray[snt]->immLevels/(float) discArray[snt]->immUses),
              (float) (discArray[snt]->immVictims == 0 ? 0 : (float) discArray[snt]->immDamage/(float) discArray[snt]->immVictims),
              discArray[snt]->immVictims,
              (float) (discArray[snt]->immUses == 0 ? 0 : (float) discArray[snt]->immLearned/(float) discArray[snt]->immUses),
              discArray[snt]->immCrits,discArray[snt]->immCritf,
              discArray[snt]->immSuccess,discArray[snt]->immFail,
              discArray[snt]->immSaves);
          buf += buf2;
      }
      desc->page_string(buf);
    } else {
      sendTo("What would you like info on?\n\r");
      sendTo(str);
    }
}
}

static int deltatime;

static void TimeTravel(const char *ch)
{
  char fileName[128];
  rentHeader h;
  FILE *fp;
  long delta;

  if (!ch)
    return;

  sprintf(fileName, "rent/%c/%s", ch[0], ch);

  // skip followers data
  if (strlen(fileName) > 4 && !strcmp(&fileName[strlen(fileName) - 4], ".fol"))
    return;
  if (strlen(fileName) > 3 && !strcmp(&fileName[strlen(fileName) - 3], ".fr"))
    return;

  if (!(fp = fopen(fileName, "r+b"))) {
    vlogf(LOG_MISC, fmt("Error updating %s's rent file!") %  ch);
    return;
  }

  if (fread(&h, sizeof(h), 1, fp) != 1) {
    vlogf(LOG_MISC, fmt("  Cannot read rent file header for %s") %  ch);
    fclose(fp);
    return;
  }
  // advance the update counter
  h.last_update += deltatime * SECS_PER_REAL_MIN;

  if (h.last_update > time(0)) {
    // don't advance update counter past the present
    // give some financial credit instead
    delta = h.last_update - time(0);
    h.last_update = time(0);
    unsigned int amt = h.total_cost * delta / SECS_PER_REAL_DAY;
    h.gold_left += amt;
    vlogf(LOG_SILENT, fmt("Crediting %s with %u gold for downtime. (left=%d)") % 
        ch % amt % h.gold_left);
  } else
    vlogf(LOG_SILENT, fmt("TimeTravel for %s done as update-advance only. (left=%d)") % 
        ch % h.gold_left);

  rewind(fp);
  if (fwrite(&h, sizeof(h), 1, fp) != 1) {
    vlogf(LOG_MISC, fmt("Cannot write updated rent file header for %s") %  h.owner);
    fclose(fp);
    return;
  }
  fclose(fp);
  return;
}

void TBeing::doTimeshift(const char *arg)
{
  char buf[256];
  TBeing *i;
  objCost cost;
  int tmp;

  if (!isImmortal())
    return;
  if (powerCheck(POWER_TIMESHIFT))
    return;
  
  if(!(deltatime=convertTo<int>(arg))){
    sendTo("Syntax: timeshift <minutes>\n\r");
    return;
  } else {
#if 0
    if (deltatime < 0) {
      sendTo("Don't be stupid.  Only positive number of minutes.\n\r");
      return;
    }
#endif
    vlogf(LOG_MISC,fmt("%s moving time back %d minutes.") % getName() %deltatime);
    dirwalk("rent/a",TimeTravel);
    dirwalk("rent/b",TimeTravel);
    dirwalk("rent/c",TimeTravel);
    dirwalk("rent/d",TimeTravel);
    dirwalk("rent/e",TimeTravel);
    dirwalk("rent/f",TimeTravel);
    dirwalk("rent/g",TimeTravel);
    dirwalk("rent/h",TimeTravel);
    dirwalk("rent/i",TimeTravel);
    dirwalk("rent/j",TimeTravel);
    dirwalk("rent/k",TimeTravel);
    dirwalk("rent/l",TimeTravel);
    dirwalk("rent/m",TimeTravel);
    dirwalk("rent/n",TimeTravel);
    dirwalk("rent/o",TimeTravel);
    dirwalk("rent/p",TimeTravel);
    dirwalk("rent/q",TimeTravel);
    dirwalk("rent/r",TimeTravel);
    dirwalk("rent/s",TimeTravel);
    dirwalk("rent/t",TimeTravel);
    dirwalk("rent/u",TimeTravel);
    dirwalk("rent/v",TimeTravel);
    dirwalk("rent/w",TimeTravel);
    dirwalk("rent/x",TimeTravel);
    dirwalk("rent/y",TimeTravel);
    dirwalk("rent/z",TimeTravel);
    for (i = character_list; i; i = i->next) {
      if (!i->isPc())
        continue;
      i->recepOffer(NULL,&cost);
      i->addToMoney(max(0,(&cost)->total_cost * deltatime * SECS_PER_REAL_MIN/SECS_PER_REAL_DAY), GOLD_RENT);
    }
    tmp = deltatime * SECS_PER_REAL_MIN;

#if 0
// there's no good reason to do this
// time_info gets recalculated based on real time since a given date at bootup
// so this is semi-pointless
// I'll leave it in the hopes that in the future, mud-time will actually
// be preserved from reboot to reboot and not based on the real world at all
    if (tmp/SECS_PER_MUD_YEAR) {
      time_info.year -= tmp/SECS_PER_MUD_YEAR;
      tmp %= SECS_PER_MUD_YEAR;
    }
    if (tmp/SECS_PER_MUD_MONTH) {
      time_info.month -= tmp/SECS_PER_MUD_MONTH;
      if (time_info.month < 0) {
        time_info.year -= 1;
        time_info.month += 12;
      }
      tmp %= SECS_PER_MUD_MONTH;
    }
    if (tmp/SECS_PER_MUD_DAY) {
      time_info.day -= tmp/SECS_PER_MUD_DAY;
      if (time_info.day < 0) {
        time_info.month -= 1;
        time_info.day += 28;
      }
      if (time_info.month < 0) {
        time_info.year -= 1;
        time_info.month += 12;
      }
      tmp %= SECS_PER_MUD_DAY;
    }
    if (tmp/SECS_PER_MUD_HOUR) {
      time_info.hours -= tmp/SECS_PER_MUD_HOUR;
      if (time_info.hours < 0) {
        time_info.day -= 1;
        time_info.hours += 48;
      }
      if (time_info.day < 0) {
        time_info.month -= 1;
        time_info.day += 28;
      }
      if (time_info.month < 0) {
        time_info.year -= 1;
        time_info.month += 12;
      }
    }
#endif
    sprintf(buf,"%s has shifted game time back %d real minutes.",getName(),deltatime);
    doSystem(buf);
    sprintf(buf,"Adjusting mud-time, rent charges for PC's rented and talens for PC's online.");
    doSystem(buf);
  }
}

void TBeing::doLog(const char *argument)
{
  TBeing *vict;
  char name_buf[100];
  Descriptor *d;
  int found = FALSE;

  if (powerCheck(POWER_LOG))
    return;

  if (!isPc())
    return;

  strcpy(name_buf, argument);

  if (!*name_buf) {
    sendTo("Put who into the logfile?\n\r");
    return;
  }
  if (*name_buf == '-') {
    if (strchr(name_buf, 'w')) {
      for (d = descriptor_list; d; d = d->next) {
        if (!d->connected && d->character) {
          if (d->character->isPlayerAction(PLR_LOGGED)) {
            sendTo(COLOR_MOBS, fmt("%s\n\r") % d->character->getName());
            found = TRUE;
          }
        }
      }
      if (!found) 
        sendTo("No one logged in is currently logged.\n\r");
    } else 
      sendTo("Syntax : log <name> -(w)\n\r");

    return;
  }
  if (!(vict = get_char_vis_world(this, name_buf, NULL, EXACT_YES))) {
    sendTo("No one here by that name.\n\r");
    return;
  } else if (vict->GetMaxLevel() > GetMaxLevel() || vict->isPlayerAction(PLR_NOSNOOP)) {
    sendTo("I don't think they would like that.\n\r");
    return;
  } else if (!vict->isPlayerAction(PLR_LOGGED)) {
    sendTo(COLOR_MOBS, fmt("%s will now be logged.\n\r") % vict->getName());
    sendTo("Please either post a reason for adding the log to the immortal board,\n\r");
    sendTo("or add a comment to the player's account (permanent) stating your reasons.\n\r");
    vict->addPlayerAction(PLR_LOGGED);
  } else {
    vict->remPlayerAction(PLR_LOGGED);
    sendTo("Logged bit has been removed.\n\r");
  }
}

void TBeing::doHostlog(const char *argument)
{
  int a, length, b;
  char buf[256];

  if (powerCheck(POWER_HOSTLOG))
    return;

  if (!isImmortal()) {
    sendTo("You cannot log hosts.\n\r");
    return;
  }
  argument = one_argument(argument, buf);

  if (!*buf) {
    sendTo("Syntax: hostlog {add <host> | rem <host> | list}\n\r");
    return;
  } else if (!strcmp(buf, "add")) {
    argument = one_argument(argument, buf);
    if (!*buf) {
      sendTo("hostlog add <host_name>\n\r");
      return;
    }
    length = strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      sendTo("Host is too long or short, please try again.\n\r");
      return;
    }
    for (a = 0; a <= numberLogHosts - 1; a++) {
      if (!strncmp(hostLogList[a], buf, length)) {
        sendTo("Host is already in hostlog database.\n\r");
        return;
      }
    }
    strcpy(hostLogList[numberLogHosts], buf);
    vlogf(LOG_MISC, fmt("%s has added host %s to the hostlog list.") %  getName() % hostLogList[numberLogHosts]);
    numberLogHosts++;
    return;
  } else if (!strcmp(buf, "rem")) {
    if (numberLogHosts <= 0) {
      sendTo("Host list is empty.\n\r");
      return;
    }
    argument = one_argument(argument, buf);

    if (!*buf) {
      sendTo("Hostlog rem <host_name>\n\r");
      return;
    }
    length = strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      sendTo("Host length is bad, please try again.\n\r");
      return;
    }
    for (a = 0; a <= numberLogHosts - 1; a++) {
      if (!strncmp(hostLogList[a], buf, length)) {
        for (b = a; b <= numberLogHosts; b++)
          strcpy(hostLogList[b], hostLogList[b + 1]);
        vlogf(LOG_MISC, fmt("%s has removed host %s from the hostlog list.") %  getName() % buf);
        numberLogHosts--;
        return;
      }
    }
    sendTo("Host is not in database.\n\r");
    return;
  } else if (!strcmp(buf, "list")) {
    if (numberLogHosts <= 0) {
      sendTo("Host list is empty.\n\r");
      return;
    }
    for (a = 0; a <= numberLogHosts - 1; a++)
      sendTo(fmt("Host: %s\n\r") % hostLogList[a]);

    return;
  } else {
    sendTo("Hostlog {add <host> | rem <host> | list}\n\r");
    return;
  }
  return;
}

void TBeing::doQuest(const char *argument)
{
  char buf[256];
  TBeing *vict;

  if (!isImmortal() || !isPc() || !hasWizPower(POWER_QUEST)) {
    doMortalQuest(argument);
    return;
  }

  argument = one_argument(argument, buf);

  if (!*buf) {
    sendTo("Syntax: quest <victim> <message>\n\r");
    return;
  } else if ((vict = get_char_room_vis(this, buf))) {
    if (vict == this) {
      sendTo("You sure must be bored to do something that dumb.\n\r");
      return;
    }
    if (vict->isImmortal() && (vict->GetMaxLevel() > GetMaxLevel())) {
      sendTo("It's a bad idea to mess with higher level gods.\n\r");
      return;
    }
    doEcho(argument);
    --(*vict);
    thing_to_room(vict, ROOM_Q_STORAGE);
    vict->doLook("", CMD_LOOK);
    vict->doSay("oops");
  } else {
    sendTo("Nobody like that seems to be available.\n\r");
    sendTo("Syntax: quest <victim> <message>\n\r");
    return;
  }
  return;
}

void TBeing::doFindEmail(const char *arg) 
{
  if (!isImmortal()) 
    return;

  if (powerCheck(POWER_FINDEMAIL))
    return;

  for (; isspace(*arg); arg++);    // pass all those spaces 
  if (!*arg) {
    sendTo("Syntax: findemail \"email\"\n\r");
    return;
  }

  char argument[256];
  strcpy(argument, arg);
  cleanCharBuf(argument);
  systask->AddTask(this, SYSTEM_FIND_EMAIL, argument);
}

void TBeing::doSysMid() 
{
  if (!isImmortal()) 
    return;

  systask->AddTask(this, SYSTEM_MAIL_IMMORT_DIR, NULL);
}

void TBeing::doSysTraceroute(const sstring &arg) 
{
  if (powerCheck(POWER_TRACEROUTE))
    return;

  if (!isImmortal())
     return;

  if (arg.empty()) {
    sendTo("Syntax: traceroute <host>\n\r");
    return;
  }

  systask->AddTask(this, SYSTEM_TRACEROUTE, arg.c_str());
}

void TBeing::doSysTasks(const sstring &arg) 
{
  if (!isImmortal())
    return;

#if 0
  if (!*arg) {
    sendTo("Syntax: tasks {enabled | disabled}\n\r");
    return;
  }
#endif

  char argument[256];
  strcpy(argument, arg.c_str());
  cleanCharBuf(argument);
  sstring lst = systask->Tasks(this, argument);
  desc->page_string(lst);
}

void TBeing::doSysLoglist()
{
  if (!hasWizPower(POWER_LOGLIST)) {
    sendTo("You don't have that power.\n\r");
    return;
  }

  if (!isImmortal())
     return;

  systask->AddTask(this, SYSTEM_LOGLIST, NULL);
}

void TBeing::doSysChecklog(const sstring &arg)
{
  char *tMarkerS, // Start
       *tMarkerE, // End
        tString[256],
        tSearch[256],
        tLog[256];
  unsigned int tIndex;

  if (!hasWizPower(POWER_CHECKLOG)) {
    sendTo("You don't have that power.\n\r");
    return;
  }

  if (!isImmortal())
    return;

  if (arg.empty()) {
    sendTo("Syntax: checklog \"string\" logfile\n\rSee loglist for list of logfiles.\n\r");
    return;
  }

  strcpy(tString, arg.c_str());
  cleanCharBuf(tString);

  if (!(tMarkerS = strchr(tString, '"')) ||
      !(tMarkerE = strchr((tMarkerS + 1), '"'))) {
    sendTo("Syntax: checklog \"string\" logfile\n\rSee loglist for list of logfiles.\n\r");
    return;
  }

  strncpy(tSearch, tMarkerS, (tMarkerE - tMarkerS));
  tSearch[(tMarkerE - tMarkerS)] = '\0';

  for (tMarkerE++; *tMarkerE == ' '; tMarkerE++);

  strcpy(tLog, tMarkerE);

  if (!tLog[0]) {
    sendTo("Syntax: checklog \"string\" logfile\n\rSee loglist for list of logfiles.\n\r");
    return;
  }

  for (tIndex = 0; tIndex < strlen(tLog); tIndex++)
    // need to be able to type "log.121299.0000" but not "../" or "."
    if ( tLog[tIndex] == '/' ||
         (tLog[tIndex] == '.' && (tIndex == 0))) {
      sendTo("Illegal log specified.  Bad, Bad god!\n\r");
      return;
    }

  sprintf(tString, "\\%s\\\" %s", tSearch, tLog);
  systask->AddTask(this, SYSTEM_CHECKLOG, tString);

  // There is a tendency for people to see what bad things folks have said
  // about them using checklog.  Obviously, not good, so we log the event
  // and watch for imms blindly checking logs on other imms, etc.
 
  // If we don't trust these people why even have the command?
  if (!hasWizPower(POWER_WIZARD))
    vlogf(LOG_MISC, fmt("%s checklogging: '%s'") %  getName() % tString);
}

void TBeing::doSysViewoutput() 
{
  char  file[32];
  sprintf(file, "tmp/%s.output", getName());

  if (!desc->m_bIsClient) 
    desc->start_page_file( file, "There is nothing to read.\n\r");
  else {
    sstring sb = "";
    file_to_sstring(file, sb);
    sb += "\n\r";
    processStringForClient(sb);
    desc->clientf(fmt("%d") % CLIENT_NOTE);
    sendTo(fmt("%s") % sb);  
    desc->clientf(fmt("%d") % CLIENT_NOTE_END);
  }
}

// dead this: DELETE_THIS
int TBeing::doExec()
{
  TObj *script;
  const char *lptr = NULL;
  char lbuf[4096];
  const char *invalidcmds[] = {"exec", "oedit", "medit", "redit", 
   "bug", "idea", "typo", "description", "comment", 0};
  int  i, broken = FALSE, rc;

  if (!isImmortal())
    return FALSE;

  TThing *t_script = searchLinkedListVis(this, "exec script", getStuff());
  script = dynamic_cast<TObj *>(t_script);
  if (!script) {
    sendTo("You don't have an executable script.\n\r");
    return FALSE;
  }
  //  Make sure there are commands to execute.
  if (!script->action_description) {
    sendTo("The script is empty.\n\r");
    return FALSE;
  }
  //  Loop through the commands and execute them.
  lptr = script->action_description;
  while (lptr) {
    //  Scan the command from the command buffer.
    if (sscanf(lptr, "%[^\n]", lbuf) != 1)
      return FALSE;

    char argument[256];
    strcpy(argument, lbuf);
    //    cleanCharBuf(argument);
    for (i = 0; invalidcmds[i]; ++i) {
      if (is_abbrev(lbuf, invalidcmds[i])) {
        sendTo(fmt("%s isn't an allowed command.\n\r") % invalidcmds[i]);
        lptr = strchr(lptr, '\r');
        if (lptr)
          ++lptr;

        broken = TRUE;
        break;
      }
    }
    if (broken) {
      broken = !broken;
      continue;
    }
    if (*argument)  {
      if ((rc = parseCommand(argument, FALSE)) == DELETE_THIS)
         return DELETE_THIS;

      // remove multi page reports to avoid problems
      if (desc && desc->showstr_head) {
        delete [] desc->showstr_head;
        desc->showstr_head = NULL;
        desc->tot_pages = desc->cur_page = 0;
      }

    }

    lptr = strchr(lptr, '\r');
    if (lptr) 
      ++lptr;
  }
  return 0;
}

void TBeing::doResize(const char *arg)
{
  TBeing *targ;
  TObj *obj = NULL;
  char buf[160], objbuf[80], charbuf[80], racebuf[80];
  int race=0;

  if (!hasWizPower(POWER_RESIZE)) {
    sendTo("You don't have that power.\n\r");
    return;
  }

  arg = one_argument(arg,objbuf);
  arg = one_argument(arg,charbuf);
  if (!*objbuf || !*charbuf) {
    sendTo("Syntax: resize <object> <character>\n\r");
    return;
  }
  if(!strcmp(charbuf, "race")){
    arg = one_argument(arg,racebuf);
    race=convertTo<int>(racebuf);
    if(race >= MAX_RACIAL_TYPES || race < 0){
      sendTo(fmt("Race number %i doesn't exist.\n\r") % race);
      return;
    }
  }

  TThing *t_obj = searchLinkedList(objbuf, getStuff());
  obj = dynamic_cast<TObj *>(t_obj);
  if (!obj) {
    sendTo(fmt("Sorry, You don't seem to have the %s.\n\r") %objbuf);
    return;
  }
  if (!(targ = get_pc_world(this, charbuf, EXACT_NO)) && !race) { 
    sendTo(fmt("I can't seem to find %s.\n\r") %charbuf);
    return;
  }
  if (!dynamic_cast<TBaseClothing *>(obj)) {
    sendTo(COLOR_OBJECTS, fmt("Umm... %s is not a piece of equipment and can't be resized.\n\r") %
         obj->getName());
    return;
  }
  if (obj->objVnum() != -1 &&
      obj_index[obj->getItemIndex()].max_exist <= 10){
    sendTo("Sorry, that artifact is too rare to be resized.\n\r");
    return;
  }
  wearSlotT slot = slot_from_bit(obj->obj_flags.wear_flags);

  if(race_vol_constants[mapSlotToFile(slot)])
    obj->setVolume((int)((double)getHeight() * race_vol_constants[mapSlotToFile(slot)]));
  
  // disassociate from global memory
  obj->swapToStrung();

  //  Remake the obj name.  
  if(obj->objVnum() != -1){
    sprintf(buf, "%s [resized]", obj->name);
    delete [] obj->name;
    obj->name = mud_str_dup(buf);
  }


  // Personalize it
  if(!race){
    sprintf(buf, "This is the personalized object of %s", targ->getName());
    delete [] obj->action_description;
    obj->action_description = mud_str_dup(buf);
    
    act("You just resized $p for $N.", FALSE, this, obj, targ, TO_CHAR);
  } else {
    sprintf(buf, "You just resized $p for race %i.", race);
    act(buf, FALSE, this, obj, 0, TO_CHAR);
  }
}

void TBeing::doHeaven(const sstring &arg)
{
  int num;
  sstring buf;

  if (powerCheck(POWER_HEAVEN))
    return;

  one_argument(arg, buf); 
  if (buf.empty() || !(num = convertTo<int>(buf))) {
    sendTo("Syntax: heaven <hours>\n\r");
    return;
  }

  if (num > 100) {
    sendTo("A higher power has decreed that 100 units is the most you may do at once.\n\r");
    return;
  }

  sendTo("You move the heavens and the world.\n\r");
  for (int i = 0; i < num; i++)
    weatherAndTime(1);
}

void TBeing::doAccount(const sstring &arg)
{
  sstring namebuf = "";
  DIR *dfd;
  sstring my_arg, buf2;
  int count = 1;
  accountFile afp;
  FILE *fp;
  sstring str;

  if (!desc)
    return;

  my_arg = one_argument(arg, namebuf);

  if (namebuf.empty())  {
    if (hasWizPower(POWER_ACCOUNT)) {
      sendTo("Syntax: account <account name> [banished | email | double | triple | immortal]\n\r");
    } else {
      sendTo("Syntax: account <account name>\n\r");
    }
    return;
  }

  if (!hasWizPower(POWER_ACCOUNT)) {
    // person isn't an imm, only let them check their own account
    if (!desc->account || !desc->account->name ||
        desc->account->name == namebuf) {
      sendTo("You may only check your own account.\n\r");
      sendTo("Syntax: account <account name>\n\r");
      return;
    }
  }

  buf2 = fmt("account/%c/%s") % namebuf.lower()[0] % namebuf.lower();
  if (!(dfd = opendir(buf2.c_str()))) {
    sendTo("No account by that name exists.\n\r");
    sendTo("Syntax: account <account name>\n\r");
    sendTo("Please do not attempt to abbreviate the account name.\n\r");
    return;
  }
  closedir(dfd);
  buf2 = fmt("account/%c/%s/account") % namebuf.lower()[0] % namebuf.lower();
  if (!(fp = fopen(buf2.c_str(), "r+"))) {
    sendTo("Cannot open account for player! Tell a coder!\n\r");
    return;
  }

  fread(&afp, sizeof(afp), 1, fp);

  // only let imms do this
  if (hasWizPower(POWER_ACCOUNT)) {
    my_arg = one_argument(arg, buf2);
    if (is_abbrev(my_arg, "banished")) {
      if (IS_SET(afp.flags, ACCOUNT_BANISHED)) {
        REMOVE_BIT(afp.flags, ACCOUNT_BANISHED);
        sendTo(fmt("You have unbanished the %s account.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s unbanished account '%s'") % getName() % afp.name);
      } else {
        SET_BIT(afp.flags, ACCOUNT_BANISHED);
        sendTo(fmt("You have set the %s account banished.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s banished account '%s'") % getName() % afp.name);
      }
      
      rewind(fp);
      fwrite(&afp, sizeof(accountFile), 1, fp);
      fclose(fp);
      return;
    } else if (is_abbrev(my_arg, "email")) {
      if (IS_SET(afp.flags, ACCOUNT_EMAIL)) {
        REMOVE_BIT(afp.flags, ACCOUNT_EMAIL);
        sendTo(fmt("You have un-email-banished the %s account.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s un-email-banished account '%s'") % getName() % afp.name);
      } else {
        SET_BIT(afp.flags, ACCOUNT_EMAIL);
        sendTo(fmt("You have set the %s account email-banished.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s email-banished account '%s'") % getName() % afp.name);
      }
      
      rewind(fp);
      fwrite(&afp, sizeof(accountFile), 1, fp);
      fclose(fp);
      return;
    } else if (is_abbrev(my_arg, "double")) {
      if (powerCheck(POWER_FLAG_IMP_POWER)) {
        fclose(fp);
        return;
      }
  
      if (IS_SET(afp.flags, ACCOUNT_ALLOW_DOUBLECLASS)) {
        REMOVE_BIT(afp.flags, ACCOUNT_ALLOW_DOUBLECLASS);
        sendTo(fmt("You revoke the %s account's ability to double-class.\n\r") % afp.name);
      } else {
        SET_BIT(afp.flags, ACCOUNT_ALLOW_DOUBLECLASS);
        sendTo(fmt("You grant the %s account the ability to double-class.\n\r") % afp.name);
      }
      
      rewind(fp);
      fwrite(&afp, sizeof(accountFile), 1, fp);
      fclose(fp);
      return;
    } else if (is_abbrev(my_arg, "triple")) {
      if (powerCheck(POWER_FLAG_IMP_POWER)) {
        fclose(fp);
        return;
      }
  
      if (IS_SET(afp.flags, ACCOUNT_ALLOW_TRIPLECLASS)) {
        REMOVE_BIT(afp.flags, ACCOUNT_ALLOW_TRIPLECLASS);
        sendTo(fmt("You revoke the %s account's ability to triple-class.\n\r") % afp.name);
      } else {
        SET_BIT(afp.flags, ACCOUNT_ALLOW_TRIPLECLASS);
        sendTo(fmt("You grant the %s account the ability to triple-class.\n\r") % afp.name);
      }
      
      rewind(fp);
      fwrite(&afp, sizeof(accountFile), 1, fp);
      fclose(fp);
      return;
    } else if (is_abbrev(my_arg, "immortal")) {
      // this is not something that should be done (manually) unless a 
      // god has left immortality entirely

      if (powerCheck(POWER_FLAG_IMP_POWER)) {
        fclose(fp);
        return;
      }

      // Given that ACCOUNT_IMMORTAL prevents imms from violating our own
      // rules, it's a bit too tempting for folks to turn off the automatic
      // checks.  I'm cynical and don't trust anyone but myself, so sue me.

      // bat is mostly gone, and i need to remove the imm flag on a retired
      // account, so i'm adding myself. - dash
      sstring tmp_name = getName();

      if ((tmp_name != "Batopr") &&
          (tmp_name != "Lapsos") &&
          (tmp_name != "Damescena") &&
          (tmp_name != "Peel") &&
          (tmp_name != "Dash") &&
          (tmp_name != "Angus")) {
        sendTo("Sorry you suck too much to do this.\n\r");
        return;
      }
  
      if (IS_SET(afp.flags, ACCOUNT_IMMORTAL)) {
        REMOVE_BIT(afp.flags, ACCOUNT_IMMORTAL);
        sendTo(fmt("You un-flag the %s account immortal.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s making account='%s' non-immortal") %  getName() % afp.name);
      } else {
        SET_BIT(afp.flags, ACCOUNT_IMMORTAL);
        sendTo(fmt("You flag the %s account as immortal.\n\r") % afp.name);
        vlogf(LOG_MISC, fmt("%s making account='%s' immortal") %  getName() % afp.name);
      }
      
      rewind(fp);
      fwrite(&afp, sizeof(accountFile), 1, fp);
      fclose(fp);
      return;
    }
  }

  fclose(fp);

  buf2 = fmt("Account email address : %s%s%s\n\r") % cyan() % afp.email % norm();
  str += buf2;

  char *tmstr = (char *) asctime(localtime(&afp.last_logon));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sstring tmpbuf = fmt("Last login : %s%s%s\n\r") %
    green() % tmstr % norm();
  str += tmpbuf;
  

  if ((afp.flags & ACCOUNT_IMMORTAL) && !hasWizPower(POWER_VIEW_IMM_ACCOUNTS)) {
    str += "This account belongs to an immortal.\n\r";
    str += "*** Information Concealed ***\n\r";
    desc->page_string(str);
    return;
  }

  if (IS_SET(afp.flags, ACCOUNT_BANISHED))
    str += "<R><f>Account is banished<z>\n\r";
  if (IS_SET(afp.flags, ACCOUNT_EMAIL))
    str += "<R><f>Account is email-banished<z>\n\r";

  listAccount(afp.name, str);
  if (count == 0)
    str += "No characters in account.\n\r";
  str += "\n\r";

  // only let imms see comments
  if (hasWizPower(POWER_ACCOUNT)) {
    buf2 = fmt("account/%c/%s/comment") % namebuf.lower()[0] % namebuf.lower();
    if ((fp = fopen(buf2.c_str(), "r"))) {
      char buf3[256];
      while (fgets(buf3, 255, fp))
        str += buf3;
      fclose(fp);
    }
  }
  desc->page_string(str.toCRLF());

  return;
}

void TBeing::doClients()
{
  if (powerCheck(POWER_CLIENTS))
    return;

  sstring tString("");

  for (Descriptor *tDesc = descriptor_list; tDesc; tDesc = tDesc->next)
    if (tDesc->m_bIsClient) {
      tString += fmt("%-16s %-34s\n\r") %
              ((tDesc->character && tDesc->character->name) ?
               tDesc->character->name : "UNDEFINED") %
              (!(tDesc->host.empty()) ? tDesc->host : "Host Unknown");
    }

  if (tString.empty()) 
    sendTo("Noone currently logged in with a client.\n\r");
  else
    sendTo(tString);
}

// returns DELETE_THIS if this should be toasted.
// returns TRUE if tbeing given by arg has already been toasted
int TBeing::doCrit(sstring arg)
{
  TThing *weap;
  int dam, rc, mod;
  wearSlotT part;
  TBeing *vict;
  sstring name_buf;
  spellNumT wtype;

  // sometimes, to be really mean in a quest, we set this command to L1
  // so mobs can do it.  We do some atypical things here to let mobs
  // invoke this command
  if (isPc() && !isImmortal()) {
    // in general, prevent mortals from critting
    incorrectCommand();
    return FALSE;
  }
  if ((isPc() || commandArray[CMD_CRIT]->minLevel >= GOD_LEVEL1) &&
      powerCheck(POWER_CRIT)) {
    // deny god without the power
    // deny mob's UNLESS we've lowered the level intentionally
    return FALSE;
  }

  arg = one_argument(arg, name_buf);
  if (!(vict = get_char_room_vis(this, name_buf))) {
    if (!(vict = fight())) {
      sendTo("Syntax: crit {victim} <crit #>\n\r");
      return FALSE;
    }
    mod = convertTo<int>(name_buf);
  } else 
    mod = convertTo<int>(arg);

  if (vict->isImmortal() && (vict->GetMaxLevel() >= GetMaxLevel()) &&
      vict != this){
    sendTo("You may not crit gods of equal or greater level!\n\r");
    return FALSE;
  }
  if (mod < 1 || mod > 100) {
    sendTo("Syntax: crit {victim} <crit #>\n\r");
    sendTo("<crit #> in range 1-100.\n\r");
    return FALSE;
  }
  weap = heldInPrimHand();
  part = vict->getPartHit(this, TRUE);
  dam = getWeaponDam(vict, weap, HAND_PRIMARY);

  if (weap)
    wtype = getAttackType(weap);
  else
    wtype = TYPE_HIT;
  
  rc = critSuccessChance(vict, weap, &part, wtype, &dam, mod);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
    return TRUE;
  } else if (!rc) {
    sendTo("critSuccess returned FALSE for some reason.\n\r");
    return FALSE;
  }
  rc = applyDamage(vict, dam, wtype);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
    return TRUE;
  }
  return FALSE;
}

static bool isSanctionedCommand(cmdTypeT tCmd)
{
  switch (tCmd) {
    case CMD_MEDIT:
    case CMD_OEDIT:
    case CMD_FEDIT:
    case CMD_SEDIT:
    case CMD_REDIT:
    case CMD_RLOAD:
    case CMD_RSAVE:
      return true;
    default:
      return false;
  }

  return false;
}

static bool verifyName(const sstring tStString)
{
           FILE *tFile;
           char  tString[256];
  unsigned int   tValue;
  // Now the 'isCreator' variable is reversed.  If they are a creator
  // it is set to false, if they are not then it's true.
           bool  isCreator = true;

  // Knocks it to lower case then ups the first letter.
  sprintf(tString, "immortals/%s/wizdata",
          tStString.lower().cap().c_str());

  // Wizfile doesn't exist, not an immortal or something else.
  // this is a moot check, go back with a false.
  // *** Since most of the sanct'ed commands revolve around the
  // *** editors it is almost certain we will need the various
  // *** files found in this dir to do squat.  This is a double
  // *** check basically.
  if (!(tFile = fopen(tString, "r")))
    return false;

  fclose(tFile);
  sprintf(tString, "player/%c/%s.wizpower",
          LOWER((tStString.c_str())[0]),
          tStString.lower().c_str());

  // Wizpowers file doesn't exist.  This is really getting bad.
  // Even tho they must be a god we MUST have this file to check
  // for there 'level' so we return false.
  if (!(tFile = fopen(tString, "r")))
    return false;

  // Simply see if they have the creator power.
  while (fscanf(tFile, "%d", &tValue) == 1)
    if (mapFileToWizPower(tValue) == POWER_WIZARD)
      isCreator = false;

  fclose(tFile);
  return isCreator;
}
/*
  All this does at the moment is renames us then performs the command
  then changes us back.  This Really should be setup to replace the
  account information but at the moment this is enough to test/verify
  its ability.  We use the Creator check down in doAs() to make
  sure it's Only used by trusted people.
 */
int TBeing::doAsOther(const sstring &tStString)
{
  sstring   tStNewName(""),
           tStCommand(""),
           tStArguments(""),
           tStOriginalName("");
  int      tRc = FALSE;
  cmdTypeT tCmd;
  sstring tStTmp=tStString;

  tStTmp = one_argument(tStTmp, tStNewName);
  tStTmp = one_argument(tStTmp, tStCommand);
  tStArguments=tStTmp;

  tCmd = searchForCommandNum(tStCommand);

  if (!isSanctionedCommand(tCmd)) {
    sendTo("You can not do this.  Sorry.\n\r");
    return FALSE;
  }

  if (!getName()) {
    sendTo("O.k.  You have no name, go get one then come back.\n\r");
    return FALSE;
  }

  if(sstring(getName()).lower() == tStNewName.lower()){
    sendTo("If you want to do it that bad, just do it man!\n\r");
    return FALSE;
  }

  if (!verifyName(tStNewName)) {
    sendTo("They are either not a god or are also a creator, sorry.\n\r");
    return FALSE;
  }

  tStOriginalName = getName();
  delete [] name;
  name = mud_str_dup(tStNewName.lower().cap());
  tStCommand += " ";
  tStCommand += tStArguments;

  tRc = parseCommand(tStCommand.c_str(), false);

  // This is more of a sanity check than anything.  Moot in most
  // senses but still a safty thing.
  if (this) {
    delete [] name;
    name = mud_str_dup(tStOriginalName);
  }

  return tRc;
}

int TBeing::doAs(const char *arg)
{
  char namebuf[80];
  int rc;

  if (!desc || !desc->original || desc->original->GetMaxLevel() <= MAX_MORT) {
    // not connected, not switched, not a god
    if (desc && isImmortal() && hasWizPower(POWER_WIZARD)) {
      // this is pretty weird, but I guess the LOWS want to be able to
      // use other imms MED, OED, RED, etc
      return doAsOther(arg);
    } else
      incorrectCommand();

    return FALSE;
  }

  // we are guaranteed to be a switched critter here

  arg = one_argument(arg, namebuf);
  // theoretically, we could insure that they are As'ing as right char
  // but lets just ignore that
  
  // for a switched, desc->original->desc == NULL
  // output will be hosed unless we change this
  desc->original->desc = desc;

  // do command, handle output
  rc = desc->original->parseCommand(arg, FALSE);

  // at this point, "this" might be screwy due to it dying
  // as XX purge with both in same room, etc
  // Probably simplist check for this will be to see if I still have a desc
  // lack of desc implies that I was forced to return inside the parseCommand()
  // as XX return leaves desc valid, but removes desc->original, check this too
  if (!desc || !desc->original)
    return rc;

  // the other possibility is that they did something that zapped the AS'er.
  // this is event where rc == DELETE_THIS
  // handle this by forcing a return
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    vlogf(LOG_BUG, fmt("doAs(): %s somehow killed self: %s") % 
            desc->original->getName() % arg);
    sendTo("Please don't do things that will cause your original character to be destroyed.\n\r");
    remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
    doReturn("", WEAR_NOWHERE, CMD_RETURN);
    return FALSE;
  }

  desc->outputProcessing();

  // fix original's desc
  desc->original->desc = NULL;

  return rc;
}

void TBeing::doComment(const char *argument)
{
  char arg[256];
  charFile st;

  if (powerCheck(POWER_COMMENT))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sendTo("Syntax: comment <player>\n\r");
    return;
  }

  if (!load_char(arg, &st)) {
    sendTo("That player does not exist.\n\r");
    return;
  }
  if (!desc) {
    sendTo("Go away.\n\r");
    return;
  }

  sendTo(fmt("Write your comment about %s, use ~ when done, or ` to cancel.\n\r") % arg);
  sendTo("Please do not make stupid comments, just list things that are important.\n\r");

  addPlayerAction(PLR_BUGGING);
  desc->connected = CON_WRITING;
  strcpy(desc->name, "Comment");
  strcpy(desc->delname, st.aname);

  desc->str = new const char *('\0');

  desc->max_str = MAX_MAIL_SIZE;
  if (desc->m_bIsClient)
    desc->clientf(fmt("%d") % CLIENT_STARTEDIT % MAX_MAIL_SIZE);

  return;
}

immortalTypeT TPerson::isImmortal(int lev) const
{
  return (isPlayerAction(PLR_IMMORTAL) &&
          (GetMaxLevel() >= max(lev, GOD_LEVEL1))) ? IMMORTAL_YES : IMMORTAL_NO;
}

bool TBeing::inQuest() const
{
  return (isPlayerAction(PLR_SOLOQUEST) || isPlayerAction(PLR_GRPQUEST));
}
