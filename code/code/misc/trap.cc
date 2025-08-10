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
#include "obj_trap_component.h"
#include "obj_portal.h"
#include "obj_open_container.h"
#include "obj_arrow.h"
#include "trap.h"

extern const char* const GRENADE_EX_DESC = "__grenade_puller";
extern const char* const TRAP_EX_DESC = "__trap_setter";

// Constants for trap mechanics
namespace {
  constexpr int TRAP_DICE_SIZE = 8;
  constexpr int TRAP_GOOF_DAMAGE_DIVISOR = 3;

  // Base values for level 1 traps - these scale with trap level
  constexpr int BASE_SLEEP_DURATION_HOURS = 1;      // Level 1: 1 hour, scales up
  constexpr int BASE_DISEASE_DURATION_HOURS = 2;    // Level 1: 2 hours, scales up
  constexpr int BASE_POISON_DURATION_HOURS = 6;     // Level 1: 6 hours, scales up
  constexpr int BASE_POISON_STR_MODIFIER = -10;     // Level 1: -10 str, scales down

  // Disease duration multipliers
  constexpr int SEVERE_DISEASE_MULTIPLIER = 4;
  constexpr int MODERATE_DISEASE_MULTIPLIER = 2;

  // Level scaling functions for trap effects
  inline int getSleepDuration(int trap_level) {
    return BASE_SLEEP_DURATION_HOURS + (trap_level - 1) / 2;  // +0.5 hours per level
  }

  inline int getDiseaseDuration(int trap_level) {
    return BASE_DISEASE_DURATION_HOURS + (trap_level - 1);    // +1 hour per level
  }

  inline int getPoisonDuration(int trap_level) {
    return BASE_POISON_DURATION_HOURS + (trap_level - 1) * 2; // +2 hours per level
  }

  inline int getPoisonStrModifier(int trap_level) {
    return BASE_POISON_STR_MODIFIER - (trap_level - 1) * 2;   // -2 str per level (more negative)
  }

  // Room effect modifiers - different for status vs damage effects
  constexpr double STATUS_ROOM_MOD = 2.0/3.0;  // 66.7% strength for status effects (poison, sleep, disease, teleport)
  constexpr double DAMAGE_ROOM_MOD = 0.5;       // 50% strength for damage effects (fire, frost, energy, acid, TNT, etc.)

  // Context-based damage multipliers - same modifier for all damage types in each context
  constexpr double GOOF_MOD = 0.5;        // When traps backfire on the setter
  constexpr double ROOM_MOD = DAMAGE_ROOM_MOD;  // Legacy alias for damage room effects
  constexpr double OTHER_SIDE_MOD = 1.0/3.0;     // When effects reach through doors/walls

  constexpr double TWO_THIRDS_DAMAGE = 2.0/3.0;  // Available for future use

  // Common trap messages
  constexpr const char* STRANGE_NOISE_MSG = "You hear a strange noise...";
  constexpr const char* NOTHING_HAPPENS_CHAR_MSG = "...But nothing happens.";
  constexpr const char* NOTHING_HAPPENS_ROOM_MSG = "...But nothing happens.";
  constexpr const char* SHRAPNEL_CHAR_MSG = "You are hit by shrapnel!";
  constexpr const char* SHRAPNEL_ROOM_MSG = "$n is hit by shrapnel.";

  // Standardized trap source messages - WHERE the trap comes from
  constexpr const char* DOOR_TRAP_CHAR_MSG = "A mechanism in the %s triggers!";
  constexpr const char* DOOR_TRAP_ROOM_MSG = "A mechanism in the %s triggers!";
  constexpr const char* PORTAL_TRAP_CHAR_MSG = "A mechanism in the %s triggers!";
  constexpr const char* PORTAL_TRAP_ROOM_MSG = "A mechanism in the %s triggers!";
  constexpr const char* CONTAINER_TRAP_CHAR_MSG = "The %s springs a trap!";
  constexpr const char* CONTAINER_TRAP_ROOM_MSG = "The %s springs a trap!";
  constexpr const char* MINE_TRAP_CHAR_MSG = "The %s detonates!";
  constexpr const char* MINE_TRAP_ROOM_MSG = "The %s detonates!";
  constexpr const char* ARROW_TRAP_CHAR_MSG = "The %s releases its trapped payload!";
  constexpr const char* ARROW_TRAP_ROOM_MSG = "The %s releases its trapped payload!";
  constexpr const char* GRENADE_TRAP_CHAR_MSG = "The %s explodes!";
  constexpr const char* GRENADE_TRAP_ROOM_MSG = "The %s explodes!";

  // Standardized trap effect messages - used across ALL contexts
  constexpr const char* POISON_EFFECT_CHAR_MSG = "You are sprayed with contact poison!";
  constexpr const char* POISON_EFFECT_ROOM_MSG = "$n is sprayed with contact poison!";
  constexpr const char* SLEEP_EFFECT_CHAR_MSG = "You are surrounded by a noxious mist!";
  constexpr const char* SLEEP_EFFECT_ROOM_MSG = "$n is surrounded by a noxious mist!";
  constexpr const char* FIRE_EFFECT_CHAR_MSG = "You are burned by the flames!";
  constexpr const char* FIRE_EFFECT_ROOM_MSG = "$n is burned by the flames.";
  constexpr const char* FROST_EFFECT_CHAR_MSG = "You are chilled by the arctic blast!";
  constexpr const char* FROST_EFFECT_ROOM_MSG = "$n is chilled by the arctic blast.";
  constexpr const char* ACID_EFFECT_CHAR_MSG = "You are surrounded by the acid cloud!";
  constexpr const char* ACID_EFFECT_ROOM_MSG = "$n is surrounded by the acid cloud.";
  constexpr const char* ENERGY_EFFECT_CHAR_MSG = "You are hit by the energy bolts!";
  constexpr const char* ENERGY_EFFECT_ROOM_MSG = "$n is hit by the energy bolts.";
  constexpr const char* SPIKE_EFFECT_CHAR_MSG = "You are impaled by the spikes!";
  constexpr const char* SPIKE_EFFECT_ROOM_MSG = "$n is impaled by the spikes.";
  constexpr const char* BLADE_EFFECT_CHAR_MSG = "You are sliced by the razor blades!";
  constexpr const char* BLADE_EFFECT_ROOM_MSG = "$n is sliced by the razor blades.";
  constexpr const char* BLUNT_EFFECT_CHAR_MSG = "You are pummeled by the heavy weights!";
  constexpr const char* BLUNT_EFFECT_ROOM_MSG = "$n is pummeled by the heavy weights.";
  constexpr const char* TELEPORT_EFFECT_CHAR_MSG = "You find yourself sucked into the vortex!";
  constexpr const char* TELEPORT_EFFECT_ROOM_MSG = "$n flails wildly, but falls into the vortex.";
  constexpr const char* DISEASE_EFFECT_CHAR_MSG = "You are surrounded by a cloud of spores!";
  constexpr const char* DISEASE_EFFECT_ROOM_MSG = "$n is surrounded by a cloud of spores.";
  constexpr const char* TNT_EFFECT_CHAR_MSG = "You are hit by the explosive shrapnel!";
  constexpr const char* TNT_EFFECT_ROOM_MSG = "$n is hit by the explosive shrapnel.";
  // Consolidated trap goof messages
  constexpr const char* GOOF_CHAR_MSG = "Your hand slips and you fall victim to your own device!";
  constexpr const char* GOOF_ROOM_MSG = "$n's hand slips and $e falls victim to $s own device!";

  // Standardized trap creation messages
  constexpr const char* TRAP_START_CHAR_MSG = "You start working on your trap.";
  constexpr const char* TRAP_START_DOOR_ROOM_MSG = "$n starts setting a trap.";
  constexpr const char* TRAP_START_CONTAINER_ROOM_MSG = "$n starts fiddling with $p.";
  constexpr const char* TRAP_START_MINE_ROOM_MSG = "$n starts constructing a land-mine.";
  constexpr const char* TRAP_START_ARROW_ROOM_MSG = "$n starts trapping an arrow.";
  constexpr const char* TRAP_START_GRENADE_ROOM_MSG = "$n starts constructing a grenade.";

  // Standardized trap completion messages
  constexpr const char* TRAP_COMPLETE_DOOR_CHAR_MSG = "The trap has been successfully set!";
  constexpr const char* TRAP_COMPLETE_CONTAINER_CHAR_MSG = "The trap has been successfully set!";
  constexpr const char* TRAP_COMPLETE_MINE_CHAR_MSG = "You have successfully constructed a land mine!";
  constexpr const char* TRAP_COMPLETE_ARROW_CHAR_MSG = "You have successfully constructed an arrow trap!";
  constexpr const char* TRAP_COMPLETE_GRENADE_CHAR_MSG = "You have successfully constructed a grenade!";

  // Standardized component assembly messages
  constexpr const char* ATTACH_CHAR_MSG = "You attach the %s to the %s.";
  constexpr const char* ATTACH_ROOM_MSG = "$n attaches the %s to the %s.";
  constexpr const char* SCREW_CHAR_MSG = "You screw the %s into the %s.";
  constexpr const char* SCREW_ROOM_MSG = "$n screws the %s into the %s.";
  constexpr const char* PLACE_CHAR_MSG = "You place the %s into the %s.";
  constexpr const char* PLACE_ROOM_MSG = "$n places the %s into the %s.";
  constexpr const char* CONNECT_CHAR_MSG = "You connect the %s to the %s.";
  constexpr const char* CONNECT_ROOM_MSG = "$n connects the %s to the %s.";
  constexpr const char* POUR_CHAR_MSG = "You pour the %s into the %s.";
  constexpr const char* POUR_ROOM_MSG = "$n pours the %s into the %s.";
  constexpr const char* ARM_CHAR_MSG = "You arm the %s mechanism.";
  constexpr const char* ARM_ROOM_MSG = "$n arms the %s mechanism.";
  constexpr const char* CONCEAL_CHAR_MSG = "You conceal the %s inside the %s.\n\r";
  constexpr const char* CONCEAL_ROOM_MSG = "$n conceals the %s inside the %s.";

  // Component name mapping for clean messages
  const char* getComponentName(int vnum) {
    switch (vnum) {
      case Obj::ST_FLINT: return "flint";
      case Obj::ST_SULPHUR: return "sulphur";
      case Obj::ST_BAG: return "bellows";
      case Obj::ST_BELLOWS: return "bellows";
      case Obj::ST_HYDROGEN: return "compressed gas";
      case Obj::ST_NEEDLE: return "needle";
      case Obj::ST_SPRING: return "spring";
      case Obj::ST_POISON: return "poison";
      case Obj::ST_CANISTER: return "canister";
      case Obj::ST_CON_POISON: return "concentrated poison";
      case Obj::ST_TRIPWIRE: return "tripwire";
      case Obj::ST_PENTAGRAM: return "pentagram";
      case Obj::ST_ATHANOR: return "athanor";
      case Obj::ST_CRYSTALINE: return "crystal";
      case Obj::ST_BOLTS: return "bolts";
      case Obj::ST_TUBING: return "tubing";
      case Obj::ST_PEBBLES: return "pebbles";
      case Obj::ST_RAZOR_DISK: return "razor discs";
      case Obj::ST_FUNGUS: return "spores";
      case Obj::ST_ACID_VIAL: return "acid vial";
      case Obj::ST_FROST: return "frost compound";
      case Obj::ST_CONCRETE: return "concrete weight";
      case Obj::ST_RAZOR_BLADE: return "razor blade";
      case Obj::ST_NOZZLE: return "nozzle";
      case Obj::ST_HOSE: return "hose";
      case Obj::ST_GAS: return "gas";
      case Obj::ST_CGAS: return "compressed gas";
      case Obj::ST_SPIKE: return "spike";
      case Obj::ST_WEDGE: return "wedge";
      case Obj::ST_BLINK: return "blink powder";
      default: return "component";
    }
  }
}

namespace {
  // Mapping tables for trap type conversion
  constexpr doorTrapT FILE_TO_TRAP_MAP[] = {
    DOOR_TRAP_NONE,     // 0
    DOOR_TRAP_POISON,   // 1
    DOOR_TRAP_SPIKE,    // 2
    DOOR_TRAP_SLEEP,    // 3
    DOOR_TRAP_TNT,      // 4
    DOOR_TRAP_BLADE,    // 5
    DOOR_TRAP_FIRE,     // 6
    DOOR_TRAP_ACID,     // 7
    DOOR_TRAP_DISEASE,  // 8
    DOOR_TRAP_HAMMER,   // 9
    DOOR_TRAP_FROST,    // 10
    DOOR_TRAP_TELEPORT, // 11
    DOOR_TRAP_ENERGY,   // 12
    DOOR_TRAP_BOLT,     // 13
    DOOR_TRAP_DISK,     // 14
    DOOR_TRAP_PEBBLE    // 15
  };

  constexpr int TRAP_TO_FILE_MAP[] = {
    0,  // DOOR_TRAP_NONE
    1,  // DOOR_TRAP_POISON
    2,  // DOOR_TRAP_SPIKE
    3,  // DOOR_TRAP_SLEEP
    4,  // DOOR_TRAP_TNT
    5,  // DOOR_TRAP_BLADE
    6,  // DOOR_TRAP_FIRE
    7,  // DOOR_TRAP_ACID
    8,  // DOOR_TRAP_DISEASE
    9,  // DOOR_TRAP_HAMMER
    10, // DOOR_TRAP_FROST
    11, // DOOR_TRAP_TELEPORT
    12, // DOOR_TRAP_ENERGY
    13, // DOOR_TRAP_BOLT
    14, // DOOR_TRAP_DISK
    15  // DOOR_TRAP_PEBBLE
  };

  constexpr size_t MAX_FILE_TRAP_VALUE = sizeof(FILE_TO_TRAP_MAP) / sizeof(FILE_TO_TRAP_MAP[0]) - 1;
}

doorTrapT mapFileToDoorTrap(int dt) {
  if (dt < 0 || dt > static_cast<int>(MAX_FILE_TRAP_VALUE)) {
    vlogf(LOG_BUG, format("Bad value (%d) in mapFileToDoorTrap") % dt);
    return MAX_TRAP_TYPES;
  }
  return FILE_TO_TRAP_MAP[dt];
}

int mapDoorTrapToFile(doorTrapT dt) {
  if (dt < 0 || dt >= MAX_TRAP_TYPES) {
    vlogf(LOG_BUG, format("Bad value (%d) in mapDoorTrapToFile") % dt);
    return -1;
  }
  return TRAP_TO_FILE_MAP[dt];
}

// Standardized trap assembly message function
void sendTrapMessage(TBeing* ch, const char* trap_type, trap_targ_t targ, int step) {
  // Get the components for this trap type and target
  int item1 = 0, item2 = 0, item3 = 0;

  if (is_abbrev(trap_type, "fire")) {
    item1 = Obj::ST_FLINT; item2 = Obj::ST_SULPHUR; item3 = Obj::ST_BAG;
  } else if (is_abbrev(trap_type, "explosive")) {
    item1 = Obj::ST_FLINT; item2 = Obj::ST_SULPHUR; item3 = Obj::ST_HYDROGEN;
  } else if (is_abbrev(trap_type, "poison")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_NEEDLE; item2 = Obj::ST_SPRING; item3 = Obj::ST_POISON;
    } else {
      item1 = Obj::ST_CANISTER; item2 = Obj::ST_SPRING; item3 = Obj::ST_CON_POISON;
    }
  } else if (is_abbrev(trap_type, "sleep")) {
    item1 = Obj::ST_NOZZLE; item2 = Obj::ST_GAS; item3 = Obj::ST_HOSE;
  } else if (is_abbrev(trap_type, "acid")) {
    if (targ == TRAP_TARG_DOOR || targ == TRAP_TARG_CONT) {
      item1 = Obj::ST_NOZZLE; item2 = Obj::ST_ACID_VIAL; item3 = Obj::ST_BELLOWS;
    } else {
      item1 = Obj::ST_CANISTER; item2 = Obj::ST_SPRING; item3 = Obj::ST_ACID_VIAL;
    }
  } else if (is_abbrev(trap_type, "spike")) {
    item1 = Obj::ST_SPIKE; item2 = Obj::ST_SPRING; item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(trap_type, "blade")) {
    item1 = Obj::ST_RAZOR_BLADE; item2 = Obj::ST_SPRING; item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(trap_type, "hammer")) {
    item1 = Obj::ST_CONCRETE; item2 = Obj::ST_WEDGE; item3 = Obj::ST_TRIPWIRE;
  } else if (is_abbrev(trap_type, "frost")) {
    item1 = Obj::ST_NOZZLE; item2 = Obj::ST_HOSE; item3 = Obj::ST_FROST;
  } else {
    // Default fallback
    item1 = Obj::ST_SPRING; item2 = Obj::ST_TRIPWIRE; item3 = Obj::ST_CANISTER;
  }

  // Send standardized messages based on step
  switch (step) {
    case 1:
      ch->sendTo(format(ATTACH_CHAR_MSG) % getComponentName(item1) % getComponentName(item2));
      act(format(ATTACH_ROOM_MSG) % getComponentName(item1) % getComponentName(item2), TRUE, ch, NULL, NULL, TO_ROOM);
      break;
    case 2:
      ch->sendTo(format(CONNECT_CHAR_MSG) % getComponentName(item2) % getComponentName(item3));
      act(format(CONNECT_ROOM_MSG) % getComponentName(item2) % getComponentName(item3), TRUE, ch, NULL, NULL, TO_ROOM);
      break;
    case 3:
      ch->sendTo(format(ARM_CHAR_MSG) % "trigger");
      act(format(ARM_ROOM_MSG) % "trigger", TRUE, ch, NULL, NULL, TO_ROOM);
      break;
    case 4:
      if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
        ch->sendTo(format(CONCEAL_CHAR_MSG) % "mechanism" % "casing");
        act(format(CONCEAL_ROOM_MSG) % "mechanism" % "casing", TRUE, ch, NULL, NULL, TO_ROOM);
      } else {
        ch->sendTo(format(PLACE_CHAR_MSG) % "assembly" % "position");
        act(format(PLACE_ROOM_MSG) % "assembly" % "position", TRUE, ch, NULL, NULL, TO_ROOM);
      }
      break;
    default:
      ch->sendTo("You continue working on the trap.");
      act("$n continues working on the trap.", TRUE, ch, NULL, NULL, TO_ROOM);
      break;
  }
}

const sstring trap_types[] = {"None", "Poison", "Spike", "Sleep", "Explosive",
  "Blade", "Fire", "Acid", "Spore", "Hammer", "Frost", "Teleport", "Power",
  "Bolt", "Disc", "Pebble", "\n"};

const char* user_trap_types[] = {"exit", "container", "mine", "grenade",
  "arrow", "\n"};

namespace {
  // Helper function to parse trap type from string
  doorTrapT parseTrapType(const char* trap_type, trap_targ_t target) {
    if (is_abbrev(trap_type, "fire")) return DOOR_TRAP_FIRE;
    if (is_abbrev(trap_type, "explosive")) return DOOR_TRAP_TNT;
    if (is_abbrev(trap_type, "poison")) return DOOR_TRAP_POISON;
    if (is_abbrev(trap_type, "sleep")) return DOOR_TRAP_SLEEP;
    if (is_abbrev(trap_type, "acid")) return DOOR_TRAP_ACID;
    if (is_abbrev(trap_type, "spore")) return DOOR_TRAP_DISEASE;
    if (is_abbrev(trap_type, "frost")) return DOOR_TRAP_FROST;
    if (is_abbrev(trap_type, "teleport")) return DOOR_TRAP_TELEPORT;
    if (is_abbrev(trap_type, "power")) return DOOR_TRAP_ENERGY;

    // Target-specific trap types
    if (target == TRAP_TARG_DOOR || target == TRAP_TARG_CONT || target == TRAP_TARG_ARROW) {
      if (is_abbrev(trap_type, "spike")) return DOOR_TRAP_SPIKE;
      if (is_abbrev(trap_type, "blade")) return DOOR_TRAP_BLADE;
      if (is_abbrev(trap_type, "pebble")) return DOOR_TRAP_PEBBLE;
    }

    if (target == TRAP_TARG_DOOR) {
      if (is_abbrev(trap_type, "hammer")) return DOOR_TRAP_HAMMER;
    }

    if (target == TRAP_TARG_MINE || target == TRAP_TARG_GRENADE) {
      if (is_abbrev(trap_type, "bolt")) return DOOR_TRAP_BOLT;
      if (is_abbrev(trap_type, "disk")) return DOOR_TRAP_DISK;
      if (is_abbrev(trap_type, "pebble")) return DOOR_TRAP_PEBBLE;
    }

    return MAX_TRAP_TYPES; // Invalid trap type
  }
}

int TBeing::springTrap(TTrap* obj) {
  const int level_adjustment = obj->getTrapLevel() - GetMaxLevel();
  const int dex_adjustment = getDexReaction() * 5;
  const int fire_percentage = 95 + level_adjustment - dex_adjustment;
  const int roll = ::number(1, 100);

  return (roll < fire_percentage);
}

int TBeing::doSetTraps(const char* arg) {
  roomDirData* exitp;
  char buf[256], task_arg[128];
  char sstring[512], trap_type[40], direct[20];
  int field, dir;
  dirTypeT door;
  doorTrapT type;
  int rc;
  TObj* obj;

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

      if (!IS_SET(exitp->condition, EXIT_CLOSED)) {
        sendTo(format("You need to close the %s first.\n\r") %
               exitp->getName().uncap());
        return FALSE;
      }
      if (IS_SET(exitp->condition, EXIT_TRAPPED)) {
        sendTo(format("When you try to trap the %s, you set off the trap that "
                      "is already there!\n\r") %
               exitp->getName().uncap());
        rc = triggerDoorTrap(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }

      type = parseTrapType(trap_type, TRAP_TARG_DOOR);
      if (type == MAX_TRAP_TYPES) {
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
      sprintf(buf, "$n starts fiddling with the %s.",
        exitp->getName().uncap().c_str());
      act(buf, TRUE, this, NULL, NULL, TO_ROOM);
      sprintf(task_arg, "%s %s", direct, trap_type);
      start_task(this, NULL, NULL, TASK_TRAP_DOOR, task_arg, 3, inRoom(), type,
        door, 5);
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

      type = parseTrapType(trap_type, TRAP_TARG_MINE);
      if (type == MAX_TRAP_TYPES) {
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

      sendTo(TRAP_START_CHAR_MSG);
      act(TRAP_START_MINE_ROOM_MSG, TRUE, this, 0, 0, TO_ROOM);
      start_task(this, NULL, NULL, TASK_TRAP_MINE, trap_type, 3, inRoom(), type,
        0, 5);
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

      type = parseTrapType(trap_type, TRAP_TARG_ARROW);
      if (type == MAX_TRAP_TYPES) {
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

      sendTo(TRAP_START_CHAR_MSG);
      act(TRAP_START_ARROW_ROOM_MSG, TRUE, this, 0, 0, TO_ROOM);
      start_task(this, obj, NULL, TASK_TRAP_ARROW, trap_type, 3, inRoom(), type,
        0, 5);
      break;
    case TRAP_TARG_GRENADE:
      if (!doesKnowSkill(SKILL_SET_TRAP_GREN)) {
        sendTo("You know nothing about making grenades.\n\r");
        return FALSE;
      }

      sscanf(sstring, "%s", trap_type);

      type = parseTrapType(trap_type, TRAP_TARG_GRENADE);
      if (type == MAX_TRAP_TYPES) {
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

      sendTo(TRAP_START_CHAR_MSG);
      act(TRAP_START_GRENADE_ROOM_MSG, TRUE, this, 0, 0, TO_ROOM);
      start_task(this, NULL, NULL, TASK_TRAP_GRENADE, trap_type, 3, inRoom(),
        type, 0, 5);
      return FALSE;
    default:
      sendTo(
        "Syntax: trap <\"exit\" | \"container\" | \"mine\" | \"grenade\"> "
        "...\n\r");
      break;
  }
  return FALSE;
}

// triggered when portal opened or entered
// returns DELETE_THIS, DELETE_ITEM
int TBeing::triggerPortalTrap(TPortal* o) {
  int rc;
  int amnt;
  TThing* t;

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  switch (o->getPortalTrapType()) {
    case DOOR_TRAP_POISON:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      trapPoison(o->getPortalTrapDam());
      break;
    case DOOR_TRAP_SLEEP:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = trapSleep(o->getPortalTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FIRE:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = trapTeleport(o->getPortalTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return rc;
    case DOOR_TRAP_SPIKE:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_DISEASE, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_HAMMER:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_BLADE:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      amnt = o->getPortalTrapDam();

      // fry people in room

      for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
        t = *(it++);
        TBeing* tbt = dynamic_cast<TBeing*>(t);
        if (tbt && this != tbt && !tbt->isImmortal()) {
          act(TNT_EFFECT_CHAR_MSG, FALSE, tbt, 0, 0, TO_CHAR);
          act(TNT_EFFECT_ROOM_MSG, FALSE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt * ROOM_MOD, o);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }

      act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_TNT, amnt, o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS | DELETE_ITEM;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS | DELETE_ITEM;

      return DELETE_ITEM;
    case DOOR_TRAP_FROST:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getPortalTrapDam(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ACID:
      sendTo(format(PORTAL_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(PORTAL_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

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
int TBeing::triggerContTrap(TOpenContainer* obj) {
  int rc = 0;
  TThing* t;
  int amnt;

  act(STRANGE_NOISE_MSG, TRUE, this, 0, 0, TO_ROOM);
  act(STRANGE_NOISE_MSG, TRUE, this, 0, 0, TO_CHAR);
  obj->remContainerFlag(CONT_TRAPPED);
  obj->remContainerFlag(CONT_CLOSED);
  obj->addContainerFlag(CONT_EMPTYTRAP);

  if (!percentChance(1)){
    act(NOTHING_HAPPENS_CHAR_MSG, TRUE, this, 0, 0, TO_CHAR);
    act(NOTHING_HAPPENS_ROOM_MSG, TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  switch (obj->getContainerTrapType()) {
    case DOOR_TRAP_FIRE:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      // bag explodes, contents go boom
      for (StuffIter it = obj->stuff.begin(); it != obj->stuff.end();) {
        t = *(it++);
        delete t;
        t = NULL;
      }
      rc = objDamage(DAMAGE_TRAP_FIRE, obj->getContainerTrapDam(), obj);

      ADD_DELETE(rc, DELETE_ITEM);
      return rc;
    case DOOR_TRAP_TNT:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      amnt = obj->getContainerTrapDam();

      // bag explodes, contents go boom
      for (StuffIter it = obj->stuff.begin(); it != obj->stuff.end();) {
        t = *(it++);
        delete t;
        t = NULL;
      }
      // fry people in room
      for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
        t = *(it++);
        TBeing* tbt = dynamic_cast<TBeing*>(t);
        if (tbt && this != tbt) {
          act(TNT_EFFECT_CHAR_MSG, FALSE, tbt, 0, 0, TO_CHAR);
          act(TNT_EFFECT_ROOM_MSG, FALSE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt * ROOM_MOD, obj);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }
      act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_TNT, amnt, obj);

      ADD_DELETE(rc, DELETE_ITEM);
      return rc;
    case DOOR_TRAP_POISON:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      trapPoison(obj->getContainerTrapDam());
      break;
    case DOOR_TRAP_SLEEP:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = trapSleep(obj->getContainerTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_SPIKE:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_DISEASE:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      trapDisease(obj->getContainerTrapDam());
      break;
    case DOOR_TRAP_TELEPORT:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = trapTeleport(obj->getContainerTrapDam());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return rc;
    case DOOR_TRAP_PEBBLE:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_BLADE:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_FROST:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_ENERGY:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, obj->getContainerTrapDam(), obj);
      return rc;
    case DOOR_TRAP_ACID:
      sendTo(format(CONTAINER_TRAP_CHAR_MSG) % fname(obj->getName()));
      act(format(CONTAINER_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);
      act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

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
int TBeing::triggerArrowTrap(TArrow* obj) {
  int rc = 0;
  TThing* t;
  int amnt;

  act(STRANGE_NOISE_MSG, TRUE, this, 0, 0, TO_ROOM);
  act(STRANGE_NOISE_MSG, TRUE, this, 0, 0, TO_CHAR);

  if (!percentChance(1)) {
    act(NOTHING_HAPPENS_CHAR_MSG, TRUE, this, 0, 0, TO_CHAR);
    act(NOTHING_HAPPENS_ROOM_MSG, TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  // Single arrow source message for ALL arrow trap cases
  sendTo(format(ARROW_TRAP_CHAR_MSG) % fname(obj->getName()));
  act(format(ARROW_TRAP_ROOM_MSG) % fname(obj->getName()), FALSE, this, 0, 0, TO_ROOM);

  switch (obj->getTrapDamType()) {
    case DOOR_TRAP_SLEEP:
      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = trapSleep(obj->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FIRE:
      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = trapTeleport(obj->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return rc;
    case DOOR_TRAP_SPIKE:
      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_DISEASE, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_BLADE:
      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      amnt = obj->getTrapDamAmount();

      // fry people in room
      for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
        t = *(it++);
        TBeing* tbt = dynamic_cast<TBeing*>(t);
        if (tbt && this != tbt && !tbt->isImmortal()) {
          act(TNT_EFFECT_CHAR_MSG, FALSE, tbt, 0, 0, TO_CHAR);
          act(TNT_EFFECT_ROOM_MSG, FALSE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt * ROOM_MOD, obj);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }

      act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_TNT, amnt, obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS | DELETE_ITEM;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS | DELETE_ITEM;

      return DELETE_ITEM;
    case DOOR_TRAP_FROST:
      act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, obj->getTrapDamAmount(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ACID:
      act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

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
int TBeing::triggerDoorTrap(dirTypeT door) {
  roomDirData *exitp, *back = NULL;
  TRoom* rp;
  int dam;
  int rc;

  exitp = exitDir(door);
  dam = dice(exitp->trap_dam, 8);

  // door traps can be triggered by means other than opening
  // eg trying to set another trap
  //  rawOpenDoor(door);

  REMOVE_BIT(exitp->condition, EXIT_TRAPPED);
  if ((rp = real_roomp(exitp->to_room)) &&
      (back = rp->dir_option[rev_dir(door)])) {
    REMOVE_BIT(back->condition, EXIT_TRAPPED);
  }

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  switch (exitp->trap_info) {
    case DOOR_TRAP_POISON:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      trapPoison(dam);
      break;
    case DOOR_TRAP_SPIKE:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_PIERCE, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_SLEEP:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = trapSleep(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_TNT:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      exitp->destroyDoor(door, in_room);

      // Room-wide TNT effects
      for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
        TThing* t = *(it++);
        TBeing* tbt = dynamic_cast<TBeing*>(t);
        if (tbt && this != tbt) {
          act(TNT_EFFECT_CHAR_MSG, FALSE, tbt, 0, 0, TO_CHAR);
          act(TNT_EFFECT_ROOM_MSG, FALSE, tbt, 0, 0, TO_ROOM);
          rc = tbt->objDamage(DAMAGE_TRAP_TNT, dam * ROOM_MOD, NULL);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }
      }

      act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_TNT, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FIRE:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_FIRE, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_ACID:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_ACID, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_DISEASE:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      trapDisease(dam);
      break;
    case DOOR_TRAP_TELEPORT:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

      rc = trapTeleport(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return rc;
    case DOOR_TRAP_HAMMER:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_BLUNT, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_BLADE:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_SLASH, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_ENERGY:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_ENERGY, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    case DOOR_TRAP_FROST:
      sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
      act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), FALSE, this, 0, 0, TO_ROOM);
      act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
      rc = objDamage(DAMAGE_TRAP_FROST, dam, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      break;
    default:
      break;
  }
  return FALSE;
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

// returns DELETE_THIS or false
int TBeing::triggerTrap(TTrap* o) {
  TThing* v;
  TBeing* tbt;
  int rc;

  o->setTrapCharges(o->getTrapCharges() - 1);

  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(POISON_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(POISON_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            tbt->trapPoison(o->getTrapDamAmount() * STATUS_ROOM_MOD);
          }
        }
      }

      act(POISON_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(POISON_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      trapPoison(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_SLEEP:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(SLEEP_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(SLEEP_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->trapSleep(o->getTrapDamAmount() * STATUS_ROOM_MOD);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      rc = trapSleep(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FIRE:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(FIRE_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(FIRE_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_FIRE, o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(TELEPORT_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(TELEPORT_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->trapTeleport(o->getTrapDamAmount() * STATUS_ROOM_MOD);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = trapTeleport(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(DISEASE_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(DISEASE_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            tbt->trapDisease(o->getTrapDamAmount() * STATUS_ROOM_MOD);
          }
        }
      }

      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      trapDisease(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_BOLT:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(SPIKE_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(SPIKE_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_PIERCE,
              o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_PEBBLE:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(BLUNT_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(BLUNT_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_BLUNT,
              o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_DISK:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(BLADE_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(BLADE_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_SLASH,
              o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(TNT_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(TNT_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc =
              tbt->objDamage(DAMAGE_TRAP_TNT, o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(TNT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_TNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FROST:
      act("An icy cloud pours out of $p.", FALSE, this, o, 0, TO_CHAR);
      act("An icy cloud pours out of $p.", FALSE, this, o, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(FROST_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(FROST_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_FROST,
              o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(FROST_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(ENERGY_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(ENERGY_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_ENERGY,
              o->getTrapDamAmount() * ROOM_MOD, o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_ACID:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), FALSE, this, 0, 0, TO_ROOM);

      if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
        for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
          v = *(it++);
          tbt = dynamic_cast<TBeing*>(v);
          if (tbt && tbt->desc && tbt != this) {
            act(ACID_EFFECT_CHAR_MSG, FALSE, tbt, o, 0, TO_CHAR);
            act(ACID_EFFECT_ROOM_MSG, FALSE, tbt, o, 0, TO_ROOM);
            rc = tbt->objDamage(DAMAGE_TRAP_ACID, o->getTrapDamAmount() * ROOM_MOD,
              o);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tbt;
              tbt = NULL;
            }
          }
        }
      }

      act(ACID_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ACID, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = acidEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    default:
      vlogf(LOG_BUG, format("Unknown trap type %d in triggerTrap (%s:%d)") %
                       o->getTrapDamType() % o->getName() % o->objVnum());
      return TRUE;
  }

  return TRUE;
}

// returns DELETE_THIs or FALSE
int TBeing::trapDoorTntDamage(int amnt, dirTypeT door) {
  TThing* t;
  int rc;

  sendToRoom("You hear a loud boom.\n\r", in_room);
  sendToRoom("The door is ripped apart by some sort of explosive.\n\r",
    in_room);

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    TBeing* tbt = dynamic_cast<TBeing*>(t);
    if (tbt && this != tbt) {
      rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt * ROOM_MOD, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom("You hear a loud boom.\n\r", exitDir(door)->to_room);
    sendToRoom("The door is ripped apart by some sort of explosive.\n\r",
      exitDir(door)->to_room);

    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && this != tbt) {
        rc = tbt->objDamage(DAMAGE_TRAP_TNT, amnt * OTHER_SIDE_MOD, NULL);
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
int TBeing::trapDoorPierceDamage(int amnt, dirTypeT door) {
  char buf[256];

  sprintf(buf,
    "You hear a loud metallic sound as a sharpened spike leaps out of the "
    "%s!\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);
  }

  act("$n is skewered by the spike.", TRUE, this, 0, 0, TO_ROOM);
  act("You are skewered by the spike.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_PIERCE, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorHammerDamage(int amnt, dirTypeT door) {
  char buf[256];

  sprintf(buf,
    "You hear a grinding noise as giant weights fall from above the %s!\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);
  }

  act("$n is hit by a falling weight.", TRUE, this, 0, 0, TO_ROOM);
  act("You are crushed by a falling weight.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_BLUNT, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorSlashDamage(int amnt, dirTypeT door) {
  char buf[256];

  sprintf(buf,
    "You hear a grinding noise as swinging blades slice out of %s!\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);
  }

  act("$n is cut by the blades.", TRUE, this, 0, 0, TO_ROOM);
  act("You are cut by the blades.", TRUE, this, 0, 0, TO_CHAR);
  return objDamage(DAMAGE_TRAP_SLASH, amnt, NULL);
}

// returns DELETE_THIS or FALSE
int TBeing::trapDoorFrostDamage(int amnt, dirTypeT door) {
  TThing* t;
  int rc;
  char buf[256];

  sprintf(buf,
    "You hear a high pitched whine as a frosty blast rushes out of the %s!\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    TBeing* tbt = dynamic_cast<TBeing*>(t);
    if (tbt && this != tbt && !tbt->isImmortal()) {
      act("$n is chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_ROOM);
      act("You are chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_CHAR);
      rc = tbt->objDamage(DAMAGE_TRAP_FROST, amnt * ROOM_MOD, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);

    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && this != tbt) {
        act("$n is chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_ROOM);
        act("You are chilled by the arctic blast.", TRUE, tbt, 0, 0, TO_CHAR);
        rc = tbt->objDamage(DAMAGE_TRAP_FROST, amnt * OTHER_SIDE_MOD, NULL);
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
int TBeing::trapDoorEnergyDamage(int amnt, dirTypeT door) {
  TThing* t;
  int rc;
  char buf[256];

  sprintf(buf,
    "You hear a powerful humming as bolts of plasma stream out of the %s!\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    TBeing* tbt = dynamic_cast<TBeing*>(t);
    if (tbt && this != tbt && !tbt->isImmortal()) {
      act("$n is hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_ROOM);
      act("You are hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_CHAR);
      rc = tbt->objDamage(DAMAGE_TRAP_ENERGY, amnt * ROOM_MOD, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);

    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && this != tbt) {
        act("$n is hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_ROOM);
        act("You are hit by the plasma bolts.", TRUE, tbt, 0, 0, TO_CHAR);
        rc = tbt->objDamage(DAMAGE_TRAP_ENERGY, amnt * OTHER_SIDE_MOD, NULL);
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
int TBeing::trapDoorFireDamage(int amnt, dirTypeT door) {
  int rc;
  char buf[256];

  sprintf(buf,
    "You feel an intense amount of heat as flames shoot from a %s!\n\r",
    fname(exitDir(door)->keyword).c_str());
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
int TBeing::trapDoorAcidDamage(int amnt, dirTypeT door) {
  TThing* t;
  int rc;
  char buf[256];

  sprintf(buf, "A strange liquid squirts everywhere as the %s is opened.\n\r",
    fname(exitDir(door)->keyword).c_str());
  sendToRoom(buf, in_room);

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    TBeing* tbt = dynamic_cast<TBeing*>(t);
    if (tbt && this != tbt) {
      rc = tbt->objDamage(DAMAGE_TRAP_ACID, amnt * ROOM_MOD, NULL);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tbt;
        tbt = NULL;
      }
    }
  }

  // blow other side too
  TRoom* rp;
  if ((rp = real_roomp(exitDir(door)->to_room))) {
    sendToRoom(buf, exitDir(door)->to_room);

    for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end();) {
      t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && this != tbt) {
        rc = tbt->objDamage(DAMAGE_TRAP_ACID, amnt * OTHER_SIDE_MOD, NULL);
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
int TBeing::trapTeleport(int amt) {
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

int TBeing::trapSleep(int amt) {
  int rc = FALSE;

  if (isImmune(IMMUNE_SLEEP, WEAR_BODY)) {
    sendTo("You yawn, but are otherwise immune to the sleep trap.\n\r");
    return FALSE;
  }

  if (!isLucky(levelLuckModifier(GetMaxLevel()))) {
    // Estimate trap level from damage amount: getTrapDamAmount() = dice(level, 8)
    // Average damage per level is 4.5, so amt/4 gives rough level estimate
    int estimated_level = max(1, amt / 4);
    int duration_hours = getSleepDuration(estimated_level);
    rc = rawSleep(0, (duration_hours * Pulse::UPDATES_PER_MUDHOUR), 1, SAVE_NO);
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

  // Estimate trap level from damage amount: getTrapDamAmount() = dice(level, 8)
  // Average damage per level is 4.5, so amt/4 gives rough level estimate
  int estimated_level = max(1, amt / 4);
  aff.duration = getDiseaseDuration(estimated_level) * Pulse::UPDATES_PER_MUDHOUR;

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
    aff.duration *= SEVERE_DISEASE_MULTIPLIER;
    act("You feel VERY sick.", FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
  } else {
    aff.duration *= MODERATE_DISEASE_MULTIPLIER;
    act("$n doesn't look so hot.", TRUE, this, 0, 0, TO_ROOM);
    act("You feel sick.", TRUE, this, 0, 0, TO_CHAR);
  }
  affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_NO);
  disease_start(this, &aff);
}

void TBeing::trapPoison(int amt) {
  affectedData af, af2;

  af.type = SPELL_POISON;

  // Estimate trap level from damage amount: getTrapDamAmount() = dice(level, 8)
  // Average damage per level is 4.5, so amt/4 gives rough level estimate
  int estimated_level = max(1, amt / 4);
  af.duration = getPoisonDuration(estimated_level) * Pulse::UPDATES_PER_MUDHOUR;
  af.modifier = getPoisonStrModifier(estimated_level);
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
  int trapdamage;
  int rc;
  TObj* obj;
  char buf1[256], buf2[256];

  if (goof_type == TRAP_TARG_DOOR) {
    half_chop(task->orig_arg, buf1, buf2);

    trapdamage = getDoorTrapDam(trap_type);
    trapdamage = dice(trapdamage, TRAP_DICE_SIZE) / TRAP_GOOF_DAMAGE_DIVISOR;

    hasTrapComps(buf2, TRAP_TARG_DOOR, -1);  // delete comps

    // Single goof message for ALL door trap cases
    act(GOOF_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
    act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_SPIKE:
        act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_BLADE:
        act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_HAMMER:
        act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
      case DOOR_TRAP_FIRE:
        act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage * GOOF_MOD, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo(
          "You slip up, and the trap you were setting goes off in your "
          "face.\n\r");
        act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        break;
    }
    // door traps
  } else if (goof_type == TRAP_TARG_CONT) {
    obj = task->obj;

    trapdamage = getContainerTrapDam(trap_type);
    trapdamage = dice(trapdamage, TRAP_DICE_SIZE) / TRAP_GOOF_DAMAGE_DIVISOR;

    hasTrapComps(task->orig_arg, TRAP_TARG_CONT, -1);  // delete comps

    // Single goof message for ALL container trap cases
    act(GOOF_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
    act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_SPIKE:
        act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_BLADE:
        act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
      case DOOR_TRAP_FIRE:
        act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage * GOOF_MOD, obj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo(
          "You slip up, and the trap you were setting goes off in your "
          "face.\n\r");
        act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        break;
    }
    // cont traps
  } else if (goof_type == TRAP_TARG_MINE) {
    trapdamage = getMineTrapDam(trap_type);
    trapdamage = dice(trapdamage, TRAP_DICE_SIZE) / TRAP_GOOF_DAMAGE_DIVISOR;

    hasTrapComps(task->orig_arg, TRAP_TARG_MINE, -1);  // delete comps

    // Single goof message for ALL mine trap cases
    act(GOOF_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
    act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_BOLT:
        act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_DISK:
        act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
        act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FIRE:
        act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FIRE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo(
          "You slip up, and the trap you were setting goes off in your "
          "face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
  } else if (goof_type == TRAP_TARG_GRENADE) {
    trapdamage = getGrenadeTrapDam(trap_type);
    trapdamage = dice(trapdamage, TRAP_DICE_SIZE) / TRAP_GOOF_DAMAGE_DIVISOR;

    hasTrapComps(task->orig_arg, TRAP_TARG_GRENADE, -1);  // delete comps

    // Single goof message for ALL grenade trap cases
    act(GOOF_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
    act(GOOF_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

    switch (trap_type) {
      case DOOR_TRAP_POISON:
        act(POISON_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(POISON_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapPoison(trapdamage);
        break;
      case DOOR_TRAP_BOLT:
        act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_PIERCE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_DISK:
        act(BLADE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLADE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_SLASH, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_PEBBLE:
        act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_BLUNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_SLEEP:
        act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapSleep(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        break;
      case DOOR_TRAP_DISEASE:
        act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        trapDisease(trapdamage);
        break;
      case DOOR_TRAP_TELEPORT:
        act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = trapTeleport(trapdamage);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_TNT:
        act(TNT_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(TNT_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_TNT, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FIRE:
        act(FIRE_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FIRE_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FIRE, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ACID:
        act(ACID_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ACID_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_ACID, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_ENERGY:
        act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);
        rc = objDamage(DAMAGE_TRAP_ENERGY, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      case DOOR_TRAP_FROST:
        act(FROST_EFFECT_CHAR_MSG, FALSE, this, 0, 0, TO_CHAR);
        act(FROST_EFFECT_ROOM_MSG, FALSE, this, 0, 0, TO_ROOM);

        rc = objDamage(DAMAGE_TRAP_FROST, trapdamage, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        break;
      default:
        sendTo("Ooops...\n\r");
        sendTo(
          "You slip up, and the trap you were setting goes off in your "
          "face.\n\r");
        act("$n's trap explodes in $s face.", FALSE, this, 0, 0, TO_ROOM);
        break;
    }
  }
  return FALSE;
}

bool TBeing::hasTrapComps(const char* type, trap_targ_t targ, int amt,
  int* price) {
  int item1 = 0, item2 = 0, item3 = 0, item4 = 0;

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
    return FALSE;
  }
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
    // trap is finished, consume components
    if (!com1 || !com2 || !com3) {
      vlogf(LOG_BUG, "Serious error in hasTrapComps");
      return FALSE;
    }

    // Try to consume charges from trap components, fall back to deleting regular objects
    TTrapComponent* trapComp1 = dynamic_cast<TTrapComponent*>(com1);
    TTrapComponent* trapComp2 = dynamic_cast<TTrapComponent*>(com2);
    TTrapComponent* trapComp3 = dynamic_cast<TTrapComponent*>(com3);

    if (trapComp1) {
      trapComp1->addToTrapComponentCharges(-1);
      if (trapComp1->getTrapComponentCharges() <= 0) {
        delete trapComp1;
      }
    } else {
      delete com1;
    }

    if (trapComp2) {
      trapComp2->addToTrapComponentCharges(-1);
      if (trapComp2->getTrapComponentCharges() <= 0) {
        delete trapComp2;
      }
    } else {
      delete com2;
    }

    if (trapComp3) {
      trapComp3->addToTrapComponentCharges(-1);
      if (trapComp3->getTrapComponentCharges() <= 0) {
        delete trapComp3;
      }
    } else {
      delete com3;
    }

    if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE) {
      if (!com4) {
        vlogf(LOG_BUG, "Serious error in hasTrapComps (2)");
        return FALSE;
      }
      TTrapComponent* trapComp4 = dynamic_cast<TTrapComponent*>(com4);
      if (trapComp4) {
        trapComp4->addToTrapComponentCharges(-1);
        if (trapComp4->getTrapComponentCharges() <= 0) {
          delete trapComp4;
        }
      } else {
        delete com4;
      }
    }
    return FALSE;
  }
  if (targ == TRAP_TARG_MINE || targ == TRAP_TARG_GRENADE)
    return (com1 && com2 && com3 && com4);
  else
    return (com1 && com2 && com3);
}

// Old sendTrapMessage function removed - replaced with sendStandardizedTrapMessage


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
  int rc;

  switch (o->getTrapDamType()) {
    case DOOR_TRAP_POISON:
      act(POISON_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(POISON_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      trapPoison(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_SLEEP:
      act(SLEEP_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(SLEEP_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      rc = trapSleep(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FIRE:
      act(FIRE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(FIRE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FIRE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = flameEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_TELEPORT:
      act(TELEPORT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(TELEPORT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = trapTeleport(o->getTrapDamAmount());
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_DISEASE:
      act(DISEASE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(DISEASE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);
      trapDisease(o->getTrapDamAmount());
      return TRUE;
    case DOOR_TRAP_BOLT:
      act(SPIKE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(SPIKE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_PIERCE, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_PEBBLE:
      act(BLUNT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(BLUNT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_BLUNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_DISK:
      act(BLADE_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(BLADE_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_SLASH, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_TNT:
      act(TNT_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(TNT_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_TNT, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_FROST:
      act(FROST_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(FROST_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_FROST, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      rc = frostEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      return TRUE;
    case DOOR_TRAP_ENERGY:
      act(ENERGY_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(ENERGY_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

      rc = objDamage(DAMAGE_TRAP_ENERGY, o->getTrapDamAmount(), o);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    case DOOR_TRAP_ACID:
      act(ACID_EFFECT_CHAR_MSG, FALSE, this, o, 0, TO_CHAR);
      act(ACID_EFFECT_ROOM_MSG, FALSE, this, o, 0, TO_ROOM);

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

int TMonster::grenadeHit(TTrap* o) {
  // first make recursive
  int rc = TBeing::grenadeHit(o);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  if (isPc())
    return rc;
  if (!rc)
    return FALSE;

  const char* tmp_desc;
  TBeing* ch = NULL;
  if (o && o->ex_description &&
      (tmp_desc = o->ex_description->findExtraDesc(GRENADE_EX_DESC))) {
    if ((ch = get_char(tmp_desc, EXACT_YES)))
      pissOff(this, ch);
  }

  return TRUE;
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

namespace {
  // Door trap damage modifiers
  constexpr int getDoorTrapDamageModifier(doorTrapT trap_type) {
    switch (trap_type) {
      case DOOR_TRAP_TNT: return 3;
      case DOOR_TRAP_POISON: return -1;
      case DOOR_TRAP_SLEEP: return 1;
      case DOOR_TRAP_ACID: return 1;
      case DOOR_TRAP_DISEASE: return 3;
      case DOOR_TRAP_FROST: return 3;
      case DOOR_TRAP_SPIKE: return -5;
      case DOOR_TRAP_BOLT: return 1;
      case DOOR_TRAP_BLADE: return -3;
      case DOOR_TRAP_DISK: return 3;
      case DOOR_TRAP_HAMMER: return -10;
      case DOOR_TRAP_PEBBLE: return -5;
      case DOOR_TRAP_ENERGY: return 5;
      case DOOR_TRAP_TELEPORT: return 5;
      default: return 0;
    }
  }

  // Helper function for room-wide trap effects with simple effects (no return value)
  template<typename SimpleEffect>
  void applySimpleRoomEffect(TBeing* caster, TRoom* room, TTrap* trap_obj,
                            const char* char_msg, const char* room_msg,
                            SimpleEffect effect_func) {
    for (StuffIter it = room->stuff.begin(); it != room->stuff.end();) {
      TThing* v = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(v);
      if (tbt && tbt->desc && tbt != caster) {
        act(char_msg, FALSE, tbt, trap_obj, 0, TO_CHAR);
        act(room_msg, FALSE, tbt, trap_obj, 0, TO_ROOM);
        effect_func(tbt, trap_obj->getTrapDamAmount() * STATUS_ROOM_MOD);
      }
    }
  }

  // Helper function for room-wide trap effects with complex effects (return DELETE_THIS)
  template<typename ComplexEffect>
  void applyComplexRoomEffect(TBeing* caster, TRoom* room, TTrap* trap_obj,
                             const char* char_msg, const char* room_msg,
                             ComplexEffect effect_func) {
    for (StuffIter it = room->stuff.begin(); it != room->stuff.end();) {
      TThing* v = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(v);
      if (tbt && tbt->desc && tbt != caster) {
        act(char_msg, FALSE, tbt, trap_obj, 0, TO_CHAR);
        act(room_msg, FALSE, tbt, trap_obj, 0, TO_ROOM);
        const int rc = effect_func(tbt, trap_obj->getTrapDamAmount() * STATUS_ROOM_MOD);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }
}

int TBeing::getDoorTrapDam(doorTrapT trap_type) {
  // base range: 10 - 35
  int damage = 10 + getSkillLevel(SKILL_SET_TRAP_DOOR) / 2;
  damage *= getDoorTrapLearn(trap_type);
  damage /= 100;
  damage += getDoorTrapDamageModifier(trap_type);
  return min(max(damage, 1), 50);
}

int TBeing::getContainerTrapDam(doorTrapT trap_type) {
  // base range: 20 - 36
  int damage = 20 + getSkillLevel(SKILL_SET_TRAP_CONT) / 3;
  damage *= getContainerTrapLearn(trap_type);
  damage /= 100;
  damage += getDoorTrapDamageModifier(trap_type);  // Same modifiers as door traps
  return min(max(damage, 1), 50);
}

int TBeing::getMineTrapDam(doorTrapT trap_type) {
  // base range: 20 - 45
  int damage = 20 + getSkillLevel(SKILL_SET_TRAP_MINE) / 2;
  damage *= getMineTrapLearn(trap_type);
  damage /= 100;
  damage += getDoorTrapDamageModifier(trap_type);  // Same modifiers as door traps
  return min(max(damage, 1), 50);
}

int TBeing::getGrenadeTrapDam(doorTrapT trap_type) {
  // because grenades are highly portable, and nail everyone in room
  // i kept the damage on them lower then other traps.
  // this is number of d8 to use when calculating damage
  // base range: 5 - 30
  int damage = 5 + getSkillLevel(SKILL_SET_TRAP_GREN) / 2;

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
int TBeing::getArrowTrapDam(doorTrapT trap_type) {
  int damage = 5 + getSkillLevel(SKILL_SET_TRAP_ARROW) / 2;

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

int TBeing::getDoorTrapLearn(doorTrapT) {
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_DOOR)) <= 0)
    return 0;

  learn = min(learn, (int)MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getContainerTrapLearn(doorTrapT) {
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_CONT)) <= 0)
    return 0;

  learn = min(learn, (int)MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getMineTrapLearn(doorTrapT) {
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_MINE)) <= 0)
    return 0;

  learn = min(learn, (int)MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getGrenadeTrapLearn(doorTrapT) {
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_GREN)) <= 0)
    return 0;

  learn = min(learn, (int)MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TBeing::getArrowTrapLearn(doorTrapT) {
  int learn;

  if ((learn = getSkillValue(SKILL_SET_TRAP_ARROW)) <= 0)
    return 0;

  learn = min(learn, (int)MAX_SKILL_LEARNEDNESS);

  return learn;
}

int TObj::trapMe(TBeing* ch, const char* trap_type) {
  act("$p is not trappable.", FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}
