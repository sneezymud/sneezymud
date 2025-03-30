#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_general_weapon.h"
#include "materials.h"
#include "skills.h"

#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doStab(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  const int STAB_MOVE = 3;

  if (checkBusy()) {
    return false;
  }

  // Ensure player even knows the skill before continuing
  if (!doesKnowSkill(SKILL_STABBING)) {
    sendTo(
      "You wouldn't even know where to begin in executing that maneuver.\n\r");
    return false;
  }

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, argument))) {
      if (!(victim = fight())) {
        sendTo("Hit whom?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  auto* weapon = dynamic_cast<TGenWeapon*>(heldInPrimHand());

  // Ensure this isn't a peaceful room
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Make sure the player has enough vitality to use the skill
  if (getMove() < STAB_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return false;
  }

  // Prevent players from attacking immortals
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("Attacking an immortal would be a bad idea.\n\r");
    return false;
  }

  // Prevent players from attacking themselves
  if (victim == this) {
    sendTo("Do you REALLY want to kill yourself?...\n\r");
    return false;
  }

  // Avoid players attacking un-harmable victims
  if (noHarmCheck(victim))
    return false;

  // Limit players from using this while mounted
  if (riding) {
    sendTo("You can't perform that attack while mounted!\n\r");
    return false;
  }

  // Ensure the player has a weapon equipped
  if (!weapon) {
    sendTo(
      "You need to hold a weapon in your primary hand to make this a "
      "success.\n\r");
    return false;
  }
  if (!weapon->canStab()) {
    act("You can't use a $o to stab.", false, this, weapon, NULL, TO_CHAR);
    return FALSE;
  }
  // Only consume vitality for mortals
  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-STAB_MOVE);

  int skillValue = getSkillValue(SKILL_STABBING);
  int successfulHit = specialAttack(victim, SKILL_STABBING);
  int successfulSkill = bSuccess(skillValue, SKILL_STABBING);

  // Success use case
  if (!victim->awake() || (successfulHit && successfulSkill &&
                            successfulHit != GUARANTEED_FAILURE)) {
    rc = stabSuccess(victim);
  }
  // Fail use case
  else {
    rc = stabFailure(victim);
  }

  // Put it here to see ALL rc values
  act(format("Debug: rc value is %d") % rc, FALSE, this, NULL, NULL, TO_CHAR);
  if (rc)
    addSkillLag(SKILL_STABBING, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  return rc;
}

int TBeing::stabSuccess(TBeing* victim) {
  int skillLevel = getSkillLevel(SKILL_STABBING);
  int dam = getSkillDam(victim, SKILL_STABBING, skillLevel,
    getAdvLearning(SKILL_STABBING));
  auto* weapon = dynamic_cast<TGenWeapon*>(heldInPrimHand());

  // Scaling damage here due to the limitations of getSkillDam and how it treats
  // skills learned at low levels This formula is designed to allow the damage
  // to scale up to 0.75% of max hp to have some effectiveness against high
  // level opponents, while dealing a respectable amount of damage to lower
  // level enemies
  static const std::map<int, float> scalingDamageConstants = {{10, 0.15},
    {20, 0.08}, {30, 0.05}, {40, 0.03}, {50, 0.02}};

  // Default value for higher level enemies
  float scalingConstant = 0.0075;

  // For enemies level 1-50, retrieving
  for (const auto& damageConstant : scalingDamageConstants) {
    if (victim->GetMaxLevel() <= damageConstant.first) {
      scalingConstant = damageConstant.second;
      break;
    }
  }

  // Apply the scaling constant
  dam = max((int)(victim->hitLimit() * scalingConstant), dam);

  wearSlotT limb = victim->getPartHit(this, false);
  int limbdam = (weapon->getCurSharp() / 100) * (dam/2);
  if (dynamic_cast<TObj*>(weapon)->isObjStat(ITEM_SPIKED)) {
    limbdam = limbdam * 1.25;
  }

  // Send description text to players in the room
  if (!victim->isLimbFlags(limb, PART_MISSING) && !victim->isUndead() &&
      !victim->isLimbFlags(limb, IMMUNE_BLEED)) {
    int duration = (this->GetMaxLevel() * 3 + 200);
    victim->rawBleed(limb, duration, SILENT_YES, CHECK_IMMUNITY_NO);
    victim->hurtLimb(limbdam, limb);
    sstring buf = format("You puncture $N's %s with your $o, leaving a bloody gash!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_CHAR);
    
    buf = format("$n punctures $N's %s with their $o, leaving a bloody gash!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_ROOM);
    
    buf = format("$n punctures your %s with their $o, leaving a bloody gash!!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_VICT);
  } else if (!victim->isLimbFlags(limb, PART_MISSING) &&
             victim->isLimbFlags(limb, IMMUNE_BLEED)) {
    sstring buf = format("You puncture $N's %s with your $o, leaving a gaping hole!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_CHAR);
    
    buf = format("$n punctures $N's %s with their $o, leaving a gaping hole!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_ROOM);
    
    buf = format("$n punctures your %s with their $o, leaving a gaping hole!!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_VICT);

  } else {
    sstring buf = format("You stab $N's %s with your $o, leaving a big wound!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_CHAR);
    
    buf = format("$n stabs $N's %s with their $o, leaving a big wound!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_ROOM);
    
    buf = format("$n stabs your %s with their $o, leaving a big wound!!") % victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_VICT);
  }
  if (weapon->isObjStat(ITEM_SPIKED)) {
    weapon->addToCurSharp(-limbdam);
    weapon->addToMaxStructPoints(-1);
    weapon->addToStructPoints(-limbdam);
    if (!victim->isLimbFlags(limb, IMMUNE_BLEED) && !victim->isUndead()) {
      victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
      act(
        "The wound begins to bleed as the spikes on $o shred flesh, but the "
        "weapon is damaged!",
        false, this, weapon, victim, TO_ROOM);
    } else {
      act(
        "The wound becomes jagged as the spikes on $o rip through flesh, but "
        "the weapon is damaged!",
        false, this, weapon, victim, TO_ROOM);
    }
    victim->hurtLimb(1, limb);
  }
  if (!victim->isLimbFlags(limb, PART_MISSING) &&
      ((victim->isLimbFlags(limb, PART_BLEEDING) ||
        victim->isLimbFlags(limb, PART_INFECTED) ||
        victim->isLimbFlags(limb, PART_BRUISED)))) {
    if (victim->getCurLimbHealth(limb) >= victim->getMaxLimbHealth(limb) / 2) {
      if (limb == WEAR_NECK || limb == WEAR_BODY || limb == WEAR_BACK ||
          limb == WEAR_WAIST) {
        // Apply normal damage without special effects
        spellNumT damageType = weapon->isPierceWeapon() ? DAMAGE_IMPALE : DAMAGE_NORMAL;
        if (!victim->isLimbFlags(limb, IMMUNE_BLEED) && !victim->isUndead()) {
          int duration = (this->GetMaxLevel() * 3 + 200);
          victim->rawBleed(limb, duration, SILENT_YES, CHECK_IMMUNITY_NO);
          sstring buf = format("You puncture $N's %s with your $o, leaving a bloody gash!") % victim->describeBodySlot(limb);
          act(buf, false, this, weapon, victim, TO_CHAR);
          buf = format("$n punctures $N's %s with their $o, leaving a bloody gash!") % victim->describeBodySlot(limb);
          act(buf, false, this, weapon, victim, TO_ROOM);
        }
        if (reconcileDamage(victim, dam, damageType) == -1)
          return DELETE_VICT;
        return true;
      } else if (limb == WEAR_HEAD && !victim->hasDisease(DISEASE_EYEBALL)) {
        affectedData tAff;
        act("You glance $N's eyes with your $o, slicing them wide open!", false,
          this, 0, victim, TO_CHAR);
        act("$n glances your eyes with $s $o, slicing them wide open!", false,
          this, 0, victim, TO_VICT);
        act("$n glances $N's eyes with $s $o, slicing them wide open!", false,
          this, 0, victim, TO_NOTVICT);

        tAff.type = AFFECT_DISEASE;
        tAff.level = 0;
        tAff.duration = PERMANENT_DURATION;
        tAff.modifier = DISEASE_EYEBALL;
        tAff.location = APPLY_NONE;
        tAff.bitvector = AFF_BLIND;
        victim->affectTo(&tAff);
        victim->rawBlind(this->GetMaxLevel(), tAff.duration, SAVE_NO);
      } else {
        victim->makePartMissing(limb, false, this);
        sstring buf = format("You slice $N's %s right off!") % victim->describeBodySlot(limb);
        act(buf, false, this, weapon, victim, TO_CHAR);
        
        buf = format("$n slices your %s right off!") % victim->describeBodySlot(limb);
        act(buf, false, this, 0, victim, TO_VICT);
        
        buf = format("$n slices $N's %s right off!") % victim->describeBodySlot(limb);
        act(buf, false, this, 0, victim, TO_NOTVICT);
      }

      // Determine damage type
      spellNumT damageType = DAMAGE_NORMAL;

      if (weapon->isPierceWeapon())
        damageType = DAMAGE_IMPALE;
      // Reconcile damage
      if (reconcileDamage(victim, dam, damageType) == -1)
        return DELETE_VICT;
      return true;
    }
  }

  if (weapon->isPoisoned()) {
    weapon->applyPoison(victim);
    act("You poison $N with your stab!", false, this, NULL, victim, TO_CHAR);
    act("That bastard $n just poisoned you!", false, this, NULL, victim,
      TO_VICT);
    act("$N gets a pale look on their face, like they've been poisoned!", false,
      this, NULL, victim, TO_NOTVICT);
  } else if (!victim->isLimbFlags(limb, PART_INFECTED) &&
             victim->isLimbFlags(limb, PART_BLEEDING) && !::number(0, 4)) {
    victim->rawInfect(limb, 250, SILENT_YES, CHECK_IMMUNITY_YES);
    sstring buf = format("Your stab to $N's %s infects it!") % victim->describeBodySlot(limb);
    act(buf, false, this, NULL, victim, TO_CHAR);
    
    buf = format("Your %s gets infected from $n's stab!") % victim->describeBodySlot(limb);
    act(buf, false, this, NULL, victim, TO_VICT);
    
    buf = format("$N's %s gets infected from $n's stab!") % victim->describeBodySlot(limb);
    act(buf, false, this, NULL, victim, TO_NOTVICT);
  }
  spellNumT damageType = TYPE_PIERCE;

  if (reconcileDamage(victim, dam, damageType) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::stabFailure(TBeing* victim) {
  if (victim->getPosition() > POSITION_DEAD) {
    act("You miss your thrust into $N.", FALSE, this, 0, victim, TO_CHAR);
    act("$n misses $s thrust into $N.", FALSE, this, 0, victim, TO_NOTVICT);
    act("$n misses $s thrust into you.", FALSE, this, 0, victim, TO_VICT);
    this->reconcileDamage(victim, 0, SKILL_STABBING);
  }

  if (reconcileDamage(victim, 0, SKILL_STABBING) == -1)
    {return DELETE_VICT;}

  return true;
}
