//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    "trap.cc" - All functions and routines related to traps 
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disc_looting.h"
#include "disease.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_open_container.h"
#include "obj_arrow.h"

extern const char * const GRENADE_EX_DESC = "__grenade_puller";
extern const char * const TRAP_EX_DESC = "__trap_setter";


const sstring trap_types[] =
{
  "None",
  "Poison",
  "Spike",
  "Sleep",
  "Explosive",
  "Blade",
  "Fire",
  "Acid",
  "Spore",
  "Hammer",
  "Frost",
  "Teleport",
  "Power",
  "Bolt",
  "Disc",
  "Pebble",
};

const char *user_trap_types[] =
{
  "exit",
  "container",
  "mine",
  "grenade",
  "arrow",
  "\n"
};

int TBeing::springTrap(TTrap *obj)
{
  int adj, fireperc, roll;

  adj = obj->getTrapLevel() - GetMaxLevel();
  adj -= getDexReaction() * 5;
  fireperc = 95 + adj;
  roll = ::number(1, 100);

  if (roll < fireperc)
    return TRUE;   // trap is sprung

  return FALSE;
}

int TBeing::doSetTraps(const char *arg)
{
  roomDirData *exitp;
  char buf[256], task_arg[128];
  char sstring[512], trap_type[40], direct[20];
  int field, dir;
  dirTypeT door;
  doorTrapT type;
  int rc;
  TObj *obj;

  // prevent people from making traps in peace rooms:
    // policeman may attack if they see you trapping
    // goofup may cause damage
  if (checkPeaceful("You are not permitted to construct traps here.\n\r"))
    return FALSE;

  bisect_arg(arg, &field, sstring, user_trap_types);

  switch (field - 1) {
    case TRAP_TARG_DOOR:  // exit traps
      if (!doesKnowSkill(SKILL_SET_TRAP_DOOR)) {
        sendTo("You know nothing about making door traps.\n\r");
        return FALSE;
      }

      sscanf(sstring, "%s %s", direct, trap_type);
      if ((dir = old_search_block(direct, 0, strlen(direct), dirs, 0)) <= 0) {
	sendTo("No such direction.\n\r");
        sendTo("Syntax: trap exit <direction> <trap-type>\n\r");
	return FALSE;
      }
      door = dirTypeT(dir - 1);
      exitp = exitDir(door);
      if (!exitp || (exitp->door_type == DOOR_NONE)) {
	sendTo("There is no door there to trap.\n\r");
	return FALSE;
      }

      if (!IS_SET(exitp->condition, EX_CLOSED)) {
        sendTo(fmt("You need to close the %s first.\n\r") %
	       exitp->getName().uncap());
        return FALSE;
      }
      if (IS_SET(exitp->condition, EX_TRAPPED)) {
	sendTo(fmt("When you try to trap the %s, you set off the trap that is already there!\n\r") % exitp->getName().uncap());
	rc = triggerDoorTrap(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
	  return DELETE_THIS;
        return FALSE;
      }
      if (is_abbrev(trap_type, "fire")) {
        type = DOOR_TRAP_FIRE;
      } else if (is_abbrev(trap_type, "explosive")) {
        type = DOOR_TRAP_TNT;
      } else if (is_abbrev(trap_type, "poison")) {
        type = DOOR_TRAP_POISON;
      } else if (is_abbrev(trap_type, "sleep")) {
        type = DOOR_TRAP_SLEEP;
      } else if (is_abbrev(trap_type, "acid")) {
        type = DOOR_TRAP_ACID;
      } else if (is_abbrev(trap_type, "spore")) {
        type = DOOR_TRAP_DISEASE;
      } else if (is_abbrev(trap_type, "spike")) {
        type = DOOR_TRAP_SPIKE;
      } else if (is_abbrev(trap_type, "blade")) {
        type = DOOR_TRAP_BLADE;
      } else if (is_abbrev(trap_type, "hammer")) {
        type = DOOR_TRAP_HAMMER;
      } else if (is_abbrev(trap_type, "frost")) {
        type = DOOR_TRAP_FROST;
      } else if (is_abbrev(trap_type, "teleport")) {
        type = DOOR_TRAP_TELEPORT;
      } else if (is_abbrev(trap_type, "power")) {
        type = DOOR_TRAP_ENERGY;
      } else {
        sendTo("No such exit trap-type.\n\r");
        sendTo("Syntax: trap exit <direction> <trap-type>\n\r");
        return FALSE;
      }
      if (!hasTrapComps(trap_type, TRAP_TARG_DOOR, 0)) {
        sendTo("You need more items to make that trap.\n\r");
        return FALSE;
      }

      if (getDoorTrapLearn(type) <= 0) {
        sendTo("You need more training before setting a door trap.\n\r");
        return FALSE;
      }
 
      sendTo("You start working on your trap.\n\r");
      sprintf(buf, "$n starts fiddling with the %s.", exitp->getName().uncap().c_str());
      act(buf, TRUE, this, NULL, NULL, TO_ROOM);
      sprintf(task_arg, "%s %s", direct, trap_type);
      start_task(this, NULL, NULL, TASK_TRAP_DOOR, task_arg, 3, inRoom(), type, door, 5);
      return FALSE;
    case TRAP_TARG_CONT:
      if (!doesKnowSkill(SKILL_SET_TRAP_CONT)) {
        sendTo("You know nothing about making container traps.\n\r");
        return FALSE;
      }
      sscanf(sstring, "%s %s", direct, trap_type);
      if (!(obj = get_obj_vis_accessible(this, direct))) {
	sendTo("No such item present.\n\r");
        sendTo("Syntax: trap container <item> <trap-type>\n\r");
	return FALSE;
      }
      rc = obj->trapMe(this, trap_type);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete obj;
        obj = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        return DELETE_THIS;
      }
      return FALSE;
    case TRAP_TARG_MINE:
      if (!doesKnowSkill(SKILL_SET_TRAP_MINE)) {
        sendTo("You know nothing about making mines.\n\r");
        return FALSE;
      }

      sscanf(sstring, "%s", trap_type);

      if (is_abbrev(trap_type, "fire")) {
        type = DOOR_TRAP_FIRE;
      } else if (is_abbrev(trap_type, "explosive")) {
        type = DOOR_TRAP_TNT;
      } else if (is_abbrev(trap_type, "poison")) {
        type = DOOR_TRAP_POISON;
      } else if (is_abbrev(trap_type, "sleep")) {
        type = DOOR_TRAP_SLEEP;
      } else if (is_abbrev(trap_type, "acid")) {
        type = DOOR_TRAP_ACID;
      } else if (is_abbrev(trap_type, "spore")) {
        type = DOOR_TRAP_DISEASE;
      } else if (is_abbrev(trap_type, "bolt")) {
        type = DOOR_TRAP_BOLT;
      } else if (is_abbrev(trap_type, "disk")) {
        type = DOOR_TRAP_DISK;
      } else if (is_abbrev(trap_type, "pebble")) {
        type = DOOR_TRAP_PEBBLE;
      } else if (is_abbrev(trap_type, "frost")) {
        type = DOOR_TRAP_FROST;
      } else if (is_abbrev(trap_type, "teleport")) {
        type = DOOR_TRAP_TELEPORT;
      } else if (is_abbrev(trap_type, "power")) {
        type = DOOR_TRAP_ENERGY;
      } else {
        sendTo("No such mine trap-type.\n\r");
        sendTo("Syntax: trap mine <trap-type>\n\r");
        return FALSE;
      }
      if (getMineTrapLearn(type) <= 0) {
        sendTo("You need more training before setting a mine trap.\n\r");
        return FALSE;
      }
 
      if (!hasTrapComps(trap_type, TRAP_TARG_MINE, 0)) {
        sendTo("You need more items to make that trap.\n\r");
        return FALSE;
      }

      sendTo("You start working on your trap.\n\r");
      act("$n starts constructing a land-mine.", TRUE, this, 0, 0, TO_ROOM);
      start_task(this, NULL, NULL, TASK_TRAP_MINE, trap_type, 3, inRoom(), type, 0, 5);
      return FALSE;
    case TRAP_TARG_ARROW:
      if (!doesKnowSkill(SKILL_SET_TRAP_ARROW)) {
        sendTo("You know nothing about making arrow traps.\n\r");
        return FALSE;
      }
      sscanf(sstring, "%s %s", direct, trap_type);
      if (!(obj = get_obj_vis_accessible(this, direct))) {
	sendTo("No such item present.\n\r");
        sendTo("Syntax: trap arrow <item> <trap-type>\n\r");
	return FALSE;
      }
      
      if (is_abbrev(trap_type, "fire")) {
        type = DOOR_TRAP_FIRE;
      } else if (is_abbrev(trap_type, "explosive")) {
        type = DOOR_TRAP_TNT;
      } else if (is_abbrev(trap_type, "sleep")) {
        type = DOOR_TRAP_SLEEP;
      } else if (is_abbrev(trap_type, "acid")) {
        type = DOOR_TRAP_ACID;
      } else if (is_abbrev(trap_type, "spore")) {
        type = DOOR_TRAP_DISEASE;
      } else if (is_abbrev(trap_type, "spike")) {
        type = DOOR_TRAP_SPIKE;
      } else if (is_abbrev(trap_type, "blade")) {
        type = DOOR_TRAP_BLADE;
      } else if (is_abbrev(trap_type, "pebble")) {
        type = DOOR_TRAP_PEBBLE;
      } else if (is_abbrev(trap_type, "frost")) {
        type = DOOR_TRAP_FROST;
      } else if (is_abbrev(trap_type, "teleport")) {
        type = DOOR_TRAP_TELEPORT;
      } else if (is_abbrev(trap_type, "power")) {
        type = DOOR_TRAP_ENERGY;
      } else {
        sendTo("No such arrow trap type.\n\r");
        sendTo("Syntax: trap arrow <trap-type>\n\r");
        return FALSE;
      }

      if (getArrowTrapLearn(type) <= 0) {
        sendTo("You need more training before setting an arrow trap.\n\r");
        return FALSE;
      }

      // TODO:: modify hasTrapComps for arrows
      if (!hasTrapComps(trap_type, TRAP_TARG_CONT, 0)) {
        sendTo("You need more items to make that trap.\n\r");
        return FALSE;
      }

      sendTo("You start working on your arrow.\n\r");
      act("$n starts trapping an arrow.", TRUE, this, 0, 0, TO_ROOM);
      start_task(this, obj, NULL, TASK_TRAP_ARROW, trap_type, 3, inRoom(), type, 0, 5);
      break;
    case TRAP_TARG_GRENADE:
      if (!doesKnowSkill(SKILL_SET_TRAP_GREN)) {
        sendTo("You know nothing about making grenades.\n\r");
        return FALSE;
      }

      sscanf(sstring, "%s", trap_type);

      if (is_abbrev(trap_type, "fire")) {
        type = DOOR_TRAP_FIRE;
      } else if (is_abbrev(trap_type, "explosive")) {
        type = DOOR_TRAP_TNT;
      } else if (is_abbrev(trap_type, "poison")) {
        type = DOOR_TRAP_POISON;
      } else if (is_abbrev(trap_type, "sleep")) {
        type = DOOR_TRAP_SLEEP;
      } else if (is_abbrev(trap_type, "acid")) {
        type = DOOR_TRAP_ACID;
      } else if (is_abbrev(trap_type, "spore")) {
        type = DOOR_TRAP_DISEASE;
      } else if (is_abbrev(trap_type, "bolt")) {
        type = DOOR_TRAP_BOLT;
      } else if (is_abbrev(trap_type, "disk")) {
        type = DOOR_TRAP_DISK;
      } else if (is_abbrev(trap_type, "pebble")) {
        type = DOOR_TRAP_PEBBLE;
      } else if (is_abbrev(trap_type, "frost")) {
        type = DOOR_TRAP_FROST;
      } else if (is_abbrev(trap_type, "teleport")) {
        type = DOOR_TRAP_TELEPORT;
      } else if (is_abbrev(trap_type, "power")) {
        type = DOOR_TRAP_ENERGY;
      } else {
        sendTo("No such grenade trap-type.\n\r");
        sendTo("Syntax: trap grenade <trap-type>\n\r");
        return FALSE;
      }
      if (getGrenadeTrapLearn(type) <= 0) {
        sendTo("You need more training before setting a grenade trap.\n\r");
        return FALSE;
      }
 
      if (!hasTrapComps(trap_type, TRAP_TARG_GRENADE, 0)) {
        sendTo("You need more items to make that trap.\n\r");
        return FALSE;
      }

      sendTo("You start working on your grenade.\n\r");
      act("$n starts constructing a grenade.", TRUE, this, 0, 0, TO_ROOM);
      start_task(this, NULL, NULL, TASK_TRAP_GRENADE, trap_type, 3, inRoom(), type, 0, 5);
      return FALSE;
    default:
      sendTo("Syntax: trap <\"exit\" | \"container\" | \"mine\" | \"grenade\"> ...\n\r");
      break;
  }
  return FALSE;
}

// triggered when portal opened or entered
// returns DELETE_THIS, DELETE_ITEM
int TBeing::triggerPortalTrap(TPortal *o)
{
  int rc;
  int amnt;
  TThing *t, *t2;

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  switch (o->getPortalTrapType()) {
    case DOOR_TRAP_POISON:
      act("A tiny needle in $p jams into your hand.",
              FALSE, this, o, 0, TO_CHAR);
      act("A tiny needle in $p jams into $n's hand.",
              FALSE, this, o, 0, TO_ROOM);
      trapPoison(o->getPortalTrapDam());
      break;
    case DOOR_TRAP_SLEEP:
      act("A puff of smoke seeps from $p, enveloping you.",
              FALSE, this, o, 0, TO_CHAR);
      act("A puff of smoke seeps from $p, enveloping $n.", 
              FALSE, this, o, 0, TO_ROOM);
      rc = trapSleep(o->getPortalTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FIRE:
      act("A column of flame shoots from a concealed jet in $p at you.",
               TRUE, this, o, 0, TO_CHAR);
      act("A column of flame shoots from a concealed jet in $p at $n.",
               TRUE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act("A chaotic, swirling vortex surrounds you.", 
               TRUE, this, o, 0, TO_CHAR);
      act("A chaotic, swirling vortex surrounds $n.", 
               TRUE, this, o, 0, TO_ROOM);

      rc = trapTeleport(o->getPortalTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return rc;
    case DOOR_TRAP_SPIKE:
      act("Sharpened spikes leap from a place of concealment in $p at you.",
               TRUE, this, o, 0, TO_CHAR);
      act("Sharpened spikes leap from a place of concealment in $p at $n.",
               TRUE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act("You are engulfed in a cloud of spores.",
                FALSE,this,0,0,TO_ROOM);
      act("$n is engulfed in a cloud of spores.",
                FALSE,this,0,0,TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_DISEASE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_HAMMER:
      act("Giant weights concealed in $p crash down on you.",
               TRUE, this, o, 0, TO_CHAR);
      act("Giant weights concealed in $p crash down on $n.",
               TRUE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_BLADE:
      act("Razor sharp blades slice from a place of concealment in $p into you.",
               TRUE, this, o, 0, TO_CHAR);
      act("Razor sharp blades slice from a place of concealment in $p into $n.",
               TRUE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      act("A massive explosion destroys $p, and spews shrapnel into the room!",
               TRUE, this, o, 0, TO_CHAR);
      act("A massive explosion destroys $p, and spews shrapnel into the room!",
               TRUE, this, o, 0, TO_ROOM);
      amnt = o->getPortalTrapDam();

      // fry people in room
      for (t = roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && this != tbt && !tbt->isImmortal()) {
          act("You are hit by shrapnel!", TRUE, tbt, 0, 0, TO_CHAR);
          act("$n is hit by shrapnel.", TRUE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt/2, o);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }

      rc = objDamage(DAMAGE_TRAP_TNT, amnt, o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS | DELETE_ITEM;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS | DELETE_ITEM;

      return DELETE_ITEM;
    case DOOR_TRAP_FROST:
      act("A frosty blast jets from a place of concealment in $p into you.",
               TRUE, this, o, 0, TO_CHAR);
      act("A frosty blast jets from a place of concealment in $p into $n.",
               TRUE, this, o, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_FROST, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("Bolts of raw plasma stream from a place of concealment in $p into you.",
               TRUE, this, o, 0, TO_CHAR);
      act("Jets of raw plasma stream from a place of concealment in $p into $n.",
               TRUE, this, o, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ACID:
      act("A steaming liquid splashes from a place of concealment in $p covering you.",
               TRUE, this, o, 0, TO_CHAR);
      act("A steaming liquid splashes from a place of concealment in $p covering $n.",
               TRUE, this, o, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_ACID, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    default:
      break;
  }
  return TRUE;
}


// returns DELETE_THIS or FALSE
// DELETE_ITEM may be |= with above.
// triggers when obj is opened
int TBeing::triggerContTrap(TOpenContainer *obj)
{
  int rc = 0;
  TThing *t, *t2;
  int amnt;

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);
  obj->remContainerFlag(CONT_TRAPPED);
  obj->remContainerFlag(CONT_CLOSED);
  obj->addContainerFlag(CONT_EMPTYTRAP);

  if (!::number(0, 100)) {
    act("...But nothing happens.",
        TRUE, this, 0, 0, TO_CHAR);
    act("...But nothing happens.",
        TRUE, this, 0, 0, TO_ROOM);

    return FALSE;
  }

  switch (obj->getContainerTrapType()) {
    case DOOR_TRAP_FIRE:
      act("$p bursts into flame.", TRUE, this, obj, 0, TO_ROOM);
      act("$p bursts into flame.", TRUE, this, obj, 0, TO_CHAR);

      // bag explodes, contents go boom
      for (t = obj->getStuff(); t;t = t2) {
        t2 = t->nextThing;
        delete t;
        t = NULL;
      }
      rc = objDamage(DAMAGE_TRAP_FIRE, obj->getContainerTrapDam(), obj);

      ADD_DELETE(rc, DELETE_ITEM);
      return rc;
    case DOOR_TRAP_TNT:
      act("$p explodes violently, spewing shrapnel into the room!",
                     TRUE, this, obj, 0, TO_ROOM);
      act("$p explodes violently, spewing shrapnel into the room!",
                     TRUE, this, obj, 0, TO_CHAR);
      amnt = obj->getContainerTrapDam();

      // bag explodes, contents go boom
      for (t = obj->getStuff(); t;t = t2) {
        t2 = t->nextThing;
        delete t;
        t = NULL;
      }
      // fry people in room
      for (t = roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && this != tbt) {
          act("You are hit by shrapnel!", TRUE, tbt, 0, 0, TO_CHAR);
          act("$n is hit by shrapnel.", TRUE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt/2, obj);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }
      rc = objDamage(DAMAGE_TRAP_TNT, amnt, obj);

      ADD_DELETE(rc, DELETE_ITEM);
      return rc;
    case DOOR_TRAP_POISON:
      act("A tiny needle in $p jams into your hand.",
              FALSE, this, obj, 0, TO_CHAR);
      trapPoison(obj->getContainerTrapDam());
      break;
    case DOOR_TRAP_SLEEP:
      act("A puff of smoke seeps from $p, enveloping you.",
              FALSE, this, obj, 0, TO_CHAR);
      act("A puff of smoke seeps from $p, enveloping $n.", 
              FALSE, this, obj, 0, TO_ROOM);
      rc = trapSleep(obj->getContainerTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_SPIKE:
      act("Sharpened spikes leap from a place of concealment in $p at you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Sharpened spikes leap from a place of concealment in $p at $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_DISEASE:
      act("A cloud of spores pours from $p, engulfing you.",
                FALSE,this, obj,0,TO_ROOM);
      act("A cloud of spores pours from $p, engulfing $n.",
                FALSE,this, obj,0,TO_ROOM);
 
      trapDisease(obj->getContainerTrapDam());
      break;
    case DOOR_TRAP_TELEPORT:
      act("As you touch $p, a chaotic, swirling vortex surrounds you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("As $n touches $p, a chaotic, swirling vortex surrounds $m.",
               TRUE, this, obj, 0, TO_ROOM);
 
      rc = trapTeleport(obj->getContainerTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return rc;
    case DOOR_TRAP_PEBBLE:
      act("Dozens of tiny pebbles shoot from $p, pelting you!",
               TRUE, this, obj, 0, TO_CHAR);
      act("Dozens of tiny pebbles shoot from $p, pelting $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_BLADE:
      act("Razor sharp blades slide forth from a place of concealment in $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Razor sharp blades slide forth from a place of concealment in $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_FROST:
      act("A frosty blast jets from a place of concealment in $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("A frosty blast jets from a place of concealment in $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_ENERGY:
      act("Bolts of plasma stream from a place of concealment in $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Bolts of plasma stream forth from a place of concealment in $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_ACID:
      act("A strange liquid pours from a place of concealment in $p onto you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("A strange liquid pours from a place of concealment in $p onto $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ACID, obj->getContainerTrapDam(), obj);
      return rc;
    default:
      break;
  }
  return FALSE;
}


// returns DELETE_THIS or FALSE
// DELETE_ITEM may be |= with above.
// triggers when arrow hits
int TBeing::triggerArrowTrap(TArrow *obj)
{
  int rc = 0;
  TThing *t, *t2;
  int amnt;

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  if (!::number(0, 100)) {
    act("...But nothing happens.",
        TRUE, this, 0, 0, TO_CHAR);
    act("...But nothing happens.",
        TRUE, this, 0, 0, TO_ROOM);

    return FALSE;
  }


  switch (obj->getTrapDamType()) {
    case DOOR_TRAP_SLEEP:
      act("A puff of smoke seeps from $p, enveloping you.",
              FALSE, this, obj, 0, TO_CHAR);
      act("A puff of smoke seeps from $p, enveloping $n.", 
              FALSE, this, obj, 0, TO_ROOM);
      rc = trapSleep(obj->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FIRE:
      act("A column of flame shoots from $p at you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("A column of flame shoots from $p at $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act("A chaotic, swirling vortex surrounds you.", 
               TRUE, this, obj, 0, TO_CHAR);
      act("A chaotic, swirling vortex surrounds $n.", 
               TRUE, this, obj, 0, TO_ROOM);

      rc = trapTeleport(obj->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return rc;
    case DOOR_TRAP_SPIKE:
      act("Sharpened spikes leap from $p at you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Sharpened spikes leap from $p at $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act("You are engulfed in a cloud of spores.",
                FALSE,this,0,0,TO_ROOM);
      act("$n is engulfed in a cloud of spores.",
                FALSE,this,0,0,TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_DISEASE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_BLADE:
      act("Razor sharp blades slice from $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Razor sharp blades slice from $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      act("A massive explosion destroys $p, and spews shrapnel into the room!",
               TRUE, this, obj, 0, TO_CHAR);
      act("A massive explosion destroys $p, and spews shrapnel into the room!",
               TRUE, this, obj, 0, TO_ROOM);
      amnt = obj->getTrapDamAmount();

      // fry people in room
      for (t = roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && this != tbt && !tbt->isImmortal()) {
          act("You are hit by shrapnel!", TRUE, tbt, 0, 0, TO_CHAR);
          act("$n is hit by shrapnel.", TRUE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt/2, obj);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }

      rc = objDamage(DAMAGE_TRAP_TNT, amnt, obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS | DELETE_ITEM;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS | DELETE_ITEM;

      return DELETE_ITEM;
    case DOOR_TRAP_FROST:
      act("A frosty blast jets from $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("A frosty blast jets from $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_FROST, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("Bolts of raw plasma stream from $p into you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("Jets of raw plasma stream from $p into $n.",
               TRUE, this, obj, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_ENERGY, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ACID:
      act("A steaming liquid splashes from $p covering you.",
               TRUE, this, obj, 0, TO_CHAR);
      act("A steaming liquid splashes from $p covering $n.",
               TRUE, this, obj, 0, TO_ROOM);
 
      rc = objDamage(DAMAGE_TRAP_ACID, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    default:
      break;
  }

  return TRUE;
}


// returns DELETE_THIS or FALSE
int TBeing::triggerDoorTrap(dirTypeT door)
{
  roomDirData *exitp, *back = NULL;
  TRoom *rp;
  int dam;
  int rc;

  exitp = exitDir(door);
  dam = dice(exitp->trap_dam, 8);

  // door traps can be triggered by means other than opening
  // eg trying to set another trap
  //  rawOpenDoor(door);
  
  REMOVE_BIT(exitp->condition, EX_TRAPPED);
  if ((rp = real_roomp(exitp->to_room)) &&
      (back = rp->dir_option[rev_dir[door]])) {
    REMOVE_BIT(back->condition, EX_TRAPPED);
  }

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  switch (exitp->trap_info) {
    case DOOR_TRAP_POISON:
      sendTo(fmt("A small needle lunges out of the %s and punctures your hand.\n\r") %
               fname(exitp->keyword));
      trapPoison(dam);
      break;
    case DOOR_TRAP_SPIKE:
      return trapDoorPierceDamage(dam, door);
    case DOOR_TRAP_SLEEP:
      sendTo("You are engulfed in a cloud of gas.\n\r");
      act("$n is engulfed in a cloud of gas.",FALSE,this,0,0,TO_ROOM);
      rc = trapSleep(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_TNT:
      exitp->destroyDoor(door,in_room);
      return trapDoorTntDamage(dam, door);
    case DOOR_TRAP_FIRE:
      return trapDoorFireDamage(dam, door);
    case DOOR_TRAP_ACID: 
      return trapDoorAcidDamage(dam, door);
    case DOOR_TRAP_DISEASE:
      sendTo("You are engulfed in a cloud of spores.\n\r");
      act("$n is engulfed in a cloud of spores.",FALSE,this,0,0,TO_ROOM);
      trapDisease(dam);
      break;
    case DOOR_TRAP_TELEPORT:
      act("A chaotic, swirling vortex surrounds you.", 
               TRUE, this, 0, 0, TO_CHAR);
      act("A chaotic, swirling vortex surrounds $n.", 
               TRUE, this, 0, 0, TO_ROOM);
 
      rc = trapTeleport(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return rc;
    case DOOR_TRAP_HAMMER:
      return trapDoorHammerDamage(dam, door);
    case DOOR_TRAP_BLADE:
      return trapDoorSlashDamage(dam, door);
    case DOOR_TRAP_ENERGY:
      return trapDoorEnergyDamage(dam, door);
    case DOOR_TRAP_FROST:
      return trapDoorFrostDamage(dam, door);
    default:
      break;
  }
  return FALSE;
}

// returns DELETE_VICT
int TTrap::moveTrapCheck(TBeing *ch, dirTypeT dir)
{
  char buf[256];
  const char *tmp_desc = NULL;
  TBeing *c;
  int rc;

  if ((isTrapEffectType(TRAP_EFF_MOVE)) && (getTrapCharges() > 0)) {

    // bypass for physical condition
    if (ch->isLevitating() || ch->isFlying())
      return FALSE;

    // if the person who set it is in my group, bypass
    if ((tmp_desc = ex_description->findExtraDesc(TRAP_EX_DESC))) {
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
int TBeing::checkForMoveTrap(dirTypeT dir)
{
  TThing *t;
  int rc;

  for (t = roomp->getStuff(); t; t = t->nextThing) {
    rc = t->moveTrapCheck(this, dir);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_THIS;
    else if (rc)
      return TRUE;
  }
  return FALSE;
}

int TTrap::insideTrapCheck(TBeing *ch, TThing *i)
{
  int rc;

  if ((isTrapEffectType(TRAP_EFF_OBJECT)) &&
      (getTrapCharges() > 0)) {
    if (ch->springTrap(this)) {
      act("As you reach into $p, you hear a strange noise...",
               FALSE, ch, i, 0, TO_CHAR);
      act("As $n reaches into $p, you hear a strange noise...",
               FALSE, this, i, 0, TO_ROOM);

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
int TBeing::checkForInsideTrap(TThing *i)
{
  TThing *t;
  int rc;

  for (t = i->getStuff(); t; t = t->nextThing) {
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
int TBeing::checkForAnyTrap(TThing *i)
{
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
int TBeing::checkForGetTrap(TThing *i)
{
  int rc;

  rc = i->getTrapCheck(this);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;
  else if (rc)
    return TRUE;
  return FALSE;
}

// returns DELETE_THIS or false
int TBeing::triggerTrap(TTrap *o)
{
  TThing *v, *v2;
  TBeing *tbt;
  int rc;

  o->setTrapCharges(o->getTrapCharges() - 1);

  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act("A small canister pops out of $p and detonates.",
              FALSE, this, o, 0, TO_CHAR);
      act("A small canister pops out of $p and detonates.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are sprayed with contact poison!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is sprayed with contact poison!",
                    FALSE, tbt, o, 0, TO_ROOM);
            tbt->trapPoison(2 * o->getTrapDamAmount() / 3);
          }
        }
      }

      act("You are sprayed with contact poison!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is sprayed with contact poison!",
                    FALSE, this, o, 0, TO_ROOM);
      trapPoison(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_SLEEP:
      act("A vaporous fog steams from $p.",
              FALSE, this, o, 0, TO_CHAR);
      act("A vaporous fog steams from $p.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are surrounded by a noxious mist!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is surrounded by a noxious mist!",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->trapSleep(2 * o->getTrapDamAmount() / 3);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are surrounded by a noxious mist!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by a noxious mist!",
                    FALSE, this, o, 0, TO_ROOM);
      rc = trapSleep(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FIRE:
      act("A tiny spark comes out of $p, just before it erupts in flame.",
              FALSE, this, o, 0, TO_CHAR);
      act("A tiny spark comes out of $p, just before it erupts in flame.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are burned by the flames!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is burned by the flames.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_FIRE, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are burned by the flames!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is burned by the flames.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act("A whirling vortex suddenly surrounds $p.",
              FALSE, this, o, 0, TO_CHAR);
      act("A whirling vortex suddenly surrounds $p.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You find yourself sucked into the vortex!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n flails wildly, but falls into the vortex.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->trapTeleport(2 * o->getTrapDamAmount() / 3);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You find yourself sucked into the vortex!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n flails wildly, but falls into the vortex.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = trapTeleport(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act("A cloud of spores puffs from $p.",
              FALSE, this, o, 0, TO_CHAR);
      act("A cloud of spores puffs from $p.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are surrounded by the thick cloud!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is surrounded by the thick cloud!",
                    FALSE, tbt, o, 0, TO_ROOM);
            tbt->trapDisease(2 * o->getTrapDamAmount() / 3);
          }
        }
      }

      act("You are surrounded by the thick cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by the thick cloud!",
                    FALSE, this, o, 0, TO_ROOM);
      trapDisease(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_BOLT:
      act("A canister pops out of $p and detonates, scattering hundreds of sharp, tiny bolts.",
              FALSE, this, o, 0, TO_CHAR);
      act("A canister pops out of $p and detonates, scattering hundreds of sharp, tiny bolts.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are hit by the deadly bolts!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is hit by the bolts.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_PIERCE, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are perforated by the bolts!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is perforated by the bolts.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_PEBBLE:
      act("A canister pops out of $p and detonates, spraying pebbles everywhere.",
              FALSE, this, o, 0, TO_CHAR);
      act("A canister pops out of $p and detonates, spraying pebbles everywhere.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are hit by the fusillade!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is hit by the pebbles.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_BLUNT, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are hit by the fusillade!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is hit by the pebbles.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_DISK:
      act("A canister pops out of $p and detonates, throwing razor-disks in all directions.",
              FALSE, this, o, 0, TO_CHAR);
      act("A canister pops out of $p and detonates, throwing razor-disks in all directions.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are hit by the slashing disks!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is hit by the razors.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_SLASH, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are slashed by the razor-disks!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is slashed by the razor-disks.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      act("A canister pops out of $p and detonates spraying white hot shrapnel and bomb fragments everywhere.",
              FALSE, this, o, 0, TO_CHAR);
      act("A canister pops out of $p and detonates spraying white hot shrapnel and bomb fragments everywhere.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are hit by the flak!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is hit by the flak.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_TNT, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are blasted by $p!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is blasted by fragments from $p.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_TNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FROST:
      act("An icy cloud pours out of $p.",
              FALSE, this, o, 0, TO_CHAR);
      act("An icy cloud pours out of $p.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are surrounded by the frosty cloud!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is surrounded by the frozen cloud.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_FROST, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are frozen by the icy cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is frozen by the icy cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("$p glows with magic, before streams of plasma streak out of it.",
              FALSE, this, o, 0, TO_CHAR);
      act("$p glows with magic, before streams of plasma streak out of it.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("You are blasted by the plasma!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("$n is blasted by the plasma.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_ENERGY, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are devastated by dozens of plasma bolts!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is devastated by dozens of plasma bolts.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_ACID:
      act("A yellow-green cloud billows out of $p.",
              FALSE, this, o, 0, TO_CHAR);
      act("A yellow-green cloud billows out of $p.",
              FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (v = roomp->getStuff(); v; v = v2) {
          v2 = v->nextThing;
          tbt = dynamic_cast<TBeing *>(v);
          if (tbt && tbt->desc && tbt != this) {
            act("The acid cloud surrounds you!",
                    FALSE, tbt, o, 0, TO_CHAR);
            act("The acid cloud surrounds $n.",
                    FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_ACID, 1 * o->getTrapDamAmount() / 2, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act("You are surrounded by the horrid acid cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by the horrid acid cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ACID, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    default:
      vlogf(LOG_BUG, fmt("Unknown trap type %d in triggerTrap (%s:%d)") %  
         o->getTrapDamType() % o->getName() % o->objVnum());
      return TRUE;
  }

  return TRUE;
}

// returns DELETE_THIs or FALSE
int TBeing::trapDoorTntDamage(int amnt, dirTypeT door)
{
  TThing *t, *tmp;
  int rc;

  sendToRoom("You hear a loud boom.\n\r", in_room);
  sendToRoom("The door is ripped apart by some sort of explosive.\n\r", in_room);

  for (t = roomp->getStuff(); t; t = tmp) {
    tmp = t->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && this != tbt) {
      rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt/2, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  } 

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom("You hear a loud boom.\n\r", exitDir(door)->to_room);
    sendToRoom("The door is ripped apart by some sort of explosive.\n\r", 
             exitDir(door)->to_room);

    for (t = rp->getStuff(); t; t = tmp) {
      tmp = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt && this != tbt) {
        rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt/4, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = NULL;
        }
      }
    } 
  }
  
  // apply top opened
  return objDamage(DAMAGE_TRAP_TNT, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorPierceDamage(int amnt, dirTypeT door)
{ 
  char buf[256];

  sprintf(buf, "You hear a loud metallic sound as a sharpened spike leaps out of the %s!\n\r", fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {

    sendToRoom(buf, exitDir(door)->to_room);

  }

  act("$n is skewered by the spike.", TRUE, this, 0, 0, TO_ROOM);
  act("You are skewered by the spike.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_PIERCE, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorHammerDamage(int amnt, dirTypeT door)
{ 
  char buf[256];

  sprintf(buf, 
       "You hear a grinding noise as giant weights fall from above the %s!\n\r",
       fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);
  }

  act("$n is hit by a falling weight.", TRUE, this, 0, 0, TO_ROOM);
  act("You are crushed by a falling weight.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_BLUNT, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorSlashDamage(int amnt, dirTypeT door)
{ 
  char buf[256];

  sprintf(buf, 
       "You hear a grinding noise as swinging blades slice out of %s!\n\r",
       fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);
  }

  act("$n is cut by the blades.", TRUE, this, 0, 0, TO_ROOM);
  act("You are cut by the blades.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_SLASH, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorFrostDamage(int amnt, dirTypeT door)
{ 
  TThing *t, *tmp;
  int rc;
  char buf[256];

  sprintf(buf, "You hear a high pitched whine as a frosty blast rushes out of the %s!\n\r", fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (t = roomp->getStuff(); t; t = tmp) {
    tmp = t->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && this != tbt && !tbt->isImmortal()) {
      act("$n is chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_ROOM);
      act("You are chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_CHAR);
      rc = tbt->objDamage(DAMAGE_TRAP_FROST, amnt/2, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {

    sendToRoom(buf, exitDir(door)->to_room);

    for (t = rp->getStuff(); t; t = tmp) {
      tmp = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt && this != tbt) {
        act("$n is chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_ROOM);
        act("You are chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_CHAR);
        rc = tbt->objDamage(DAMAGE_TRAP_FROST, amnt/3, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = NULL;
        }
      }
    }
  }

  act("$n is frozen by the arctic blast.", TRUE, this, 0, 0, TO_ROOM);
  act("You are frozen by the arctic blast.", TRUE, this, 0, 0, TO_CHAR);

  rc = frostEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return objDamage(DAMAGE_TRAP_FROST, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorEnergyDamage(int amnt, dirTypeT door)
{ 
  TThing *t, *tmp;
  int rc;
  char buf[256];

  sprintf(buf, "You hear a powerful humming as bolts of plasma stream out of the %s!\n\r", fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (t = roomp->getStuff(); t; t = tmp) {
    tmp = t->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && this != tbt && !tbt->isImmortal()) {
      act("$n is hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_ROOM);
      act("You are hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_CHAR);
      rc = tbt->objDamage(DAMAGE_TRAP_ENERGY, amnt/2, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {

    sendToRoom(buf, exitDir(door)->to_room);

    for (t = rp->getStuff(); t; t = tmp) {
      tmp = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt && this != tbt) {
        act("$n is hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_ROOM);
        act("You are hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_CHAR);
        rc = tbt->objDamage(DAMAGE_TRAP_ENERGY, amnt/3, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = NULL;
        }
      }
    }
  }

  act("$n is blasted by numerous plasma bolts!", TRUE, this, 0, 0, TO_ROOM);
  act("You are blasted by numerous plasma bolts!", TRUE, this, 0, 0, TO_CHAR);

  return objDamage(DAMAGE_TRAP_ENERGY, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorFireDamage(int amnt, dirTypeT door)
{
  int rc;
  char buf[256];

  sprintf(buf, "You feel an intense amount of heat as flames shoot from a %s!\n\r", fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  act("$n is enveloped by flames.", TRUE, this, 0, 0, TO_ROOM);
  act("You are enveloped by fire.", TRUE, this, 0, 0, TO_CHAR);

  rc = flameEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
     return DELETE_THIS;

  rc = objDamage(DAMAGE_TRAP_FIRE, amnt, NULL);
  return rc;
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorAcidDamage(int amnt, dirTypeT door)
{
  TThing *t, *tmp;
  int rc;
  char buf[256];

  sprintf(buf, "A strange liquid squirts everywhere as the %s is opened.\n\r",
       fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (t = roomp->getStuff(); t; t = tmp) {
    tmp = t->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && this != tbt) {
      rc = tbt->objDamage(DAMAGE_TRAP_ACID, amnt/2, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom *rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {

    sendToRoom(buf, exitDir(door)->to_room);

    for (t = rp->getStuff(); t; t = tmp) {
      tmp = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt && this != tbt) {
        rc = tbt->objDamage(DAMAGE_TRAP_ACID, amnt/3, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = NULL;
        }
      }
    }
  }

 
  rc = acidEngulfed();
  if (IS_SET_DELETE(rc, DELETE_THIS))
     return DELETE_THIS;

  return objDamage(DAMAGE_TRAP_ACID, amnt, NULL);
}

// returns DELETE_THIS
int TBeing::trapTeleport(int amt)
{
  int rc;

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

int TBeing::trapSleep(int amt)
{
  int rc = FALSE;

  if (isImmune(IMMUNE_SLEEP, WEAR_BODY)) {
    sendTo("You yawn, but are otherwise immune to the sleep trap.\n\r");
    return FALSE;
  }

  if (!isLucky(levelLuckModifier(GetMaxLevel()))) {
    // at 2 minutes per mudhour, let's not make this too painful
    rc = rawSleep(0, (2 * UPDATES_PER_MUDHOUR), 1, SAVE_NO);
  } else
    sendTo("You feel sleepy, but you recover.\n\r");

  return rc;
}

void TBeing::trapDisease(int amt)
{
  affectedData aff;

  aff.type = AFFECT_DISEASE;
  aff.modifier = DISEASE_FLU;
  aff.level = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.duration = 4 * UPDATES_PER_MUDHOUR;

  if (isImmortal() ||
      isImmune(IMMUNE_DISEASE, WEAR_BODY)) {
    act("Hmmm, lucky you, it doesn't seem to have had any effect.",
            FALSE, this, 0, 0, TO_CHAR);
    return;
  } else if (isLucky(amt) && isTough()) {
    act("You are able to shake off most of the effects, but you still feel somewhat sick.", FALSE, this, 0, 0, TO_CHAR);
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
  return;
}

void TBeing::trapPoison(int amt)
{
  affectedData af, af2;

  af.type = SPELL_POISON;
  af.duration = 12 * UPDATES_PER_MUDHOUR;
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
  if (isImmortal() ||
      isImmune(IMMUNE_POISON, WEAR_BODY)) {
    act("Hmmm, lucky you, it doesn't seem to have had any effect.", FALSE, this, 0, 0, TO_CHAR);
  } else if (isImmune(IMMUNE_POISON, WEAR_BODY)) {
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
    affectTo(&af2);
    act("You are able to shake off most of the effects, but you still feel somewhat sick.", FALSE, this, 0, 0, TO_CHAR);
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

void TBeing::informMess()
{
  switch (getPosition()) {
    case POSITION_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.",
	  TRUE, this, 0, 0, TO_ROOM);
      act("You are mortally wounded, and will die soon, if not aided.",
	  FALSE, this, 0, 0, TO_CHAR);
      break;
    case POSITION_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.",
	  TRUE, this, 0, 0, TO_ROOM);
      act("You are incapacitated and you will slowly die, if not aided.",
	  FALSE, this, 0, 0, TO_CHAR);
      break;
    case POSITION_STUNNED:
      act("$n is stunned, but will probably regain consciousness.",
	  TRUE, this, 0, 0, TO_ROOM);
      act("You're stunned, but you will probably regain consciousness.",
	  FALSE, this, 0, 0, TO_CHAR);
      break;
    case POSITION_DEAD:
      act("$n is dead! R.I.P.", TRUE, this, 0, 0, TO_ROOM);
      act("You are dead!  Sorry...", FALSE, this, 0, 0, TO_CHAR);
      break;
    default:			// >= POSITION SLEEPING 
      break;
  }
}

// may return DELETE_THIS
// ch->task is still valid here, use it to parse for stuff as needed
int TBeing::goofUpTrap(doorTrapT trap_type, trap_targ_t goof_type)
{
  int trapdamage;
  int rc;
  TObj *obj;
  char buf1[256], buf2[256];

  if (goof_type == TRAP_TARG_DOOR) {
    half_chop(task->orig_arg, buf1, buf2);
 
    trapdamage = getDoorTrapDam(trap_type);
    trapdamage = dice(trapdamage, 8) / 3;

    hasTrapComps(buf2, TRAP_TARG_DOOR, -1);  // delete comps

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act("You knick yourself and got some of the poison in the wound.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n knicks $mself and gets some of the poison in the wound.",
            FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_SPIKE:
        act("You jostle the frame, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n jostles the frame, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("You are impaled by the spikes!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pierced by $s trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_BLADE:
        act("You jostle the frame, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n jostles the frame, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Swinging blades slice into your body!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is sliced by $s razor trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_HAMMER:
        act("You jostle the frame, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n jostles the frame, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Heavy weights fall on you!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is crushed by $s hammer trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act("You slip up and are caught in your own trap!",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
           return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act("You slip up, and drop the spores.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own spore trap.",
                FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own teleport trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
      case DOOR_TRAP_FIRE:
        act("You jostle the frame, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n jostles the frame, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your fiery trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's fiery trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act("You slip up and spill the acid on yourself!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips and spills caustic acid upon $mself.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own energy trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act("You jostle the frame, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n jostles the frame, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your frost trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's frost trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage/2, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo("You slip up, and the trap you were setting goes off in your face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
    // door traps
  } else if (goof_type == TRAP_TARG_CONT) {
 
    trapdamage = getContainerTrapDam(trap_type);
    trapdamage = dice(trapdamage, 8) / 3;
    obj = task->obj;

    hasTrapComps(task->orig_arg, TRAP_TARG_CONT, -1);  // delete comps

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act("You knick yourself and got some of the poison in the wound.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n knicks $mself and gets some of the poison in the wound.",
            FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_SPIKE:
        act("You jostle $p, and your trap goes off in your face!",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n jostles $p, and $s trap goes off in $s face!",
            FALSE, this, obj, 0, TO_ROOM);
        act("You are impaled by the spikes!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pierced by $s trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_BLADE:
        act("You jostle $p, and your trap goes off in your face!",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n jostles $p, and $s trap goes off in $s face!",
            FALSE, this, obj, 0, TO_ROOM);
        act("Swinging blades slice into your body!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is sliced by $s razor trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act("You jostle $p, and your trap goes off in your face!",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n jostles $p, and $s trap goes off in $s face!",
            FALSE, this, obj, 0, TO_ROOM);
        act("Dozens of pebbles pelt you!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pelted by $s pebble trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act("You slip up and are caught in your own trap!",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act("You slip up, and drop the spores.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own spore trap.",
                FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own teleport trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
      case DOOR_TRAP_FIRE:
        act("You jostle $p, and your trap goes off in your face!",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n jostles $p, and $s trap goes off in $s face!",
            FALSE, this, obj, 0, TO_ROOM);
        act("Your fiery trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's fiery trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act("You slip up and spill the acid on yourself!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips and spills caustic acid upon $mself.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own energy trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act("You jostle $p, and your trap goes off in your face!",
            FALSE, this, obj, 0, TO_CHAR);
        act("$n jostles $p, and $s trap goes off in $s face!",
            FALSE, this, obj, 0, TO_ROOM);
        act("Your frost trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's frost trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage/2, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo("You slip up, and the trap you were setting goes off in your face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
    // cont traps
  } else if (goof_type == TRAP_TARG_MINE) {
 
    trapdamage = getMineTrapDam(trap_type);
    trapdamage = dice(trapdamage, 8) / 3;

    hasTrapComps(task->orig_arg, TRAP_TARG_MINE, -1);  // delete comps

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act("You knick yourself and got some of the poison in the wound.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n knicks $mself and gets some of the poison in the wound.",
            FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_BOLT:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("You are perforated by the tiny bolts, as they go flying everywhere!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pierced by $s trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_DISK:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Razor disks scatter everywhere and slice into your body!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is sliced by $s disk trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Dozens of pebbles pelt you!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pelted by $s pebble trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act("You slip up and are caught in your own trap!",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act("You slip up, and drop the spores.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own spore trap.",
                FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own teleport trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your explosive trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's explosive trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FIRE:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your fiery trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's fiery trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FIRE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act("You slip up and spill the acid on yourself!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips and spills caustic acid upon $mself.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own energy trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your frost trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's frost trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo("You slip up, and the trap you were setting goes off in your face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
  } else if (goof_type == TRAP_TARG_GRENADE) {
 
    trapdamage = getGrenadeTrapDam(trap_type);
    trapdamage = dice(trapdamage, 8) / 3;

    hasTrapComps(task->orig_arg, TRAP_TARG_GRENADE, -1);  // delete comps

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act("You knick yourself and got some of the poison in the wound.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n knicks $mself and gets some of the poison in the wound.",
            FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_BOLT:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("You are perforated by the tiny bolts, as they go flying everywhere!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pierced by $s trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_DISK:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Razor disks scatter everywhere and slice into your body!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is sliced by $s disk trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Dozens of pebbles pelt you!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n is pelted by $s pebble trap.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act("You slip up and are caught in your own trap!",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act("You slip up, and drop the spores.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own spore trap.",
                FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own teleport trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your explosive trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's explosive trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FIRE:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your fiery trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's fiery trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FIRE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act("You slip up and spill the acid on yourself!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips and spills caustic acid upon $mself.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act("You slip up, and lose control of the magical forces.",
                FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up and is caught in $s own energy trap.",
                FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act("You slip up, and your trap goes off in your face!",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n slips up, and $s trap goes off in $s face!",
            FALSE, this, 0, 0, TO_ROOM);
        act("Your frost trap engulfs you.",
            FALSE, this, 0, 0, TO_CHAR);
        act("$n's frost trap engulfs $m.",
            FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo("You slip up, and the trap you were setting goes off in your face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
  }
  return FALSE;
}

bool TBeing::hasTrapComps(const char *type, trap_targ_t targ, int amt, int *price)
{
  int item1 = 0, item2 = 0, item3 = 0, item4 = 0;

  if (is_abbrev(type, "fire")) {
    item1 = ST_FLINT;
    item2 = ST_SULPHUR;
    item3 = ST_BAG;
  } else if (is_abbrev(type, "explosive")) {
    item1 = ST_FLINT;
    item2 = ST_SULPHUR;
    item3 = ST_HYDROGEN;
  } else if (is_abbrev(type, "poison")) {
    if (targ == TRAP_TARG_DOOR ||
        targ == TRAP_TARG_CONT) {
      item1 = ST_NEEDLE;
      item2 = ST_SPRING;
      item3 = ST_POISON;
    } else if (targ == TRAP_TARG_MINE ||
               targ == TRAP_TARG_GRENADE) {
      item1 = ST_CANISTER;
      item2 = ST_SPRING;
      item3 = ST_CON_POISON;
    }
  } else if (is_abbrev(type, "sleep")) {
    item1 = ST_NOZZLE;
    item2 = ST_GAS;
    item3 = ST_HOSE;
  } else if (is_abbrev(type, "acid")) {
    if (targ == TRAP_TARG_DOOR ||
        targ == TRAP_TARG_CONT) {
      item1 = ST_NOZZLE;
      item2 = ST_ACID_VIAL;
      item3 = ST_BELLOWS;
    } else if (targ == TRAP_TARG_MINE ||
               targ == TRAP_TARG_GRENADE) {
      item1 = ST_CANISTER;
      item2 = ST_SPRING;
      item3 = ST_ACID_VIAL;
    }
  } else if (is_abbrev(type, "spore")) {
    if (targ == TRAP_TARG_DOOR ||
        targ == TRAP_TARG_CONT) {
      item1 = ST_FUNGUS;
      item2 = ST_NOZZLE;
      item3 = ST_BELLOWS;
    } else if (targ == TRAP_TARG_MINE ||
               targ == TRAP_TARG_GRENADE) {
      item1 = ST_CANISTER;
      item2 = ST_SPRING;
      item3 = ST_FUNGUS;
    }
  } else if (is_abbrev(type, "spike")) {
    if (targ != TRAP_TARG_DOOR &&
        targ != TRAP_TARG_CONT) 
      vlogf(LOG_MISC, fmt("spike trap being set  with trap targ: %d") % targ);

    item1 = ST_SPIKE;
    item2 = ST_SPRING;
    item3 = ST_TRIPWIRE;
  } else if (is_abbrev(type, "bolt")) {
    if (targ != TRAP_TARG_MINE && 
        targ != TRAP_TARG_GRENADE) 
      vlogf(LOG_MISC, fmt("bolt trap being set  with trap targ: %d") % targ);

    item1 = ST_TUBING;
    item2 = ST_CGAS;
    item3 = ST_BOLTS;
  } else if (is_abbrev(type, "blade")) {
    if (targ != TRAP_TARG_DOOR &&
        targ != TRAP_TARG_CONT)
      vlogf(LOG_MISC, fmt("blade trap being set  with trap targ: %d") % targ);

    item1 = ST_RAZOR_BLADE;
    item2 = ST_SPRING;
    item3 = ST_TRIPWIRE;
  } else if (is_abbrev(type, "disk")) {
    if (targ != TRAP_TARG_MINE &&
        targ != TRAP_TARG_GRENADE)
      vlogf(LOG_MISC, fmt("disk trap being set  with trap targ: %d") % targ);

    item1 = ST_RAZOR_DISK;
    item2 = ST_SPRING;
    item3 = ST_CANISTER;
  } else if (is_abbrev(type, "hammer")) {
    if (targ != TRAP_TARG_DOOR) 
      vlogf(LOG_MISC, fmt("hammer trap being set  with trap targ: %d") % targ);

    item1 = ST_CONCRETE;
    item2 = ST_WEDGE;
    item3 = ST_TRIPWIRE;
  } else if (is_abbrev(type, "pebble")) {
    if (targ != TRAP_TARG_CONT && 
        targ != TRAP_TARG_MINE && 
        targ != TRAP_TARG_GRENADE) 
      vlogf(LOG_MISC, fmt("pebble trap being set  with trap targ: %d") % targ);

    item1 = ST_TUBING;
    item2 = ST_CGAS;
    item3 = ST_PEBBLES;
  } else if (is_abbrev(type, "frost")) {
    item1 = ST_NOZZLE;
    item2 = ST_HOSE;
    item3 = ST_FROST;
  } else if (is_abbrev(type, "teleport")) {
    if (targ == TRAP_TARG_DOOR ||
        targ == TRAP_TARG_CONT) {
      item1 = ST_PENTAGRAM;
      item2 = ST_TRIPWIRE;
      item3 = ST_BLINK;
    } else if (targ == TRAP_TARG_MINE ||
               targ == TRAP_TARG_GRENADE) {
      item1 = ST_PENTAGRAM;
      item2 = ST_CRYSTALINE;
      item3 = ST_BLINK;
    }
  } else if (is_abbrev(type, "power")) {
    if (targ == TRAP_TARG_DOOR ||
        targ == TRAP_TARG_CONT) {
      item1 = ST_PENTAGRAM;
      item2 = ST_TRIPWIRE;
      item3 = ST_ATHANOR;
    } else if (targ == TRAP_TARG_MINE ||
               targ == TRAP_TARG_GRENADE) {
      item1 = ST_PENTAGRAM;
      item2 = ST_CRYSTALINE;
      item3 = ST_ATHANOR;
    }
  } else {
    vlogf(LOG_MISC, fmt("Bad call to hasTrapComps() : %s") %  type);
    return FALSE;
  }
  item1 = real_object(item1);
  item2 = real_object(item2);
  item3 = real_object(item3);

  TThing *com4 = NULL;

  if (targ == TRAP_TARG_MINE) {
    item4 = ST_CASE_MINE;
    item4 = real_object(item4);
    com4 = searchLinkedListVis(this, obj_index[item4].name, getStuff());
  } else if (targ == TRAP_TARG_GRENADE) {
    item4 = ST_CASE_GRENADE;
    item4 = real_object(item4);
    com4 = searchLinkedListVis(this, obj_index[item4].name, getStuff());
  }

  TThing * com1 = searchLinkedListVis(this, obj_index[item1].name, getStuff());
  TThing * com2 = searchLinkedListVis(this, obj_index[item2].name, getStuff());
  TThing * com3 = searchLinkedListVis(this, obj_index[item3].name, getStuff());

  if (price) {
    *price = 0;
    TObj *obj;
    if (com1) {
      obj = dynamic_cast<TObj *>(com1);
      *price += obj->obj_flags.cost;
    }
    if (com2) {
      obj = dynamic_cast<TObj *>(com2);
      *price += obj->obj_flags.cost;
    }
    if (com3) {
      obj = dynamic_cast<TObj *>(com3);
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

void TBeing::sendTrapMessage(const char *type, trap_targ_t targ, int num)
{
  sstring buf;

  if (is_abbrev(type, "fire")) {
    if (num == 1) {
      sendTo("You pour your sulphur into a small bag.\n\r");
      act("$n pours some sulphur into a small bag.", 
             TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      sendTo("You stick a flint halfway down into the bag of sulphur.\n\r");
      act("$n puts a small flint down into a small bag.", 
                TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      sendTo("You close the top of the bag around the flint.\n\r");
      act("$n wraps the top of $s bag around a small flint.", 
              TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You trap the %s with your bag.\n\r") % 
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n jimmys the %s with $s bag.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You trap $p with your bag.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n jimmys $p with $s bag.", 
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You situate the bag inside the mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n situates $s bag inside a mine casing.", 
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You situate the bag inside the grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n situates $s bag inside a grenade casing.", 
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "explosive")) {
    if (num == 1) {
      sendTo("You attach your sulphur to the hydrogen bottle's neck.\n\r");
      act("$n attaches some sulphur to a bottle of hydrogen.",
           TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      sendTo("You wedge a piece of flint into the bottle's neck.\n\r");
      act("$n wedges a piece of flint into $s bottle.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      sendTo("You pour some more sulphur around the piece of flint.\n\r");
      act("$n pours some more sulphur around the flint.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You afix the bottle to the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n jimmys the %s with $s bottle.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You afix the bottle to $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n jimmys $p with $s bottle.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You situate the bottle inside the mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n situates the bottle inside a mine casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You situate the bottle inside the grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n situates the bottle inside a grenade casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "poison")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You attach a needle to the tiny spring.\n\r");
        act("$n attaches a thin needle to $s tiny spring.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You pour the contact poison into the canister.\n\r");
        act("$n pours a murky liquid into a canister.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You screw the needle apparatus into the vial of poison.\n\r");
        act("$n screws the needle apparatus into the vial of poison.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You afix the spring to the canister.\n\r");
        act("$n fiddles with a canister.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You release the poison into the needle.\n\r");
        act("$n releases the poison into the needle.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You gently place the canister apparatus inside the mine casing.\n\r");
        act("$n puts the canister inside a mine casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You gently place the canister apparatus inside a grenade casing.\n\r");
        act("$n puts the canister inside a grenade casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }

    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the poisoned needle inside the %s's lock.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the poisoned needle inside the %s's lock.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the poisoned needle inside $p's lock.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the poisoned needle inside $p's lock.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You springload the canister and arm the land mine.\n\r");
        act("$n fiddles with a land mine.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You take the safeties off the canister and prepare the grenade for use.\n\r");
        act("$n fiddles with a grenade.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "sleep")) {
    if (num == 1) {
      sendTo("You screw a small nozzle into a hose.\n\r");
      act("$n screw a small nozzle into a hose.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      sendTo("You clamp the hose snuggly around the nozzle.\n\r");
      act("$n clamps the hose snuggly around the nozzle.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      sendTo("You afix the hose to the vial of gas.\n\r");
      act("$n afixes the hose to $s vial of gas.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the gas apparatus within the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the apparatus inside the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the gas apparatus inside $p's lock.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the gas apparatus inside $p's lock.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You conceal the gas apparatus inside a mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n conceals the gas apparatus inside a mine casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You conceal the gas apparatus inside a grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n conceals the gas apparatus inside a grenade casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "frost")) {
    if (num == 1) {
      sendTo("You screw a small nozzle into a hose.\n\r");
      act("$n screw a small nozzle into a hose.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      sendTo("You clamp the hose snuggly around the nozzle.\n\r");
      act("$n clamps the hose snuggly around the nozzle.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      sendTo("You afix the hose to the cylinder of liquid frost.\n\r");
      act("$n afixes the hose to $s cylinder of liquid frost.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the frost apparatus into the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the apparatus into the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the frost apparatus inside $p's lock.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the frost apparatus inside $p's lock.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You conceal the frost apparatus inside a mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n conceals the frost apparatus inside a mine casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You conceal the frost apparatus inside a grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n conceals the frost apparatus inside a grenade casing.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "acid")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You attach the vial of acid to the bellows.\n\r");
        act("$n attach the vial of acid to your bellows.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You pour the acid into the canister.\n\r");
        act("$n pours a bubbly liquid into a canister.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You attach the nozzle to the end of the bellows.\n\r");
        act("$n attaches the nozzle to $s bellows.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You afix the spring to the canister.\n\r");
        act("$n fiddles with a canister.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You release the acid into the bellows.\n\r");
        act("$n releases the acid into the bellows.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You gently place the canister apparatus inside the mine casing.\n\r");
        act("$n puts the canister inside a mine casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You gently place the canister apparatus inside a grenade casing.\n\r");
        act("$n puts the canister inside a grenade casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the trap inside the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the trap inside the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the trap inside $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the trap inside $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You springload the canister and arm the land mine.\n\r");
        act("$n fiddles with a land mine.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You take the safeties off the canister and prepare the grenade for use.\n\r");
        act("$n fiddles with a grenade.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "spore")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT ) {
        sendTo("You carefully stuff the fungus into the bellows.\n\r");
        act("$n stuffs some fungus into $s bellows.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You pour the fungus spores into the canister.\n\r");
        act("$n pours some fungus spores into a canister.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT ) {
        sendTo("You cap the end of the bellows with a nozzle.\n\r");
        act("$n caps the bellows with a nozzle.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return; 
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You afix the spring to the canister.\n\r");
        act("$n fiddles with a canister.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT ) {
        sendTo("You shake the bellows, causing the fungus within to release its spores.\n\r");
        act("$n shakes the bellows vigorously.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You gently place the canister apparatus inside the mine casing.\n\r");
        act("$n puts the canister inside a mine casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You gently place the canister apparatus inside a grenade casing.\n\r");
        act("$n puts the canister inside a grenade casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the trap inside the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the trap inside the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the trap inside $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the trap inside $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        sendTo("You springload the canister and arm the land mine.\n\r");
        act("$n fiddles with a land mine.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You take the safeties off the canister and prepare the grenade for use.\n\r");
        act("$n fiddles with a grenade.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "spike")) {
    if (num == 1) {
      sendTo("You afix the spring to your sharpened spike.\n\r");
      act("$n afixes a spring to a sharpened spike.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the spike inside the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the spike inside the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the spike inside $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the spike inside $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      sendTo("You tie the tripwire to the spike apparatus and springload it.\n\r");
      act("$n ties a tripwire to the spike and fiddles with it some more.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You stretch the tripwire across the %s and tie it off.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stretches a tripwire across the %s and ties it off.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You stretch the tripwire across $p's lock and tie it off.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n stretches $s tripwire across $p's lock and ties it off.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "bolt")) {
    if (num == 1) {
      act("You attach some tubing to the outlet valve of the compressed gas.",
           FALSE, this, 0, 0, TO_CHAR);
      act("$n fiddles with some tubing and a vial.",
           TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      act("You pour the bolts into the tubing.",
           FALSE, this, 0, 0, TO_CHAR);
      act("$n pours some bolts into the tubing.",
           TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      act("You arm the trigger mechanism on the compressed gas.",
             FALSE, this, 0, 0, TO_CHAR);
      act("$n arm the trigger mechanism on the compressed gas.",
             TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_MINE) {
        act("You conceal the tubing inside the mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n fiddles with a mine casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You conceal the tubing inside the grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n fiddles with a grenade casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "blade")) {
    if (num == 1) {
      sendTo("You afix the spring to your razor sharp blade.\n\r");
      act("$n afixes a spring to $s razor blade.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You conceal the razor blade inside the %s.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n conceals the razor blade inside the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You conceal the razor blade inside $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n conceals the razor blade inside $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      sendTo("You tie the tripwire to the razor apparatus and springload it.\n\r");
      act("$n ties a tripwire to the razor and fiddles with it some more.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You stretch the tripwire across the %s and tie it off.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stretches a tripwire across the %s and ties it off.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You stretch the tripwire across $p's lock and tie it off.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n stretches $s tripwire across $p's lock and ties it off.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "disk")) {
    if (num == 1) {
      sendTo("You pour the razor sharp disks into the canister.\n\r");
      act("$n pours some razor sharp disks into a canister.",
           TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      sendTo("You afix the spring to the canister.\n\r");
      act("$n fiddles with a canister.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      if (targ == TRAP_TARG_MINE) {
        sendTo("You gently place the canister apparatus inside the mine casing.\n\r");
        act("$n puts the canister inside a mine casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You gently place the canister apparatus inside a grenade casing.\n\r");
        act("$n puts the canister inside a grenade casing.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_MINE) {
        sendTo("You springload the canister and arm the land mine.\n\r");
        act("$n fiddles with a land mine.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        sendTo("You take the safeties off the canister and prepare the grenade for use.\n\r");
        act("$n fiddles with a grenade.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "hammer")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You pry open the frame of a %s with your wedge.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n pries at a %s's frame.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You shove the concrete up above the %s's frame.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stuffs something bulky above the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo("You tie the tripwire to the frame wedge.\n\r");
        act("$n ties a tripwire to $s booby trap.",
                 TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You stretch the tripwire across the %s and tie it off.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stretches a tripwire across the %s and ties it off.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "pebble")) {
    if (num == 1) {
      act("You attach some tubing to the outlet valve of the compressed gas.",
           FALSE, this, 0, 0, TO_CHAR);
      act("$n fiddles with some tubing and a vial.",
           TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 2) {
      act("You pour the pebbles into the tubing.",
           FALSE, this, 0, 0, TO_CHAR);
      act("$n pours some stones into the tubing.",
           TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      act("You arm the trigger mechanism on the compressed gas.",
             FALSE, this, 0, 0, TO_CHAR);
      act("$n arm the trigger mechanism on the compressed gas.",
             TRUE, this, 0, NULL, TO_ROOM);
      return;
    } else if (num == 4) {
      if (targ == TRAP_TARG_CONT) {
        act("You conceal the tubing on $p.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n fiddles with $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You conceal the tubing inside the mine casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n fiddles with a mine casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You conceal the tubing inside the grenade casing.",
             FALSE, this, 0, 0, TO_CHAR);
        act("$n fiddles with a grenade casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "teleport")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You plaster a pentagram to the %s to focus the magical forces.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n plasters a pentagram to the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You plaster a pentagram to $p to focus the magical forces.", 
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n plasters a pentagram to $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You plaster a pentagram to a mine casing to focus the magical forces.", 
             FALSE, this, 0, 0, TO_CHAR);
        act("$n plasters a pentagram to a mine casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You plaster a pentagram to a grenade casing to focus the magical forces.", 
             FALSE, this, 0, 0, TO_CHAR);
        act("$n plasters a pentagram to a grenade casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      sendTo("You sprinkle the blink powder around the edges of the pentagram.\n\r");
      act("$n sprinkles some dust on the pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR ||
          targ == TRAP_TARG_CONT) {
        sendTo("You bond the tripwire to one side of the pentagram.\n\r");
        act("$n fiddles with a bit of wire and $s pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You trace along the pentagram with the crystalline.\n\r");
        act("$n fiddles with a crystal and $s pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You stretch the tripwire across the %s and tie it off.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stretches a tripwire across the %s and ties it off.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You stretch the tripwire across $p's lock and tie it off.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n stretches $s tripwire across $p's lock and ties it off.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        act("You snap the crystalline, activating the magical forces in the pentagram.",
             FALSE, this, 0, 0, TO_CHAR);
        act("As $n snaps $s crystalline in half, the pentagram glows with magic.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  } else if (is_abbrev(type, "power")) {
    if (num == 1) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You plaster a pentagram to the %s to focus the magical forces.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n plasters a pentagram to the %s.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You plaster a pentagram to $p to focus the magical forces.", 
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n plasters a pentagram to $p.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE) {
        act("You plaster a pentagram to a mine casing to focus the magical forces.", 
             FALSE, this, 0, 0, TO_CHAR);
        act("$n plasters a pentagram to a mine casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_GRENADE) {
        act("You plaster a pentagram to a grenade casing to focus the magical forces.", 
             FALSE, this, 0, 0, TO_CHAR);
        act("$n plasters a pentagram to a grenade casing.",
             TRUE, this, 0, NULL, TO_ROOM);
        return;
      }
    } else if (num == 2) {
      sendTo("You position the refined athanor in the center of the pentagram.\n\r");
      act("$n puts something inside the pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
      return;
    } else if (num == 3) {
      if (targ == TRAP_TARG_DOOR||
          targ == TRAP_TARG_CONT) {
        sendTo("You bond the tripwire to one side of the pentagram.\n\r");
        act("$n fiddles with a bit of wire and $s pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        sendTo("You trace along the pentagram with the crystalline.\n\r");
        act("$n fiddles with a crystal and $s pentagram.",
               TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    } else if (num == 4) {
      if (targ == TRAP_TARG_DOOR) {
        sendTo(fmt("You stretch the tripwire across the %s and tie it off.\n\r") %
           fname(roomp->dir_option[task->flags]->keyword));
        buf=fmt("$n stretches a tripwire across the %s and ties it off.") %
           fname(roomp->dir_option[task->flags]->keyword);
        act(buf, TRUE, this, NULL, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_CONT) {
        act("You stretch the tripwire across $p's lock and tie it off.",
             FALSE, this, task->obj, 0, TO_CHAR);
        act("$n stretches $s tripwire across $p's lock and ties it off.",
             TRUE, this, task->obj, NULL, TO_ROOM);
        return;
      } else if (targ == TRAP_TARG_MINE ||
                 targ == TRAP_TARG_GRENADE) {
        act("You snap the crystalline, activating the magical forces in the pentagram.",
             FALSE, this, 0, 0, TO_CHAR);
        act("As $n snaps $s crystalline in half, the pentagram glows with magic.",
             TRUE, this, NULL, NULL, TO_ROOM);
        return;
      }
    }
  }

  vlogf(LOG_BUG, fmt("Bad trap type (%s, %d, %d) with character %s") %  
             type % targ % num % getName());
  return;
}

void TBeing::throwGrenade(TTrap *o, dirTypeT dir)
{
  char buf[256];
  TRoom *rp = NULL;

  if (!clearpath(inRoom(), dir) ||
      !roomp->dir_option[dir] ||
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

  if (!o->isTrapEffectType(TRAP_EFF_ARMED3 | TRAP_EFF_ARMED2 | TRAP_EFF_ARMED1)) {
    // don't arm it if already armed
    o->armGrenade(this);
  }

  if (rp->isRoomFlag(ROOM_PEACEFUL)) {
    *roomp += *o;
    act("$n hits some strange barrier and bounces back at you!",
         FALSE, o, 0, 0, TO_ROOM);
    return;
  }

  *rp += *o;
  sprintf(buf, "$n bounces into the room from the %s.", dirs[rev_dir[dir]]);
  act(buf, TRUE, o, 0, 0, TO_ROOM);

  return;
}

int TBeing::grenadeHit(TTrap *o)
{
  int rc;

  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act("You are sprayed with contact poison!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is sprayed with contact poison!",
                    FALSE, this, o, 0, TO_ROOM);
      trapPoison(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_SLEEP:
      act("You are surrounded by a noxious mist!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by a noxious mist!",
                    FALSE, this, o, 0, TO_ROOM);
      rc = trapSleep(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FIRE:
      act("You are burned by the flames!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is burned by the flames.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act("You find yourself sucked into the vortex!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n flails wildly, but falls into the vortex.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = trapTeleport(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act("You are surrounded by the thick cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by the thick cloud!",
                    FALSE, this, o, 0, TO_ROOM);
      trapDisease(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_BOLT:
      act("You are perforated by the bolts!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is perforated by the bolts.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_PEBBLE:
      act("You are hit by the fusillade!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is hit by the pebbles.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_DISK:
      act("You are slashed by the razor-disks!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is slashed by the razor-disks.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      act("You are blasted by $p!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is blasted by fragments from $p.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_TNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FROST:
      act("You are frozen by the icy cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is frozen by the icy cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("You are devastated by dozens of plasma bolts!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is devastated by dozens of plasma bolts.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_ACID:
      act("You are surrounded by the horrid acid cloud!",
                    FALSE, this, o, 0, TO_CHAR);
      act("$n is surrounded by the horrid acid cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ACID, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
         return DELETE_THIS;

      return TRUE;
    default:
      return TRUE;
  }

  return FALSE;
}

int TMonster::grenadeHit(TTrap *o)
{
  // first make recursive
  int rc = TBeing::grenadeHit(o);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (isPc())
    return rc;
  if (!rc)
    return FALSE;

  const char * tmp_desc;
  TBeing *ch = NULL;
  if ((tmp_desc = o->ex_description->findExtraDesc(GRENADE_EX_DESC))) {
    if ((ch = get_char(tmp_desc, EXACT_YES)))
      pissOff(this,ch);
  }

  return TRUE;
}

int TObj::grenadeHit(TTrap *o)
{
  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act("$n is sprayed with contact poison which quickly evaporates!",
                    FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_SLEEP:
      return TRUE;
    case DOOR_TRAP_FIRE:
      act("$n is burned by the flames.",
                    FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_TELEPORT:
      return TRUE;
    case DOOR_TRAP_DISEASE:
      return TRUE;
    case DOOR_TRAP_BOLT:
      act("$n is perforated by the bolts.",
                    FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_PEBBLE:
      act("$n is hit by the pebbles.",
                    FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_DISK:
      act("$n is slashed by the razor-disks.",
                    FALSE, this, o, 0, TO_ROOM);
      return TRUE;
    case DOOR_TRAP_TNT:
      act("$n is blasted by fragments from $p.",
                    FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    case DOOR_TRAP_FROST:
      act("$n is frozen by the icy cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act("$n is devastated by dozens of plasma bolts.",
                    FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    case DOOR_TRAP_ACID:
      act("$n is surrounded by the horrid acid cloud.",
                    FALSE, this, o, 0, TO_ROOM);

      return TRUE;
    default:
      return TRUE;
  }

  return FALSE;
}

int TBeing::getDoorTrapDam(doorTrapT trap_type)
{
  // this is number of d8 to use when calculating damage
  // base range: 10 - 35
  int damage = 10 + getSkillLevel(SKILL_SET_TRAP_DOOR)/2;

  damage *= getDoorTrapLearn(trap_type);
  damage /= 100;

  switch (trap_type) {
    case DOOR_TRAP_TNT:
      damage += 3;
      break; 
    case DOOR_TRAP_POISON:
      damage -= 1;
      break; 
    case DOOR_TRAP_SLEEP:
      damage += 1;
      break; 
    case DOOR_TRAP_ACID:
      damage += 1;
      break; 
    case DOOR_TRAP_DISEASE:
      damage += 3;
      break; 
    case DOOR_TRAP_FROST:
      damage += 3;
      break; 
    case DOOR_TRAP_SPIKE:
      damage -= 5;
      break; 
    case DOOR_TRAP_BOLT:
      damage += 1;
      break; 
    case DOOR_TRAP_BLADE:
      damage -= 3;
      break; 
    case DOOR_TRAP_DISK:
      damage += 3;
      break; 
    case DOOR_TRAP_HAMMER:
      damage -= 10;
      break; 
    case DOOR_TRAP_PEBBLE:
      damage -= 5;
      break; 
    case DOOR_TRAP_ENERGY:
      damage += 5;
      break; 
    case DOOR_TRAP_TELEPORT:
      damage += 5;
      break; 
    default:
      break;
  }
  damage = min(max(damage, 1), 50);
  return damage;
}

int TBeing::getContainerTrapDam(doorTrapT trap_type)
{
  // this is number of d8 to use when calculating damage
  // base range: 20 - 36
  int damage = 20 + getSkillLevel(SKILL_SET_TRAP_CONT)/3;

  damage *= getContainerTrapLearn(trap_type);
  damage /= 100;

  switch (trap_type) {
    case DOOR_TRAP_TNT:
      damage += 3;
      break; 
    case DOOR_TRAP_POISON:
      damage -= 1;
      break; 
    case DOOR_TRAP_SLEEP:
      damage += 1;
      break; 
    case DOOR_TRAP_ACID:
      damage += 1;
      break; 
    case DOOR_TRAP_DISEASE:
      damage += 3;
      break; 
    case DOOR_TRAP_FROST:
      damage += 3;
      break; 
    case DOOR_TRAP_SPIKE:
      damage -= 5;
      break; 
    case DOOR_TRAP_BOLT:
      damage += 1;
      break; 
    case DOOR_TRAP_BLADE:
      damage -= 3;
      break; 
    case DOOR_TRAP_DISK:
      damage += 3;
      break; 
    case DOOR_TRAP_HAMMER:
      damage -= 10;
      break; 
    case DOOR_TRAP_PEBBLE:
      damage -= 5;
      break; 
    case DOOR_TRAP_ENERGY:
      damage += 5;
      break; 
    case DOOR_TRAP_TELEPORT:
      damage += 5;
      break; 
    default:
      break;
  }
  damage = min(max(damage, 1), 50);
  return damage;
}

int TBeing::getMineTrapDam(doorTrapT trap_type)
{
  // this is number of d8 to use when calculating damage
  // base range: 20 - 45
  int damage = 20 + getSkillLevel(SKILL_SET_TRAP_MINE)/2;

  damage *= getMineTrapLearn(trap_type);
  damage /= 100;

  switch (trap_type) {
    case DOOR_TRAP_TNT:
      damage += 3;
      break; 
    case DOOR_TRAP_POISON:
      damage -= 1;
      break; 
    case DOOR_TRAP_SLEEP:
      damage += 1;
      break; 
    case DOOR_TRAP_ACID:
      damage += 1;
      break; 
    case DOOR_TRAP_DISEASE:
      damage += 3;
      break; 
    case DOOR_TRAP_FROST:
      damage += 3;
      break; 
    case DOOR_TRAP_SPIKE:
      damage -= 5;
      break; 
    case DOOR_TRAP_BOLT:
      damage += 1;
      break; 
    case DOOR_TRAP_BLADE:
      damage -= 3;
      break; 
    case DOOR_TRAP_DISK:
      damage += 3;
      break; 
    case DOOR_TRAP_HAMMER:
      damage -= 10;
      break; 
    case DOOR_TRAP_PEBBLE:
      damage -= 5;
      break; 
    case DOOR_TRAP_ENERGY:
      damage += 5;
      break; 
    case DOOR_TRAP_TELEPORT:
      damage += 5;
      break; 
    default:
      break;
  }
  damage = min(max(damage, 1), 50);
  return damage;
}

int TBeing::getGrenadeTrapDam(doorTrapT trap_type)
{
  // because grenades are highly portable, and nail everyone in room
  // i kept the damage on them lower then other traps.
  // this is number of d8 to use when calculating damage
  // base range: 5 - 30
  int damage = 5 + getSkillLevel(SKILL_SET_TRAP_GREN)/2;

  damage *= getGrenadeTrapLearn(trap_type);
  damage /= 100;

  switch (trap_type) {
    case DOOR_TRAP_TNT:
      damage += 3;
      break; 
    case DOOR_TRAP_POISON:
      damage -= 1;
      break; 
    case DOOR_TRAP_SLEEP:
      damage += 1;
      break; 
    case DOOR_TRAP_ACID:
      damage += 1;
      break; 
    case DOOR_TRAP_DISEASE:
      damage += 3;
      break; 
    case DOOR_TRAP_FROST:
      damage += 3;
      break; 
    case DOOR_TRAP_SPIKE:
      damage -= 5;
      break; 
    case DOOR_TRAP_BOLT:
      damage += 1;
      break; 
    case DOOR_TRAP_BLADE:
      damage -= 3;
      break; 
    case DOOR_TRAP_DISK:
      damage += 3;
      break; 
    case DOOR_TRAP_HAMMER:
      damage -= 10;
      break; 
    case DOOR_TRAP_PEBBLE:
      damage -= 5;
      break; 
    case DOOR_TRAP_ENERGY:
      damage += 5;
      break; 
    case DOOR_TRAP_TELEPORT:
      damage += 5;
      break; 
    default:
      break;
  }
  damage = min(max(damage, 1), 50);
  return damage;
}


// just copied this from grenades
int TBeing::getArrowTrapDam(doorTrapT trap_type)
{
  int damage = 5 + getSkillLevel(SKILL_SET_TRAP_ARROW)/2;

  damage *= getArrowTrapLearn(trap_type);
  damage /= 100;

  switch (trap_type) {
    case DOOR_TRAP_TNT:
      damage += 3;
      break; 
    case DOOR_TRAP_SLEEP:
      damage += 1;
      break; 
    case DOOR_TRAP_ACID:
      damage += 1;
      break; 
    case DOOR_TRAP_DISEASE:
      damage += 3;
      break; 
    case DOOR_TRAP_FROST:
      damage += 3;
      break; 
    case DOOR_TRAP_SPIKE:
      damage -= 5;
      break; 
    case DOOR_TRAP_BOLT:
      damage += 1;
      break; 
    case DOOR_TRAP_BLADE:
      damage -= 3;
      break; 
    case DOOR_TRAP_DISK:
      damage += 3;
      break; 
    case DOOR_TRAP_HAMMER:
      damage -= 10;
      break; 
    case DOOR_TRAP_PEBBLE:
      damage -= 5;
      break; 
    case DOOR_TRAP_ENERGY:
      damage += 5;
      break; 
    case DOOR_TRAP_TELEPORT:
      damage += 5;
      break; 
    default:
      break;
  }
  damage = min(max(damage, 1), 50);
  return damage;
}


int TBeing::getDoorTrapLearn(doorTrapT)
{
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_DOOR)) <= 0)
    return 0;

  learn = min(learn, (int) MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getContainerTrapLearn(doorTrapT)
{
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_CONT)) <= 0)
    return 0;

  learn = min(learn, (int) MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getMineTrapLearn(doorTrapT)
{
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_MINE)) <= 0)
    return 0;

  learn = min(learn, (int) MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getGrenadeTrapLearn(doorTrapT)
{
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_GREN)) <= 0)
    return 0;

  learn = min(learn, (int) MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getArrowTrapLearn(doorTrapT)
{
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_ARROW)) <= 0)
    return 0;

  learn = min(learn, (int) MAX_SKILL_LEARNEDNESS);

  return learn;
}


int TObj::trapMe(TBeing *ch, const char *trap_type)
{
  act("$p is not trappable.", FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}
