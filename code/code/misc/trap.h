//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "spells.h"
#include "sstring.h"

class TBeing;
class TThing;

const unsigned int TRAP_EFF_MOVE = (1 << 0);     // 1  trigger on movement
const unsigned int TRAP_EFF_OBJECT = (1 << 1);   // 2  trigger on get or put
const unsigned int TRAP_EFF_ROOM = (1 << 2);     // 4  affect all in room
const unsigned int TRAP_EFF_NORTH = (1 << 3);    // 8  movement in this dir
const unsigned int TRAP_EFF_EAST = (1 << 4);     // 16
const unsigned int TRAP_EFF_SOUTH = (1 << 5);    // 32
const unsigned int TRAP_EFF_WEST = (1 << 6);     // 64
const unsigned int TRAP_EFF_UP = (1 << 7);       // 128
const unsigned int TRAP_EFF_DOWN = (1 << 8);     // 256
const unsigned int TRAP_EFF_NE = (1 << 9);       // 512
const unsigned int TRAP_EFF_NW = (1 << 10);      // 1024
const unsigned int TRAP_EFF_SE = (1 << 11);      // 2048
const unsigned int TRAP_EFF_SW = (1 << 12);      // 4096
const unsigned int TRAP_EFF_THROW = (1 << 13);   // 8192
const unsigned int TRAP_EFF_ARMED1 = (1 << 14);  // 16384
const unsigned int TRAP_EFF_ARMED2 = (1 << 15);  // 32768
const unsigned int TRAP_EFF_ARMED3 = (1 << 16);  // 65538

const int MAX_TRAP_EFF = 17;  // move and change
// these values are same for traps, doors, portals and containers

enum doorTrapT {
  DOOR_TRAP_NONE,
  DOOR_TRAP_POISON,
  DOOR_TRAP_SPIKE,
  DOOR_TRAP_SLEEP,
  DOOR_TRAP_TNT,
  DOOR_TRAP_BLADE,
  DOOR_TRAP_FIRE,
  DOOR_TRAP_ACID,
  DOOR_TRAP_DISEASE,
  DOOR_TRAP_HAMMER,
  DOOR_TRAP_FROST,
  DOOR_TRAP_TELEPORT,
  DOOR_TRAP_ENERGY,
  DOOR_TRAP_BOLT,
  DOOR_TRAP_DISK,
  DOOR_TRAP_PEBBLE,

  MAX_TRAP_TYPES
};

enum trap_targ_t {
  TRAP_TARG_DOOR,
  TRAP_TARG_CONT,
  TRAP_TARG_MINE,
  TRAP_TARG_GRENADE,
  TRAP_TARG_ARROW
};

extern const sstring trap_types[];
extern const char* const GRENADE_EX_DESC;
extern const char* const TRAP_EX_DESC;

extern const int TrapDir[];
extern const char* const trap_effects[MAX_TRAP_EFF];
extern doorTrapT mapFileToDoorTrap(int);
extern int mapDoorTrapToFile(doorTrapT);

// The set-trap skill that governs each trap_targ_t, indexed by target.
extern const spellNumT trapSetSkill[];
// Resolve a trap's recorded setter to a live, creditable being, or nullptr.
// nullptr for any trap with no recorded setter (all world/mob-loaded traps),
// which routes its damage to the unattributed objDamage() fallback.
TBeing* trapSetter(const TThing* carrier);
doorTrapT parseTrapType(const char* name, trap_targ_t target);
// Single source of trap component vnums for a (type, target); fills the three
// reagent vnums and returns false for an unrecognized type. Shared by trap
// gating/consumption, the set-trap flavor messages, and disarm reclaim.
bool trapComponents(const char* type, trap_targ_t targ, int& item1, int& item2,
  int& item3);

// Passive trap-sense on look. The caller resolves whether the target is
// trapped and rolls Detect Trap; this reports the result. A successful skill
// roll (skillDetected) reveals the trap type and level; otherwise a perceptive
// looker gets only a vague warning. Pass the trapped TThing as obj (for proper
// $p naming) for containers/portals, or obj == nullptr with doorName set for
// exits.
void describeTrapToLooker(TBeing* ch, const TThing* obj,
  const sstring& doorName, bool skillDetected, int trapType, int trapDam);
