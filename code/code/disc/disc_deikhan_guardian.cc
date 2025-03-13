#include "being.h"
#include "combat.h"
#include "disc_deikhan_guardian.h"

// divineRescue will apply a heal to the target/victim and a short duration buff
// the buff is mainly so the free heal can't be spammed.
void divineRescue(TBeing* caster, TBeing* victim) {
  if (!caster->bSuccess(caster->getSkillValue(SKILL_DIVINE_RESCUE),
        SKILL_DIVINE_RESCUE))
    return;

  if (victim->affectedBySpell(SKILL_DIVINE_RESCUE)) {
    act("Your deities refuse to bless $N again so soon.", FALSE, caster, 0,
      victim, TO_CHAR);
    return;
  }

  affectedData aff;
  int casterlevel = caster->GetMaxLevel();
  int level = caster->getSkillLevel(SKILL_DIVINE_RESCUE);
  int adv_learn = caster->getAdvLearning(SKILL_DIVINE_RESCUE);

  // Max 100 point heal
  int hp = casterlevel + (adv_learn / 2);

  // lets make this similar to the avenger to continue the Deikhan flavor
  act("$N glows with a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", FALSE, caster, 0,
    victim, TO_NOTVICT);
  act("$N glows with a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", FALSE, caster, 0,
    victim, TO_CHAR);
  act("You glow with a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", FALSE, caster, 0,
    victim, TO_VICT);

  // caster->sendTo("You fail the rescue.\n\r");
  act("You are blessed as $n rescues you.", FALSE, caster, 0, victim, TO_VICT);

  act("$N is blessed as $n intercedes on $S behalf.", FALSE, caster, 0, victim,
    TO_NOTVICT);
  // TO_CHAR TO_ROOM
  act("$N is blessed as you intercede on $S behalf.", FALSE, caster, 0, victim,
    TO_CHAR);

  if (victim->getHit() < victim->hitLimit()) {
    caster->reconcileHelp(victim, discArray[SKILL_DIVINE_RESCUE]->alignMod);
    checkFactionHelp(caster, victim);
  }

  aff.type = SKILL_DIVINE_RESCUE;
  aff.level = level;
  aff.duration = Pulse::UPDATES_PER_MUDHOUR / 4;
  aff.modifier = -50;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;
  victim->affectTo(&aff, -1);
  // Now heal
  victim->addToHit(hp);
  victim->updatePos();
}


int synostodweomer(TBeing* caster, TBeing* v, int level, short bKnown) {
  int hitp = 0;
  affectedData aff;
  int casterlevel = caster->GetMaxLevel();

  if (!caster->isImmortal() &&
      caster->checkForSkillAttempt(SPELL_SYNOSTODWEOMER)) {
    act("You are not prepared to try to Snyostodweomer again so soon.", FALSE,
      caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (v->affectedBySpell(SPELL_SYNOSTODWEOMER)) {
    caster->sendTo(COLOR_MOBS,
      format("%s is already affected by Snyostodweomer.\n\r") % v->getName());
    return FALSE;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_SYNOSTODWEOMER)) {
    act("Power rushes through your fingers as you bless $N.", FALSE, caster,
      NULL, v, TO_CHAR);
    act("Power rushes through $n's fingers as $e blesses $N.", TRUE, caster,
      NULL, v, TO_NOTVICT);
    act("Power surges through $n's fingers as $e blesses you.", TRUE, caster,
      NULL, v, TO_VICT);

    if (!caster->isImmortal()) {
      aff.type = AFFECT_SKILL_ATTEMPT;
      aff.location = APPLY_NONE;
      aff.duration = caster->durationModify(SPELL_SYNOSTODWEOMER,
        max(min(casterlevel / 12, 5), 1) * Pulse::UPDATES_PER_MUDHOUR);
      aff.bitvector = 0;
      aff.modifier = SPELL_SYNOSTODWEOMER;
      caster->affectTo(&aff, -1);
    }

    // hps to add to the vict
    int learnedness =
      caster->getDiscipline(DISC_DEIKHAN_GUARDIAN)->getLearnedness();
    // max of 60
    hitp = 10 + (caster->GetMaxLevel() * learnedness / 100);

    if (critSuccess(caster, SPELL_SYNOSTODWEOMER)) {
      CS(SPELL_SYNOSTODWEOMER);
      hitp += 10;
    }

    aff.type = SPELL_SYNOSTODWEOMER;
    aff.level = level;
    // aff.duration = max(min(casterlevel/12, 5), 1) *
    // Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = hitp;
    aff.location = APPLY_HIT;
    aff.bitvector = 0;
    v->affectTo(&aff, -1);

    v->updatePos();
    caster->updatePos();
    caster->reconcileHelp(v, 0.11);
    return SPELL_SUCCESS;
  } else {
    caster->sendTo("Nothing seems to happen.\n\r");
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

int synostodweomer(TBeing* caster, TBeing* v) {
  int level, ret;
  int rc = 0;

  if (!bPassClericChecks(caster, SPELL_SYNOSTODWEOMER))
    return FALSE;

  if (!caster->isImmortal() &&
      caster->checkForSkillAttempt(SPELL_SYNOSTODWEOMER)) {
    act("You are not prepared to try to Snyostodweomer again so soon.", FALSE,
      caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (v->affectedBySpell(SPELL_SYNOSTODWEOMER)) {
    caster->sendTo(COLOR_MOBS,
      format("%s is already affected by Snyostodweomer.\n\r") % v->getName());
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_SYNOSTODWEOMER);

  ret = synostodweomer(caster, v, level,
    caster->getSkillValue(SPELL_SYNOSTODWEOMER));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}
