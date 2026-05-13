#include "ansi.h"
#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_general_weapon.h"
#include "materials.h"
#include "skills.h"
#include "spells.h"
#include "cmd_stab.h"

static int stabOffAtk(TBeing* thief, TBeing* victim, TGenWeapon* weapon);

static int stabPrimAtk(TBeing* thief, TBeing* victim, TGenWeapon* primWeapon,
  TGenWeapon* offWeapon);

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

static void stabMissMsg(TBeing* thief, TBeing* victim, TGenWeapon* weapon) {
  act("You miss your thrust into $N.", FALSE, thief, weapon, victim, TO_CHAR);
  act("$n misses $s thrust into you.", FALSE, thief, weapon, victim, TO_VICT);
  act("$n misses $s thrust into $N.", FALSE, thief, weapon, victim, TO_NOTVICT);
}

static void stabPoisonCheck(TBeing* victim, TGenWeapon* weapon) {
  if (weapon->isPoisoned())
    weapon->applyPoison(victim);
}

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
      victim->incrementBleedStack(limb, duration);
    } else {
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

// Resolves a single stab attempt for one weapon — hit roll through weapon
// spec proc. Shared between primary-hand and offhand attacks.
static int stabCore(TBeing* thief, TBeing* victim, TGenWeapon* weapon) {
  int specResult = thief->specialAttack(victim, SKILL_STABBING, 0, STAT_DEX,
    STAT_SPE, STAT_AGI, STAT_PER, false);
  if (specResult != COMPLETE_SUCCESS && specResult != GUARANTEED_SUCCESS) {
    stabMissMsg(thief, victim, weapon);
    return TRUE;
  }

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

  wearSlotT limb = victim->getPartHit(thief, FALSE);
  if (!victim->hasPart(limb))
    limb = victim->getCritPartHit();

  stabHitMsg(thief, victim, weapon, limb);

  if (thief->reconcileDamage(victim, dam, SKILL_STABBING) == -1)
    return DELETE_VICT;

  stabBleedCheck(thief, victim, weapon, limb);
  stabPoisonCheck(victim, weapon);

  int specRc = weapon->checkSpec(victim, CMD_STAB,
    reinterpret_cast<char*>(limb), thief);
  if (IS_SET_DELETE(specRc, DELETE_ITEM))
    return DELETE_THIS;  // thief destroyed by weapon spec
  if (IS_SET_DELETE(specRc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

static int stabOffAtk(TBeing* thief, TBeing* victim, TGenWeapon* weapon) {
  if (!weapon || !weapon->canStab())
    return FALSE;

  return stabCore(thief, victim, weapon);
}

static int stabPrimAtk(TBeing* thief, TBeing* victim, TGenWeapon* primWeapon,
  TGenWeapon* offWeapon) {
  int rc = stabCore(thief, victim, primWeapon);

  if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS))
    return rc;

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
        rc = stabOffAtk(thief, victim, offWeapon);
        if (IS_SET_DELETE(rc, DELETE_VICT) || IS_SET_DELETE(rc, DELETE_THIS))
          return rc;
      }
    }
  }

  return rc;
}

static int stab(TBeing* thief, TBeing* victim, bool isChain = false) {
  const int STAB_MOVE = 2;

  if (thief == victim) {
    thief->sendTo("Hey now, let's not be stupid.\n\r");
    return FALSE;
  }

  if (thief->checkPeaceful("Naughty, naughty.  None of that here.\n\r"))
    return FALSE;

  TBeing* thiefMount = dynamic_cast<TBeing*>(thief->riding);
  if (thief->riding && !thiefMount) {
    act("You can't stab anyone from atop $p!", false, thief, thief->riding,
      nullptr, TO_CHAR);
    return false;
  }
  if (thiefMount && thief->getSkillValue(SKILL_RIDE) < 80) {
    thief->sendTo("You aren't a skilled enough rider to stab while mounted!\n\r");
    return false;
  }

  TBeing* victimMount = dynamic_cast<TBeing*>(victim->riding);
  bool thiefAirborne = thief->isFlying() || (thiefMount && thiefMount->isFlying());
  bool victimAirborne = victim->isFlying() || (victimMount && victimMount->isFlying());
  // Mid-fight against the same target bypasses the airborne mismatch —
  // engaging a flying opponent puts them in stab range. (Backstab has no
  // such carve-out; its mid-fight gate already requires off-feet, which
  // flying opponents inherently aren't.)
  if (victimAirborne && !thiefAirborne && thief->fight() != victim) {
    thief->sendTo("You can't stab a flying person with your feet on the ground!\n\r");
    return false;
  }

  if (thief->noHarmCheck(victim))
    return FALSE;

  TGenWeapon* primWeapon =
    dynamic_cast<TGenWeapon*>(thief->heldInPrimHand());
  TGenWeapon* offWeapon =
    dynamic_cast<TGenWeapon*>(thief->heldInSecHand());

  bool primCanStab = primWeapon && (primWeapon->canStab() || primWeapon->isPolearm());
  bool offCanStab = offWeapon && offWeapon->canStab();

  if (!primCanStab && !offCanStab) {
    thief->sendTo("You need a stabbing weapon to do that.\n\r");
    return FALSE;
  }

  if (!isChain) {
    if (thief->getMove() < STAB_MOVE) {
      thief->sendTo("You are too tired to stab.\n\r");
      return FALSE;
    }
    thief->addToMove(-STAB_MOVE);
  }

  int bKnown = thief->getSkillValue(SKILL_STABBING);
  bool stabSuccess = !victim->awake() || thief->bSuccess(bKnown, SKILL_STABBING);

  victim->addHated(thief);

  if (!stabSuccess) {
    TGenWeapon* weapon = primCanStab ? primWeapon : offWeapon;
    stabMissMsg(thief, victim, weapon);
    return TRUE;
  }

  if (primCanStab)
    return stabPrimAtk(thief, victim, primWeapon, offWeapon);
  else
    return stabOffAtk(thief, victim, offWeapon);
}

// Chain entry point for backstab. Runs the full stab (preconditions + attack)
// without charging the stab move cost — the chained stab is a follow-on to
// backstab, not an independent action.
int stabChain(TBeing* thief, TBeing* victim) {
  return stab(thief, victim, /*isChain=*/true);
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
  const int lagRc = rc;

  // Clean up locally-resolved victim before the DELETE_THIS early return so
  // combined DELETE_THIS | DELETE_VICT doesn't leak it. lagRc preserves the
  // original DELETE_VICT bit so addSkillLag still applies its kill cap.
  if (IS_SET_DELETE(rc, DELETE_VICT) && !vict) {
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (IS_SET_DELETE(rc, DELETE_THIS))
    return rc;

  if (lagRc)
    addSkillLag(SKILL_STABBING, lagRc);

  return rc;
}
