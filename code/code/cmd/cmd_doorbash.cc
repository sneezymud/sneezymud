#include <boost/format.hpp>
#include <string.h>
#include <algorithm>
#include <cstdio>
#include <string>

#include "being.h"
#include "comm.h"
#include "defs.h"
#include "enum.h"
#include "extern.h"
#include "handler.h"
#include "log.h"
#include "parse.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"
#include "stats.h"
#include "structs.h"
#include "thing.h"

// returns DELETE_THIS
int TBeing::slamIntoWall(roomDirData* exitp) {
  char doorname[128];
  char buf[256];

  if (!exitp->keyword.empty()) {
    if (fname(exitp->keyword) == "secret" ||
        IS_SET(exitp->condition, EXIT_SECRET)) {
      strcpy(doorname, "wall");
    } else
      strcpy(doorname, fname(exitp->keyword).c_str());
  } else
    strcpy(doorname, "barrier");

  sendTo(format("You slam against the %s with no effect.\n\r") % doorname);
  sendTo("OUCH!  That REALLY Hurt!\n\r");
  sprintf(buf, "$n crashes against the %s with no effect.\n\r", doorname);
  act(buf, false, this, 0, 0, TO_ROOM);
  if (reconcileDamage(this, (::number(1, 10) * 2), DAMAGE_COLLISION) == -1)
    return DELETE_THIS;

  if (!isImmortal()) {
    affectedData aff;

    aff.type = SKILL_DOORBASH;
    aff.duration = 2 * Pulse::UPDATES_PER_MUDHOUR / 3;
    aff.bitvector = AFF_STUNNED;
    affectTo(&aff, -1);
  }

#if 0
  addToWait(combatRound(12));
#else
  int rc;
  rc = crashLanding(POSITION_RESTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
#endif

  return true;
}

static int doorbash(TBeing* caster, dirTypeT dir) {
  int was_in;
  char buf[256];
  roomDirData* exitp;
  TRoom* rp;
  int rc;
  int dam;
  int height;

  if (caster->getMove() < 10) {
    caster->sendTo("You're too tired to do that.\n\r");
    return false;
  }
  if (caster->riding) {
    caster->sendTo("Yeah... right... while mounted.\n\r");
    return false;
  }

  if (!(exitp = caster->exitDir(dir))) {
    vlogf(LOG_BUG, "bad exit in doorbash (2)");
    return false;
  }

  rp = real_roomp(exitp->to_room);

  if (dir == DIR_UP) {
    if (rp->isAirSector() && !caster->isFlying()) {
      caster->sendTo("You would need to be flying to go there!\n\r");
      return false;
    }
  }
  sprintf(buf, "$n charges %swards.", dirs[dir]);
  act(buf, false, caster, 0, 0, TO_ROOM);
  caster->sendTo(format("You charge %swards.\n\r") % dirs[dir]);

  if (caster->willBumpHeadDoor(exitp, &height)) {
    caster->sendTo(
      "Belatedly, you realize the exit is a bit too short for you to charge at "
      "successfully.\n\r");
    rc = caster->slamIntoWall(exitp);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return false;
  }

  if ((IS_SET(exitp->condition, EXIT_DESTROYED)) ||
      !IS_SET(exitp->condition, EXIT_CLOSED)) {
    was_in = caster->in_room;
    --(*caster);
    thing_to_room(caster, exitp->to_room);
    caster->doLook("", CMD_LOOK);

    rc = caster->displayMove(dir, was_in, 1);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (!caster->isImmortal())
      caster->addToMove(-10);
    return true;
  }
  if (!caster->isImmortal())
    caster->addToMove(-10);

  int bKnown = caster->getSkillValue(SKILL_DOORBASH);

  if ((2 * exitp->weight > caster->maxWieldWeight(nullptr, HAND_TYPE_PRIM)) ||
      (2 * exitp->lock_difficulty > bKnown) ||
      (exitp->door_type == DOOR_PORTCULLIS) ||
      (exitp->door_type == DOOR_DRAWBRIDGE) ||
      IS_SET(exitp->condition, EXIT_CAVED_IN) ||
      IS_SET(exitp->condition, EXIT_WARDED)) {
    // doors too much for them regardless
    rc = caster->slamIntoWall(exitp);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return true;
  }
  dam = dice(exitp->weight, ::number(4, 10));

  if (caster->bSuccess(bKnown, SKILL_DOORBASH)) {
    // this check used to be done before the bSuccess roll, but
    // it wound up causing PCs to virtually never increase
    // their skill
    // don't make this too easy, thieves take a long time to pick a lock
    // at max learning, chance should be (100-lock_diff)
    // lock_diff=100 should never be bashable
    int chance = (bKnown - exitp->lock_difficulty);
    // heavier doors should be harder to dbash
    // brawn should offset
    chance *= caster->plotStat(STAT_CURRENT, STAT_BRA, 2, 10, 5);
    chance /= exitp->weight;
    chance = min(max(0, chance), 100);

    if (::number(0, 99) < chance) {
      rc = caster->slamIntoWall(exitp);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return true;
    }

    sprintf(buf, "$n slams into the %s, and it bursts open!",
      fname(exitp->keyword).c_str());
    act(buf, false, caster, 0, 0, TO_ROOM);
    caster->sendTo(format("You slam into the %s, and it bursts open!\n\r") %
                   fname(exitp->keyword));
    int room = caster->in_room;
    if (IS_SET(exitp->condition, EXIT_TRAPPED)) {
      rc = caster->triggerDoorTrap(dir);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      caster->sendTo(
        "Aarrggh!  You've triggered some insidious door-trap!\n\r");
    }
    exitp->destroyDoor(dir, room);
    if (caster->reconcileDamage(caster, dam, SKILL_DOORBASH) == -1)
      return DELETE_THIS;

    if (!caster->isAgile(0)) {
      was_in = caster->in_room;

      --(*caster);
      thing_to_room(caster, exitp->to_room);
      caster->doLook("", CMD_LOOK);
      rc = caster->displayMove(dir, was_in, 1);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      if (!caster->isImmortal())
        caster->addToMove(-10);
      return true;
    } else {
      if (!caster->isImmortal())
        caster->addToMove(-5);
      return true;
    }
  } else {
    caster->sendTo("You just don't know the nuances of door-bashing.\n\r");
    rc = caster->slamIntoWall(exitp);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return true;
  }
  return true;
}

/* skill to allow fighters to break down doors */
int TBeing::doDoorbash(const sstring& argument) {
  int rc;
  dirTypeT dir;
  bool ok;
  roomDirData* exitp;

  if (!doesKnowSkill(SKILL_DOORBASH)) {
    sendTo("You know nothing about door bashing.\n\r");
    return false;
  }
  sstring type = argument.word(0);
  sstring direction = argument.word(1);

  if (type.empty()) {
    sendTo("You must specify a direction.\n\r");
    return false;
  }

  if ((dir = findDoor(type.c_str(), direction.c_str(), DOOR_INTENT_OPEN,
         SILENT_YES)) >= MIN_DIR)
    ok = true;
  else {
    act("$n looks around, bewildered.", false, this, 0, 0, TO_ROOM);
    act("You can't seem to find what you are looking for.", false, this, 0, 0,
      TO_CHAR);
    return false;
  }
  if (!ok || !(exitp = exitDir(dir))) {
    vlogf(LOG_BUG, "Bad exit in doorbash!");
    return false;
  }
  rc = doorbash(this, dir);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (rc)
    addSkillLag(SKILL_DOORBASH, rc);

  return rc;
}
