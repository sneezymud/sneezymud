//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "combat.h" - interface to combat.c
//
///////////////////////////////////////////////////////////////////////////

#pragma once

class TBeing;

// these ONEHIT_MESS may be used with DELETE's, use caution!
constexpr int ONEHIT_MESS_CRIT_S = (1 << 0);
constexpr int ONEHIT_MESS_LIMB = (1 << 1);
constexpr int ONEHIT_MESS_DODGE = (1 << 2);

constexpr int MAX_NPC_CORPSE_TIME = 5;
constexpr int MAX_PC_CORPSE_EMPTY_TIME = 10;
constexpr int MAX_PC_CORPSE_EQUIPPED_TIME = 100;

constexpr int GUARANTEED_FAILURE = -1;
constexpr int GUARANTEED_SUCCESS = -2;
constexpr int PARTIAL_SUCCESS = 2;
constexpr int COMPLETE_SUCCESS = 1;
constexpr int FAILURE = 0;

constexpr int SITUATIONAL_MOD_LOWER_BOUND = -20;
constexpr int SITUATIONAL_MOD_UPPER_BOUND = 20;
constexpr int SUCCESS_THRESHOLD = 50;
constexpr int PARTIAL_SUCCESS_THRESHOLD = 80;

constexpr int COMBAT_SOLO_KILL = 1;
constexpr int COMBAT_RESTRICT_XP = 2;

constexpr int MAX_COMBAT_ATTACKERS = 9999;

extern TBeing* gCombatList;

bool restrict_xp(const TBeing* caster, TBeing* victim, int duration);
void doToughness(TBeing* ch);
