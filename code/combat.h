//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: combat.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.1 - All rights reserved, SneezyMUD Coding Team
//      "combat.h" - interface to combat.c
//
///////////////////////////////////////////////////////////////////////////

#ifndef __COMBAT_H
#define __COMBAT_H

const int SENT_MESS    = (1<<0);
const int DAMAGED_LIMB = (1<<1);

const int MAX_NPC_CORPSE_TIME = 5;
const int MAX_PC_CORPSE_EMPTY_TIME = 10;
const int MAX_PC_CORPSE_EQUIPPED_TIME = 100;

const int GUARANTEED_FAILURE = -1;
const int GUARANTEED_SUCCESS = -2;

const int COMBAT_SOLO_KILL     = 1;

const int MAX_COMBAT_ATTACKERS   = 6;

extern TBeing *gCombatList;

#endif
