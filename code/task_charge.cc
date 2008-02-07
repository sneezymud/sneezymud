//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

extern void warn_busy(TBeing *);

void stop_charge(TBeing *ch)
{
  ch->sendTo("You pull your steed to a violent halt!\n\r");
  act("$n pulls $N to a violent halt.",
      FALSE, ch, 0, ch->riding, TO_ROOM);
  ch->stopTask();
}

int TThing::ChargePulse(TBeing *ch)
{
  ch->sendTo("We think were funny today, don't we?\n\r");
  ch->stopTask();
  return TRUE;
}

// This rather nasty function handles moving the charger to the next room,
// and making a ton of checks in the process:
// if (moveHorse)
//   FALSE = Move Failed, Stop Task
//   TRUE  = Clean Move, Continue
// else
//      -2 = Clean fly, Continue
//   FALSE = In the Same Room
//    TRUE = In the Next room but we already did messages.
int taskChargeMoveInto(int to_room, TBeing *ch, bool moveHorse)
{
  TRoom  *from_here,
         *to_here;
  int     iHeight = 0,
          nRc,
          nMoveCost;
  char    tString[256];
  TBeing *tHorse;

  from_here = ch->roomp;
  to_here   = real_roomp(to_room);
  tHorse    = dynamic_cast<TBeing *>(ch->riding);
  sprintf(tString, "%i", ch->task->flags);

  // Handle 'flying out of the room' check first.
  if (!moveHorse) {
    if (from_here->isFlyingSector()) {
      ch->sendTo("Luckily you can fly freely here.\n\r");
      act("$n begins to fly after taking to the air.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_FLYING);

      return FALSE;
    }

    if (from_here->isUnderwaterSector()) {
      ch->sendTo("You are plunged into the water and stop shortly after.\n\r");
      act("$n flies through the water like a torpedo and stops shortly afterwards.",
          TRUE, ch, 0, 0, TO_ROOM);

      return FALSE;
    }

    if (ch->willBumpHeadDoor(from_here->dir_option[ch->task->flags], &iHeight) &&
        ((ch->getPosHeight() * 4 / 5) > iHeight)) {
      ch->sendTo("Your head suddenly impacts above the exit, OUCH!!!\n\r");
      act("$n slams into the area above the exit which stops them ever so gently...",
          TRUE, ch, 0, 0, TO_ROOM);

      if (ch->reconcileDamage(ch, ::number(20, 40), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      return FALSE;
    }

    nRc = from_here->checkSpec(ch, CMD_ROOM_ATTEMPTED_EXIT, tString, from_here);

    if (IS_SET_DELETE(nRc, DELETE_THIS))
      return DELETE_THIS;

    if (nRc == TRUE || !to_here || to_here->isRoomFlag(ROOM_PEACEFUL)) {
      ch->sendTo("You slam into something which suddenly stops you.\n\r");
      act("$n suddenly slams into something which stops there flight plan.",
          TRUE, ch, 0, 0, TO_ROOM);

      if (ch->reconcileDamage(ch, ::number(10, 20), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      return FALSE;
    }

    ch->foodNDrink(from_here->getSectorType(), 1);

    if (ch->isPc())
      ch->specials.last_direction = getDirFromChar(tString);

    --(*ch);
    thing_to_room(ch, to_room);

    if (to_here->isAirSector() || to_here->isVertSector()) {
      ch->sendTo("As you fly into the room you land on a cushion of....AIR!!!!\n\r");
      act("$n looks downwards in horror.",
          TRUE, ch, 0, 0, TO_ROOM);

      nRc = ch->checkFalling();
      if (IS_SET_DELETE(nRc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    }

    if (to_here->isFlyingSector()) {
      ch->sendTo("As you fly into the room you realize you can now fly freely.\n\r");
      act("$n beings to fly after taking to the air.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_FLYING);

      return TRUE;
    }

    if (to_here->isWaterSector()) {
      ch->sendTo("You Slam into the water as you come down full force!\n\r");
      act("$n slams into the water making a huge splash and killing there swan dive.",
          TRUE, ch, 0, 0, TO_ROOM);

      if (ch->reconcileDamage(ch, ::number(20, 30), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      return TRUE;
    }

    return -2;
  } else {
    if (from_here->isFlyingSector()) {
      ch->sendTo("Your mount doesn't want to just charge out of the room.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (ch->willBumpHeadDoor(from_here->dir_option[ch->task->flags], &iHeight) &&
        ((ch->getPosHeight() * 4 / 5) > iHeight)) {
      ch->sendTo("You notice that the exit is not tall enough for you and your mount.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    sprintf(tString, "%i", ch->task->flags);
    nRc = from_here->checkSpec(ch, CMD_ROOM_ATTEMPTED_EXIT, tString, from_here);

    if (IS_SET_DELETE(nRc, DELETE_THIS))
      return DELETE_THIS;

    if (nRc == TRUE || !to_room || to_here->isRoomFlag(ROOM_PEACEFUL) ||
        (to_here->getMoblim() && (MobCountInRoom(to_here->getStuff()) >= to_here->getMoblim()))) {
      ch->sendTo("Your mount refuses to continue in your charge so you decide to stop.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    nMoveCost = (TerrainInfo[from_here->getSectorType()]->movement +
                 TerrainInfo[to_here->getSectorType()]->movement) / 2;

    if ((TerrainInfo[from_here->getSectorType()]->movement % 2) &&
        !(TerrainInfo[to_here->getSectorType()]->movement % 2))
      nMoveCost++;

    if (to_here->isWaterSector()) {
      ch->sendTo("Your mount refuses to charge into the water, you're forced to stop.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (from_here->isWaterSector()) {
      ch->sendTo("Your mount refuses to charge out of the water, you're forced to stop.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (ch->bothLegsHurt()) {
      ch->sendTo(COLOR_MOBS, fmt("Riding %s without working legs is painful!\n\r") %
                 tHorse->getName());
      ch->addToMove(-5);
      if (!::number(0, 1)) {
        nRc = ch->fallOffMount(tHorse, POSITION_SITTING);
        if (IS_SET_DELETE(nRc, DELETE_THIS))
          return DELETE_THIS;

        ch->sendTo("Guess that stops your charging for now.\n\r");
        ch->stopTask();

        return FALSE;
      }

      if (ch->reconcileDamage(ch, ::number(0, 2), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;
    }

    if (!ch->riding) {
      ch->sendTo("You have to be able to stay on your mount to charge.\n\r");
      ch->stopTask();

      return FALSE;
    }

    if (tHorse->bothLegsHurt()) {
      act("$N has no working legs for you to charge with.",
          FALSE, ch, 0, tHorse, TO_CHAR);
      stop_charge(ch);

      return FALSE;
    }

    if (ch->eitherLegHurt()) {
      ch->sendTo("A damaged leg or foot makes it tough to ride!\n\r");
      ch->addToMove(-2);

      if (ch->reconcileDamage(ch, ::number(0, 1), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      if (!ch->riding) {
        ch->sendTo("Have to be able to stay on your mount to charge.\n\r");
        ch->stopTask();

        return FALSE;
      }

      if (!::number(0, 5)) {
        nRc = ch->fallOffMount(tHorse, POSITION_SITTING);
        if (IS_SET_DELETE(nRc, DELETE_THIS))
          return DELETE_THIS;

        ch->sendTo("Guess that stops your charging for now.\n\r");
        ch->stopTask();

        return FALSE;
      }

      if (!ch->riding) {
        ch->sendTo("Have to be able to stay on your mount to charge.\n\r");
        ch->stopTask();

        return FALSE;
      }

      if (!::number(0, 3)) {
        act("$N stumbles due to $S injuries and throws you.",
            FALSE, ch, 0, tHorse, TO_CHAR);

        if (ch->reconcileDamage(ch, ::number(0, 3), DAMAGE_NORMAL) == -1)
          return DELETE_THIS;

        ch->stopTask();

        if (!ch->riding)
          return FALSE;

        nRc = ch->fallOffMount(tHorse, POSITION_SITTING);
        if (IS_SET_DELETE(nRc, DELETE_THIS))
          return DELETE_THIS;

        return FALSE;
      }
    }

    if (compareWeights(ch->riding->getWeight(), ch->getTotalWeight(TRUE)) == 1) {
      act("$N collapses beneath your weight.",
               FALSE, ch, 0, ch->riding, TO_CHAR);
      act("$N collapses beneath $n's weight.",
               FALSE, ch, 0, ch->riding, TO_NOTVICT);
      act("You collapse beneath $n's weight.",
          FALSE, ch, 0, ch->riding, TO_VICT);

      if (tHorse)
        tHorse->setMove(0);

      ch->addToMove(-2);
      nRc = ch->fallOffMount(ch->riding, POSITION_SITTING);

      if (IS_SET_DELETE(nRc, DELETE_THIS))
        return DELETE_THIS;

      if (ch->reconcileDamage(ch, ::number(0, 1), DAMAGE_NORMAL) == -1)
        return DELETE_THIS;

      return FALSE;
    }

    if (ch->getCond(DRUNK) > 9) {
      ch->sendTo("You are just a wee bit too drunk to charge, sober up a little first.\n\r");
      act("$n drives $s mount a bit odd and decides to stop.",
          FALSE, ch, 0, tHorse, TO_ROOM);
      ch->stopTask();

      return FALSE;
    }

    if (tHorse->isLevitating())
      nMoveCost /= 4;

    if (tHorse->isFlying())
      nMoveCost = 1;

    if (tHorse->getMove() < nMoveCost) {
      ch->sendTo("Your mount is too exhausted to continue charging.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (ch->getMove() < 1) {
      ch->sendTo("You are too tired to continue.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if ((to_here->isFlyingSector() || to_here->isAirSector()) && !tHorse->isFlying()) {
      ch->sendTo("Your mount doesn't seem to want to charge into there, so you stop.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (to_here->isUnderwaterSector()) {
      ch->sendTo("Your mount refuses to charge under water.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (from_here->isUnderwaterSector()) {
      ch->sendTo("Your mount refuses to charge through the water.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    if (to_here->isVertSector()) {
      ch->sendTo("Your mount refuses to charge in that direction, you're forced to stop.\n\r");
      stop_charge(ch);

      return FALSE;
    }

    ch->foodNDrink(from_here->getSectorType(), 1);

    nRc = tHorse->checkForMoveTrap(getDirFromChar(tString));
    if (IS_SET_DELETE(nRc, DELETE_THIS))
      return DELETE_THIS;
    else if (nRc)
      return FALSE;

    --(*tHorse);
    thing_to_room(tHorse, to_room);
  }

  if (ch->isPc())
    ch->specials.last_direction = getDirFromChar(tString);

  --(*ch);
  thing_to_room(ch, to_room);

  return TRUE;
}

// is called when the player is just charging into the room.
// First stop in moving to the next room.
// Make basic checks and act appropriatly.
int ChargeRoom(TBeing *ch)
{
  int nRc;

  nRc = taskChargeMoveInto(ch->roomp->dir_option[ch->task->flags]->to_room, ch, true);

  if (IS_SET_DELETE(nRc, DELETE_THIS))
    return DELETE_THIS;

  act("$n charges into the room upon $N.",
      FALSE, ch, 0, ch->riding, TO_ROOM);
  ch->doLook("", CMD_LOOK);

  return TRUE;
}

// is called when player is jettisoned from his mount into the next room.
// First stop in flying to next room.
// Make basic checks and act appropriatly.
int ChargeFlyIntoRoom(TBeing *ch, roomDirData *rExit)
{
  int  nRoom = rExit->to_room,
       nRc;
  sstring nString;

  nRc = taskChargeMoveInto(nRoom, ch, false);

  if (ch)
    ch->stopTask();

  if (IS_SET_DELETE(nRc, DELETE_THIS))
    return DELETE_THIS;

  if (nRc == TRUE || nRc == FALSE)
    return TRUE;

  nString=fmt("$n suddenly flies into the room and smashes into the %s.") %
    real_roomp(nRoom)->describeGround();
  act(nString, TRUE, ch, 0, 0, TO_ROOM);
  ch->sendTo(fmt("You fly into the next room and smash into the %s.\n\r") %
             real_roomp(nRoom)->describeGround());
  ch->doLook("", CMD_LOOK);

  return TRUE;
}

int ChargeHitDoor(TBeing *ch, roomDirData *rExit)
{
  int     Damage = 0;
  float   FracDam;
  TBeing *tHorse;

  tHorse = dynamic_cast<TBeing *>(ch->riding);
  ch->sendTo(fmt("You charge towards %s!\n\r") %
             (IS_SET(rExit->condition, EX_CLOSED) ? "a door" : "the exit"));

  if (!ch->isAgile(0) && tHorse && !tHorse->hasSaddle() && !::number(0, 3)) {
    Damage  = ::number(10, 20);
    FracDam = (float)Damage / 100;
    Damage  = (int)(ch->getHit() * FracDam);

    Damage = max(10, (ch->getHit() > 0 ? Damage : 10));

    if (IS_SET(rExit->condition, EX_CLOSED)) {
      act("$N suddenly halts, sending you flying into the door.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into a door.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
    } else if (IS_SET(rExit->condition, EX_CAVED_IN)) {
      act("$N suddenly halts, sending you flying into the cave in.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into a cave in.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
    } else if (IS_SET(rExit->condition, EX_NOENTER) ||
             IS_SET(rExit->condition, EX_WARDED)) {
      act("$N suddenly halts, sending you flying towards the exit.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      ch->sendTo("You suddenly hit something and fall to the ground.\n\r");
      act("$N suddenly halts, sending $n flying towards an exit.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
      act("$n suddenly hits something and falls to the ground.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
    } else {
      act("$N suddenly halts, sending you flying into the next room.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, sending $n flying into the next room.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
      ChargeFlyIntoRoom(ch, rExit);
    }

    if (ch->reconcileDamage(ch, Damage, DAMAGE_NORMAL) == -1)
      return DELETE_THIS;
  } else {
    if (tHorse && tHorse->hasSaddle()) {
      act("$N suddenly halts, but you luckily stay in the saddle.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, luckly $n was able to stay in the saddle.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
    } else {
      act("$N suddenly halts, but you were able to stay seated...this time.",
          TRUE, ch, 0, ch->riding, TO_CHAR);
      act("$N suddenly halts, luckly $n was able to stay seated.",
          TRUE, ch, 0, ch->riding, TO_ROOM);
    }
  }

  ch->stopTask();
  return TRUE;
}

int ChargeHitWall(TBeing *ch)
{
  int     Damage;
  float   FracDam;
  TBeing *tHorse;

  tHorse = dynamic_cast<TBeing *>(ch->riding);
  ch->stopTask();

  ch->sendTo("You charge towards a wall!\n\r");
  if (!ch->isAgile(0) && tHorse && !tHorse->hasSaddle() && !::number(0, 3)) {
    act("$N suddenly halts, sending you flying into the wall.",
        TRUE, ch, 0, ch->riding, TO_CHAR);

    Damage  = ::number(10, 20);
    FracDam = (float)Damage / 100;
    Damage  = (int)(ch->getHit() * FracDam);
    Damage  = max(10, (ch->getHit() > 0 ? Damage : 10));

    if (ch->reconcileDamage(ch, Damage, DAMAGE_NORMAL) == -1)
      return DELETE_THIS;
  } else {
    act("$N suddenly halts, but you luckily stay in the saddle.",
        TRUE, ch, 0, ch->riding, TO_CHAR);
  }

  return TRUE;
}

int TBeing::ChargePulse(TBeing *ch)
{
  roomDirData *rExit;
  TThing      *tMonster;
  char         nString[256];
  int          nRc = TRUE;

  for (tMonster = ch->roomp->getStuff(); tMonster; tMonster = tMonster->nextThing) {
    if (!dynamic_cast<TMonster *>(tMonster))
      continue;

    if (mob_specials[GET_MOB_SPE_INDEX(tMonster->spec)].proc == payToll) {
      act("You usher $n and his mount to a stop.",
          FALSE, tMonster, 0, ch, TO_CHAR);
      act("$n ushers you to a stop.",
          FALSE, tMonster, 0, ch, TO_VICT);
      act("$N ushers $n to a stop.",
          FALSE, tMonster, 0, ch, TO_NOTVICT);
      stop_charge(ch);
      return TRUE;
    }
  }

  if (ch->task->timeLeft > 0) {
    if (!(rExit = ch->roomp->dir_option[ch->task->flags]) ||
        IS_SET(rExit->condition, EX_CLOSED) && IS_SET(rExit->condition, EX_SECRET))
      return ChargeHitWall(ch);

    if (IS_SET(rExit->condition, EX_CLOSED  ) ||
        IS_SET(rExit->condition, EX_NOENTER ) ||
        IS_SET(rExit->condition, EX_CAVED_IN) ||
        IS_SET(rExit->condition, EX_WARDED  ) ||
        real_roomp(rExit->to_room)->isRoomFlag(ROOM_PEACEFUL)) {
      return ChargeHitDoor(ch, rExit);
    }

    ch->task->timeLeft--;
    ch->sendTo(fmt("You charge %s.\n\r") % dirs[ch->task->flags]);
    sprintf(nString, "$n charges %s.", dirs[ch->task->flags]);
    act(nString, FALSE, ch, 0, 0, TO_ROOM);

    nRc = ChargeRoom(ch);
  } else {
    // Either hit 0 or were doing an inf run-until-hit thing.
    for (tMonster = ch->roomp->getStuff(); tMonster; tMonster = tMonster->nextThing) {
      if (!dynamic_cast<TMonster *>(tMonster))
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
          IS_SET(rExit->condition, EX_CLOSED) && IS_SET(rExit->condition, EX_SECRET))
        return ChargeHitWall(ch);

      if (IS_SET(rExit->condition, EX_CLOSED  ) ||
          IS_SET(rExit->condition, EX_NOENTER ) ||
          IS_SET(rExit->condition, EX_CAVED_IN) ||
          IS_SET(rExit->condition, EX_WARDED  ) ||
          real_roomp(rExit->to_room)->isRoomFlag(ROOM_PEACEFUL)) {
        return ChargeHitDoor(ch, rExit);
      }

      ch->sendTo(fmt("You charge %s.\n\r") % dirs[ch->task->flags]);
      sprintf(nString, "$n charges %s.", dirs[ch->task->flags]);
      act(nString, FALSE, ch, 0, 0, TO_ROOM);

      nRc = ChargeRoom(ch);
    } else {
      ch->sendTo("You pull your mount to a stop seeing your target isn't here.\n\r");
      act("$n suddenly stops, looking around for something or someone.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
    }
  }

  return nRc;
}

void startChargeTask(TBeing *ch, const char *tString)
{
        char  Name[256]    = "\0",
              nString[256] = "\0",
              zString[256] = "\0";
  const char *tArg;
        int   Distance     = -1;
  dirTypeT    Direction    = DIR_NONE;

  tArg = tString;
  for (; isspace(*tArg); tArg++);

  if (!ch || !tString || !*tString) {
    vlogf(LOG_BUG, "startChargeTask() called with bad arguments.");
    return;
  }

  if (!ch->riding) {
    ch->sendTo("Next time try riding something first, it actually helps...\n\r");
    return;
  }

  if ((ch != ch->riding->horseMaster())) {
    ch->sendTo("I'm sure they would really love that, really...\n\r");
    return;
  }

  if (!dynamic_cast<TMonster *>(ch->riding)) {
    act("You slap the back of $P but it doesn't seem to move much...",
        TRUE, ch, ch->riding, 0, TO_CHAR);
    return;
  }

  half_chop(tArg, nString, zString);
  tArg = zString;
  for (; isspace(*tArg); tArg++);

  // charge <north/east/south/ect..> bird (1)
  Direction = getDirFromChar(nString);

  if (Direction <= DIR_NONE || Direction >= MAX_DIR || !*tArg) {
    ch->sendTo("Syntax: charge <direction> <target> <distance>\n\r");
    return;
  }

  half_chop(tArg, Name, zString);
  tArg = zString;
  for (; isspace(*tArg); tArg++);

  if (*tArg) {
    Distance = convertTo<int>(tArg);

    if (Distance <= 0) {
      ch->sendTo("That's funny, might we try it again?\n\r");
      return;
    } else if (Distance >= 100) {
      ch->sendTo("Sure you want to push your mount like that, lets try a lesser distance.\n\r");
      return;
    }
  }

  if (!ch->roomp->dir_option[Direction]) {
    sprintf(nString, "You point $N %s...Right at a wall, let's not.", dirs[Direction]);
    act(nString, TRUE, ch, 0, ch->riding, TO_CHAR);
    return;
  }

  // timeLeft = Distance of charge, -1 if no distance supplied
  // flags    = Direction of charge
  start_task(ch, NULL, NULL, TASK_MOUNTCHARGING, Name, Distance, ch->in_room, 0, Direction, 40);

  sprintf(nString, "You point $N %s.", dirs[Direction]);
  act(nString, TRUE, ch, 0, ch->riding, TO_CHAR);
  sprintf(nString, "$n points $N %s, preparing to charge.", dirs[Direction]);
  act(nString, FALSE, ch, 0, ch->riding, TO_ROOM);
}

int task_charge(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TBeing *Mount = NULL;

  if (ch->isLinkdead() || ch->in_room < 0 || ch->getPosition() < POSITION_RESTING) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (!(Mount = dynamic_cast<TBeing *>(ch->riding))) {
    ch->sendTo("Where'd your mount go??\n\r");
    ch->stopTask();
    return TRUE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT);
      return Mount->ChargePulse(ch);
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You pull your mount to a sudden stop.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n pulls $N to a sudden stop.",
          FALSE, ch, 0, Mount, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue your charge while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;
  }

  return TRUE;
}
