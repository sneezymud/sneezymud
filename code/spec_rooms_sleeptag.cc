/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_rooms_sleeptag.cc"
  All functions and routines related to the sleeptag automated game.

  Created 12/05/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"

const int SLEEPTAG_CONTROL_ROOM = 48;

void sleepTagReport(int, const char *, ...);

struct SPEntry
{
  bool isActive;

  TThing *tPlayer;
  struct SPEntry *tNext;
};

struct SSTControl
{
  bool   isActive;
  time_t tTimeStarted;

  struct SPEntry *tPlayer;
};

void sleepTagEndGame(SSTControl *);


/*
  To-Be-Added:
    1. Code to start/enter game.
    2. Code to remove staffs from people leaving control room.
    3. Code to monitor isActive player movements for the in-room party.
    4. Code to end a game, declare a winner and specify total time passed.
    5. Code to force ending of a game.
    6. Code to prevent control <-> players 'tell' communication.
 */
int sleepTagControl(TBeing *tBeing, cmdTypeT tCmd, const char *tArg, TRoom *tRoom)
{
  SSTControl *tJob   = NULL;
  SPEntry    *tEntry = NULL;

  if (tRoom->act_ptr)
    tJob = static_cast<SSTControl *>(tRoom->act_ptr);

  switch (tCmd) {
    case CMD_GENERIC_PULSE:
      if (tJob && tJob->isActive) {
        for (tEntry = tJob->tPlayer; tEntry; tEntry = tEntry->tNext) {
          if (!tEntry->isActive)
            continue;

          // If they are active an in the game, then just return;
          if (tEntry->tPlayer->roomp && tEntry->tPlayer->roomp->getZone() == tRoom->getZone())
            return FALSE;

          tEntry->isActive = false;
        }

        // We have 0 active players and the game is still active.  End it now.
        sleepTagEndGame(tJob);
        delete static_cast<SSTControl *>(tRoom->act_ptr);
        tRoom->act_ptr = NULL;
      }

    default:
      break;
  }

  return FALSE;
}

void sleepTagEndGame(SSTControl *tJob)
{
  SPEntry *tEntry,
          *tNextEntry;

  for (tEntry = tJob->tPlayer; tEntry; ) {
    tNextEntry = tEntry->tNext;
    delete tEntry;
    tEntry = tNextEntry;
  }

  tJob->tPlayer = NULL;
}

int sleepTagRoom(TBeing *tBeing, cmdTypeT tCmd, const char *tArg, TRoom *tRoom)
{
  TThing *tThing;
  TBeing *tPerson;
  TRoom  *tRP;
  TStaff *tStaff = NULL;
  TWand  *tWand = NULL;

  switch (tCmd) {
    case CMD_GENERIC_PULSE:
      for (tThing = tRoom->stuff; tThing; tThing = tThing->nextThing)
        if ((tPerson = dynamic_cast<TBeing *>(tThing))) {
          tPerson->setMove(tPerson->getMove());

          if (tPerson->isPc()) {
            tPerson->setCond(FULL, 20);
            tPerson->setCond(THIRST, 24);

            if (tPerson->isAffected(AFF_SLEEP))
              if ((tRP = real_roomp(SLEEPTAG_CONTROL_ROOM))) {
                --(*tPerson);
                *tRP += *tPerson;
              } else
                vlogf(0, "Unable to load room %d for sleeptag move!", SLEEPTAG_CONTROL_ROOM);

            sleepTagReport(tRoom->getZone(), "%s has lost this game...", tPerson->getName());
          } else {
            if (tPerson->isAffected(AFF_SLEEP)) {
              --(*tPerson);
              delete tPerson;
              tPerson = NULL;
            } else
              ;
	      //tPerson->sleepTagMonsterSearch();
          }
        }

      return TRUE;
      break;
    case CMD_SHOUT:
      if (!tArg || !*tArg)
        tBeing->sendTo("Shout is good, but usually you shout something...\n\r");
      else
        sleepTagReport(tRoom->getZone(),
                       "<g>%s<z> shouts, \"%s<1>\"\n\r", tBeing->getName(), tArg);

      return TRUE;
      break;
    case CMD_TRANSFER:
      if (!tBeing->hasWizPower(POWER_WIZARD)) {
        tBeing->sendTo("I'm afraid you can not do that here.\n\r");
        return TRUE;
      }

      break;
    case CMD_USE:
      if (!tBeing->equipment[HOLD_LEFT] && !tBeing->equipment[HOLD_RIGHT])
        tBeing->sendTo("You must be something in order to use something.\n\r");
      else if (!tArg || !*tArg)
        tBeing->sendTo("Use is good, but usually you use something...\n\r");
      else if (!is_abbrev(tArg, tBeing->equipment[HOLD_LEFT]->getName()) &&
               !is_abbrev(tArg, tBeing->equipment[HOLD_RIGHT]->getName()))
        tBeing->sendTo("You are not holding that.  Sorry...\n\r");
      else {
        if (is_abbrev(tArg, tBeing->equipment[HOLD_LEFT]->getName()))
          tThing = tBeing->equipment[HOLD_LEFT];
        else
          tThing = tBeing->equipment[HOLD_RIGHT];

        if (!(tStaff = dynamic_cast<TStaff *>(tThing)) &&
            !(tWand  = dynamic_cast<TWand  *>(tThing)))
          tBeing->sendTo("That object is neither staff nor wand, not allowed here.\n\r");
        else if ((tStaff && tStaff->getSpell() != SPELL_SLUMBER) ||
                 (tWand  && tWand->getSpell() != SPELL_SLUMBER))
          tBeing->sendTo("That is not a slumber attempt, bad bad person!\n\r");
        else {
          // Set charges to 1 so the following useMe eats it up.
          // This keeps the staff technically 'empty'
          if (tStaff)
            tStaff->setCurCharges(1);
          else
            tWand->setCurCharges(1);

          int tRc = (tStaff ? tStaff->useMe(tBeing, NULL) : tWand->useMe(tBeing, NULL));

          if (IS_SET_DELETE(tRc, DELETE_THIS)) {
            if (tStaff) {
              delete tStaff;
              tStaff = NULL;
            } else {
              delete tWand;
              tWand = NULL;
            }
          }

          if (IS_SET_DELETE(tRc, DELETE_VICT))
            return DELETE_VICT;
        }
      }
      break;
    case CMD_POUR:
    case CMD_HIT:
    case CMD_CAST:
    case CMD_ORDER:
    case CMD_SNEAK:
    case CMD_HIDE:
    case CMD_BACKSTAB:
    case CMD_PICK:
    case CMD_STEAL:
    case CMD_BASH:
    case CMD_RESCUE:
    case CMD_KICK:
    case CMD_PRAY:
    case CMD_RECITE:
    case CMD_SLAY:
    case CMD_TRACK:
    case CMD_BODYSLAM:
    case CMD_INVISIBLE:
    case CMD_SHOVE:
    case CMD_HEADBUTT:
    case CMD_SUBTERFUGE:
    case CMD_THROW:
    case CMD_SCRIBE:
    case CMD_BREW:
    case CMD_GRAPPLE:
    case CMD_QUIVPALM:
    case CMD_FEIGNDEATH:
    case CMD_SPRINGLEAP:
    case CMD_MEND_LIMB:
    case CMD_LAYHANDS:
    case CMD_SET_TRAP:
    case CMD_SHOOT:
    case CMD_ATTACK:
    case CMD_KILL:
    case CMD_TURN:
    case CMD_BERSERK:
    case CMD_KNEESTRIKE:
    case CMD_POISON_WEAPON:
    case CMD_GARROTTE:
    case CMD_STAB:
    case CMD_CUDGEL:
    case CMD_SMITE:
    case CMD_CHARGE:
    case CMD_HURL:
    case CMD_SEEKWATER:
    case CMD_FORAGE:
    case CMD_APPLY_HERBS:
    case CMD_STOMP:
    case CMD_SHOULDER_THROW:
    case CMD_CHOP:
    case CMD_DIVINE:
    case CMD_BUTCHER:
    case CMD_SKIN:
    case CMD_TAN:
    case CMD_CHI:
    case CMD_DISENGAGE:
    case CMD_CONCEAL:
    case CMD_CAMP:
    case CMD_CHARM:
    case CMD_BEFRIEND:
    case CMD_TRANSFIX:
    case CMD_BARKSKIN:
    case CMD_TRANSFORM:
    case CMD_WHITTLE:
    case CMD_SMOKE:
    case CMD_RETRAIN:
      tBeing->sendTo("This command has been sanctioned as not-usable.  Sorry...\n\r");
      return TRUE;
      break;
    case CMD_IDEA:
    case CMD_TYPO:
    case CMD_BUG:
    case CMD_WRITE:
      tBeing->sendTo("Wait til you leave this place first partner!\n\r");
      return TRUE;
      break;
    default:
      break;
  }

  return FALSE;
}

void sleepTagReport(int tZone, const char *tString, ...)
{
  TBeing *tBeing;
  char   *tBuffer;
  va_list tAp;

  tBuffer = new char[2 * MAX_STRING_LENGTH];

  va_start(tAp, tString);
  (void) vsnprintf(tBuffer, (2 * MAX_STRING_LENGTH), tString, tAp);
  va_end(tAp);

  for (tBeing = character_list; tBeing; tBeing = tBeing->next)
    if (tBeing->isPc() && tBeing->roomp &&
        tBeing->roomp->getZone() == tZone &&
        dynamic_cast<TPerson *>(tBeing))
      tBeing->sendTo(COLOR_COMM, tBuffer);

  delete tBuffer;
  tBuffer = NULL;
}
