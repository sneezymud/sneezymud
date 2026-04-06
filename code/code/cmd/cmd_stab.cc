#include "ansi.h"
#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_general_weapon.h"
#include "materials.h"
#include "skills.h"

// Forward declarations
static int stabOffAtk(TBeing* thief, TBeing* victim, TGenWeapon* weapon);
static int stabPrimAtk(TBeing* thief, TBeing* victim, TGenWeapon* primWeapon,
  TGenWeapon* offWeapon);

// Display hit message for a successful stab
static void stabHitMsg(TBeing* thief, TBeing* victim, TGenWeapon* weapon,
  wearSlotT limb) {
  sstring limbName(victim->describeBodySlot(limb));

  switch (::number(0, 3)) {
    case 0:
      act(format("You thrust your $o into $N's %s!") % limbName, FALSE, thief,
        weapon, victim, TO_CHAR, ANSI_ORANGE);
      act(format("$n thrusts $s $o into your %s!") % limbName, FALSE, thief,
        weapon, victim, TO_VICT, ANSI_ORANGE);
      act(format("$n thrusts $s $o into $N's %s!") % limbName, FALSE, thief,
        weapon, victim, TO_NOTVICT, ANSI_ORANGE);
      break;
    case 1:
      act(format("You stab $N in $S %s with your $o!") % limbName, FALSE, thief,
        weapon, victim, TO_CHAR, ANSI_ORANGE);
      act(format("$n stabs you in your %s with $s $o!") % limbName, FALSE, thief,
        weapon, victim, TO_VICT, ANSI_ORANGE);
      act(format("$n stabs $N in $S %s with $s $o!") % limbName, FALSE, thief,
        weapon, victim, TO_NOTVICT, ANSI_ORANGE);
      break;
    case 2:
      act(format("You gouge $N in $S %s with your $o!") % limbName, FALSE, thief,
        weapon, victim, TO_CHAR, ANSI_ORANGE);
      act(format("$n gouges you in your %s with $s $o!") % limbName, FALSE,
        thief, weapon, victim, TO_VICT, ANSI_ORANGE);
      act(format("$n gouges $N in $S %s with $s $o!") % limbName, FALSE, thief,
        weapon, victim, TO_NOTVICT, ANSI_ORANGE);
      break;
    default:
      act(format("You puncture $N's %s with your $o!") % limbName, FALSE, thief,
        weapon, victim, TO_CHAR, ANSI_ORANGE);
      act(format("$n punctures your %s with $s $o!") % limbName, FALSE, thief,
        weapon, victim, TO_VICT, ANSI_ORANGE);
      act(format("$n punctures $N's %s with $s $o!") % limbName, FALSE, thief,
        weapon, victim, TO_NOTVICT, ANSI_ORANGE);
      break;
  }
}

// Display miss message for a failed stab
static void stabMissMsg(TBeing* thief, TBeing* victim, TGenWeapon* weapon) {
  act("You miss your thrust into $N.", FALSE, thief, weapon, victim, TO_CHAR);
  act("$n misses $s thrust into you.", FALSE, thief, weapon, victim, TO_VICT);
  act("$n misses $s thrust into $N.", FALSE, thief, weapon, victim, TO_NOTVICT);
}

// Apply poison from weapon if present; returns DELETE_VICT if poison kills
static int stabPoisonCheck(TBeing* victim, TGenWeapon* weapon) {
  if (weapon->isPoisoned())
    return weapon->applyPoison(victim);
  return 0;
}

// Check for bleeding based on weapon sharpness vs limb hardness
static void stabBleedCheck(TBeing* thief, TBeing* victim, TGenWeapon* weapon,
  wearSlotT limb) {
  if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED, limb))
    return;

  int sharpness = weapon->getCurSharp();
  int hardness = getHardnessSpec(victim, limb);
  int chance = sharpness - hardness;

  if (chance > 0 && percentChance(chance)) {
    int duration = sharpness * Pulse::UPDATES_PER_MUDHOUR / 2;

    if (victim->isLimbFlags(limb, PART_BLEEDING)) {
      // Already bleeding - increment the stack
      victim->incrementBleedStack(limb, duration);
    } else {
      // New bleed
      sstring limbName(victim->describeBodySlot(limb));
      act(format("Your stab opens a <r>bleeding wound<z> on $N's %s!") % limbName,
        FALSE, thief, nullptr, victim, TO_CHAR);
      act(format("$n's stab opens a <r>bleeding wound<z> on your %s!") % limbName,
        FALSE, thief, nullptr, victim, TO_VICT);
      act(format("$n's stab opens a <r>bleeding wound<z> on $N's %s!") % limbName,
        FALSE, thief, nullptr, victim, TO_NOTVICT);

      victim->rawBleed(limb, duration, SILENT_YES, CHECK_IMMUNITY_NO);
    }
  }
}

// Offhand stab attack - uses SKILL_STABBING for specAttack
static int stabOffAtk(TBeing* thief, TBeing* victim, TGenWeapon* weapon) {
  if (!weapon || !weapon->canStab())
    return FALSE;

  // Use DEX/SPE for offense, AGI/PER for defense
  int specResult = thief->specialAttack(victim, SKILL_STABBING, 0, STAT_DEX,
    STAT_SPE, STAT_AGI, STAT_PER, false);
  if (specResult != COMPLETE_SUCCESS && specResult != GUARANTEED_SUCCESS) {
    stabMissMsg(thief, victim, weapon);
    victim->addHated(thief);
    return TRUE;
  }

  // Calculate damage for offhand
  int level = thief->getSkillLevel(SKILL_STABBING);
  int dam = thief->getSkillDam(victim, SKILL_STABBING, level,
    thief->getAdvLearning(SKILL_STABBING));
  dam = thief->getActualDamage(victim, weapon, dam, SKILL_STABBING);
  
  if (dam <= 0) {
    act("Your $o fails to penetrate $N's thick hide.", FALSE, thief, weapon,
      victim, TO_CHAR);
    act("$n's $o fails to penetrate your thick hide.", FALSE, thief, weapon,
      victim, TO_VICT);
    act("$n's $o fails to penetrate $N's thick hide.", FALSE, thief, weapon,
      victim, TO_NOTVICT);
    return TRUE;
  }

  // Get target limb
  wearSlotT limb = victim->getPartHit(thief, FALSE);
  if (!victim->hasPart(limb))
    limb = victim->getCritPartHit();

  // Show hit message
  stabHitMsg(thief, victim, weapon, limb);

  // Apply damage
  if (thief->reconcileDamage(victim, dam, SKILL_STABBING) == -1)
    return DELETE_VICT;

  // Check for bleeding and poison
  stabBleedCheck(thief, victim, weapon, limb);
  if (IS_SET_DELETE(stabPoisonCheck(victim, weapon), DELETE_VICT))
    return DELETE_VICT;

  // Check weapon spec proc
  if (weapon->checkSpec(victim, CMD_STAB, reinterpret_cast<char*>(limb),
        thief) == DELETE_VICT)
    return DELETE_VICT;

  return TRUE;
}

// Primary hand stab attack - also attempts offhand if dual wield succeeds
static int stabPrimAtk(TBeing* thief, TBeing* victim, TGenWeapon* primWeapon,
  TGenWeapon* offWeapon) {
  // Use DEX/SPE for offense, AGI/PER for defense
  int specResult = thief->specialAttack(victim, SKILL_STABBING, 0, STAT_DEX,
    STAT_SPE, STAT_AGI, STAT_PER, false);
  if (specResult != COMPLETE_SUCCESS && specResult != GUARANTEED_SUCCESS) {
    stabMissMsg(thief, victim, primWeapon);
    victim->addHated(thief);
    return TRUE;
  }

  // Calculate damage for primary
  int level = thief->getSkillLevel(SKILL_STABBING);
  int dam = thief->getSkillDam(victim, SKILL_STABBING, level,
    thief->getAdvLearning(SKILL_STABBING));
  dam = thief->getActualDamage(victim, primWeapon, dam, SKILL_STABBING);

  if (dam <= 0) {
    act("Your $o fails to penetrate $N's thick hide.", FALSE, thief, primWeapon,
      victim, TO_CHAR);
    act("$n's $o fails to penetrate your thick hide.", FALSE, thief, primWeapon,
      victim, TO_VICT);
    act("$n's $o fails to penetrate $N's thick hide.", FALSE, thief, primWeapon,
      victim, TO_NOTVICT);
    return TRUE;
  }

  // Get target limb
  wearSlotT limb = victim->getPartHit(thief, FALSE);
  if (!victim->hasPart(limb))
    limb = victim->getCritPartHit();

  // Show hit message
  stabHitMsg(thief, victim, primWeapon, limb);

  // Apply damage
  if (thief->reconcileDamage(victim, dam, SKILL_STABBING) == -1)
    return DELETE_VICT;

  // Check for bleeding and poison
  stabBleedCheck(thief, victim, primWeapon, limb);
  if (IS_SET_DELETE(stabPoisonCheck(victim, primWeapon), DELETE_VICT))
    return DELETE_VICT;

  // Check weapon spec proc
  if (primWeapon->checkSpec(victim, CMD_STAB, reinterpret_cast<char*>(limb),
        thief) == DELETE_VICT)
    return DELETE_VICT;

  // Attempt offhand attack if dual wielding a canStab weapon and victim still exists
  if (victim && offWeapon && offWeapon->canStab()) {
    int thiefDual = thief->getSkillValue(SKILL_DUAL_WIELD_THIEF);
    int warDual = thief->getSkillValue(SKILL_DUAL_WIELD);
    if (thief->getAdvLearning(SKILL_STABBING) > 50) {
      if ((thief->doesKnowSkill(SKILL_DUAL_WIELD) &&
          thief->bSuccess(warDual, SKILL_DUAL_WIELD)) ||
          ((thief->doesKnowSkill(SKILL_DUAL_WIELD_THIEF) &&
            thief->bSuccess(thiefDual, SKILL_DUAL_WIELD_THIEF)))) {
        act("You shift your weight and ready an attack with your offhand.",
          FALSE, thief, nullptr, victim, TO_CHAR, ANSI_PURPLE);
        act("$n shifts $s weight and readies an attack with $s offhand.", FALSE,
          thief, nullptr, victim, TO_ROOM, ANSI_PURPLE);
        act("$n shifts $s weight and readies an attack with $s offhand.", FALSE,
          thief, nullptr, victim, TO_VICT, ANSI_PURPLE);
        int rc = stabOffAtk(thief, victim, offWeapon);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      }
    }
  }

  return TRUE;
}

static int stab(TBeing* thief, TBeing* victim) {
  const int STAB_MOVE = 2;

  if (thief == victim) {
    thief->sendTo("Hey now, let's not be stupid.\n\r");
    return FALSE;
  }

  if (thief->checkPeaceful("Naughty, naughty.  None of that here.\n\r"))
    return FALSE;

  if (thief->riding) {
    thief->sendTo("Not while mounted!\n\r");
    return FALSE;
  }

  if (dynamic_cast<TBeing*>(victim->riding)) {
    thief->sendTo("Not while that person is mounted!\n\r");
    return FALSE;
  }

  if (thief->noHarmCheck(victim))
    return FALSE;

  // Get weapons
  TGenWeapon* primWeapon =
    dynamic_cast<TGenWeapon*>(thief->heldInPrimHand());
  TGenWeapon* offWeapon =
    dynamic_cast<TGenWeapon*>(thief->heldInSecHand());

  bool primCanStab = primWeapon && primWeapon->canStab();
  bool offCanStab = offWeapon && offWeapon->canStab();

  // Require at least one stabbing weapon
  if (!primCanStab && !offCanStab) {
    thief->sendTo("You need a stabbing weapon to do that.\n\r");
    return FALSE;
  }

  if (thief->getMove() < STAB_MOVE) {
    thief->sendTo("You are too tired to stab.\n\r");
    return FALSE;
  }

  // Deduct move cost and add skill lag upfront (once for entire stab attempt)
  thief->addToMove(-STAB_MOVE);
  thief->addSkillLag(SKILL_STABBING, 0);
  victim->addHated(thief);

  // Check bSuccess for stab skill
  int bKnown = thief->getSkillValue(SKILL_STABBING);
  bool stabSuccess = !victim->awake() || thief->bSuccess(bKnown, SKILL_STABBING);

  if (!stabSuccess) {
    // Failed - show miss message with whichever weapon we would have used
    TGenWeapon* weapon = primCanStab ? primWeapon : offWeapon;
    stabMissMsg(thief, victim, weapon);
    return TRUE;
  }

  // Success - route to appropriate attack function
  if (primCanStab)
    return stabPrimAtk(thief, victim, primWeapon, offWeapon);
  else
    return stabOffAtk(thief, victim, offWeapon);
}

int TBeing::doStab(const char* argument, TBeing* vict) {
  TBeing* victim;
  char namebuf[256];
  int rc;

  if (!doesKnowSkill(SKILL_STABBING)) {
    sendTo("You haven't learned how to stab yet.\n\r");
    return FALSE;
  }
  if (checkBusy()) {
    return FALSE;
  }
  strcpy(namebuf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      if (!(victim = fight())) {
        sendTo("Stab whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (IS_SET(victim->specials.act, ACT_IMMORTAL) || victim->isImmortal()) {
    sendTo("Your stab attempt has no effect on your immortal target.\n\r");
    return FALSE;
  }
  rc = stab(this, victim);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
