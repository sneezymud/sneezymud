#include <stdio.h>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "disease.h"
#include "combat.h"
#include "disc_thief_looting.h"
#include "obj_trap.h"
#include "obj_trap_component.h"
#include "trap.h"
#include "obj_portal.h"
#include "low.h"

int TBeing::doSearch(const char* argument) {
  int rc;

  if (!doesKnowSkill(SKILL_SEARCH)) {
    sendTo("You are not trained in how to recognize secret passages!\n\r");
    return FALSE;
  }

  if (riding) {
    sendTo("You cannot search while riding.\n\r");
    return FALSE;
  }
  for (; isspace(*argument); argument++)
    ;

  if (!*argument) {
    sendTo("You begin searching for secret exits.\n\r");
    act("$n begins searching the walls for something.", FALSE, this, 0, 0,
      TO_ROOM);
    start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1, 0, 4);
  } else {
    for (rc = 0; rc < MAX_DIR; rc++) {
      if (is_abbrev(argument, dirs[rc])) {
        start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1, rc + 100,
          4);
        return TRUE;
      }
    }
    // there's probably a better way to do this
    if (!strcmp(argument, "ne")) {
      start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1,
        DIR_NORTHEAST + 100, 4);
      return TRUE;
    } else if (!strcmp(argument, "nw")) {
      start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1,
        DIR_NORTHWEST + 100, 4);
      return TRUE;
    } else if (!strcmp(argument, "se")) {
      start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1,
        DIR_SOUTHEAST + 100, 4);
      return TRUE;
    } else if (!strcmp(argument, "sw")) {
      start_task(this, NULL, NULL, TASK_SEARCH, "", 0, in_room, 1,
        DIR_SOUTHWEST + 100, 4);
      return TRUE;
    }

    sendTo("You look and look, but cannot seem to find that direction.\n\r");
  }
  return TRUE;
}

int detectSecret(TBeing* thief) {
  int j;
  roomDirData* fdd;
  char buf[128];
  int move_cost;

  move_cost = 30;

  *buf = '\0';

  if (thief->getMove() < move_cost) {
    thief->sendTo("You are too tired to search.  Maybe later...\n\r");
    return FALSE;
  }
  if (thief->riding) {
    thief->sendTo("You can't search while mounted.\n\r");
    return FALSE;
  }
  int bKnown = thief->getSkillValue(SKILL_SEARCH);

  if (thief->doesKnowSkill(SKILL_SEARCH))
    thief->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_SEARCH, 5);

  for (j = 0; j < 10; j++) {
    if ((fdd = thief->roomp->dir_option[j])) {
      if (((j < 4) || (j > 5))) {
        sprintf(buf, "$n searches the %s wall for secret doors.", dirs[j]);
        act(buf, FALSE, thief, 0, 0, TO_ROOM);
      } else if (j == 4)
        act("$n searches the ceiling for secret doors.", FALSE, thief, 0, 0,
          TO_ROOM);
      else
        act("$n searches the $g for secret doors.", FALSE, thief, 0, 0,
          TO_ROOM);

      if (!IS_SET(fdd->condition, EXIT_SECRET) ||
          !IS_SET(fdd->condition, EXIT_CLOSED) ||
          fdd->keyword == "_unique_door_")
        continue;

      if (thief->bSuccess(bKnown, SKILL_SEARCH)) {
        thief->sendTo(format("Secret door found %s! Door is named %s.\n\r") %
                      dirs[j] %
                      (!fdd->keyword.empty() ? fname(fdd->keyword)
                                             : "NO NAME. TELL A GOD"));
        sprintf(buf, "$n exclaims, \"Look %s! A SECRET door named %s!\"\n\r",
          dirs[j],
          (!fdd->keyword.empty() ? fname(fdd->keyword).c_str()
                                 : "NO NAME. TELL A GOD"));
        act(buf, FALSE, thief, 0, 0, TO_ROOM);
        thief->setMove(max(0, (thief->getMove() - 30)));
        thief->gainTaskExp(0, 50);
        thief->doSave(SILENT_YES);
        return TRUE;
      }
    }
  }
  thief->sendTo("No secret doors found in this area.\n\r");
  act("$n searches and searches, but comes up empty.", FALSE, thief, 0, 0,
    TO_ROOM);
  thief->setMove(max(0, (thief->getMove() - 30)));
  return TRUE;
}

int TBeing::disarmTrap(const char* arg, TObj* tp) {
  int rc;
  TObj* trap;
  char type[256], dir[256];
  dirTypeT door;

  if (!doesKnowSkill(SKILL_DISARM_TRAP)) {
    sendTo("You know nothing about removing traps.\n\r");
    return FALSE;
  }

  argument_interpreter(arg, type, cElements(type), dir, cElements(dir));

  if ((trap = tp) || (trap = get_obj_vis_accessible(this, type))) {
    rc = disarmTrapObj(this, trap);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      trap = nullptr;
    if (rc)
      addSkillLag(SKILL_DISARM_TRAP, rc);

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return FALSE;
  } else if ((door = findDoor(type, dir, DOOR_INTENT_OPEN, SILENT_YES)) >= 0) {
    rc = disarmTrapDoor(this, door);
    if (rc)
      addSkillLag(SKILL_DISARM_TRAP, rc);

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return FALSE;
  } else {
    // needed for "disarm elite weapon"
    sendTo(format("You can't find \"%s\" here.\n\r") % arg);
    return FALSE;
  }

  return FALSE;
}

// Helper function to reclaim trap components during disarming
bool reclaimTrapComps(TBeing* thief, sstring trap_type, TTrap* trap) {
  std::vector<int> components;
  size_t comp_recovered = 0;
  bool casing_recovered = false;

  // Determine trap target type for correct component selection
  trap_targ_t targ = TRAP_TARG_DOOR; // default for door traps (trap == nullptr)
  if (trap) {
    if (trap->isTrapEffectType(TRAP_EFF_THROW))
      targ = TRAP_TARG_GRENADE;
    else if (trap->isTrapEffectType(TRAP_EFF_MOVE))
      targ = TRAP_TARG_MINE;
    else
      targ = TRAP_TARG_CONT;
  }

  int item1, item2, item3;
  if (!getTrapComponents(trap_type.c_str(), targ, item1, item2, item3))
    return false;

  components = {item1, item2, item3};

  // Calculate chance of recovering each component
  int recovery_chance = 50 + (thief->getSkillValue(SKILL_DISARM_TRAP) / 2);

  for (auto item_vnum : components) {
    // Roll for each component
    if (::number(1, 100) <= recovery_chance) {
      TObj* comp = read_object(item_vnum, VIRTUAL);
      if (comp) {
        // If it's a trap component, set it up with charges
        TTrapComponent* trapComp = dynamic_cast<TTrapComponent*>(comp);
        if (trapComp) {
          trapComp->setTrapComponentCharges(1); // Salvaged components have 1 charge
        }

        // Notify player of recovered component first
        thief->sendTo(format("You carefully recover %s from the trap.\n\r") %
                      comp->shortDescr);

        // Then add to inventory (which may trigger merge messages)
        *thief += *comp;
        comp_recovered++;
      }
    }
  }

  // Also try to recover the casing for grenades and mines
  if (targ == TRAP_TARG_GRENADE || targ == TRAP_TARG_MINE) {
    int casing_vnum = (targ == TRAP_TARG_GRENADE)
      ? Obj::ST_CASE_GRENADE
      : Obj::ST_CASE_MINE;

    if (casing_vnum != -1 && ::number(1, 100) <= recovery_chance) {
      TObj* casing = read_object(casing_vnum, VIRTUAL);
      if (casing) {
        // If it's a trap component, set it up with charges
        TTrapComponent* trapComp = dynamic_cast<TTrapComponent*>(casing);
        if (trapComp) {
          trapComp->setTrapComponentCharges(1); // Salvaged components have 1 charge
        }

        // Notify player of recovered casing first
        thief->sendTo(format("You carefully recover %s from the trap.\n\r") %
                      casing->shortDescr);

        // Then add to inventory (which may trigger merge messages)
        *thief += *casing;
        casing_recovered = true;
      }
    }
  }

  if (comp_recovered == 0 && !casing_recovered) {
    thief->sendTo(
      "You were unable to salvage any components from the trap.\n\r");
    return false;
  } else if (comp_recovered < components.size()) {
    thief->sendTo("You managed to salvage some components from the trap.\n\r");
  } else {
    thief->sendTo(
      "You successfully recovered all components from the trap!\n\r");
  }

  return true;
}

int TObj::disarmMe(TBeing* thief) {
  thief->sendTo("I don't think that's a trap.\n\r");
  return FALSE;
}

int TTrap::disarmMe(TBeing* thief) {
  int rc;
  char trap_type[80];
  int bKnown = thief->getSkillValue(SKILL_DISARM_TRAP);

  if (getTrapCharges() <= 0) {
    thief->sendTo("That trap is already disarmed.\n\r");
    return FALSE;
  }

  strcpy(trap_type, trap_types[getTrapDamType()].c_str());

  if (thief->bSuccess(bKnown, SKILL_DISARM_TRAP)) {
    thief->sendTo(format("Click.  You disarm the %s trap.\n\r") % trap_type);
    act("$n disarms $p.", FALSE, thief, this, 0, TO_ROOM);

    // Try to salvage components if the thief has the appropriate trap-setting skills
    bool canSalvage = false;
    if (isTrapEffectType(TRAP_EFF_THROW) && thief->doesKnowSkill(SKILL_SET_TRAP_GREN)) {
      canSalvage = true;
    } else if (isTrapEffectType(TRAP_EFF_MOVE) && !isTrapEffectType(TRAP_EFF_THROW) &&
               thief->doesKnowSkill(SKILL_SET_TRAP_MINE)) {
      canSalvage = true;
    } else if (!isTrapEffectType(TRAP_EFF_THROW) && !isTrapEffectType(TRAP_EFF_MOVE) &&
               thief->doesKnowSkill(SKILL_SET_TRAP_CONT)) {
      canSalvage = true;
    }

    if (canSalvage) {
      reclaimTrapComps(thief, trap_type, this);
    } else {
      thief->sendTo(
        "You lack the knowledge to salvage components from this type of "
        "trap.\n\r");
    }

    // Trap is spent — remove from world and notify caller
    --(*this);
    delete this;
    return DELETE_ITEM;
  } else {
    thief->sendTo("Click. (whoops)\n\r");
    act("$n tries to disarm $p.", FALSE, thief, this, 0, TO_ROOM);
    rc = thief->triggerTrap(this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  }
}

int disarmTrapObj(TBeing* thief, TObj* trap) {
  int rc;
  rc = trap->disarmMe(thief);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }
  return rc;
}

int disarmTrapDoor(TBeing* thief, dirTypeT door) {
  int learnedness;
  int rc;
  roomDirData *exitp, *back = NULL;
  TRoom* rp;
  char buf[256], doorbuf[80], trap_type[80];

  exitp = thief->exitDir(door);
  strcpy(doorbuf, fname(exitp->keyword).c_str());

  if (!IS_SET(exitp->condition, EXIT_TRAPPED)) {
    thief->sendTo(format("I don't think the %s is trapped.\n\r") % doorbuf);
    return FALSE;
  }

  int bKnown = thief->getSkillValue(SKILL_DISARM_TRAP);

  strcpy(trap_type, trap_types[exitp->trap_info].c_str());
  learnedness = min((int)MAX_SKILL_LEARNEDNESS, 2 * bKnown);

  if (thief->bSuccess(learnedness, SKILL_DISARM_TRAP)) {
    thief->sendTo(format("Click.  You disarm the %s trap in the %s.\n\r") %
                  trap_type % doorbuf);
    sprintf(buf, "$n disarms the %s trap in the %s.", trap_type, doorbuf);
    act(buf, FALSE, thief, 0, 0, TO_ROOM);

    // Try to salvage components from door traps if the thief has the appropriate skill
    if (thief->doesKnowSkill(SKILL_SET_TRAP_DOOR)) {
      reclaimTrapComps(thief, trap_type, nullptr); // nullptr indicates door trap
    } else {
      thief->sendTo(
        "You lack the knowledge to salvage components from this type of "
        "trap.\n\r");
    }

    REMOVE_BIT(exitp->condition, EXIT_TRAPPED);
    if ((rp = real_roomp(exitp->to_room)) &&
        (back = rp->dir_option[rev_dir(door)])) {
      REMOVE_BIT(back->condition, EXIT_TRAPPED);
    }
    return TRUE;
  } else {
    thief->sendTo("Click. (whoops)\n\r");
    sprintf(buf, "$n tries to disarm the trap in the %s.", doorbuf);
    act(buf, FALSE, thief, 0, 0, TO_ROOM);
    rc = thief->triggerDoorTrap(door);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }
}

int TThing::detectMe(TBeing* thief) const { return FALSE; }

int TPortal::detectMe(TBeing* thief) const {
  int bKnown = thief->getSkillValue(SKILL_DETECT_TRAP);

  if (!isPortalFlag(EXIT_TRAPPED))
    return FALSE;

  // opening a trapped portal
  if (thief->bSuccess(bKnown, SKILL_DETECT_TRAP)) {
    CS(SKILL_DETECT_TRAP);
    return TRUE;
  } else {
    CF(SKILL_DETECT_TRAP);
    return FALSE;
  }
}

int TTrap::detectMe(TBeing* thief) const {
  int bKnown = thief->getSkillValue(SKILL_DETECT_TRAP);

  // randomly seen when in room
  // reduced detection rate
  if (thief->bSuccess(bKnown / 10 + 1, SKILL_DETECT_TRAP))
    return TRUE;
  else
    return FALSE;
}

// returns TRUE if trap detected
int detectTrapObj(TBeing* thief, const TThing* trap) {
  return trap->detectMe(thief);
}

int detectTrapDoor(TBeing* thief, int) {
  int bKnown = thief->getSkillValue(SKILL_DETECT_TRAP);

  if (thief->bSuccess(bKnown / 3 + 1, SKILL_DETECT_TRAP))
    return TRUE;
  else
    return FALSE;
}
