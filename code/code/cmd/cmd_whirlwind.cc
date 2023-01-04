#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

static int whirlwind(TBeing *caster, TBeing *victim, int castLevel, spellNumT damageType)
{
  int successfulHit = caster->specialAttack(victim, SKILL_WHIRLWIND);
  int dam = 0;

  // Success case
  if (!victim->awake() || 
    (successfulHit && successfulHit != GUARANTEED_FAILURE)) {
    act("$n hits $N with a spinning attack!", FALSE, caster, 0, victim, TO_NOTVICT);
    act("You hit $N with a spinning attack!", FALSE, caster, 0, victim, TO_CHAR);
    act("$n hits you with a spinning attack!", FALSE, caster, 0, victim, TO_VICT);
    dam = caster->getSkillDam(victim, SKILL_WHIRLWIND, castLevel, caster->getAdvLearning(SKILL_WHIRLWIND));
  }
  // Failure case
  else {
    act("$n's spinning attack misses $N.", FALSE, caster, 0, victim, TO_NOTVICT);
    act("Your spinning attack misses $N.", FALSE, caster, 0, victim, TO_CHAR);
    act("$n's spinning attack misses you.", FALSE, caster, 0, victim, TO_VICT);
  }

  if (caster->reconcileDamage(victim, dam, damageType) == -1) 
    return DELETE_VICT;

  return TRUE;
}

int TBeing::doWhirlwind()
{
  const int WHIRLWIND_MOVE = 20;
  int rc = 0;
  affectedData aff1;
  
  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_WHIRLWIND)) {
    sendTo("You know nothing about whirlwind attacks.\n\r");
    return FALSE;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  // Adding a lockout 
  if (affectedBySpell(SKILL_WHIRLWIND)) {
    sendTo("You are still recovering from your last whirlwind attack and cannot use this ability again at this time.\n\r");
    return FALSE;
  }

  if (getMove() < WHIRLWIND_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return FALSE;
  }

  auto *weapon = dynamic_cast<TBaseWeapon *>(heldInPrimHand());
  if (!weapon){
      sendTo("You need to hold a weapon in your primary attack to attempt this maneuver.\n\r");	 
      return FALSE;
  }

  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-WHIRLWIND_MOVE);
  
  
  // Assessing an armor penalty regardless of success - this penalty greatly scales with level
  // to ensure that players are not overly penalized early on
  aff1.type = SKILL_WHIRLWIND;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = 1;
  aff1.bitvector = 0;
  affectTo(&aff1, -1);

  int skillLevel = getSkillValue(SKILL_WHIRLWIND);
  int successfulSkill = bSuccess(skillLevel, SKILL_WHIRLWIND);

  // Unsuccessful skill attempt
  if (!successfulSkill) {
    rc = whirlwindFail();
  } else { 
    rc = whirlwindSuccess();
  }

  return rc;
}

int TBeing::whirlwindSuccess() { 
  int rc = 0;
  auto *weapon = dynamic_cast<TBaseWeapon *>(heldInPrimHand());

  // Send messages to caster/room
  act("You perform a sweeping attack, striking out at every opponent nearby!", FALSE, this, NULL, NULL, TO_CHAR);
  act("$n performs a sweeping attack, striking out at everyone nearby!", FALSE, this, NULL, NULL, TO_ROOM);
  
  // Determine damage type
  spellNumT damageType = DAMAGE_NORMAL;
  if (weapon->isBluntWeapon())
    damageType = DAMAGE_CAVED_SKULL;
  else if (weapon->isPierceWeapon())
    damageType = DAMAGE_IMPALE;
  else if (weapon->isSlashWeapon())
    damageType = DAMAGE_HACKED;

  // Loop for each person in room and determine if they're a valid whirlwind target. If so, add them
  // to the vector for later.
  std::vector<TBeing *> validTargets{};
  for (TThing *thing : roomp->stuff) {
    auto *being = dynamic_cast<TBeing *>(thing);
    if (!being || (being == this) || inGroup(*being) ||
        (being->isPc() && IS_SET(desc->autobits, AUTO_NOHARM)) || being->isImmortal() ||
        IS_SET(being->specials.act, ACT_IMMORTAL))
      continue;

    validTargets.push_back(being);
  }

  // Apply whirlwind damage and delete dead victims in a separate loop. This is necessary because
  // when applying the damage from whirlwind there's a chance the victim will have an immediate flee
  // triggered when taken below 9% health. If this happens during the previous loop, the victim is
  // removed from the TRoom::stuff list being iterated and causes the iterator to become invalid
  // before the loop is complete, triggering a crash. Doing it in a secondary loop afterwards
  // prevents this.
  for (TBeing *being: validTargets) {
    if (being->inRoom() != in_room)
      continue;

    int castLevel = getSkillLevel(SKILL_WHIRLWIND);
    rc = whirlwind(this, being, castLevel, damageType);
    if (!IS_SET_DELETE(rc, DELETE_VICT))
      continue;

    delete being;
    being = nullptr;
  }
  // end loop 

  return rc;
}

int TBeing::whirlwindFail() {
    act("You attempt to perform a sweeping attack, but fail miserably!", FALSE, this, NULL, NULL, TO_CHAR);
    act("$n attempts to perform a sweeping attack, but instead just ends up looking silly!", FALSE, this, NULL, NULL, TO_ROOM);
    return FALSE;
}