//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "room.h"
#include "extern.h"
#include "handler.h"
#include "being.h"
#include "monster.h"
#include "spec_mobs.h"

extern void warn_busy(TBeing*);

void stop_charge(TBeing* ch) {
  ch->sendTo("You pull your steed to a violent halt!\n\r");
  act("$n pulls $N to a violent halt.", FALSE, ch, 0, ch->riding, TO_ROOM);
  ch->stopTask();
}

int TThing::ChargePulse(TBeing* ch) {
  ch->sendTo("We think were funny today, don't we?\n\r");
  ch->stopTask();
  return TRUE;
}

// Streamlined charge movement function
// Returns: TRUE = successful move, FALSE = move failed, DELETE_THIS = character died
int taskChargeMoveInto(int to_room, TBeing* ch, bool moveHorse) {
  TRoom *from_here, *to_here;
  TBeing* tHorse;
  dirTypeT dir;

  from_here = ch->roomp;
  to_here = real_roomp(to_room);
  tHorse = dynamic_cast<TBeing*>(ch->riding);
  dir = (dirTypeT)ch->task->flags;

  // Handle dismounted/flying case (character was thrown from mount)
  if (!moveHorse) {
    // Use normal movement system for dismounted character
    int moveResult = ch->rawMove(dir);
    if (IS_SET_DELETE(moveResult, DELETE_THIS))
      return DELETE_THIS;

    if (moveResult == FALSE) {
      ch->sendTo("You slam into something which stops your flight.\n\r");
      if (ch->reconcileDamage(ch, ::number(10, 20), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
      return FALSE;
    }

    return TRUE;
  }

  // Handle mounted case - use riding system for movement
  if (!tHorse) {
    ch->sendTo("You need to be mounted to charge!\n\r");
    stop_charge(ch);
    return FALSE;
  }

  // Charge-specific mount restrictions
  if (to_here->isWaterSector() || from_here->isWaterSector()) {
    ch->sendTo("Your mount refuses to charge through water.\n\r");
    stop_charge(ch);
    return FALSE;
  }

  if (to_here->isUnderwaterSector() || from_here->isUnderwaterSector()) {
    ch->sendTo("Your mount refuses to charge underwater.\n\r");
    stop_charge(ch);
    return FALSE;
  }

  if ((to_here->isFlyingSector() || to_here->isAirSector()) && !tHorse->isFlying()) {
    ch->sendTo("Your mount refuses to charge into the air.\n\r");
    stop_charge(ch);
    return FALSE;
  }

  if (to_here->isVertSector()) {
    ch->sendTo("Your mount refuses to charge in that direction.\n\r");
    stop_charge(ch);
    return FALSE;
  }

  // Use normal mounted movement system
  int moveResult = ch->rawMove(dir);
  if (IS_SET_DELETE(moveResult, DELETE_THIS))
    return DELETE_THIS;

  if (moveResult == FALSE) {
    ch->sendTo("Your mount refuses to continue the charge.\n\r");
    stop_charge(ch);
    return FALSE;
  }

  return TRUE;
}

// is called when the player is just charging into the room.
// First stop in moving to the next room.
// Make basic checks and act appropriatly.
int ChargeRoom(TBeing* ch) {
  int nRc;

  nRc = taskChargeMoveInto(ch->roomp->dir_option[ch->task->flags]->to_room, ch,
    true);

  if (IS_SET_DELETE(nRc, DELETE_THIS))
    return DELETE_THIS;

  act("$n charges into the room upon $N.", FALSE, ch, 0, ch->riding, TO_ROOM);
  ch->doLook("", CMD_LOOK);

  return TRUE;
}

// is called when player is jettisoned from his mount into the next room.
// First stop in flying to next room.
// Make basic checks and act appropriatly.
int ChargeFlyIntoRoom(TBeing* ch, roomDirData* rExit) {
  int nRoom = rExit->to_room, nRc;
  sstring nString;

  nRc = taskChargeMoveInto(nRoom, ch, false);

  if (ch)
    ch->stopTask();

  if (IS_SET_DELETE(nRc, DELETE_THIS))
    return DELETE_THIS;

  if (nRc == TRUE || nRc == FALSE)
    return TRUE;

  nString = format("$n suddenly flies into the room and smashes into the %s.") %
            real_roomp(nRoom)->describeGround();
  act(nString, TRUE, ch, 0, 0, TO_ROOM);
  ch->sendTo(format("You fly into the next room and smash into the %s.\n\r") %
             real_roomp(nRoom)->describeGround());
  ch->doLook("", CMD_LOOK);

  return TRUE;
}

int ChargeHitDoor(TBeing* ch, roomDirData* rExit) {
  int Damage = 0;
  float FracDam;
  TBeing* tHorse;

  tHorse = dynamic_cast<TBeing*>(ch->riding);
  ch->sendTo(format("You charge towards %s!\n\r") %
             (IS_SET(rExit->condition, EXIT_CLOSED) ? "a door" : "the exit"));

  if (!ch->isAgile(0) && tHorse && !tHorse->hasSaddle() && !::number(0, 3)) {
    Damage = ::number(10, 20);
    FracDam = (float)Damage / 100;
    Damage = (int)(ch->getHit() * FracDam);

    Damage = max(10, (ch->getHit() > 0 ? Damage : 10));

    if (IS_SET(rExit->condition, EXIT_CLOSED)) {
      act("$N suddenly halts, sending you flying into the door.", TRUE, ch, 0,
        ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into a door.", TRUE, ch, 0,
        ch->riding, TO_ROOM);
    } else if (IS_SET(rExit->condition, EXIT_CAVED_IN)) {
      act("$N suddenly halts, sending you flying into the cave in.", TRUE, ch,
        0, ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into a cave in.", TRUE, ch, 0,
        ch->riding, TO_ROOM);
    } else if (IS_SET(rExit->condition, EXIT_NOENTER) ||
               IS_SET(rExit->condition, EXIT_WARDED)) {
      act("$N suddenly halts, sending you flying towards the exit.", TRUE, ch,
        0, ch->riding, TO_CHAR);
      ch->sendTo("You suddenly hit something and fall to the ground.\n\r");
      act("$N suddenly halts, sending $n flying towards an exit.", TRUE, ch, 0,
        ch->riding, TO_ROOM);
      act("$n suddenly hits something and falls to the ground.", TRUE, ch, 0,
        ch->riding, TO_ROOM);
    } else {
      act("$N suddenly halts, sending you flying into the next room.", TRUE, ch,
        0, ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into the next room.", TRUE, ch,
        0, ch->riding, TO_ROOM);
      ChargeFlyIntoRoom(ch, rExit);
    }

    if (ch->reconcileDamage(ch, Damage, DAMAGE_NORMAL) == -1)
      return DELETE_THIS;
  } else {
    if (tHorse && tHorse->hasSaddle()) {
      act("$N suddenly halts, but you luckily stay in the saddle.", TRUE, ch, 0,
        ch->riding, TO_CHAR);
      act("$N suddenly halts, luckly $n was able to stay in the saddle.", TRUE,
        ch, 0, ch->riding, TO_ROOM);
    } else {
      act("$N suddenly halts, but you were able to stay seated...this time.",
        TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, luckly $n was able to stay seated.", TRUE, ch, 0,
        ch->riding, TO_ROOM);
    }
  }

  ch->stopTask();
  return TRUE;
}

int ChargeHitWall(TBeing* ch) {
  int Damage;
  float FracDam;
  TBeing* tHorse;

  tHorse = dynamic_cast<TBeing*>(ch->riding);
  ch->stopTask();

  ch->sendTo("You charge towards a wall!\n\r");
  if (!ch->isAgile(0) && tHorse && !tHorse->hasSaddle() && !::number(0, 3)) {
    act("$N suddenly halts, sending you flying into the wall.", TRUE, ch, 0,
      ch->riding, TO_CHAR);

    Damage = ::number(10, 20);
    FracDam = (float)Damage / 100;
    Damage = (int)(ch->getHit() * FracDam);
    Damage = max(10, (ch->getHit() > 0 ? Damage : 10));

    if (ch->reconcileDamage(ch, Damage, DAMAGE_NORMAL) == -1)
      return DELETE_THIS;
  } else {
    act("$N suddenly halts, but you luckily stay in the saddle.", TRUE, ch, 0,
      ch->riding, TO_CHAR);
  }

  return TRUE;
}

int TBeing::ChargePulse(TBeing* ch) {
  roomDirData* rExit;
  TThing* tMonster = NULL;
  char nString[256];
  int nRc = TRUE;

  for (StuffIter it = ch->roomp->stuff.begin();
       it != ch->roomp->stuff.end() && (tMonster = *it); ++it) {
    if (!dynamic_cast<TMonster*>(tMonster))
      continue;

    if (mob_specials[GET_MOB_SPE_INDEX(tMonster->spec)].proc == payToll) {
      act("You usher $n and his mount to a stop.", FALSE, tMonster, 0, ch,
        TO_CHAR);
      act("$n ushers you to a stop.", FALSE, tMonster, 0, ch, TO_VICT);
      act("$N ushers $n to a stop.", FALSE, tMonster, 0, ch, TO_NOTVICT);
      stop_charge(ch);
      return TRUE;
    }
  }

  if (ch->task->timeLeft > 0) {
    if (!(rExit = ch->roomp->dir_option[ch->task->flags]) ||
        (IS_SET(rExit->condition, EXIT_CLOSED) &&
          IS_SET(rExit->condition, EXIT_SECRET)))
      return ChargeHitWall(ch);

    if (IS_SET(rExit->condition, EXIT_CLOSED) ||
        IS_SET(rExit->condition, EXIT_NOENTER) ||
        IS_SET(rExit->condition, EXIT_CAVED_IN) ||
        IS_SET(rExit->condition, EXIT_WARDED) ||
        real_roomp(rExit->to_room)->isRoomFlag(ROOM_PEACEFUL)) {
      return ChargeHitDoor(ch, rExit);
    }

    ch->task->timeLeft--;
    ch->sendTo(format("You charge %s.\n\r") % dirs[ch->task->flags]);
    sprintf(nString, "$n charges %s.", dirs[ch->task->flags]);
    act(nString, FALSE, ch, 0, 0, TO_ROOM);

    nRc = ChargeRoom(ch);
  } else {
    // Either hit 0 or were doing an inf run-until-hit thing.
    for (StuffIter it = ch->roomp->stuff.begin();
         it != ch->roomp->stuff.end() && (tMonster = *it); ++it) {
      if (!dynamic_cast<TMonster*>(tMonster))
        continue;

      if (isname(ch->task->orig_arg, tMonster->name)) {
        ch->sendTo("You have found your prey!\n\r");
        sprintf(nString, "charge %s", ch->task->orig_arg);
        ch->addCommandToQue(nString);
        ch->stopTask();
        return TRUE;
      }
    }

    if (ch->task->timeLeft < 0) {
      if (!(rExit = ch->roomp->dir_option[ch->task->flags]) ||
          (IS_SET(rExit->condition, EXIT_CLOSED) &&
            IS_SET(rExit->condition, EXIT_SECRET)))
        return ChargeHitWall(ch);

      if (IS_SET(rExit->condition, EXIT_CLOSED) ||
          IS_SET(rExit->condition, EXIT_NOENTER) ||
          IS_SET(rExit->condition, EXIT_CAVED_IN) ||
          IS_SET(rExit->condition, EXIT_WARDED) ||
          real_roomp(rExit->to_room)->isRoomFlag(ROOM_PEACEFUL)) {
        return ChargeHitDoor(ch, rExit);
      }

      ch->sendTo(format("You charge %s.\n\r") % dirs[ch->task->flags]);
      sprintf(nString, "$n charges %s.", dirs[ch->task->flags]);
      act(nString, FALSE, ch, 0, 0, TO_ROOM);

      nRc = ChargeRoom(ch);
    } else {
      ch->sendTo(
        "You pull your mount to a stop seeing your target isn't here.\n\r");
      act("$n suddenly stops, looking around for something or someone.", FALSE,
        ch, 0, 0, TO_ROOM);
      ch->stopTask();
    }
  }

  return nRc;
}

void startChargeTask(TBeing* ch, const char* tString) {
  char Name[256] = "\0", nString[256] = "\0", zString[256] = "\0";
  const char* tArg;
  int Distance = -1;
  dirTypeT Direction = DIR_NONE;

  tArg = tString;
  for (; isspace(*tArg); tArg++)
    ;

  if (!ch || !tString || !*tString) {
    vlogf(LOG_BUG, "startChargeTask() called with bad arguments.");
    return;
  }

  if (!ch->riding) {
    ch->sendTo(
      "Next time try riding something first, it actually helps...\n\r");
    return;
  }

  if ((ch != ch->riding->horseMaster())) {
    ch->sendTo("I'm sure they would really love that, really...\n\r");
    return;
  }

  if (!dynamic_cast<TMonster*>(ch->riding)) {
    act("You slap the back of $P but it doesn't seem to move much...", TRUE, ch,
      ch->riding, 0, TO_CHAR);
    return;
  }

  half_chop(tArg, nString, zString);
  tArg = zString;
  for (; isspace(*tArg); tArg++)
    ;

  // charge <north/east/south/ect..> bird (1)
  Direction = getDirFromChar(nString);

  if (Direction <= DIR_NONE || Direction >= MAX_DIR || !*tArg) {
    ch->sendTo("Syntax: charge <direction> <target> <distance>\n\r");
    return;
  }

  half_chop(tArg, Name, zString);
  tArg = zString;
  for (; isspace(*tArg); tArg++)
    ;

  if (*tArg) {
    Distance = convertTo<int>(tArg);

    if (Distance <= 0) {
      ch->sendTo("That's funny, might we try it again?\n\r");
      return;
    } else if (Distance >= 100) {
      ch->sendTo(
        "Sure you want to push your mount like that, lets try a lesser "
        "distance.\n\r");
      return;
    }
  }

  if (!ch->roomp->dir_option[Direction]) {
    sprintf(nString, "You point $N %s...Right at a wall, let's not.",
      dirs[Direction]);
    act(nString, TRUE, ch, 0, ch->riding, TO_CHAR);
    return;
  }

  // timeLeft = Distance of charge, -1 if no distance supplied
  // flags    = Direction of charge
  start_task(ch, NULL, NULL, TASK_MOUNTCHARGING, Name, Distance, ch->in_room, 0,
    Direction, 40);

  sprintf(nString, "You point $N %s.", dirs[Direction]);
  act(nString, TRUE, ch, 0, ch->riding, TO_CHAR);
  sprintf(nString, "$n points $N %s, preparing to charge.", dirs[Direction]);
  act(nString, FALSE, ch, 0, ch->riding, TO_ROOM);
}

int task_charge(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj* obj) {
  TBeing* Mount = NULL;

  if (ch->isLinkdead() || ch->in_room < 0 ||
      ch->getPosition() < POSITION_RESTING) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (!(Mount = dynamic_cast<TBeing*>(ch->riding))) {
    ch->sendTo("Where'd your mount go??\n\r");
    ch->stopTask();
    return TRUE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, Pulse::MOBACT);
      return Mount->ChargePulse(ch);
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You pull your mount to a sudden stop.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n pulls $N to a sudden stop.", FALSE, ch, 0, Mount, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo(
        "You are unable to continue your charge while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;
  }

  return TRUE;
}
