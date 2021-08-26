#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doFocusAttack(const char *argument, TBeing *vict)
{
  const int FOCUS_ATTACK_MOVE = 10;
  TBeing *victim = nullptr;
  
  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_FOCUS_ATTACK)) {
    sendTo("You know nothing about focused attacks.\n\r");
    return FALSE;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  // Adding a lockout 
  if (affectedBySpell(SKILL_FOCUS_ATTACK)) {
    sendTo("You are still recovering from your last focused attack and cannot use this ability again at this time.\n\r");
    return FALSE;
  }

  if (getMove() < FOCUS_ATTACK_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return FALSE;
  }

  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL))) 
    addToMove(-FOCUS_ATTACK_MOVE);
  
  if (!(victim = vict) && 
     (!(victim = get_char_room_vis(this, argument))) && 
     (!(victim = fight()))) {
    sendTo("Who do you want to attack?\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) { 
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("You cannot attack an immortal.\n\r");
    return FALSE;
  }
  
  affectedData aff1;
  aff1.type = SKILL_FOCUS_ATTACK;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR/2;
  aff1.bitvector = 0;
  affectTo(&aff1, -1);

  if (!bSuccess(getSkillValue(SKILL_FOCUS_ATTACK), SKILL_FOCUS_ATTACK)) {
    act("You attempt to perform a focused attack, but lose your concentration!", FALSE, this, NULL, NULL, TO_CHAR);
    act("$n attempts to focus, but loses $s concentration.", FALSE, this, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  act("You focus on identifying a weakness in your opponent's defense.", FALSE, this, NULL, NULL, TO_CHAR);
  act("$n concentrates on $s opponent, focusing intensely!", FALSE, this, NULL, NULL, TO_ROOM);

  // Set the flag that we will later check for to trigger an attack
  SET_BIT(specials.affectedBy, AFF_FOCUS_ATTACK);

  // If the characters aren't currently fighting, initiate combat
  if (!fight()) {
    setCharFighting(victim);
    setVictFighting(victim);
    reconcileHurt(victim, 0.01);
  }

  return TRUE;
}
