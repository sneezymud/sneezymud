//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "combat.h" - interface to combat.c
//
///////////////////////////////////////////////////////////////////////////

#ifndef __COMBAT_H
#define __COMBAT_H

// these ONEHIT_MESS may be used with DELETE's, use caution!
const int ONEHIT_MESS_CRIT_S    = (1<<0);
const int ONEHIT_MESS_LIMB      = (1<<1);
const int ONEHIT_MESS_DODGE     = (1<<2);

const int MAX_NPC_CORPSE_TIME = 5;
const int MAX_PC_CORPSE_EMPTY_TIME = 10;
const int MAX_PC_CORPSE_EQUIPPED_TIME = 100;

const int GUARANTEED_FAILURE = -1;
const int GUARANTEED_SUCCESS = -2;

const int COMBAT_SOLO_KILL     = 1;

const int MAX_COMBAT_ATTACKERS   = 9999;

extern TBeing *gCombatList;

#endif
