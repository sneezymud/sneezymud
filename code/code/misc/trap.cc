//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    "trap.cc" - All functions and routines related to traps
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "handler.h"
#include "room.h"
#include "extern.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "disc_thief_looting.h"
#include "disease.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_open_container.h"
#include "obj_arrow.h"
#include "trap.h"

extern const char* const GRENADE_EX_DESC = "__grenade_puller";
extern const char* const TRAP_EX_DESC = "__trap_setter";

doorTrapT mapFileToDoorTrap(int dt) {
  switch (dt) {
    case 0:
      return DOOR_TRAP_NONE;
    case 1:
      return DOOR_TRAP_POISON;
    case 2:
      return DOOR_TRAP_SPIKE;
    case 3:
      return DOOR_TRAP_SLEEP;
    case 4:
      return DOOR_TRAP_TNT;
    case 5:
      return DOOR_TRAP_BLADE;
    case 6:
      return DOOR_TRAP_FIRE;
    case 7:
      return DOOR_TRAP_ACID;
    case 8:
      return DOOR_TRAP_DISEASE;
    case 9:
      return DOOR_TRAP_HAMMER;
    case 10:
      return DOOR_TRAP_FROST;
    case 11:
      return DOOR_TRAP_TELEPORT;
    case 12:
      return DOOR_TRAP_ENERGY;
    case 13:
      return DOOR_TRAP_BOLT;
    case 14:
      return DOOR_TRAP_DISK;
    case 15:
      return DOOR_TRAP_PEBBLE;
  }

  vlogf(LOG_BUG, format("Bad value (%d) in mapFileToDoorTrap") % dt);
  return MAX_TRAP_TYPES;
}

int mapDoorTrapToFile(doorTrapT dt) {
  switch (dt) {
    case DOOR_TRAP_NONE:
      return 0;
    case DOOR_TRAP_POISON:
      return 1;
    case DOOR_TRAP_SPIKE:
      return 2;
    case DOOR_TRAP_SLEEP:
      return 3;
    case DOOR_TRAP_TNT:
      return 4;
    case DOOR_TRAP_BLADE:
      return 5;
    case DOOR_TRAP_FIRE:
      return 6;
    case DOOR_TRAP_ACID:
      return 7;
    case DOOR_TRAP_DISEASE:
      return 8;
    case DOOR_TRAP_HAMMER:
      return 9;
    case DOOR_TRAP_FROST:
      return 10;
    case DOOR_TRAP_TELEPORT:
      return 11;
    case DOOR_TRAP_ENERGY:
      return 12;
    case DOOR_TRAP_BOLT:
      return 13;
    case DOOR_TRAP_DISK:
      return 14;
    case DOOR_TRAP_PEBBLE:
      return 15;
    case MAX_TRAP_TYPES:
      break;
  }

  vlogf(LOG_BUG, format("Bad value (%d) in mapDoorTrapToFile") % dt);
  return -1;
}

const sstring trap_types[] = {"None", "Poison", "Spike", "Sleep", "Explosive",
  "Blade", "Fire", "Acid", "Spore", "Hammer", "Frost", "Teleport", "Power",
  "Bolt", "Disc", "Pebble", "\n"};

void describeTrapToLooker(TBeing* ch, const TThing* obj,
  const sstring& doorName, bool skillDetected, int trapType, int trapDam) {
  if (skillDetected) {
    sstring type = sstring(trap_types[trapType]).uncap();
    if (obj)
      act(format("You spot an insidious %s trap rigged to $p (level %d).") %
            type % trapDam,
        true, ch, obj, nullptr, TO_CHAR);
    else
      ch->sendTo(format("You spot an insidious %s trap rigged to the %s "
                        "(level %d).\n\r") %
                 type % doorName % trapDam);
  } else if (ch->isPerceptive()) {
    if (obj)
      act("It seems like someone may have trapped $p.", true, ch, obj, nullptr,
        TO_CHAR);
    else
      ch->sendTo(format("It seems like someone may have trapped the %s.\n\r") %
                 doorName);
  }
}

int trapDamMod(doorTrapT type) {
  switch (type) {
    case DOOR_TRAP_TNT:
    case DOOR_TRAP_DISEASE:
    case DOOR_TRAP_FROST:
    case DOOR_TRAP_DISK:
      return 3;
    case DOOR_TRAP_ENERGY:
    case DOOR_TRAP_TELEPORT:
      return 5;
    case DOOR_TRAP_SLEEP:
    case DOOR_TRAP_ACID:
    case DOOR_TRAP_BOLT:
      return 1;
    case DOOR_TRAP_POISON:
      return -1;
    case DOOR_TRAP_BLADE:
      return -3;
    case DOOR_TRAP_SPIKE:
    case DOOR_TRAP_PEBBLE:
      return -5;
    case DOOR_TRAP_HAMMER:
      return -10;
    default:
      return 0;
  }
}

// Indexed by trap_targ_t: DOOR=0, CONT=1, MINE=2, GRENADE=3, ARROW=4
const TrapSourceInfo trapSourceInfo[] = {
  /* TRAP_TARG_DOOR    */ {10, 2, SKILL_SET_TRAP_DOOR},
  /* TRAP_TARG_CONT    */ {20, 3, SKILL_SET_TRAP_CONT},
  /* TRAP_TARG_MINE    */ {20, 2, SKILL_SET_TRAP_MINE},
  /* TRAP_TARG_GRENADE */ {5, 2, SKILL_SET_TRAP_GREN},
  /* TRAP_TARG_ARROW   */ {5, 2, SKILL_SET_TRAP_ARROW},
};

doorTrapT parseTrapType(const char* name, trap_targ_t target) {
  // Universal types
  if (is_abbrev(name, "fire"))
    return DOOR_TRAP_FIRE;
  if (is_abbrev(name, "explosive"))
    return DOOR_TRAP_TNT;
  if (is_abbrev(name, "sleep"))
    return DOOR_TRAP_SLEEP;
  if (is_abbrev(name, "acid"))
    return DOOR_TRAP_ACID;
  if (is_abbrev(name, "spore"))
    return DOOR_TRAP_DISEASE;
  if (is_abbrev(name, "frost"))
    return DOOR_TRAP_FROST;
  if (is_abbrev(name, "teleport"))
    return DOOR_TRAP_TELEPORT;
  if (is_abbrev(name, "power"))
    return DOOR_TRAP_ENERGY;
  // Poison: everywhere except arrows
  if (target != TRAP_TARG_ARROW && is_abbrev(name, "poison"))
    return DOOR_TRAP_POISON;
  // spike/blade: door, container, arrow
  if (target == TRAP_TARG_DOOR || target == TRAP_TARG_CONT ||
      target == TRAP_TARG_ARROW) {
    if (is_abbrev(name, "spike"))
      return DOOR_TRAP_SPIKE;
    if (is_abbrev(name, "blade"))
      return DOOR_TRAP_BLADE;
  }
  // hammer: door only
  if (target == TRAP_TARG_DOOR && is_abbrev(name, "hammer"))
    return DOOR_TRAP_HAMMER;
  // pebble: container, arrow, mine, grenade
  if (target == TRAP_TARG_CONT || target == TRAP_TARG_ARROW ||
      target == TRAP_TARG_MINE || target == TRAP_TARG_GRENADE) {
    if (is_abbrev(name, "pebble"))
      return DOOR_TRAP_PEBBLE;
  }
  // bolt/disk: mine, grenade
  if (target == TRAP_TARG_MINE || target == TRAP_TARG_GRENADE) {
    if (is_abbrev(name, "bolt"))
      return DOOR_TRAP_BOLT;
    if (is_abbrev(name, "disk"))
      return DOOR_TRAP_DISK;
  }
  return MAX_TRAP_TYPES;
}

int TBeing::springTrap(TTrap* obj) {
  int adj, fireperc, roll;

  adj = obj->getTrapLevel() - GetMaxLevel();
  adj -= getDexReaction() * 5;
  fireperc = 95 + adj;
  roll = ::number(1, 100);

  if (roll < fireperc)
    return TRUE;  // trap is sprung

  return FALSE;
}

int TBeing::doSetTraps(const char* arg) {
  // prevent people from making traps in peace rooms:
  // policeman may attack if they see you trapping
  // goofup may cause damage
  if (checkPeaceful("You are not permitted to construct traps here.\n\r"))
    return false;

  // Grammar: trap <target> <type>.  The trap-damage TYPE is always the last
  // whitespace token; everything before it is the target (a "<name> [dir]"
  // pair, disarm-style).  Targets resolve object-first-then-door, so
  // containers/arrows/portals dispatch through the polymorphic TObj::trapMe
  // and doors go through findDoor.  mine/grenade are explicit creation
  // keywords and take precedence over object lookup.  Each target path (a
  // trapMe override or a makeXxxTrap helper) owns its own skill/component/learn
  // gates; this dispatcher only routes.
  sstring input = sstring(arg).trim();
  sstring type = input.lastWord();
  sstring targetRef = input.dropLastWord().trim();

  if (targetRef.empty() || type.empty()) {
    sendTo(
      "Trap what, with what?  e.g. 'trap north fire' or 'trap chest poison'."
      "\n\r");
    return false;
  }

  sstring targetName = targetRef.word(0);

  if (is_abbrev(targetName, "mine"))
    return makeMineTrap(type.c_str());
  if (is_abbrev(targetName, "grenade"))
    return makeGrenadeTrap(type.c_str());

  if (TObj* obj = get_obj_vis_accessible(this, targetName)) {
    int rc = obj->trapMe(this, type.c_str());
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    return false;
  }

  char name[256], dir[256];
  argument_interpreter(targetRef.c_str(), name, cElements(name), dir,
    cElements(dir));
  // A bare direction ("trap north fire") names the exit directly, the way the
  // legacy "trap exit <dir>" command did.  findDoor only treats its second
  // argument as a direction, so resolve a leading direction ourselves before
  // falling back to findDoor for named doors ("trap gate fire", "trap gate
  // north fire").
  dirTypeT door = DIR_NONE;
  if (!*dir) {
    if (int d = old_search_block(name, 0, strlen(name), dirs, 0); d > 0)
      door = dirTypeT(d - 1);
  }
  if (door == DIR_NONE)
    door = findDoor(name, dir, DOOR_INTENT_OPEN, SILENT_YES);
  if (door >= 0)
    return makeDoorTrap(door, type.c_str());

  sendTo(format("You can't find \"%s\" here.\n\r") % targetRef);
  return false;
}

// Door trap creation, reached via the findDoor() resolution in doSetTraps.
// The body is the preserved legacy door logic.
int TBeing::makeDoorTrap(dirTypeT door, const char* trap_type) {
  if (!doesKnowSkill(SKILL_SET_TRAP_DOOR)) {
    sendTo("You know nothing about making door traps.\n\r");
    return false;
  }

  roomDirData* exitp = exitDir(door);
  if (!exitp || (exitp->door_type == DOOR_NONE)) {
    sendTo("There is no door there to trap.\n\r");
    return false;
  }

  if (!IS_SET(exitp->condition, EXIT_CLOSED)) {
    sendTo(
      format("You need to close the %s first.\n\r") % exitp->getName().uncap());
    return false;
  }
  if (IS_SET(exitp->condition, EXIT_TRAPPED)) {
    sendTo(format("When you try to trap the %s, you set off the trap that "
                  "is already there!\n\r") %
           exitp->getName().uncap());
    int rc = triggerDoorTrap(door);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return false;
  }

  doorTrapT type = parseTrapType(trap_type, TRAP_TARG_DOOR);
  if (type == MAX_TRAP_TYPES) {
    sendTo("No such door trap-type.\n\r");
    return false;
  }
  if (!hasTrapComps(trap_type, TRAP_TARG_DOOR, 0)) {
    sendTo("You need more items to make that trap.\n\r");
    return false;
  }
  if (getTrapLearn(TRAP_TARG_DOOR) <= 0) {
    sendTo("You need more training before setting a door trap.\n\r");
    return false;
  }

  sendTo("You start working on your trap.\n\r");
  act(format("$n starts fiddling with the %s.") % exitp->getName().uncap(),
    true, this, nullptr, nullptr, TO_ROOM);
  sstring task_arg = sstring(dirs[door]) + " " + trap_type;
  start_task(this, nullptr, nullptr, TASK_TRAP_DOOR, task_arg.c_str(), 3,
    inRoom(), type, door, 5);
  return false;
}

int TBeing::makeMineTrap(const char* trap_type) {
  if (!doesKnowSkill(SKILL_SET_TRAP_MINE)) {
    sendTo("You know nothing about making mines.\n\r");
    return false;
  }

  doorTrapT type = parseTrapType(trap_type, TRAP_TARG_MINE);
  if (type == MAX_TRAP_TYPES) {
    sendTo("No such mine trap-type.\n\r");
    return false;
  }
  if (getTrapLearn(TRAP_TARG_MINE) <= 0) {
    sendTo("You need more training before setting a mine trap.\n\r");
    return false;
  }
  if (!hasTrapComps(trap_type, TRAP_TARG_MINE, 0)) {
    sendTo("You need more items to make that trap.\n\r");
    return false;
  }

  sendTo("You start working on your trap.\n\r");
  act("$n starts constructing a land-mine.", true, this, nullptr, nullptr,
    TO_ROOM);
  start_task(this, nullptr, nullptr, TASK_TRAP_MINE, trap_type, 3, inRoom(),
    type, 0, 5);
  return false;
}

int TBeing::makeGrenadeTrap(const char* trap_type) {
  if (!doesKnowSkill(SKILL_SET_TRAP_GREN)) {
    sendTo("You know nothing about making grenades.\n\r");
    return false;
  }

  doorTrapT type = parseTrapType(trap_type, TRAP_TARG_GRENADE);
  if (type == MAX_TRAP_TYPES) {
    sendTo("No such grenade trap-type.\n\r");
    return false;
  }
  if (getTrapLearn(TRAP_TARG_GRENADE) <= 0) {
    sendTo("You need more training before setting a grenade trap.\n\r");
    return false;
  }
  if (!hasTrapComps(trap_type, TRAP_TARG_GRENADE, 0)) {
    sendTo("You need more items to make that trap.\n\r");
    return false;
  }

  sendTo("You start working on your grenade.\n\r");
  act("$n starts constructing a grenade.", true, this, nullptr, nullptr,
    TO_ROOM);
  start_task(this, nullptr, nullptr, TASK_TRAP_GRENADE, trap_type, 3, inRoom(),
    type, 0, 5);
  return false;
}

// triggered when portal opened or entered
// returns DELETE_THIS, DELETE_ITEM
int TBeing::triggerPortalTrap(TPortal* o) {
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_ROOM);
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_CHAR);

  auto type = static_cast<doorTrapT>(o->getPortalTrapType());

  if (type == DOOR_TRAP_TNT) {
    act("$p is destroyed by the blast!", true, this, o, nullptr, TO_ROOM);
  } else {
    // A portal trap fires once and then goes inert. Clear EXIT_TRAPPED on this
    // portal AND its linked partner (warning the far room) so it can't
    // re-trigger from either side -- parity with door traps, which clear both
    // sides on trigger.
    portal_flag_change(o, EXIT_TRAPPED,
      "%s violently discharges a trap from the other side!\n\r", REMOVE_TYPE);
  }

  int rc = applyTrapEffect(type, o->getPortalTrapDam(), o);

  if (type == DOOR_TRAP_TNT) {
    // The blast destroys this portal; take its linked partner down with it so
    // the far side isn't left as a dangling one-way portal (parity with a door
    // cave-in, which collapses both sides).
    if (TPortal* partner = o->findMatchingPortal()) {
      sstring msg =
        format("%s is destroyed by a blast from the other side!\n\r") %
        sstring(partner->shortDescr).cap();
      sendToRoom(msg.c_str(), partner->in_room);
      delete partner;
    }
    ADD_DELETE(rc, DELETE_ITEM);
  }

  return rc;
}

// returns DELETE_THIS or FALSE
// DELETE_ITEM may be |= with above.
// triggers when obj is opened
int TBeing::triggerContTrap(TOpenContainer* obj) {
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_ROOM);
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_CHAR);
  obj->remContainerFlag(CONT_TRAPPED);
  obj->remContainerFlag(CONT_CLOSED);
  obj->addContainerFlag(CONT_EMPTYTRAP);

  if (!::number(0, 100)) {
    act("...But nothing happens.", true, this, nullptr, nullptr, TO_CHAR);
    act("...But nothing happens.", true, this, nullptr, nullptr, TO_ROOM);
    return false;
  }

  doorTrapT type = obj->getContainerTrapType();

  // carrier fate for destructive types: container + contents destroyed
  bool destroysContainer = (type == DOOR_TRAP_FIRE || type == DOOR_TRAP_TNT);
  if (destroysContainer) {
    act("$p is destroyed by its own trap!", true, this, obj, nullptr, TO_ROOM);
    for (StuffIter it = obj->stuff.begin(); it != obj->stuff.end();) {
      TThing* t = *(it++);
      delete t;
    }
  }

  int rc = applyTrapEffect(type, obj->getContainerTrapDam(), obj);

  if (destroysContainer)
    ADD_DELETE(rc, DELETE_ITEM);
  return rc;
}

// returns DELETE_THIS or FALSE
// triggers when arrow hits
int TBeing::triggerArrowTrap(TArrow* obj) {
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_ROOM);
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_CHAR);

  if (!::number(0, 100)) {
    act("...But nothing happens.", true, this, nullptr, nullptr, TO_CHAR);
    act("...But nothing happens.", true, this, nullptr, nullptr, TO_ROOM);
    return false;
  }

  // arrow = single-target (the struck victim); carrier = the arrow
  return applyTrapEffect(obj->getTrapDamType(), obj->getTrapLevel(), obj);
}

// returns DELETE_THIS or FALSE
int TBeing::triggerDoorTrap(dirTypeT door) {
  roomDirData* exitp = exitDir(door);
  int dam = exitp->trap_dam;  // raw power; applyTrapEffect rolls dice(dam, 8)
  doorTrapT type = static_cast<doorTrapT>(exitp->trap_info);

  // door traps can be triggered by means other than opening
  // eg trying to set another trap
  //  rawOpenDoor(door);

  // Clear the trapped flag on both sides before anything else so
  // the damage loops can't re-trigger the same trap.
  REMOVE_BIT(exitp->condition, EXIT_TRAPPED);
  TRoom* far = real_roomp(exitp->to_room);
  roomDirData* back = far ? far->dir_option[rev_dir(door)] : nullptr;
  if (back)
    REMOVE_BIT(back->condition, EXIT_TRAPPED);

  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_ROOM);
  act("You hear a strange noise...", true, this, nullptr, nullptr, TO_CHAR);

  // TNT destroys the door before the blast.  Read dam and type above, and
  // capture far before calling destroyDoor — destroyDoor only modifies
  // condition bits, so far (the room pointer) remains valid after the call.
  if (type == DOOR_TRAP_TNT)
    exitp->destroyDoor(door, in_room);

  // Same-room bystanders take half damage.
  if (roomp) {
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && tbt != this && !tbt->isImmortal()) {
        int brc = tbt->applyTrapEffect(type, dam, nullptr, nullptr, 2);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // Far-side room occupants take half damage.
  if (far) {
    for (StuffIter it = far->stuff.begin(); it != far->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && !tbt->isImmortal()) {
        int brc = tbt->applyTrapEffect(type, dam, nullptr, nullptr, 2);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // Opener takes full damage last so bystander loops run on a live `this`.
  int rc = applyTrapEffect(type, dam, nullptr);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return false;
}

// returns DELETE_VICT
int TTrap::moveTrapCheck(TBeing* ch, dirTypeT dir) {
  char buf[256];
  const char* tmp_desc = NULL;
  TBeing* c;
  int rc;

  if ((isTrapEffectType(TRAP_EFF_MOVE)) && (getTrapCharges() > 0)) {
    // bypass for physical condition
    if (ch->isLevitating() || ch->isFlying())
      return FALSE;

    // if the person who set it is in my group, bypass
    if (ex_description &&
        (tmp_desc = ex_description->findExtraDesc(TRAP_EX_DESC))) {
      if ((c = get_char(tmp_desc, EXACT_YES)))
        if (ch->inGroup(*c))
          return FALSE;
    }

    if (IS_SET(getTrapEffectType(), TrapDir[dir])) {
      if (ch->springTrap(this)) {
        sprintf(buf, "$n starts to leave %s when you hear a strange noise...",
          dirs[dir]);
        act(buf, TRUE, ch, 0, 0, TO_ROOM);
        sprintf(buf, "You start to leave %s when you hear a strange noise...",
          dirs[dir]);
        act(buf, TRUE, ch, 0, 0, TO_CHAR);

        rc = ch->triggerTrap(this);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_VICT;
        if (rc)
          return TRUE;
        return FALSE;
      }
    }
  }
  return FALSE;
}

// returns DELETE_THIS
// TRUE if prevent motion, else FALSE
int TBeing::checkForMoveTrap(dirTypeT dir) {
  TThing* t = NULL;
  int rc;

  for (StuffIter it = roomp->stuff.begin();
       it != roomp->stuff.end() && (t = *it); ++it) {
    rc = t->moveTrapCheck(this, dir);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }
  return FALSE;
}

int TTrap::insideTrapCheck(TBeing* ch, TThing* i) {
  int rc;

  if ((isTrapEffectType(TRAP_EFF_OBJECT)) && (getTrapCharges() > 0)) {
    if (ch->springTrap(this)) {
      act("As you reach into $p, you hear a strange noise...", FALSE, ch, i, 0,
        TO_CHAR);
      act("As $n reaches into $p, you hear a strange noise...", FALSE, this, i,
        0, TO_ROOM);

      rc = ch->triggerTrap(this);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      if (rc)
        return TRUE;
      return FALSE;
    }
  }
  return FALSE;
}

// returns DELETE_THIS
// if trap, return TRUE else FALSE
// check for a trap INSIDE a container.
// triggers whenever anything inside the container is put/got
int TBeing::checkForInsideTrap(TThing* i) {
  TThing* t = NULL;
  int rc;

  for (StuffIter it = i->stuff.begin(); it != i->stuff.end() && (t = *it);
       ++it) {
    rc = t->insideTrapCheck(this, i);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }
  return FALSE;
}

// returns DELETE_THIS
// triggered == TRUE, else FALSE
int TBeing::checkForAnyTrap(TThing* i) {
  int rc;

  rc = i->anyTrapCheck(this);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;
  else if (rc)
    return TRUE;
  return FALSE;
}

// returns DELETE_THIS
// returns TRUE if trap exists, else FALSE
int TBeing::checkForGetTrap(TThing* i) {
  int rc;

  rc = i->getTrapCheck(this);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;
  else if (rc)
    return TRUE;
  return FALSE;
}

// returns DELETE_THIS or true
int TBeing::triggerTrap(TTrap* o) {
  o->setTrapCharges(o->getTrapCharges() - 1);

  doorTrapT type = o->getTrapDamType();
  int dam = o->getTrapLevel();  // raw power; applyTrapEffect rolls dice(dam, 8)

  // Bystanders first (half damage), so we never touch `this` mid-iteration.
  // Splash hits all beings except the primary (`!= this`) and immortals.
  if (o->isTrapEffectType(TRAP_EFF_ROOM) && roomp) {
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && tbt != this && !tbt->isImmortal()) {
        int brc = tbt->applyTrapEffect(type, dam, o, nullptr, 2);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          // die() -> genericKillFix() has already called reformGroup(),
          // DeleteHatreds(), and DeleteFears() before returning DELETE_THIS,
          // so a bare delete here is safe — matching the old per-type loops.
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // Triggerer takes full damage.
  int rc = applyTrapEffect(type, dam, o);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return true;
}

// Single point where trap *physical* damage is dealt. `this` is the victim.
// carrier = the trap object (for death-log/source attribution), may be null.
// setter = the trap's setter for XP/kill credit; null today (unattributed,
// identical to historical behavior). Attribution wiring is a separate effort.
int TBeing::dealTrapDamage(spellNumT damageClass, int dam, TThing* carrier,
  TBeing* setter) {
  if (setter && setter != this)
    return setter->applyDamage(this, dam, damageClass);
  return objDamage(damageClass, dam, carrier);
}

// Apply a per-type trap effect to `this` (the victim).
// `trapPower` is the raw trap power (the d8 dice count): door/container/portal
// trap_dam, or an object trap's level. The damage roll lives here so every
// trigger path rolls identically — `dam = dice(trapPower, 8) / denom`.
// `denom` reduces the roll: 1 for the primary victim, 2 for the half-damage
// room/far-side splash, 6 for goof self-damage.
// `carrier` is the trap object (may be nullptr).
// `setter` is the trap's setter (may be nullptr).
// Returns DELETE_THIS if the victim was deleted, else 0.
int TBeing::applyTrapEffect(doorTrapT type, int trapPower, TThing* carrier,
  TBeing* setter, int denom) {
  int rc;
  int dam = dice(trapPower, 8) / denom;

  // An agile victim rolls with a trap's physical force, taking only half.
  // This applies to the damage types only — afflictions (poison/sleep/disease/
  // teleport) are resisted by their own immunity/luck/CON saves, not agility,
  // so you can't "dodge" a lungful of sleep gas. The check scales with trap
  // power (`isAgile(-trapPower / 2)`): stronger traps are harder to roll with.
  bool affliction = (type == DOOR_TRAP_POISON || type == DOOR_TRAP_SLEEP ||
                     type == DOOR_TRAP_DISEASE || type == DOOR_TRAP_TELEPORT);
  if (!affliction && isAgile(-trapPower / 2)) {
    dam = (dam + 1) / 2;
    act("You roll with the trap, taking only a glancing blow!", false, this,
      nullptr, nullptr, TO_CHAR);
    act("$n rolls deftly with the trap, taking only a glancing blow.", true,
      this, nullptr, nullptr, TO_ROOM);
  }

  switch (type) {
    case DOOR_TRAP_POISON:
      act("You are sprayed with contact poison!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is sprayed with contact poison!", false, this, nullptr, nullptr,
        TO_ROOM);
      trapPoison(dam);
      return 0;

    case DOOR_TRAP_SLEEP:
      act("You are surrounded by a noxious mist!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n is surrounded by a noxious mist!", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = trapSleep(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_DISEASE:
      act("You are surrounded by the thick cloud!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n is surrounded by the thick cloud!", false, this, nullptr, nullptr,
        TO_ROOM);
      trapDisease(dam);
      return 0;

    case DOOR_TRAP_TELEPORT:
      act("You find yourself sucked into the vortex!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n flails wildly, but falls into the vortex.", false, this, nullptr,
        nullptr, TO_ROOM);
      rc = trapTeleport(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_FIRE:
      act("You are burned by the flames!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is burned by the flames.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_FIRE, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_FROST:
      act("You are frozen by the icy cloud!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is frozen by the icy cloud.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_FROST, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_ACID:
      act("You are surrounded by the horrid acid cloud!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n is surrounded by the horrid acid cloud.", false, this, nullptr,
        nullptr, TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_ACID, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_ENERGY:
      act("You are devastated by dozens of plasma bolts!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n is devastated by dozens of plasma bolts.", false, this, nullptr,
        nullptr, TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_ENERGY, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_TNT:
      act("You are blasted by the explosive!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is blasted by the explosion.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_TNT, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_SPIKE:
      act("You are impaled by the spikes!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is pierced by the spikes.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_BOLT:
      act("You are perforated by the bolts!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is perforated by the bolts.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_BLADE:
      act("You are slashed by the razor-blades!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is slashed by the razor-blades.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_SLASH, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_DISK:
      act("You are slashed by the razor-disks!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is slashed by the razor-disks.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_SLASH, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_HAMMER:
      act("You are crushed by the heavy weights!", false, this, nullptr,
        nullptr, TO_CHAR);
      act("$n is crushed by the hammer trap.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_BLUNT, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_PEBBLE:
      act("You are hit by the fusillade!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is hit by the pebbles.", false, this, nullptr, nullptr, TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_BLUNT, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return 0;

    case DOOR_TRAP_NONE:
    case MAX_TRAP_TYPES:
    default:
      vlogf(LOG_BUG,
        format("applyTrapEffect: unknown trap type %d on victim %s") % type %
          getName());
      return 0;
  }
}

// returns DELETE_THIS
int TBeing::trapTeleport(int amt) {
  int rc;

  if (isImmune(IMMUNE_SUMMON, WEAR_BODY)) {
    sendTo("You resist the teleportation effect.\n\r");
    act("$n resists the teleportation effect.", FALSE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  if (isLucky(levelLuckModifier(GetMaxLevel()))) {
    sendTo("You feel strange, but the effect fades.\n\r");
    act("Nothing seems to happen.", FALSE, this, 0, 0, TO_ROOM);
    return FALSE;
  }
  rc = genericTeleport(SILENT_NO);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return FALSE;
}

int TBeing::trapSleep(int amt) {
  int rc = FALSE;

  if (isImmune(IMMUNE_SLEEP, WEAR_BODY)) {
    sendTo("You yawn, but are otherwise immune to the sleep trap.\n\r");
    return FALSE;
  }

  if (!isLucky(levelLuckModifier(GetMaxLevel()))) {
    // at 2 minutes per mudhour, let's not make this too painful
    rc = rawSleep(0, (2 * Pulse::UPDATES_PER_MUDHOUR), 1, SAVE_NO);
  } else
    sendTo("You feel sleepy, but you recover.\n\r");

  return rc;
}

void TBeing::trapDisease(int amt) {
  affectedData aff;

  aff.type = AFFECT_DISEASE;
  aff.modifier = DISEASE_FLU;
  aff.level = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.duration = 4 * Pulse::UPDATES_PER_MUDHOUR;

  if (isImmortal() || isImmune(IMMUNE_DISEASE, WEAR_BODY)) {
    act("Hmmm, lucky you, it doesn't seem to have had any effect.", FALSE, this,
      0, 0, TO_CHAR);
    return;
  } else if (isLucky(amt) && isTough()) {
    act(
      "You are able to shake off most of the effects, but you still feel "
      "somewhat sick.",
      FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
  } else if (!isLucky(amt) && !isTough()) {
    aff.duration *= 4;
    act("You feel VERY sick.", FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
  } else {
    aff.duration *= 2;
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
    act("You feel sick.", TRUE, this, 0, 0, TO_CHAR);
  }
  affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_NO);
  disease_start(this, &aff);
}

void TBeing::trapPoison(int amt) {
  affectedData af, af2;

  af.type = SPELL_POISON;
  af.duration = 12 * Pulse::UPDATES_PER_MUDHOUR;
  af.modifier = -20;
  af.location = APPLY_STR;
  af.bitvector = AFF_POISON;

  af2.type = AFFECT_DISEASE;
  af2.level = 0;
  af2.duration = af.duration;
  af2.modifier = DISEASE_POISON;
  af2.location = APPLY_NONE;
  af2.bitvector = AFF_POISON;

  // check immunity, each successive check is easier then last
  // each failure makes time longer
  if (isImmortal() || isImmune(IMMUNE_POISON, WEAR_BODY)) {
    act("Hmmm, lucky you, it doesn't seem to have had any effect.", FALSE, this,
      0, 0, TO_CHAR);
  } else if (isImmune(IMMUNE_POISON, WEAR_BODY)) {
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
    affectTo(&af2);
    act(
      "You are able to shake off most of the effects, but you still feel "
      "somewhat sick.",
      FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
    disease_start(this, &af2);
  } else if (isImmune(IMMUNE_POISON, WEAR_BODY)) {
    af.duration *= 2;
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
    affectTo(&af2);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
    act("You feel sick.", TRUE, this, 0, 0, TO_CHAR);
    disease_start(this, &af2);
  } else {
    af.duration *= 4;
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
    affectTo(&af2);
    act("You feel VERY sick.", FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
    disease_start(this, &af2);
  }
}

void TBeing::informMess() {
  switch (getPosition()) {
    case POSITION_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE,
        this, 0, 0, TO_ROOM);
      act("You are mortally wounded, and will die soon, if not aided.", FALSE,
        this, 0, 0, TO_CHAR);
      break;
    case POSITION_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, this,
        0, 0, TO_ROOM);
      act("You are incapacitated and you will slowly die, if not aided.", FALSE,
        this, 0, 0, TO_CHAR);
      break;
    case POSITION_STUNNED:
      act("$n is stunned, but will probably regain consciousness.", TRUE, this,
        0, 0, TO_ROOM);
      act("You're stunned, but you will probably regain consciousness.", FALSE,
        this, 0, 0, TO_CHAR);
      break;
    case POSITION_DEAD:
      act("$n is dead! R.I.P.", TRUE, this, 0, 0, TO_ROOM);
      act("You are dead!  Sorry...", FALSE, this, 0, 0, TO_CHAR);
      break;
    default:  // >= POSITION SLEEPING
      break;
  }
}

// may return DELETE_THIS
// ch->task is still valid here, use it to parse for stuff as needed
int TBeing::goofUpTrap(doorTrapT trap_type, trap_targ_t goof_type) {
  // Consume components — door traps encode the trap type in the first word of
  // orig_arg, so skip past it with half_chop to get the type string.
  const char* comps = task->orig_arg;
  char buf1[256], buf2[256];
  if (goof_type == TRAP_TARG_DOOR) {
    half_chop(task->orig_arg, buf1, buf2);
    comps = buf2;
  }
  hasTrapComps(comps, goof_type, -1);

  // Goof self-damage is uniformly 1/6 of a successful trap's roll (the denom=6
  // below): this preserves the old door/container value (they applied /3 then
  // /2) and halves the old mine/grenade value (they applied only /3).
  int dam = getTrapDam(goof_type, trap_type);  // raw power; rolled below

  act("You slip up, and your trap goes off in your face!", false, this, nullptr,
    nullptr, TO_CHAR);
  act("$n slips up, and $s trap goes off in $s face!", false, this, nullptr,
    nullptr, TO_ROOM);

  int rc = applyTrapEffect(trap_type, dam, task->obj, nullptr, 6);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return false;
}

namespace {
  // Standardized component assembly messages
  constexpr const char* ATTACH_CHAR_MSG = "You attach the %s to the %s.\n\r";
  constexpr const char* ATTACH_ROOM_MSG = "$n attaches the %s to the %s.";
  constexpr const char* PLACE_CHAR_MSG = "You place the %s into the %s.\n\r";
  constexpr const char* PLACE_ROOM_MSG = "$n places the %s into the %s.";
  constexpr const char* CONNECT_CHAR_MSG = "You connect the %s to the %s.\n\r";
  constexpr const char* CONNECT_ROOM_MSG = "$n connects the %s to the %s.";
  constexpr const char* ARM_CHAR_MSG = "You arm the %s mechanism.\n\r";
  constexpr const char* ARM_ROOM_MSG = "$n arms the %s mechanism.";
  constexpr const char* CONCEAL_CHAR_MSG =
    "You conceal the %s inside the %s.\n\r";
  constexpr const char* CONCEAL_ROOM_MSG = "$n conceals the %s inside the %s.";

  // Component name mapping for clean messages
  const char* getComponentName(int vnum) {
    switch (vnum) {
      case Obj::ST_FLINT:
        return "flint";
      case Obj::ST_SULPHUR:
        return "sulphur";
      case Obj::ST_BAG:
        return "bag";
      case Obj::ST_BELLOWS:
        return "bellows";
      case Obj::ST_HYDROGEN:
        return "compressed gas";
      case Obj::ST_NEEDLE:
        return "needle";
      case Obj::ST_SPRING:
        return "spring";
      case Obj::ST_POISON:
        return "poison";
      case Obj::ST_CANISTER:
        return "canister";
      case Obj::ST_CON_POISON:
        return "concentrated poison";
      case Obj::ST_TRIPWIRE:
        return "tripwire";
      case Obj::ST_PENTAGRAM:
        return "pentagram";
      case Obj::ST_ATHANOR:
        return "athanor";
      case Obj::ST_CRYSTALINE:
        return "crystal";
      case Obj::ST_BOLTS:
        return "bolts";
      case Obj::ST_TUBING:
        return "tubing";
      case Obj::ST_PEBBLES:
        return "pebbles";
      case Obj::ST_RAZOR_DISK:
        return "razor discs";
      case Obj::ST_FUNGUS:
        return "spores";
      case Obj::ST_ACID_VIAL:
        return "acid vial";
      case Obj::ST_FROST:
        return "frost compound";
      case Obj::ST_CONCRETE:
        return "concrete weight";
      case Obj::ST_RAZOR_BLADE:
        return "razor blade";
      case Obj::ST_NOZZLE:
        return "nozzle";
      case Obj::ST_HOSE:
        return "hose";
      case Obj::ST_GAS:
        return "gas";
      case Obj::ST_CGAS:
        return "compressed gas";
      case Obj::ST_SPIKE:
        return "spike";
      case Obj::ST_WEDGE:
        return "wedge";
      case Obj::ST_BLINK:
        return "blink powder";
      default:
        return "component";
    }
  }
}  // namespace

// Single source of trap component vnums for a (type, target): used by
// hasTrapComps() to gate and consume, and by sendTrapMessage() to name the
// components. A new trap type needs one entry here, not bespoke message
// prose. Returns false for an unrecognized type.
bool trapComponents(const char* type, trap_targ_t targ, int& item1, int& item2,
  int& item3) {
  item1 = item2 = item3 = 0;
  if (is_abbrev(type, "fire")) {
    item1 = Obj::ST_FLINT;
    item2 = Obj::ST_SULPHUR;
    item3 = Obj::ST_BAG;
  } else if (is_abbrev(type, "explosive")) {
    item1 = Obj::ST_FLINT;
    item2 = Obj::ST_SULPHUR;
    item3 = Obj::ST_HYDROGEN;
  } else if (is_abbrev(type, "poison")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_NEEDLE;
      item2 = Obj::ST_SPRING;
      item3 = Obj::ST_POISON;
    } else if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      item1 = Obj::ST_CANISTER;
      item2 = Obj::ST_SPRING;
      item3 = Obj::ST_CON_POISON;
    }
  } else if (is_abbrev(type, "sleep")) {
    item1 = Obj::ST_NOZZLE;
    item2 = Obj::ST_GAS;
    item3 = Obj::ST_HOSE;
  } else if (is_abbrev(type, "acid")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_NOZZLE;
      item2 = Obj::ST_ACID_VIAL;
      item3 = Obj::ST_BELLOWS;
    } else if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      item1 = Obj::ST_CANISTER;
      item2 = Obj::ST_SPRING;
      item3 = Obj::ST_ACID_VIAL;
    }
  } else if (is_abbrev(type, "spore")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_FUNGUS;
      item2 = Obj::ST_NOZZLE;
      item3 = Obj::ST_BELLOWS;
    } else if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      item1 = Obj::ST_CANISTER;
      item2 = Obj::ST_SPRING;
      item3 = Obj::ST_FUNGUS;
    }
  } else if (is_abbrev(type, "spike")) {
    if (targ != TRAP_TARG_DOOR && targ != TRAP_TARG_CONT)
      vlogf(LOG_MISC,
        format("spike trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_SPIKE;
    item2 = Obj::ST_SPRING;
    item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(type, "bolt")) {
    if (targ != TRAP_TARG_MINE && targ != TRAP_TARG_GRENADE)
      vlogf(LOG_MISC, format("bolt trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_TUBING;
    item2 = Obj::ST_CGAS;
    item3 = Obj::ST_BOLTS;
  } else if (is_abbrev(type, "blade")) {
    if (targ != TRAP_TARG_DOOR && targ != TRAP_TARG_CONT)
      vlogf(LOG_MISC,
        format("blade trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_RAZOR_BLADE;
    item2 = Obj::ST_SPRING;
    item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(type, "disk")) {
    if (targ != TRAP_TARG_MINE && targ != TRAP_TARG_GRENADE)
      vlogf(LOG_MISC, format("disk trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_RAZOR_DISK;
    item2 = Obj::ST_SPRING;
    item3 = Obj::ST_CANISTER;
  } else if (is_abbrev(type, "hammer")) {
    if (targ != TRAP_TARG_DOOR)
      vlogf(LOG_MISC,
        format("hammer trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_CONCRETE;
    item2 = Obj::ST_WEDGE;
    item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(type, "pebble")) {
    if (targ != TRAP_TARG_CONT && targ != TRAP_TARG_MINE &&
        targ != TRAP_TARG_GRENADE)
      vlogf(LOG_MISC,
        format("pebble trap being set  with trap targ: %d") % targ);

    item1 = Obj::ST_TUBING;
    item2 = Obj::ST_CGAS;
    item3 = Obj::ST_PEBBLES;
  } else if (is_abbrev(type, "frost")) {
    item1 = Obj::ST_NOZZLE;
    item2 = Obj::ST_HOSE;
    item3 = Obj::ST_FROST;
  } else if (is_abbrev(type, "teleport")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_PENTAGRAM;
      item2 = Obj::ST_TRIPWIRE;
      item3 = Obj::ST_BLINK;
    } else if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      item1 = Obj::ST_PENTAGRAM;
      item2 = Obj::ST_CRYSTALINE;
      item3 = Obj::ST_BLINK;
    }
  } else if (is_abbrev(type, "power")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_PENTAGRAM;
      item2 = Obj::ST_TRIPWIRE;
      item3 = Obj::ST_ATHANOR;
    } else if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      item1 = Obj::ST_PENTAGRAM;
      item2 = Obj::ST_CRYSTALINE;
      item3 = Obj::ST_ATHANOR;
    }
  } else {
    vlogf(LOG_MISC, format("Bad call to hasTrapComps() : %s") % type);
    return false;
  }
  return true;
}

bool TBeing::hasTrapComps(const char* type, trap_targ_t targ, int amt,
  int* price) {
  int item1 = 0, item2 = 0, item3 = 0, item4 = 0;

  if (!trapComponents(type, targ, item1, item2, item3))
    return FALSE;
  item1 = real_object(item1);
  item2 = real_object(item2);
  item3 = real_object(item3);

  TThing* com4 = NULL;

  if (targ == TRAP_TARG_MINE) {
    item4 = Obj::ST_CASE_MINE;
    item4 = real_object(item4);
    com4 = searchLinkedListVis(this, obj_index[item4].name, stuff);
  } else if (targ == TRAP_TARG_GRENADE) {
    item4 = Obj::ST_CASE_GRENADE;
    item4 = real_object(item4);
    com4 = searchLinkedListVis(this, obj_index[item4].name, stuff);
  }

  TThing* com1 = searchLinkedListVis(this, obj_index[item1].name, stuff);
  TThing* com2 = searchLinkedListVis(this, obj_index[item2].name, stuff);
  TThing* com3 = searchLinkedListVis(this, obj_index[item3].name, stuff);

  if (price) {
    *price = 0;
    TObj* obj;
    if (com1) {
      obj = dynamic_cast<TObj*>(com1);
      *price += obj->obj_flags.cost;
    }
    if (com2) {
      obj = dynamic_cast<TObj*>(com2);
      *price += obj->obj_flags.cost;
    }
    if (com3) {
      obj = dynamic_cast<TObj*>(com3);
      *price += obj->obj_flags.cost;
    }
  }

  if (amt == -1) {
    // trap is finished, delete the items
    if (!com1 || !com2 || !com3) {
      vlogf(LOG_BUG, "Serious error in hasTrapComps");
      return FALSE;
    }
    delete com1;
    delete com2;
    delete com3;
    if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      if (!com4) {
        vlogf(LOG_BUG, "Serious error in hasTrapComps (2)");
        return FALSE;
      }
      delete com4;
    }
    return FALSE;
  }
  if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE)
    return (com1 && com2 && com3 && com4);
  else
    return (com1 && com2 && com3);
}

void TBeing::sendTrapMessage(const char* trap_type, trap_targ_t targ,
  int step) {
  int item1 = 0, item2 = 0, item3 = 0;
  if (!trapComponents(trap_type, targ, item1, item2, item3)) {
    item1 = Obj::ST_SPRING;
    item2 = Obj::ST_TRIPWIRE;
    item3 = Obj::ST_CANISTER;
  }

  switch (step) {
    case 1:
      sendTo(format(ATTACH_CHAR_MSG) % getComponentName(item1) %
             getComponentName(item2));
      act(format(ATTACH_ROOM_MSG) % getComponentName(item1) %
            getComponentName(item2),
        true, this, nullptr, nullptr, TO_ROOM);
      break;
    case 2:
      sendTo(format(CONNECT_CHAR_MSG) % getComponentName(item2) %
             getComponentName(item3));
      act(format(CONNECT_ROOM_MSG) % getComponentName(item2) %
            getComponentName(item3),
        true, this, nullptr, nullptr, TO_ROOM);
      break;
    case 3:
      sendTo(format(ARM_CHAR_MSG) % "trigger");
      act(format(ARM_ROOM_MSG) % "trigger", true, this, nullptr, nullptr,
        TO_ROOM);
      break;
    case 4:
      if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
        sendTo(format(CONCEAL_CHAR_MSG) % "mechanism" % "casing");
        act(format(CONCEAL_ROOM_MSG) % "mechanism" % "casing", true, this,
          nullptr, nullptr, TO_ROOM);
      } else if (task && task->obj) {
        // container, portal, or arrow — name the object being trapped
        act("You finish trapping $p.", false, this, task->obj, nullptr,
          TO_CHAR);
        act("$n finishes trapping $p.", true, this, task->obj, nullptr,
          TO_ROOM);
      } else if (task && roomp && roomp->dir_option[task->flags]) {
        // door — name the exit being trapped
        sstring doorname = roomp->dir_option[task->flags]->getName().uncap();
        sendTo(format("You finish trapping the %s.\n\r") % doorname);
        act(format("$n finishes trapping the %s.") % doorname, true, this,
          nullptr, nullptr, TO_ROOM);
      } else {
        sendTo(format(PLACE_CHAR_MSG) % "assembly" % "position");
        act(format(PLACE_ROOM_MSG) % "assembly" % "position", true, this,
          nullptr, nullptr, TO_ROOM);
      }
      break;
    default:
      sendTo("You continue working on the trap.\n\r");
      act("$n continues working on the trap.", true, this, nullptr, nullptr,
        TO_ROOM);
      break;
  }
}

void TBeing::throwGrenade(TTrap* o, dirTypeT dir) {
  char buf[256];
  TRoom* rp = NULL;

  if (!clearpath(inRoom(), dir) || !roomp->dir_option[dir] ||
      !(rp = real_roomp(roomp->dir_option[dir]->to_room))) {
    act("There's no place to throw $p there.", FALSE, this, o, 0, TO_CHAR);
    return;
  }
  if (roomp->isUnderwaterSector()) {
    act("There's no way to throw $p underwater.", FALSE, this, o, 0, TO_CHAR);
    return;
  }

  sprintf(buf, "You throw $p %s.", dirs[dir]);
  act(buf, FALSE, this, o, 0, TO_CHAR);
  sprintf(buf, "$n throws $p %s.", dirs[dir]);
  act(buf, TRUE, this, o, 0, TO_ROOM);

  if (o->equippedBy) {
    unequip(getPrimaryHold());
  } else {
    --(*o);
  }

  if (!o->isTrapEffectType(
        TRAP_EFF_ARMED3 | TRAP_EFF_ARMED2 | TRAP_EFF_ARMED1)) {
    // don't arm it if already armed
    o->armGrenade(this);
  }

  if (rp->isRoomFlag(ROOM_PEACEFUL)) {
    *roomp += *o;
    act("$n hits some strange barrier and bounces back at you!", FALSE, o, 0, 0,
      TO_ROOM);
    return;
  }

  *rp += *o;
  sprintf(buf, "$n bounces into the room from the %s.", dirs[rev_dir(dir)]);
  act(buf, TRUE, o, 0, 0, TO_ROOM);
}

int TBeing::grenadeHit(TTrap* o) {
  return applyTrapEffect(o->getTrapDamType(), o->getTrapLevel(), o);
}

int TMonster::grenadeHit(TTrap* o) {
  // first make recursive
  int rc = TBeing::grenadeHit(o);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (isPc())
    return rc;

  const char* tmp_desc;
  TBeing* ch = nullptr;
  if (o && o->ex_description &&
      (tmp_desc = o->ex_description->findExtraDesc(GRENADE_EX_DESC))) {
    if ((ch = get_char(tmp_desc, EXACT_YES)))
      pissOff(this, ch);
  }

  return rc;
}

int TObj::grenadeHit(TTrap* o) {
  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act("$n is sprayed with contact poison which quickly evaporates!", FALSE,
        this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_SLEEP:
      return TRUE;
    case DOOR_TRAP_FIRE:
      act("$n is burned by the flames.", FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_TELEPORT:
      return TRUE;
    case DOOR_TRAP_DISEASE:
      return TRUE;
    case DOOR_TRAP_BOLT:
      act("$n is perforated by the bolts.", FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_PEBBLE:
      act("$n is hit by the pebbles.", FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_DISK:
      act("$n is slashed by the razor-disks.", FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_TNT:
      act("$n is blasted by fragments from $p.", FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    case DOOR_TRAP_FROST:
      act("$n is frozen by the icy cloud.", FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("$n is devastated by dozens of plasma bolts.", FALSE, this, o, 0,
        TO_ROOM);

      return TRUE;
    case DOOR_TRAP_ACID:
      act("$n is surrounded by the horrid acid cloud.", FALSE, this, o, 0,
        TO_ROOM);

      return TRUE;
    default:
      return TRUE;
  }

  return FALSE;
}

int TBeing::getTrapDam(trap_targ_t targ, doorTrapT type) {
  const TrapSourceInfo& si = trapSourceInfo[targ];
  int damage = si.baseDam + getSkillLevel(si.setSkill) / si.skillDivisor;
  damage = damage * getTrapLearn(targ) / 100;
  damage += trapDamMod(type);
  return min(max(damage, 1), 50);
}

int TBeing::getTrapLearn(trap_targ_t targ) {
  int learn = getSkillValue(trapSourceInfo[targ].setSkill);
  if (learn <= 0)
    return 0;
  return min(learn, static_cast<int>(MAX_SKILL_LEARNEDNESS));
}

int TObj::trapMe(TBeing* ch, const char* trap_type) {
  act("$p is not trappable.", FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}
