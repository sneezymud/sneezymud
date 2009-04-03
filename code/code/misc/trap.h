//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __TRAP_H
#define __TRAP_H

const unsigned int TRAP_EFF_MOVE      = (1<<0);   // 1  trigger on movement
const unsigned int TRAP_EFF_OBJECT    = (1<<1);   // 2  trigger on get or put
const unsigned int TRAP_EFF_ROOM      = (1<<2);   // 4  affect all in room
const unsigned int TRAP_EFF_NORTH     = (1<<3);   // 8  movement in this dir
const unsigned int TRAP_EFF_EAST      = (1<<4);   // 16
const unsigned int TRAP_EFF_SOUTH     = (1<<5);   // 32
const unsigned int TRAP_EFF_WEST      = (1<<6);   // 64
const unsigned int TRAP_EFF_UP        = (1<<7);   // 128
const unsigned int TRAP_EFF_DOWN      = (1<<8);   // 256
const unsigned int TRAP_EFF_NE        = (1<<9);   // 512
const unsigned int TRAP_EFF_NW        = (1<<10);  // 1024
const unsigned int TRAP_EFF_SE        = (1<<11);  // 2048
const unsigned int TRAP_EFF_SW        = (1<<12);  // 4096
const unsigned int TRAP_EFF_THROW     = (1<<13);  // 8192
const unsigned int TRAP_EFF_ARMED1    = (1<<14);  // 16384
const unsigned int TRAP_EFF_ARMED2    = (1<<15);  // 32768
const unsigned int TRAP_EFF_ARMED3    = (1<<16);  // 65538

const int MAX_TRAP_EFF       = 17;  // move and change

// these values are same for traps, doors, portals and containers

enum doorTrapT {
   DOOR_TRAP_NONE, 	DOOR_TRAP_POISON,	DOOR_TRAP_SPIKE,
   DOOR_TRAP_SLEEP, 	DOOR_TRAP_TNT,		DOOR_TRAP_BLADE,
   DOOR_TRAP_FIRE,	DOOR_TRAP_ACID,		DOOR_TRAP_DISEASE,
   DOOR_TRAP_HAMMER, 	DOOR_TRAP_FROST, 	DOOR_TRAP_TELEPORT,
   DOOR_TRAP_ENERGY, 	DOOR_TRAP_BOLT, 	DOOR_TRAP_DISK,
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
extern const char * const GRENADE_EX_DESC;
extern const char * const TRAP_EX_DESC;

#endif
