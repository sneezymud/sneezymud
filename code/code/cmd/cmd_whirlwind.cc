#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

static int whirlwind(TBeing *caster, TBeing *victim, int castLevel, spellNumT damageType)
{
  int successfulHit = caster->specialAttack(victim, SKILL_WHIRLWIND);
  int dam = caster->getSkillDam(victim, SKILL_WHIRLWIND, castLevel, caster->getAdvLearning(SKILL_WHIRLWIND));

  // Success case
  if (!victim->awake() || 
    (successfulHit && successfulHit != GUARANTEED_FAILURE)) {

    act("$n hits $N with a spinning attack!", FALSE, caster, 0, victim, TO_NOTVICT);
    act("You hit $N with a spinning attack!", FALSE, caster, 0, victim, TO_CHAR);
    act("$n hits you with a spinning attack!", FALSE, caster, 0, victim, TO_VICT);
  }
  // Failure case
  else
  {
    if (caster->reconcileDamage(victim, 0, damageType) == -1) 
      return DELETE_VICT;
    return FALSE;
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

  int castLevel = getSkillLevel(SKILL_WHIRLWIND);
  int skillLevel = getSkillValue(SKILL_WHIRLWIND);
  int successfulSkill = bSuccess(skillLevel, SKILL_WHIRLWIND);

  // Unsuccessful skill attempt
  if (!successfulSkill) {
    act("You attempt to perform a sweeping attack, but fail miserably!", FALSE, this, NULL, NULL, TO_CHAR);
    act("$n attempts to perform a sweeping attack, but instead just ends up looking silly!", FALSE, this, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  // Successful skill attempt
  // Send messages to caster/room
  act("You perform a sweeping attack, striking every opponent nearby!", FALSE, this, NULL, NULL, TO_CHAR);
  act("$n performs a sweeping attack, striking everyone nearby!", FALSE, this, NULL, NULL, TO_ROOM);
  
  // Determine damage type
  spellNumT damageType = DAMAGE_NORMAL;
  if (weapon->isBluntWeapon())
    damageType = DAMAGE_CAVED_SKULL;
  else if (weapon->isPierceWeapon())
    damageType = DAMAGE_IMPALE;
  else if (weapon->isSlashWeapon())
    damageType = DAMAGE_HACKED;

  // Loop for each person in room
  std::vector<TBeing *> toDelete{};
  for (TThing *thing : roomp->stuff) {
    auto *being = dynamic_cast<TBeing *>(thing);
    if (!being || (being == this) || inGroup(*being) ||
        (being->isPc() && IS_SET(desc->autobits, AUTO_NOHARM)) || being->isImmortal() ||
        IS_SET(being->specials.act, ACT_IMMORTAL))
      continue;

    rc = whirlwind(this, being, castLevel, damageType);
    if (!IS_SET_DELETE(rc, DELETE_VICT))
      continue;

    toDelete.push_back(being);
  }

  for (TBeing *being: toDelete) {
    delete being;
    being = nullptr;
  }
  // end loop 

  return rc;
}
