#include "handler.h"
#include "extern.h"
#include "room.h"
#include "games.h"
#include "being.h"
#include "disc_thief_traps.h"
#include "obj_tool.h"

void TObj::pickMe(TBeing* thief) {
  act("$p: That's not a container.", false, thief, this, 0, TO_CHAR);
}

int TBeing::doPick(const char* argument) {
  char type[80], dir[80];
  int rc;

  // Gin Game Command, not Thief skill
  if (gGin.check(this)) {
    gGin.draw(this, argument);
    return TRUE;
  }
  if (!doesKnowSkill(SKILL_PICK_LOCK)) {
    sendTo("You know nothing about picking locks.\n\r");
    return FALSE;
  }
  // Thief skill
  argument_interpreter(argument, type, cElements(type), dir, cElements(dir));

  if (!*type) {
    sendTo("Pick what?\n\r");
    return FALSE;
  }
  rc = pickLocks(this, argument, type, dir);
  return rc;
}

int TThing::pickWithMe(TBeing* thief, const char*, const char*, const char*) {
  thief->sendTo(
    "You need to hold a lock pick in your primary hand in order to pick "
    "locks.\n\r");
  return FALSE;
}

int TTool::pickWithMe(TBeing* thief, const char* argument, const char* type,
  const char* dir) {
  dirTypeT door;
  roomDirData* exitp = NULL;
  TObj* obj;
  TBeing* victim;

  if ((getToolType() != TOOL_LOCKPICK) || (getToolUses() <= 0)) {
    thief->sendTo(
      "You need to hold a lock pick in your primary hand in order to pick "
      "locks.\n\r");
    return FALSE;
  }
  int bKnown = thief->getSkillValue(SKILL_PICK_LOCK);

  // moved door check before obj check as "pick gate s" seemed to
  // pick up objs with "s" in the name, not sure why gate was ignored though
  if ((door = thief->findDoor(type, dir, DOOR_INTENT_UNLOCK, SILENT_YES)) >=
      MIN_DIR) {
    exitp = thief->exitDir(door);
    if (exitp->door_type == DOOR_NONE)
      thief->sendTo("That's absurd.\n\r");

    if (!IS_SET(exitp->condition, EXIT_CLOSED))
      thief->sendTo("You realize that the door is already open.\n\r");
    else if (exitp->key < 0)
      thief->sendTo("You can't seem to spot any lock to pick.\n\r");
    else if (!IS_SET(exitp->condition, EXIT_LOCKED))
      thief->sendTo("Oh.. it wasn't locked at all.\n\r");
    else {
      act("$n begins fiddling with a lock.", FALSE, thief, 0, 0, TO_ROOM);
      act("You begin fiddling with a lock.", FALSE, thief, 0, 0, TO_CHAR);
      thief->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_PICK_LOCK,
        8);

      // silly, but what if they sit down and pick the lock...
      if (thief->task)
        thief->stopTask();

      start_task(thief, NULL, NULL, TASK_PICKLOCKS, "", 0, thief->in_room, door,
        0, 120 - bKnown);
    }
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, thief,
               &victim, &obj)) {
    obj->pickMe(thief);
  } else
    thief->sendTo("You don't see that here.\n\r");

  return TRUE;
}

int pickLocks(TBeing* thief, const char* argument, const char* type,
  const char* dir) {
  TThing* pick;

  if (!thief->doesKnowSkill(SKILL_PICK_LOCK)) {
    thief->sendTo("You don't know to pick locks!\n\r");
    return FALSE;
  }
  if (!(pick = thief->heldInPrimHand())) {
    thief->sendTo(
      "You need to hold a lock pick in your primary hand in order to pick "
      "locks.\n\r");
    return FALSE;
  }
  pick->pickWithMe(thief, argument, type, dir);
  return TRUE;
}