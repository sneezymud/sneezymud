#include "handler.h"
#include "being.h"
#include "disease.h"
#include "combat.h"
#include "disc_cleric_cures.h"
#include "disc_deikhan.h"

// DEIKHAN AURAS
//
// For each aura we'll use a skill that is invoked, by the deikhan, using the
// 'aura' command. Once the aura is invoked the deikhan will receive a buff that
// based on the pulse proc every 3.6 seconds will proc the aura buff on
// groupmates.
//
// The skill for a specific aura should be in the form:
//    SKILL_AURA_MIGHT
// and should look like this in the 'score' command
// Projecting: Aura of Might.  Approx. duration : 1 day, 3 hours, 39 minutes
// The duration of this should be long or permanent.
//
// The buff applied by the proc for that aura should then be:
//    SPELL_AURA_MIGHT
// and should look like this in 'score'
// Affected: Aura of Might.  Approx. duration : 2 minutes
// The buff itself should be very short lived and consistent duration across
// auras The idea being if you leave the room without the deikhan you will lose
// the aura _almost_ immediately
//

// These are the hooks that are supported now though others are possible.
// Generally CMD_GENERIC_PULSE should be used for skills to proc the aura.
//
// BALANCE NOTE: While possible to use the skill with other hooks to proc on the
// deikhan only to make some affects that are more advanced and not desired on
// the entire groupt is preferable to proc on spells and check if the being is a
// deikhan. This allows multiple deikhans in a group to get all the benefits of
// an aura even if they aren't projecting it.

// cmd == CMD_GENERIC_PULSE
//    This is the generic pulse ever 3.6 seconds
//    We'll use this to proc short term buffs or regen via the aura

// cmd == CMD_BEING_HIT
//    Hook into when a someone hits

// cmd = CMD_BEING_BEEN_HIT
//    Hook into when you a person with a buff or aura is hit

const int MAX_AURA = 5;
spellNumT auraArray[MAX_AURA] = {SKILL_AURA_MIGHT, SKILL_AURA_REGENERATION,
  SKILL_AURA_GUARDIAN, SKILL_AURA_VENGEANCE, SKILL_AURA_ABSOLUTION};

const int AURA_DURATION = 9;
const int PROJECTION_DURATION = 4850;

// checkAura is called by various hooks in the code
// similar to how object procs work. This is our entrypoint
// into procing auras or doing cool things via the buffs
int TBeing::checkAura(cmdTypeT cmd, TBeing* t) {
  int rc;

  if (cmd == CMD_GENERIC_PULSE) {
    // First spell affects
    if (affectedBySpell(SPELL_AURA_REGENERATION)) {
      regenAuraPulse(this);
    }

    // deikhans proc the buff onto others
    if (affectedBySpell(SKILL_AURA_MIGHT)) {
      rc = procAuraOfMight(this);
      return rc;
    }
    if (affectedBySpell(SKILL_AURA_REGENERATION)) {
      rc = procAuraOfRegeneration(this);
      return rc;
    }
    if (affectedBySpell(SKILL_AURA_GUARDIAN)) {
      rc = procAuraGuardian(this);
      return rc;
    }
    if (affectedBySpell(SKILL_AURA_VENGEANCE)) {
      rc = procAuraOfVengeance(this);
      return rc;
    }
    if (affectedBySpell(SKILL_AURA_ABSOLUTION)) {
      rc = procAuraOfAbsolution(this);
    }

  } else if (cmd == CMD_BEING_BEEN_HIT) {
    // Nothing using this yet
    return false;
  } else if (cmd == CMD_BEING_HIT) {
    if (affectedBySpell(SPELL_AURA_MIGHT)) {
      rc = auraOfMightOnHit(this, t);
      return rc;
    }
  }
  return false;
}

//
// AURA OF MIGHT
//

// procAuraOfMight is the proc on CMD_GENERIC_PULSE that actually buffs
// the deikhan and party members if the deikhan is projecting this aura
int procAuraOfMight(TBeing* caster) {
  TBeing* tch = NULL;
  TThing* t = NULL;
  short bKnown;
  bKnown = caster->getSkillValue(SKILL_AURA_MIGHT);
  affectedData aff, aff2;

  aff.type = SPELL_AURA_MIGHT;
  aff.level = bKnown;
  aff.duration = AURA_DURATION;
  aff.modifier = 20;
  aff.location = APPLY_STR;
  aff.bitvector = 0;

  aff2.type = SPELL_AURA_MIGHT;
  aff2.level = bKnown;
  aff2.duration = AURA_DURATION;
  aff2.modifier = -40;
  aff2.location = APPLY_ARMOR;
  aff2.bitvector = 0;

  for (StuffIter it = caster->roomp->stuff.begin();
       it != caster->roomp->stuff.end() && (t = *it); ++it) {
    tch = dynamic_cast<TBeing*>(t);
    if (!tch)
      continue;
    if (tch->inGroup(*caster)) {
      tch->affectJoin(tch, &aff, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      tch->affectJoin(tch, &aff2, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      // caster->reconcileHelp(tch, discArray[spell]->alignMod);
    }
  }
  return TRUE;
}

// auraOfMightOnHit processes the onhit proc of group members affected
// by the aura
int auraOfMightOnHit(TBeing* caster, TBeing* vict) {
  int rc, dam;
  if (!caster || !vict)
    return FALSE;
  if (::number(0, 9))
    return FALSE;

  act(
    "Your <r>Aura of Might<z><b> flares and sears $N with <W>Holy Fire<z><b> "
    "as you strike.",
    false, caster, 0, vict, TO_CHAR, ANSI_BLUE);
  act(
    "$n's <r>Aura of Might<z><b> flares and sears you with <W>Holy Fire<z><b>.",
    false, caster, 0, vict, TO_VICT, ANSI_BLUE);
  act(
    "$n's <r>Aura of Might<z><b> flares and sears $N with <W>Holy Fire<z><b>.",
    false, caster, 0, vict, TO_NOTVICT, ANSI_BLUE);

  dam = ::number(5, 25);
  rc = caster->reconcileDamage(vict, dam, DAMAGE_FIRE);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}

//
// AURA OF REGENERATION
//

// procAuraOfRegeneration is the proc on CMD_GENERIC_PULSE to buff
// deikhan and party members
int procAuraOfRegeneration(TBeing* caster) {
  TBeing* tch = NULL;
  TThing* t = NULL;
  short bKnown;
  bKnown = caster->getSkillValue(SKILL_AURA_REGENERATION);
  affectedData aff;

  aff.type = SPELL_AURA_REGENERATION;
  aff.level = bKnown;
  aff.duration = AURA_DURATION;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  for (StuffIter it = caster->roomp->stuff.begin();
       it != caster->roomp->stuff.end() && (t = *it); ++it) {
    tch = dynamic_cast<TBeing*>(t);
    if (!tch)
      continue;
    if (tch->inGroup(*caster)) {
      tch->affectJoin(tch, &aff, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      // caster->reconcileHelp(tch, discArray[spell]->alignMod);
    }
  }
  return TRUE;
}

// regenAuraPulse is handled on pulse to regen those buffed
int regenAuraPulse(TBeing* caster) {
  caster->addToHit(max(1, (int)(caster->hitGain() / 10.0)));
  caster->setMove(
    min(caster->getMove() + caster->moveGain() / 8, (int)caster->moveLimit()));
  caster->gainCondition(THIRST, 1);
  caster->gainCondition(FULL, 1);
  return TRUE;
}

//
// Guardian Aura
//

// procAuraGuardian is the proc on CMD_GENERIC_PULSE that actually buffs
// the deikhan and party members if the deikhan is projecting this aura
int procAuraGuardian(TBeing* caster) {
  TBeing* tch = NULL;
  TThing* t = NULL;
  short bKnown;
  bKnown = caster->getSkillValue(SKILL_AURA_GUARDIAN);
  affectedData aff, aff2;

  aff.type = SPELL_AURA_GUARDIAN;
  aff.level = bKnown;
  aff.duration = AURA_DURATION;
  aff.modifier = 35;
  aff.location = APPLY_HIT;
  aff.bitvector = 0;

  aff2.type = SPELL_AURA_GUARDIAN;
  aff2.level = bKnown;
  aff2.duration = AURA_DURATION;
  aff2.modifier = 12;
  aff2.location = APPLY_PROTECTION;
  aff2.bitvector = 0;

  // Also adds defense in defendRound()

  for (StuffIter it = caster->roomp->stuff.begin();
       it != caster->roomp->stuff.end() && (t = *it); ++it) {
    tch = dynamic_cast<TBeing*>(t);
    if (!tch)
      continue;
    if (tch->inGroup(*caster)) {
      if (tch->hasClass(CLASS_DEIKHAN))
        aff2.modifier += 5;

      tch->affectJoin(tch, &aff, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      tch->affectJoin(tch, &aff2, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      // caster->reconcileHelp(tch, discArray[spell]->alignMod);
    }
  }
  return TRUE;
}

//
// Aura of Vengeance
//

// procAuraOfVengeance is the proc on CMD_GENERIC_PULSE that actually buffs
// the deikhan and party members if the deikhan is projecting this aura
int procAuraOfVengeance(TBeing* caster) {
  TBeing* tch = NULL;
  TThing* t = NULL;
  short bKnown;
  bKnown = caster->getSkillValue(SKILL_AURA_VENGEANCE);
  affectedData aff;

  aff.type = SPELL_AURA_VENGEANCE;
  aff.level = bKnown;
  aff.duration = AURA_DURATION;
  aff.modifier = 2;
  aff.location = APPLY_HITROLL;
  aff.bitvector = 0;

  // also adds atacks and crit chance elsewhere

  for (StuffIter it = caster->roomp->stuff.begin();
       it != caster->roomp->stuff.end() && (t = *it); ++it) {
    tch = dynamic_cast<TBeing*>(t);
    if (!tch)
      continue;
    if (tch->inGroup(*caster)) {
      tch->affectJoin(tch, &aff, AVG_DUR_YES, AVG_EFF_YES, FALSE);
      // caster->reconcileHelp(tch, discArray[spell]->alignMod);
    }
  }
  return TRUE;
}

//
// Aura of Absolution
//

// procAuraOfAbsolution is the proc on CMD_GENERIC_PULSE that actually buffs
// the deikhan and party members if the deikhan is projecting this aura
int procAuraOfAbsolution(TBeing* caster) {
  TBeing* tch = NULL;
  TThing* t = NULL;
  short bKnown;
  bKnown = caster->getSkillValue(SKILL_AURA_ABSOLUTION);
  affectedData aff;

  aff.type = SPELL_AURA_ABSOLUTION;
  aff.level = bKnown;
  aff.duration = AURA_DURATION;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  bool done_warmth = false;

  for (StuffIter it = caster->roomp->stuff.begin();
       it != caster->roomp->stuff.end() && (t = *it); ++it) {
    tch = dynamic_cast<TBeing*>(t);
    if (!tch)
      continue;
    if (tch->inGroup(*caster)) {
      // First we apply the aura buff even though it doesn't do anything yet
      // The experience is consistent with other auras
      tch->affectJoin(tch, &aff, AVG_DUR_YES, AVG_EFF_YES, FALSE);
    }
  }

  if (!::number(0, 3)) {
    for (StuffIter it = caster->roomp->stuff.begin();
         it != caster->roomp->stuff.end() && (t = *it); ++it) {
      tch = dynamic_cast<TBeing*>(t);
      if (!tch)
        continue;
      if (tch->inGroup(*caster)) {
        doAbsolutionAuraAffects(caster, tch, done_warmth);
      }
    }
  }

  // Sometimes if we do nothing we'll still show a warmth message for funsies
  if (!done_warmth && !::number(0, 39)) {
    doWarmth(caster);
  }

  return TRUE;
}

// moving this to it's own function
void doWarmth(TBeing* caster) {
  act("$n's absolution aura emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", 0,
    caster, 0, 0, TO_ROOM);
  act("Your absolution aura emanates a gentle <r>w<R>a<Y>rm<R>t<1><r>h.<1>", 0,
    caster, 0, 0, TO_CHAR);
}

// doAbsolutionAuraAffects
// We'll try to add some randomness but hit all these affects:
// - heal
// - salve
// - clot
// - cure poison
void doAbsolutionAuraAffects(TBeing* caster, TBeing* target,
  bool& done_warmth) {
  // Do Heals
  if (target->getHit() < target->hitLimit() && ::number(0, 3)) {
    if (!done_warmth) {
      doWarmth(caster);
      done_warmth = true;
    }
    // Vindicator level heals
    colorAct(COLOR_SPELLS, "$n glows briefly with an <p>indigo hue<1>.", FALSE,
      target, NULL, 0, TO_ROOM);
    colorAct(COLOR_SPELLS, "You glow briefly with an <p>indigo hue<1>.", FALSE,
      target, NULL, 0, TO_CHAR);

    int hp = caster->getSkillDam(caster, SPELL_HEAL_CRITICAL, 50, 100);
    target->addToHit(hp);
    target->updatePos();
    return;
  }

  // Do Salve
  wearSlotT slot = target->getRandomHurtPart();
  if (slot != WEAR_NOWHERE && ::number(0, 2)) {
    if (!done_warmth) {
      doWarmth(caster);
      done_warmth = true;
    }
    salve(caster, target, 20, 0, SKILL_WOHLIN);
    return;
  }

  // Do Clot
  slot = target->getRandomPart(PART_BLEEDING, FALSE, TRUE);
  if (slot != WEAR_NOWHERE) {
    if (!done_warmth) {
      doWarmth(caster);
      done_warmth = true;
    }
    clot(caster, target, 0, 0, SKILL_WOHLIN);
    return;
  }

  // Do Cure Poison
  if ((target->isAffected(AFF_POISON) ||
        target->affectedBySpell(SPELL_POISON) ||
        target->affectedBySpell(SPELL_POISON_DEIKHAN) ||
        target->hasDisease(DISEASE_FOODPOISON)) &&
      !::number(0, 2)) {
    if (!done_warmth) {
      doWarmth(caster);
      done_warmth = true;
    }

    act(
      "You suddenly start sweating profusely as the poison is extracted from "
      "your body.",
      FALSE, caster, NULL, target, TO_VICT);
    act(
      "$N suddenly starts sweating profusely as the poison is extracted from "
      "$S body.",
      FALSE, caster, NULL, target, TO_ROOM);

    target->affectFrom(SPELL_POISON_DEIKHAN);
    target->affectFrom(SPELL_POISON);
    target->diseaseFrom(DISEASE_FOODPOISON);
    target->diseaseFrom(DISEASE_POISON);

    return;
  }
}

//
// End AURAS
//

// removeAllAuras use when stopping or changing auras
void removeAllAuras(TBeing* vict) {
  int i;

  // iterate through
  for (i = 0; i < MAX_AURA; i++) {
    if (vict->affectedBySpell(auraArray[i])) {
      vict->spellWearOff(auraArray[i], SAFE_NO);
      vict->affectFrom(auraArray[i]);
    }
  }

  return;
}

// projectAura called via aura command
void projectAura(TBeing* caster, spellNumT aura) {
  if (!caster->doesKnowSkill(aura)) {
    caster->sendTo(
      format("You know nothing about %s.\n\r") % discArray[aura]->name);
    return;
  }

  if (caster->affectedBySpell(aura)) {
    caster->sendTo(
      format("You are already projecting an %s.\n\r") % discArray[aura]->name);
    return;
  }

  if (!caster->bSuccess(aura)) {
    caster->sendTo(
      format("You fail to project an %s.\n\r") % discArray[aura]->name);
    return;
  }

  removeAllAuras(caster);
  short bKnown;
  bKnown = caster->getSkillValue(aura);
  affectedData aff;

  caster->sendTo(format("You begin to project an %s.") % discArray[aura]->name);

  aff.type = aura;
  aff.level = bKnown;
  aff.duration = PROJECTION_DURATION;
  aff.location = APPLY_NONE;

  caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);
  caster->addSkillLag(aura, 0);
}

// The "aura" command implementation
void TBeing::doAura(const sstring& arg) {
  // Check to see if they know any auras and tell them if they don't

  // Should the aura be the first arg or should there be subcommands like
  // <aura list> <aura show> <aura project name> <aura stop>
  //
  int i;
  Descriptor* d;

  if (!hasClass(CLASS_DEIKHAN)) {
    sendTo("Only Deikhans can project auras.\n\r");
    return;
  }

  sstring cmd = arg.word(0);

  if (is_abbrev(cmd, "might")) {
    projectAura(this, SKILL_AURA_MIGHT);
    return;
  } else if (is_abbrev(cmd, "regeneration")) {
    projectAura(this, SKILL_AURA_REGENERATION);
    return;
  } else if (is_abbrev(cmd, "guardian")) {
    projectAura(this, SKILL_AURA_GUARDIAN);
    return;
  } else if (is_abbrev(cmd, "vengeance")) {
    projectAura(this, SKILL_AURA_VENGEANCE);
    return;
  } else if (is_abbrev(cmd, "absolution")) {
    projectAura(this, SKILL_AURA_ABSOLUTION);
    return;
  } else if (is_abbrev(cmd, "off") || is_abbrev(cmd, "stop")) {
    removeAllAuras(this);
    return;
  } else if (cmd != "") {
    sendTo("You must specify an aura command.\n\r");
    sendTo("Syntax: aura <*aura name*|off|stop>\n\r");
    return;
  }

  sstring Buf;

  Buf += "Deikhan Auras:\n\r";

  if (!(d = desc))
    return;

  // Show all auras
  for (i = 0; i < MAX_AURA; i++) {
    Buf += format("%s%-22.22s%s %-19.19s") % cyan() %
           discArray[auraArray[i]]->name % norm() %
           how_good(getSkillValue(auraArray[i]));
    // We want to show which aura is in use
    if (affectedBySpell(auraArray[i])) {
      Buf += "projecting";
    }
    Buf += "\n\r";
  }

  d->page_string(Buf);
  return;
}