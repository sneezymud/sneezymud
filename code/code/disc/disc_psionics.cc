#include <stdio.h>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "combat.h"
#include "person.h"
#include "low.h"
#include "colorstring.h"
#include "obj_portal.h"
#include "monster.h"
#include "disc_psionics.h"

int TBeing::doPTell(const char* arg, bool visible) {
  if (!doesKnowSkill(SKILL_PSITELEPATHY)) {
    sendTo("You are not telepathic!\n\r");
    return FALSE;
  }

  if (getMana() < discArray[SKILL_PSITELEPATHY]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return FALSE;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return FALSE;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo("What a dumb master you have, charmed mobiles can't tell.\n\r");
    return FALSE;
  }

  char n[100];
  char m[MAX_INPUT_LENGTH + 40];
  half_chop(arg, n, m);

  const sstring name = n;
  sstring message = m;

  if (name.empty() || message.empty()) {
    sendTo("Whom do you wish to telepath what??\n\r");
    return FALSE;
  }

  TBeing* vict = nullptr;
  if (!(vict = get_pc_world(this, name, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, name, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, name, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, name, NULL, EXACT_NO))) {
          sendTo(format("You fail to telepath to '%s'\n\r") % name);
          return FALSE;
        }
      }
    }
  }
  if (isPlayerAction(PLR_GODNOSHOUT) && (vict->GetMaxLevel() <= MAX_MORT)) {
    sendTo(
      "You have been sanctioned by the gods and can't telepath to them!!\n\r");
    return FALSE;
  }
  if (this == vict) {
    sendTo("You try to telepath yourself something.\n\r");
    return FALSE;
  }
  if (dynamic_cast<TMonster*>(vict) && !(vict->desc)) {
    sendTo("No-one by that name here.\n\r");
    return FALSE;
  }
  if (!vict->desc) {
    act("$E can't hear you.", TRUE, this, NULL, vict, TO_CHAR);
    return FALSE;
  }
  if (vict->desc->connected) {
    act("$E is editing or writing. Try again later.", TRUE, this, NULL, vict,
      TO_CHAR);
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_MAILING)) {
    sendTo("They are mailing. Try again later.\n\r");
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_BUGGING)) {
    sendTo("They are critiquing the mud.  Try again later.\n\r");
    return FALSE;
  }

  int rc =
    vict->triggerSpecialOnPerson(this, CMD_OBJ_TOLD_TO_PLAYER, message.c_str());
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
    return DELETE_VICT;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;

  if (rc)
    return FALSE;

  learnFromDoing(SKILL_PSITELEPATHY, SILENT_NO, 0);

  const sstring capbuf = vict->pers(this);
  sstring nameBuf = capbuf.cap();

  if (vict->hasColor()) {
    if (hasColorStrings(NULL, capbuf, 2)) {
      if (IS_SET(vict->desc->plr_color, PLR_COLOR_MOBS)) {
        nameBuf =
          colorString(vict, vict->desc, nameBuf, NULL, COLOR_MOBS, FALSE);
      } else {
        nameBuf = format("<p>%s<z>") % colorString(vict, vict->desc, nameBuf,
                                         NULL, COLOR_NONE, FALSE);
      }
    } else {
      nameBuf = format("<p>%s<z>") % nameBuf;
    }
  }

  sendTo(COLOR_COMM,
    format("<G>You telepath %s<z>, \"%s\"\n\r") % vict->getName() %
      colorString(this, desc, message, NULL, COLOR_BASIC, FALSE));

  // we only color the sstring to the victim, so leave this AFTER
  // the stuff we send to the teller.
  message.convertStringColor("<c>");
  vict->sendTo(COLOR_COMM,
    format("%s telepaths you, \"<c>%s<z>\"\n\r") % nameBuf % message);

  if (vict->desc->m_bIsClient ||
      IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
    vict->desc->clientf(
      format("%d|%s|%s") % CLIENT_TELL %
      colorString(vict, vict->desc, nameBuf, NULL, COLOR_NONE, FALSE) %
      colorString(vict, vict->desc, format("<c>%s<z>") % message, NULL,
        COLOR_NONE, FALSE));
  }

  // set up last teller for reply's use
  // If it becomes a "someone tells you", ignore
  if (vict->desc && isPc() && vict->canSee(this, INFRA_YES) && isPc())
    strncpy(vict->desc->last_teller, this->name.c_str(),
      cElements(vict->desc->last_teller) - 1);

  if (desc && inGroup(*vict))
    desc->talkCount = time(0);

  if (vict->desc &&
      (vict->isPlayerAction(PLR_AFK) ||
        (IS_SET(vict->desc->autobits, AUTO_AFK) && (vict->getTimer() >= 5))))
    act("$N appears to be away from $S terminal at the moment.", TRUE, this, 0,
      vict, TO_CHAR);

  reconcileMana(SKILL_PSITELEPATHY, FALSE);

  return TRUE;
}

int TBeing::doPSay(const char* arg) {
  if (!doesKnowSkill(SKILL_PSITELEPATHY)) {
    sendTo("You are not telepathic!\n\r");
    return FALSE;
  }

  if (getMana() < discArray[SKILL_PSITELEPATHY]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return FALSE;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return FALSE;
  }

  if (desc)
    desc->talkCount = time(0);

  for (; isspace(*arg); arg++)
    ;

  if (!*arg)
    sendTo("Yes, but WHAT do you want to say telepathically?\n\r");
  else {
    learnFromDoing(SKILL_PSITELEPATHY, SILENT_NO, 0);

    const sstring message = arg;

    sendTo(COLOR_COMM,
      format("<g>You think to the room, <z>\"%s%s\"\n\r") %
        colorString(this, desc, message, NULL, COLOR_BASIC, FALSE) % norm());

    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* tmp = *(it++);
      TBeing* mob = nullptr;
      Descriptor* d = nullptr;

      if (!(mob = dynamic_cast<TBeing*>(tmp)))
        continue;

      if (!(d = mob->desc) || mob == this)
        continue;

      const auto capbuf = sstring(mob->pers(this)).cap();
      sstring tmpbuf =
        colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE);

      if (mob->isPc()) {
        if (hasColorStrings(NULL, capbuf, 2)) {
          if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
            tmpbuf =
              colorString(mob, mob->desc, capbuf, NULL, COLOR_MOBS, FALSE);
            mob->sendTo(COLOR_COMM, format("%s thinks, \"%s%s\"\n\r") % tmpbuf %
                                      message % mob->norm());
            if (d->m_bIsClient ||
                IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              const sstring garbedBuf =
                colorString(this, mob->desc, message, NULL, COLOR_NONE, FALSE);
              d->clientf(format("%d|%s|%s") % CLIENT_SAY % tmpbuf % garbedBuf);
            }
          } else {
            mob->sendTo(COLOR_COMM,
              format("<c>%s thinks, <z>\"%s\"\n\r") % tmpbuf % message);
            if (d->m_bIsClient ||
                IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              const sstring garbedBuf =
                colorString(this, mob->desc, message, NULL, COLOR_NONE, FALSE);
              d->clientf(
                format("%d|%s|%s") % CLIENT_SAY %
                colorString(this, mob->desc, format("<c>%s<z>") % tmpbuf, NULL,
                  COLOR_NONE, FALSE) %
                garbedBuf);
            }
          }
        } else {
          mob->sendTo(COLOR_COMM,
            format("<c>%s thinks, <z>\"%s\"\n\r") % tmpbuf % message);
          if (d->m_bIsClient ||
              IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
            const sstring garbedBuf =
              colorString(this, mob->desc, message, NULL, COLOR_NONE, FALSE);
            d->clientf(format("%d|%s|%s") % CLIENT_SAY %
                       colorString(this, mob->desc, format("<c>%s<z>") % tmpbuf,
                         NULL, COLOR_NONE, FALSE) %
                       garbedBuf);
          }
        }
      } else {
        mob->sendTo(COLOR_COMM,
          format("%s thinks, \"%s\"\n\r") % getName().cap() %
            colorString(this, mob->desc, message, NULL, COLOR_COMM, FALSE));
        if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
          d->clientf(
            format("%d|%s|%s") % CLIENT_SAY % getName().cap() %
            colorString(this, mob->desc, message, NULL, COLOR_NONE, FALSE));
        }
      }
    }

    // everyone needs to see the say before the response gets triggered
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* tmp = *(it++);
      auto* mob = dynamic_cast<TBeing*>(tmp);
      if (!mob)
        continue;

      if (mob == this)
        continue;

      if (isPc() && !mob->isPc()) {
        auto* tmons = dynamic_cast<TMonster*>(mob);
        tmons->aiSay(this, nullptr);
        int rc = tmons->checkResponses(this, nullptr, message, CMD_SAY);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tmons;
          tmons = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
    }
  }

  reconcileMana(SKILL_PSITELEPATHY, FALSE);

  return TRUE;
}

void TBeing::doPShout(const sstring& message) {
  if (!doesKnowSkill(SKILL_PSITELEPATHY)) {
    sendTo("You are not telepathic!\n\r");
    return;
  }

  if (getMana() < discArray[SKILL_PSITELEPATHY]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return;
  }

  if (message.empty()) {
    sendTo("What do you wish to broadcast to the world?\n\r");
    return;
  } else {
    learnFromDoing(SKILL_PSITELEPATHY, SILENT_NO, 0);

    sendTo(COLOR_SPELLS,
      format("You telepathically send the message, \"%s<z>\"\n\r") % message);

    for (Descriptor* i = descriptor_list; i; i = i->next) {
      if (i->character && (i->character != this) && !i->connected &&
          !i->character->checkSoundproof() &&
          (dynamic_cast<TMonster*>(i->character) ||
            (!IS_SET(i->autobits, AUTO_NOSHOUT)) ||
            !i->character->isPlayerAction(PLR_GODNOSHOUT))) {
        i->character->sendTo(COLOR_SPELLS,
          format(
            "Your mind is flooded with a telepathic message from %s.\n\r") %
            getName());
        i->character->sendTo(COLOR_SPELLS,
          format("The message is, \"%s%s\"\n\r") % message %
            i->character->norm());
      }
    }
  }

  reconcileMana(SKILL_PSITELEPATHY, FALSE);
}

void TBeing::doTelevision(const char* arg) {
  int target;
  char buf1[128];
  TBeing* vict;
  bool visible = TRUE;

  if (!doesKnowSkill(SKILL_TELE_VISION)) {
    sendTo("You have not yet mastered psionics well enough to do that.\n\r");
    return;
  }

  if (getMana() < discArray[SKILL_TELE_VISION]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return;
  }

  if (!*arg) {
    sendTo("Whom do you wish to television??\n\r");
    return;
  } else if (!(vict = get_pc_world(this, arg, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, arg, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, arg, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, arg, NULL, EXACT_NO))) {
          sendTo(format("You can't sense '%s' anywhere.\n\r") % arg);
          return;
        }
      }
    }
  }

  if (vict->isImmortal() || vict->specials.act & ACT_IMMORTAL) {
    nothingHappens(SILENT_YES);
    act("Look through the eyes of an immortal?  I think not!", false, this, 0,
      0, TO_CHAR);
    return;
  }

  target = vict->roomp->number;

  if (target == Room::NOCTURNAL_STORAGE || target == Room::VOID ||
      target == Room::IMPERIA || target == Room::HELL ||
      target == Room::STORAGE || target == Room::POLY_STORAGE ||
      target == Room::CORPSE_STORAGE || target == Room::Q_STORAGE ||
      target == Room::DONATION || target == Room::DUMP) {
    nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.", false, this, 0, 0, TO_CHAR);
    return;
  }

  reconcileMana(SKILL_TELE_VISION, FALSE);

  if (bSuccess(SKILL_TELE_VISION)) {
    sprintf(buf1, "You peer through the eyes of %s and see...",
      vict->getName().c_str());
    act(buf1, FALSE, this, 0, 0, TO_CHAR);

    sprintf(buf1, "%d look", target);
    doAt(buf1, true);

    return;
  } else {
    sprintf(buf1, "You can't seem to get a handle on %s's mind.",
      vict->getName().c_str());
    act(buf1, FALSE, this, 0, 0, TO_CHAR);

    return;
  }

  return;
}

void TBeing::doMindfocus(const char*) {
  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo("You are already focusing your mind.\n\r");
    return;
  }

  if (getMana() < discArray[SKILL_MIND_FOCUS]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return;
  }

  int bKnown = getSkillValue(SKILL_MIND_FOCUS);
  affectedData aff;

  if (bSuccess(bKnown, SKILL_MIND_FOCUS)) {
    act("You begin to focus your mind on regenerating your psionic powers.",
      TRUE, this, NULL, NULL, TO_CHAR);

    aff.type = SKILL_MIND_FOCUS;
    aff.level = bKnown;
    aff.duration =
      durationModify(SKILL_MIND_FOCUS, 4 * Pulse::UPDATES_PER_MUDHOUR);
    aff.location = APPLY_NONE;
    affectTo(&aff, -1);
  } else {
    act("You try to focus your mind, but you can't concentrate.", TRUE, this,
      NULL, NULL, TO_CHAR);
  }

  reconcileMana(SKILL_MIND_FOCUS, FALSE);

  return;
}

TBeing* psiAttackChecks(TBeing* caster, spellNumT sk, const char* tString) {
  char tTarget[256];
  TObj* tobj = NULL;
  TBeing* tVictim = NULL;

  if (caster->checkBusy())
    return NULL;

  if (!caster->doesKnowSkill(sk)) {
    caster->sendTo(
      format("You do not have the skill to use %s.\n\r") % discArray[sk]->name);
    return NULL;
  }

  if (caster->getMana() < discArray[sk]->minMana) {
    caster->sendTo("You don't have enough mana.\n\r");
    return NULL;
  }

  if (caster->affectedBySpell(SKILL_MIND_FOCUS)) {
    caster->sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return NULL;
  }

  if (tString && *tString) {
    strcpy(tTarget, tString);
    generic_find(tTarget, FIND_CHAR_ROOM, caster, &tVictim, &tobj);
  } else if (caster->fight()) {
    tVictim = caster->fight();
  }

  if (!tVictim) {
    caster->sendTo(
      format("Who do you want to use %s on?\n\r") % discArray[sk]->name);
    return NULL;
  }

  if (caster->checkPeaceful(
        "You feel too peaceful to contemplate violence here.\n\r") ||
      tVictim->isImmortal() || tVictim->inGroup(*caster))
    return NULL;

  caster->reconcileMana(sk, FALSE);

  return tVictim;
}

void psiAttackFailMsg(TBeing* ch, TBeing* tVictim) {
  act("You fail to breach $N's mind with your psionic powers.", FALSE, ch, NULL,
    tVictim, TO_CHAR);
  act(
    "You feel a malevolent psionic power emanating from $n towards you, but it "
    "quickly dissipates.",
    TRUE, ch, NULL, tVictim, TO_VICT);
}

int TBeing::doPsiblast(const char* tString) {
  // decreases int/wis/foc
  TBeing* tVictim = NULL;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSI_BLAST, tString)))
    return FALSE;

  int bKnown = getSkillValue(SKILL_PSI_BLAST);
  int tDamage = 0;

  if (bSuccess(bKnown, SKILL_PSI_BLAST)) {
    act("You send a blast of psionic power towards $N!", FALSE, this, NULL,
      tVictim, TO_CHAR);

    act("A look of shocked pain appears on $N's face.", TRUE, this, NULL,
      tVictim, TO_CHAR);
    act("$n sends a blast of psionic power into your mind.", TRUE, this, NULL,
      tVictim, TO_VICT);
    act("A look of shocked pain appears on $N's face as $n glares at $M.", TRUE,
      this, NULL, tVictim, TO_NOTVICT);

    if (!tVictim->affectedBySpell(SKILL_PSI_BLAST)) {
      affectedData aff;
      int count = 0;

      // I do a success roll for each affect to mix things up a bit

      if (bSuccess(bKnown, SKILL_PSI_BLAST)) {
        aff.type = SKILL_PSI_BLAST;
        aff.level = bKnown;
        aff.duration = durationModify(SKILL_PSI_BLAST,
          (bKnown / 10) * Pulse::UPDATES_PER_MUDHOUR);
        aff.location = APPLY_INT;
        aff.modifier = -(::number(bKnown / 3, bKnown / 2));
        tVictim->affectTo(&aff, -1);
        ++count;
      }

      if (bSuccess(bKnown, SKILL_PSI_BLAST)) {
        aff.type = SKILL_PSI_BLAST;
        aff.level = bKnown;
        aff.duration = durationModify(SKILL_PSI_BLAST,
          (bKnown / 10) * Pulse::UPDATES_PER_MUDHOUR);
        aff.location = APPLY_WIS;
        aff.modifier = -(::number(bKnown / 3, bKnown / 2));
        tVictim->affectTo(&aff, -1);
        ++count;
      }

      if (bSuccess(bKnown, SKILL_PSI_BLAST)) {
        aff.type = SKILL_PSI_BLAST;
        aff.level = bKnown;
        aff.duration = durationModify(SKILL_PSI_BLAST,
          (bKnown / 10) * Pulse::UPDATES_PER_MUDHOUR);
        aff.location = APPLY_FOC;
        aff.modifier = -(::number(bKnown / 3, bKnown / 2));
        tVictim->affectTo(&aff, -1);
        ++count;
      }

      if (count) {
        act("$N seems to have suffered a temporary disorientation.", TRUE, this,
          NULL, tVictim, TO_CHAR);
        act("You seem to be suffering from a temporary disorientation.", TRUE,
          this, NULL, tVictim, TO_VICT);
        act("$N seems to have suffered a temporary disorientation.", TRUE, this,
          NULL, tVictim, TO_NOTVICT);
      }
    }

    tDamage = getSkillDam(tVictim, SKILL_PSI_BLAST,
      getSkillLevel(SKILL_PSI_BLAST), getAdvLearning(SKILL_PSI_BLAST));
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_PSI_BLAST);

  addSkillLag(SKILL_PSI_BLAST, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  return TRUE;
}

int TBeing::doMindthrust(const char* tString) {
  TBeing* tVictim = NULL;

  if (!(tVictim = psiAttackChecks(this, SKILL_MIND_THRUST, tString)))
    return FALSE;

  int bKnown = getSkillValue(SKILL_MIND_THRUST);
  int tDamage = 0;

  if (bSuccess(bKnown, SKILL_MIND_THRUST)) {
    act("You use your psionic powers to stab at $N's mind!", FALSE, this, NULL,
      tVictim, TO_CHAR);

    act("$N winces in pain.", TRUE, this, NULL, tVictim, TO_CHAR);
    act("$n squints at you, causing a sharp stabbing pain in your mind.", TRUE,
      this, NULL, tVictim, TO_VICT);
    act("$n squints at $N causing $M to wince suddenly.", TRUE, this, NULL,
      tVictim, TO_NOTVICT);
    tDamage = getSkillDam(tVictim, SKILL_MIND_THRUST,
      getSkillLevel(SKILL_MIND_THRUST), getAdvLearning(SKILL_MIND_THRUST));

    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), FALSE);
    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), FALSE);
    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), FALSE);
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);
  addSkillLag(SKILL_MIND_THRUST, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  return TRUE;
}

int TBeing::doPsycrush(const char* tString) {
  // blindness and/or flee
  TBeing* tVictim = NULL;
  int doflee = 0;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSYCHIC_CRUSH, tString)))
    return FALSE;

  int bKnown = getSkillValue(SKILL_PSYCHIC_CRUSH);
  int tDamage = 0;
  int level = getSkillLevel(SKILL_PSYCHIC_CRUSH);

  if (bSuccess(bKnown, SKILL_PSYCHIC_CRUSH)) {
    act("You reach out with your psionic powers and CRUSH $N's psyche!", FALSE,
      this, NULL, tVictim, TO_CHAR);

    if (bSuccess(bKnown / 4, SKILL_PSYCHIC_CRUSH) &&
        !tVictim->affectedBySpell(SPELL_BLINDNESS) &&
        !tVictim->isAffected(AFF_TRUE_SIGHT) &&
        !tVictim->isAffected(AFF_CLARITY) &&
        !isNotPowerful(tVictim, level, SPELL_BLINDNESS, SILENT_YES)) {
      act("$N's eyes open wide in shock.", TRUE, this, NULL, tVictim, TO_CHAR);
      act(
        "$n's psychic crush is too much for you to bear and your vision goes "
        "blank.",
        TRUE, this, NULL, tVictim, TO_VICT);
      act("$N's eyes open wide in shock as $n glares at $m.", TRUE, this, NULL,
        tVictim, TO_NOTVICT);

      // very short duration
      int duration = durationModify(SKILL_PSYCHIC_CRUSH, Pulse::COMBAT * 2);

      tVictim->rawBlind(level, duration, SAVE_NO);

    } else {
      act("$N winces in pain.", TRUE, this, NULL, tVictim, TO_CHAR);
      act("$n crushes your psyche.", TRUE, this, NULL, tVictim, TO_VICT);
      act("$N winces in pain as $n glares at $m.", TRUE, this, NULL, tVictim,
        TO_NOTVICT);
    }

    if (bSuccess(bKnown, SKILL_PSYCHIC_CRUSH) &&
        !isNotPowerful(tVictim, level, SPELL_FEAR, SILENT_YES)) {  // flee
      doflee = 1;
    }

    tDamage = getSkillDam(tVictim, SKILL_PSYCHIC_CRUSH,
      getSkillLevel(SKILL_PSYCHIC_CRUSH), getAdvLearning(SKILL_PSYCHIC_CRUSH));

  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_PSYCHIC_CRUSH);
  addSkillLag(SKILL_PSYCHIC_CRUSH, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (doflee && tVictim) {
    act("An overwhelming sense of panic causes you to flee.", TRUE, this, NULL,
      tVictim, TO_VICT);
    tVictim->doFlee("");
  }

  return TRUE;
}

int kwaveDamage(TBeing* caster, TBeing* victim) {
  int rc = victim->crashLanding(POSITION_SITTING);
  if (IS_SET_ONLY(rc, DELETE_VICT))
    return DELETE_VICT;

  float wt = combatRound(discArray[SKILL_KINETIC_WAVE]->lag);
  wt += 1;
  victim->addToWait((int)wt);

  if (victim->spelltask)
    victim->addToDistracted(1, FALSE);

  int damage = caster->getSkillDam(victim, SKILL_KINETIC_WAVE,
    caster->getSkillLevel(SKILL_KINETIC_WAVE),
    caster->getAdvLearning(SKILL_KINETIC_WAVE));
  return caster->reconcileDamage(victim, damage, SKILL_KINETIC_WAVE);
}

int TBeing::doKwave(const char* tString) {
  TBeing* tVictim = NULL;
  int rc = 0;

  if (!(tVictim = psiAttackChecks(this, SKILL_KINETIC_WAVE, tString)))
    return FALSE;

  int successfulSkill = bSuccess(SKILL_KINETIC_WAVE);
  int successfulHit = specialAttack(tVictim, SKILL_KINETIC_WAVE);

  if (successfulSkill &&
      (successfulHit == COMPLETE_SUCCESS || successfulHit == PARTIAL_SUCCESS)) {
    act("You set loose a wave of kinetic force at $N!", FALSE, this, NULL,
      tVictim, TO_CHAR);

    if (successfulHit == COMPLETE_SUCCESS) {
      if (tVictim->riding) {
        act("You knock $N off $p.", FALSE, this, tVictim->riding, tVictim,
          TO_CHAR);
        act("$n knocks $N off $p.", FALSE, this, tVictim->riding, tVictim,
          TO_NOTVICT);
        act("$n knocks you off $p.", FALSE, this, tVictim->riding, tVictim,
          TO_VICT);
        tVictim->dismount(POSITION_STANDING);
      }

      act("$n sends $N sprawling with a kinetic force wave!", FALSE, this, 0,
        tVictim, TO_NOTVICT);
      act("You send $N sprawling.", FALSE, this, 0, tVictim, TO_CHAR);
      act("You tumble as $n knocks you over with a kinetic wave.", FALSE, this,
        0, tVictim, TO_VICT, ANSI_BLUE);

      rc = kwaveDamage(this, tVictim);
    }
    // Partial success - damage only
    else {
      int damage = getSkillDam(tVictim, SKILL_KINETIC_WAVE,
        getSkillLevel(SKILL_KINETIC_WAVE), getAdvLearning(SKILL_KINETIC_WAVE));
      rc = reconcileDamage(tVictim, damage, SKILL_KINETIC_WAVE);
    }

    if (IS_SET_ONLY(rc, DELETE_VICT)) {
      delete tVictim;
      tVictim = NULL;
    }
  } else {
    psiAttackFailMsg(this, tVictim);
    rc = reconcileDamage(tVictim, 0, SKILL_KINETIC_WAVE);
  }

  addSkillLag(SKILL_KINETIC_WAVE, rc);

  return TRUE;
}

int TBeing::doPsidrain(const char* tString) {
  TBeing* tVictim = NULL;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSIDRAIN, tString)))
    return FALSE;

  int successfulSkill = bSuccess(SKILL_PSIDRAIN);

  if (!successfulSkill) {
    sendTo("You fail your attempt to drain your victim.\n\r");
    return FALSE;
  }

  // non-mindflayer race
  if (getRace() != RACE_MFLAYER && !isImmortal()) {
    // Adding a lockout
    if (tVictim->affectedBySpell(SKILL_PSIDRAIN)) {
      sendTo("You cannot psionically drain that target again so soon.\n\r");
      return FALSE;
    }

    int perc = ::number(10, 60);

    // reduce amount significantly if victim is a dumb animal
    if (tVictim->isDumbAnimal())
      perc /= 5;

    short int addmana = (int)((manaLimit() * (perc / 2)) / 100.0);
    addmana = min(addmana, tVictim->manaLimit());
    addToMana(addmana);

    colorAct(COLOR_SPELLS,
      "You mentally drain $N, siphoning off $S <Y>energy <z>with your mind!",
      TRUE, this, NULL, tVictim, TO_CHAR);
    colorAct(COLOR_SPELLS,
      "$n mentally drains you, siphoning off your <Y>energy <z>with $s mind!",
      TRUE, this, NULL, tVictim, TO_VICT);
    colorAct(COLOR_SPELLS,
      "$n mentally drains $N, siphoning off $S <Y>energy <z>with $s mind!",
      TRUE, this, NULL, tVictim, TO_NOTVICT);

    int rc = reconcileDamage(tVictim, 100, SKILL_PSIDRAIN);
    tVictim->addToMana(-addmana);
    addSkillLag(SKILL_PSIDRAIN, rc);

    affectedData aff1;
    aff1.type = SKILL_PSIDRAIN;
    aff1.duration = Pulse::UPDATES_PER_MUDHOUR;
    aff1.bitvector = 0;
    aff1.location = APPLY_AGI;
    aff1.modifier = perc / 3;
    tVictim->affectTo(&aff1, -1);

    if (IS_SET_ONLY(rc, DELETE_VICT)) {
      delete tVictim;
      tVictim = NULL;
      REM_DELETE(rc, DELETE_VICT);
    }

    return TRUE;
  } else {
    // check incap or mortal etc
    if (tVictim->getPosition() > POSITION_INCAP) {
      sendTo("You can only drain incapacitated victims.\n\r");
      return FALSE;
    }

    int perc = ::number(15, 25);

    // reduce amount significantly if victim is a dumb animal
    if (tVictim->isDumbAnimal())
      perc /= 5;

    // plus or minus 5 percent for level difference
    if (tVictim->GetMaxLevel() > GetMaxLevel())
      perc += min(5, tVictim->GetMaxLevel() - GetMaxLevel());
    else if (tVictim->GetMaxLevel() < GetMaxLevel())
      perc -= min(5, GetMaxLevel() - tVictim->GetMaxLevel());

    short int addhit = (int)((hitLimit() * perc) / 100.0);
    short int addmana = (int)((manaLimit() * (perc / 2)) / 100.0);

    addhit = min(addhit, tVictim->hitLimit());
    addmana = min(addmana, tVictim->manaLimit());

    addToHit(addhit);
    addToMana(addmana);

    colorAct(COLOR_SPELLS,
      "<k>You wrap your tentacles around $N's head and begin to devour $S "
      "energy.<1>",
      TRUE, this, NULL, tVictim, TO_CHAR);
    colorAct(COLOR_SPELLS,
      "<k>$n wraps $s tentacles around your head and begins to devour your "
      "energy.<1>",
      TRUE, this, NULL, tVictim, TO_VICT);
    colorAct(COLOR_SPELLS,
      "<k>$n wraps $s tentacles around $N's head and begins to devour $S "
      "energy.<1>",
      TRUE, this, NULL, tVictim, TO_NOTVICT);

    int rc = reconcileDamage(tVictim, 100, SKILL_PSIDRAIN);
    addSkillLag(SKILL_PSIDRAIN, rc);

    if (IS_SET_ONLY(rc, DELETE_VICT)) {
      delete tVictim;
      tVictim = NULL;
      REM_DELETE(rc, DELETE_VICT);
    } else {
      psiAttackFailMsg(this, tVictim);
    }

    return TRUE;
  }
}

int TBeing::doDfold(const char* tString) {
  TBeing* tVictim = NULL;
  int rc, location = 0;
  sstring sbuf;

  if (!doesKnowSkill(SKILL_DIMENSIONAL_FOLD)) {
    sendTo("You are not telepathic!\n\r");
    return FALSE;
  }

  if (getMana() < discArray[SKILL_DIMENSIONAL_FOLD]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return FALSE;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return FALSE;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo(
      "What a dumb master you have, charmed mobiles can't open dimensional "
      "folds.\n\r");
    return FALSE;
  }

  // IF tstring is null or "home" then create a fold to the word of recall spot
  const char* home = "home";

  if (!tString || tString == NULL || tString[0] == '\0' ||
      is_abbrev(tString, home)) {
    if (player.hometown)
      location = player.hometown;
    else
      location = Room::CS;
  } else {
    // parse to find victim
    if (!(tVictim = get_pc_world(this, tString, EXACT_YES, INFRA_NO, TRUE))) {
      if (!(tVictim = get_pc_world(this, tString, EXACT_NO, INFRA_NO, TRUE))) {
        sendTo(format("You fail to create a dimensional fold to '%s'\n\r") %
               tString);
        return FALSE;
      }
    }
    // Valid targets?
    if (this == tVictim) {
      sendTo(
        "You can't open a dimensional fold to yourself. How would that even "
        "work?\n\r");
      return FALSE;
    }

    if (tVictim->GetMaxLevel() > MAX_MORT && !isImmortal()) {
      sendTo(
        "Their mind is too powerful for you to open a dimensional fold "
        "to.\n\r");
      return FALSE;
    }

    location = tVictim->in_room;
  }

  TRoom* rp = real_roomp(location);

  if (!rp) {
    vlogf(LOG_BUG, "Attempt to dimensional fold to a NULL room.");
    return FALSE;
  }

  if (!isImmortal() &&
      (rp->isRoomFlag(ROOM_PRIVATE) || rp->isRoomFlag(ROOM_HAVE_TO_WALK) ||
        zone_table[rp->getZoneNum()].enabled == FALSE)) {
    act("Your mind can't seem to focus on that area.", FALSE, this, 0, 0,
      TO_CHAR);
    return FALSE;
  }

  int bKnown = getSkillValue(SKILL_DIMENSIONAL_FOLD);

  if (bSuccess(bKnown, SKILL_DIMENSIONAL_FOLD)) {
    TPortal* tmp_obj = new TPortal(rp);
    tmp_obj->setPortalNumCharges(1);
    tmp_obj->obj_flags.decay_time = 1;
    tmp_obj->name = "portal fold";
    tmp_obj->shortDescr =
      "<k>a darkly pulsating <1><B>bluish<1><k>-<1><P>purple<1><k> portal<1>";
    sbuf = format(
             "<k>a darkly pulsating <1><B>bluish<1><k>-<1><P>purple<1><k> "
             "portal to %s beckons.<1>") %
           rp->name;
    tmp_obj->setDescr(sbuf);

    *roomp += *tmp_obj;

    TPortal* next_tmp_obj = new TPortal(roomp);
    next_tmp_obj->setPortalNumCharges(1);
    next_tmp_obj->obj_flags.decay_time = 1;
    next_tmp_obj->name = "portal fold";
    next_tmp_obj->shortDescr =
      "<k>a darkly pulsating <1><B>bluish<1><k>-<1><P>purple<1><k> portal<1>";
    sbuf = format(
             "<k>a darkly pulsating <1><B>bluish<1><k>-<1><P>purple<1><k> "
             "portal to %s beckons.<1>") %
           roomp->name;
    next_tmp_obj->setDescr(sbuf);

    *rp += *next_tmp_obj;

    act("$n winces in concentration.", TRUE, this, NULL, NULL, TO_ROOM);
    act(format("You wince as you focus on %s.") % rp->name, TRUE, this, NULL,
      NULL, TO_CHAR);
    act("$p suddenly appears out of a swirling mist.", TRUE, this, tmp_obj,
      NULL, TO_ROOM);
    act("$p suddenly appears out of a swirling mist.", TRUE, this, tmp_obj,
      NULL, TO_CHAR);

    rp->sendTo(format("%s suddenly appears out of a swirling mist.\n\r") %
               next_tmp_obj->shortDescr.cap());

    rc = TRUE;
  } else {
    sendTo("Your mind lacks the focus to create a dimensional fold.\n\r");
    rc = FALSE;
  }

  reconcileMana(SKILL_DIMENSIONAL_FOLD, FALSE);
  addSkillLag(SKILL_DIMENSIONAL_FOLD, rc);
  return rc;
}
