#include <cstdio>

#include "being.h"
#include "combat.h"
#include "comm.h"
#include "connect.h"
#include "defs.h"
#include "disc_leverage.h"
#include "discipline.h"
#include "enum.h"
#include "extern.h"
#include "handler.h"
#include "immunity.h"
#include "limbs.h"
#include "skills.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"

int TBeing::doBoneBreak(const char* argument, TBeing* vict) {
  int rc;
  TBeing* v;

  if (checkBusy(nullptr)) {
    return false;
  }
  if (getPosition() == POSITION_CRAWLING) {
    sendTo("You can't bone break while crawling.\n\r");
    return false;
  }
  if (!doesKnowSkill(SKILL_BONEBREAK)) {
    sendTo("You know nothing about breaking bones.\n\r");
    return false;
  }

  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, argument))) {
      if (!(v = fight())) {
        sendTo("Break whose bones?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*v)) {
    sendTo("That person doesn't seem to be around.\n\r");
    return false;
  }
  rc = bonebreak(this, v);
  addSkillLag(SKILL_BONEBREAK, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete v;
    v = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

enum bbMissType {
  TYPE_DEFAULT,
  TYPE_MONK,
  TYPE_DEFENSE
};

int bonebreakMiss(TBeing* c, TBeing* v, bbMissType type) {
  switch (type) {
    case TYPE_MONK:
      act("$n tries to grapple $N, but $E cleverly avoids $s moves.", false, c,
        0, v, TO_NOTVICT);
      act(
        "$N cleverly avoids your attempt to get $M into a bone breaking hold.",
        false, c, 0, v, TO_CHAR);
      act("$n tries grapple you into a bone breaking hold but you avoid it.",
        false, c, 0, v, TO_VICT);
      break;
    case TYPE_DEFENSE:
      act("$n attempts to get $N into a bone breaking hold, but fails.", false,
        c, 0, v, TO_NOTVICT);
      act("$N's focus is too great for your bone breaking hold.", false, c, 0,
        v, TO_CHAR);
      act(
        "$n tries get you into a bone breaking hold, but your focus is too "
        "great.",
        false, c, 0, v, TO_VICT);
      break;
    default:  // TYPE_DEFAULT
      act("$n attempts to get $N into a bone breaking hold, but fails.", false,
        c, 0, v, TO_NOTVICT);
      act("$N avoids your puny attempt to get $M into a bone breaking hold.",
        false, c, 0, v, TO_CHAR);
      act("$n tries get you into a bone breaking hold, but you avoid it.",
        false, c, 0, v, TO_VICT);
  }

  c->reconcileDamage(v, 0, SKILL_BONEBREAK);
  return true;
}

int bonebreakHit(TBeing* c, TBeing* victim) {
  wearSlotT slot;
  int casterLevel = c->GetMaxLevel(), dam = ::number(1, casterLevel), rc,
      levelDifference = casterLevel - victim->GetMaxLevel();
  char limb[256], buf[512];

  if (levelDifference > -10) {
    // find a suitable bone to break
    for (slot = (wearSlotT)number(MIN_WEAR, MAX_WEAR - 1);;
         slot = (wearSlotT)number(MIN_WEAR, MAX_WEAR - 1)) {
      if (notBreakSlot(slot, false))
        continue;
      if (!victim->slotChance(slot) || victim->isLimbFlags(slot, PART_BROKEN))
        continue;
      if (!victim->hasPart(slot))
        continue;
      break;
    }

    if (victim->isImmune(IMMUNE_BONE_COND, slot, c->GetMaxLevel())) {
      act("You grab ahold of $N but you just can't seem to break any bones.",
        false, c, 0, victim, TO_CHAR);
      act("$n grabs ahold of you, but is incapable of breaking your bones.",
        false, c, 0, victim, TO_VICT);
      act(
        "$n grabs ahold of $N, but doesn't seem to be able to break any bones.",
        false, c, 0, victim, TO_NOTVICT);

      if ((rc = c->reconcileDamage(victim, 0, SKILL_BONEBREAK)) == -1)
        return DELETE_VICT;
      return true;  // lag them anyway
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    switch (critSuccess(c, SKILL_BONEBREAK)) {
      case CRIT_S_KILL:
        sprintf(buf,
          "You yell out a mighty warcry and rip $N's %s completely off!", limb);
        act(buf, false, c, 0, victim, TO_CHAR);
        sprintf(buf,
          "$n yells out a mighty warcry and rips your %s completely off!",
          limb);
        act(buf, false, c, 0, victim, TO_VICT);
        sprintf(buf,
          "$n yells out a mighty warcry and rips $N's %s completely off!",
          limb);
        act(buf, false, c, 0, victim, TO_NOTVICT);

        victim->makePartMissing(slot, false);
        victim->rawBleed(slot, PERMANENT_DURATION, SILENT_NO,
          CHECK_IMMUNITY_YES);
        if (c->desc)
          c->desc->career.crit_sev_limbs++;
        if (victim->desc)
          victim->desc->career.crit_sev_limbs_suff++;
        break;
      default:
        sprintf(buf, "You grab onto $N's %s and break it!", limb);
        act(buf, false, c, 0, victim, TO_CHAR);
        sprintf(buf, "$n grabs your %s and breaks it.  Ohh the pain!", limb);
        act(buf, false, c, 0, victim, TO_VICT);
        sprintf(buf,
          "You hear a muffled SNAP as $n grabs $N's %s and breaks it!", limb);
        act(buf, false, c, 0, victim, TO_NOTVICT);
        victim->addToLimbFlags(slot, PART_BROKEN);
    }

    victim->dropWeapon(slot);
  } else {
    sprintf(buf,
      "You slam your leg into $N's %s, inflicting considerable damage but "
      "failing to break the bone!",
      limb);
    act(buf, false, c, 0, victim, TO_CHAR);
    sprintf(buf,
      "$n slams $s leg into your %s, inflicting considerable damage but "
      "failing to break the bone.  Ohh the pain!",
      limb);
    act(buf, false, c, 0, victim, TO_VICT);
    sprintf(buf,
      "You hear a meaty THUD as $n slams $s leg into $N's %s, dealing "
      "considerable damage but failing to break any bones!",
      limb);
    act(buf, false, c, 0, victim, TO_NOTVICT);

    affectedData aff1;
    aff1.type = SKILL_BONEBREAK;
    aff1.duration = Pulse::UPDATES_PER_MUDHOUR * 2;
    aff1.bitvector = 0;
    aff1.location = APPLY_SPE;
    aff1.modifier = dam;
    victim->affectTo(&aff1, -1);
  }
  if ((rc = c->reconcileDamage(victim, dam, SKILL_BONEBREAK)) == -1)
    return DELETE_VICT;

  return true;
}

int bonebreak(TBeing* caster, TBeing* victim) {
  int ok = 0, rc = 0;
  wearSlotT slot;
  const int BONEBREAK_MOVE = 15;

  if (caster->checkPeaceful(
        "You feel too peaceful to contemplate violence.\n\r"))
    return false;

  if (victim == caster) {
    caster->sendTo(
      "You consider breaking your own bones, but can't determine how to get "
      "leverage...\n\r");
    return false;
  }
  if (caster->riding) {
    caster->sendTo("You would fall off your mount if you tried that!\n\r");
    return false;
  }
  if (caster->noHarmCheck(victim))
    return false;

  if ((victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) &&
      caster->GetMaxLevel() < victim->GetMaxLevel()) {
    caster->sendTo("Attacking an immortal wouldn't be very smart.\n\r");
    return false;
  }
  if (caster->getMove() < BONEBREAK_MOVE) {
    caster->sendTo("You lack the vitality.\n\r");
    return false;
  }

  if (victim->isFlying()) {
    caster->sendTo("You can't get ahold of something that is flying.\n\r");
    return false;
  }

  if (victim->raceHasNoBones()) {
    caster->sendTo("You can't seem to locate anything worth breaking.\n\r");
    return false;
  }

  if (victim->affectedBySpell(SKILL_BONEBREAK)) {
    caster->sendTo(
      "You can't attempt that maneuver again so soon against this target.\n\r");
    return false;
  }

  // check whether or not there is a bone left to break
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))
      continue;
    if (!victim->slotChance(slot))
      continue;
    if (victim->isLimbFlags(slot, PART_BROKEN)) {
      continue;
    }
    if (!victim->hasPart(slot))
      continue;
    ok++;
  }
  if (!ok) {
    caster->sendTo("You've already broken everything you can.\n\r");
    return false;
  }

  int level = caster->getSkillLevel(SKILL_BONEBREAK);
  int bKnown = caster->getSkillValue(SKILL_BONEBREAK);
  int successfulSkill = caster->bSuccess(bKnown, SKILL_BONEBREAK);
  int successfulHit = caster->specialAttack(victim, SKILL_BONEBREAK);

  caster->addToMove(-(BONEBREAK_MOVE));

  if (successfulSkill && successfulHit && successfulHit != GUARANTEED_FAILURE &&
      !caster->isNotPowerful(victim, level, SKILL_BONEBREAK, SILENT_YES)) {
    if (victim->canCounterMove(bKnown / 3)) {
      SV(SKILL_BONEBREAK);
      rc = bonebreakMiss(caster, victim, TYPE_MONK);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      return true;
    }

    if (victim->canFocusedAvoidance(bKnown / 2)) {
      SV(SKILL_BONEBREAK);
      rc = bonebreakMiss(caster, victim, TYPE_DEFENSE);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      return true;
    }

    return (bonebreakHit(caster, victim));
  } else {
    rc = bonebreakMiss(caster, victim, TYPE_DEFAULT);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  return true;
}
