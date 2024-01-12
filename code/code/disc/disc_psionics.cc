//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <cstdio>

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

CDPsionics::CDPsionics() :
  CDiscipline(),
  skTelepathy(),
  skTeleSight(),
  skTeleVision(),
  skMindFocus(),
  skPsiBlast(),
  skMindThrust(),
  skPsyCrush(),
  skKineticWave(),
  skMindPreservation(),
  skTelekinesis(),
  skPsiDrain(),
  skDimensionalFold() {}

CDPsionics::CDPsionics(const CDPsionics& a) :
  CDiscipline(a),
  skTelepathy(a.skTelepathy),
  skTeleSight(a.skTeleSight),
  skTeleVision(a.skTeleVision),
  skMindFocus(a.skMindFocus),
  skPsiBlast(a.skPsiBlast),
  skMindThrust(a.skMindThrust),
  skPsyCrush(a.skPsyCrush),
  skKineticWave(a.skKineticWave),
  skMindPreservation(a.skMindPreservation),
  skTelekinesis(a.skTelekinesis),
  skPsiDrain(a.skPsiDrain),
  skDimensionalFold(a.skDimensionalFold) {}

CDPsionics& CDPsionics::operator=(const CDPsionics& a) {
  if (this == &a)
    return *this;
  CDiscipline::operator=(a);
  //  skAdvancedPsionics = a.skAdvancedPsionics;
  skTelepathy = a.skTelepathy;
  skTeleSight = a.skTeleSight;
  skTeleVision = a.skTeleVision;
  skMindFocus = a.skMindFocus;
  skPsiBlast = a.skPsiBlast;
  skMindThrust = a.skMindThrust;
  skPsyCrush = a.skPsyCrush;
  skKineticWave = a.skKineticWave;
  skMindPreservation = a.skMindPreservation;
  skTelekinesis = a.skTelekinesis;
  skPsiDrain = a.skPsiDrain;
  skDimensionalFold = a.skDimensionalFold;

  return *this;
}

CDPsionics::~CDPsionics() {}

int TBeing::doPTell(const char* arg, bool visible) {
  if (!doesKnowSkill(SKILL_PSITELEPATHY)) {
    sendTo("You are not telepathic!\n\r");
    return false;
  }

  if (getMana() < discArray[SKILL_PSITELEPATHY]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return false;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return false;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo("What a dumb master you have, charmed mobiles can't tell.\n\r");
    return false;
  }

  char n[100];
  char m[MAX_INPUT_LENGTH + 40];
  half_chop(arg, n, m);

  const sstring name = n;
  sstring message = m;

  if (name.empty() || message.empty()) {
    sendTo("Whom do you wish to telepath what??\n\r");
    return false;
  }

  TBeing* vict = nullptr;
  if (!(vict = get_pc_world(this, name, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, name, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, name, nullptr, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, name, nullptr, EXACT_NO))) {
          sendTo(format("You fail to telepath to '%s'\n\r") % name);
          return false;
        }
      }
    }
  }
  if (isPlayerAction(PLR_GODNOSHOUT) && (vict->GetMaxLevel() <= MAX_MORT)) {
    sendTo(
      "You have been sanctioned by the gods and can't telepath to them!!\n\r");
    return false;
  }
  if (this == vict) {
    sendTo("You try to telepath yourself something.\n\r");
    return false;
  }
  if (dynamic_cast<TMonster*>(vict) && !(vict->desc)) {
    sendTo("No-one by that name here.\n\r");
    return false;
  }
  if (!vict->desc) {
    act("$E can't hear you.", true, this, nullptr, vict, TO_CHAR);
    return false;
  }
  if (vict->desc->connected) {
    act("$E is editing or writing. Try again later.", true, this, nullptr, vict,
      TO_CHAR);
    return false;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_MAILING)) {
    sendTo("They are mailing. Try again later.\n\r");
    return false;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_BUGGING)) {
    sendTo("They are critiquing the mud.  Try again later.\n\r");
    return false;
  }

  int rc =
    vict->triggerSpecialOnPerson(this, CMD_OBJ_TOLD_TO_PLAYER, message.c_str());
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = nullptr;
    return DELETE_VICT;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;

  if (rc)
    return false;

  learnFromDoing(SKILL_PSITELEPATHY, SILENT_NO, 0);

  const sstring capbuf = vict->pers(this);
  sstring nameBuf = capbuf.cap();

  if (vict->hasColor()) {
    if (hasColorStrings(nullptr, capbuf, 2)) {
      if (IS_SET(vict->desc->plr_color, PLR_COLOR_MOBS)) {
        nameBuf =
          colorString(vict, vict->desc, nameBuf, nullptr, COLOR_MOBS, false);
      } else {
        nameBuf = format("<p>%s<z>") % colorString(vict, vict->desc, nameBuf,
                                         nullptr, COLOR_NONE, false);
      }
    } else {
      nameBuf = format("<p>%s<z>") % nameBuf;
    }
  }

  sendTo(COLOR_COMM,
    format("<G>You telepath %s<z>, \"%s\"\n\r") % vict->getName() %
      colorString(this, desc, message, nullptr, COLOR_BASIC, false));

  // we only color the sstring to the victim, so leave this AFTER
  // the stuff we send to the teller.
  message.convertStringColor("<c>");
  vict->sendTo(COLOR_COMM,
    format("%s telepaths you, \"<c>%s<z>\"\n\r") % nameBuf % message);

  if (vict->desc->m_bIsClient ||
      IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
    vict->desc->clientf(
      format("%d|%s|%s") % CLIENT_TELL %
      colorString(vict, vict->desc, nameBuf, nullptr, COLOR_NONE, false) %
      colorString(vict, vict->desc, format("<c>%s<z>") % message, nullptr,
        COLOR_NONE, false));
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
    act("$N appears to be away from $S terminal at the moment.", true, this, 0,
      vict, TO_CHAR);

  reconcileMana(SKILL_PSITELEPATHY, false);

  return true;
}

int TBeing::doPSay(const char* arg) {
  if (!doesKnowSkill(SKILL_PSITELEPATHY)) {
    sendTo("You are not telepathic!\n\r");
    return false;
  }

  if (getMana() < discArray[SKILL_PSITELEPATHY]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return false;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return false;
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
        colorString(this, desc, message, nullptr, COLOR_BASIC, false) % norm());

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
        colorString(mob, mob->desc, capbuf, nullptr, COLOR_NONE, false);

      if (mob->isPc()) {
        if (hasColorStrings(nullptr, capbuf, 2)) {
          if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
            tmpbuf =
              colorString(mob, mob->desc, capbuf, nullptr, COLOR_MOBS, false);
            mob->sendTo(COLOR_COMM, format("%s thinks, \"%s%s\"\n\r") % tmpbuf %
                                      message % mob->norm());
            if (d->m_bIsClient ||
                IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              const sstring garbedBuf =
                colorString(this, mob->desc, message, nullptr, COLOR_NONE, false);
              d->clientf(format("%d|%s|%s") % CLIENT_SAY % tmpbuf % garbedBuf);
            }
          } else {
            mob->sendTo(COLOR_COMM,
              format("<c>%s thinks, <z>\"%s\"\n\r") % tmpbuf % message);
            if (d->m_bIsClient ||
                IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              const sstring garbedBuf =
                colorString(this, mob->desc, message, nullptr, COLOR_NONE, false);
              d->clientf(
                format("%d|%s|%s") % CLIENT_SAY %
                colorString(this, mob->desc, format("<c>%s<z>") % tmpbuf, nullptr,
                  COLOR_NONE, false) %
                garbedBuf);
            }
          }
        } else {
          mob->sendTo(COLOR_COMM,
            format("<c>%s thinks, <z>\"%s\"\n\r") % tmpbuf % message);
          if (d->m_bIsClient ||
              IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
            const sstring garbedBuf =
              colorString(this, mob->desc, message, nullptr, COLOR_NONE, false);
            d->clientf(format("%d|%s|%s") % CLIENT_SAY %
                       colorString(this, mob->desc, format("<c>%s<z>") % tmpbuf,
                         nullptr, COLOR_NONE, false) %
                       garbedBuf);
          }
        }
      } else {
        mob->sendTo(COLOR_COMM,
          format("%s thinks, \"%s\"\n\r") % getName().cap() %
            colorString(this, mob->desc, message, nullptr, COLOR_COMM, false));
        if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
          d->clientf(
            format("%d|%s|%s") % CLIENT_SAY % getName().cap() %
            colorString(this, mob->desc, message, nullptr, COLOR_NONE, false));
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
          tmons = nullptr;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
    }
  }

  reconcileMana(SKILL_PSITELEPATHY, false);

  return true;
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

  reconcileMana(SKILL_PSITELEPATHY, false);
}

void TBeing::doTelevision(const char* arg) {
  int target;
  char buf1[128];
  TBeing* vict;
  bool visible = true;

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
      if (!(vict = get_char_vis_world(this, arg, nullptr, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, arg, nullptr, EXACT_NO))) {
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

  reconcileMana(SKILL_TELE_VISION, false);

  if (bSuccess(SKILL_TELE_VISION)) {
    sprintf(buf1, "You peer through the eyes of %s and see...",
      vict->getName().c_str());
    act(buf1, false, this, 0, 0, TO_CHAR);

    sprintf(buf1, "%d look", target);
    doAt(buf1, true);

    return;
  } else {
    sprintf(buf1, "You can't seem to get a handle on %s's mind.",
      vict->getName().c_str());
    act(buf1, false, this, 0, 0, TO_CHAR);

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
      true, this, nullptr, nullptr, TO_CHAR);

    aff.type = SKILL_MIND_FOCUS;
    aff.level = bKnown;
    aff.duration =
      durationModify(SKILL_MIND_FOCUS, 4 * Pulse::UPDATES_PER_MUDHOUR);
    aff.location = APPLY_NONE;
    affectTo(&aff, -1);
  } else {
    act("You try to focus your mind, but you can't concentrate.", true, this,
      nullptr, nullptr, TO_CHAR);
  }

  reconcileMana(SKILL_MIND_FOCUS, false);

  return;
}

TBeing* psiAttackChecks(TBeing* caster, spellNumT sk, const char* tString) {
  char tTarget[256];
  TObj* tobj = nullptr;
  TBeing* tVictim = nullptr;

  if (caster->checkBusy())
    return nullptr;

  if (!caster->doesKnowSkill(sk)) {
    caster->sendTo(
      format("You do not have the skill to use %s.\n\r") % discArray[sk]->name);
    return nullptr;
  }

  if (caster->getMana() < discArray[sk]->minMana) {
    caster->sendTo("You don't have enough mana.\n\r");
    return nullptr;
  }

  if (caster->affectedBySpell(SKILL_MIND_FOCUS)) {
    caster->sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return nullptr;
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
    return nullptr;
  }

  if (caster->checkPeaceful(
        "You feel too peaceful to contemplate violence here.\n\r") ||
      tVictim->isImmortal() || tVictim->inGroup(*caster))
    return nullptr;

  caster->reconcileMana(sk, false);

  return tVictim;
}

void psiAttackFailMsg(TBeing* ch, TBeing* tVictim) {
  act("You fail to breach $N's mind with your psionic powers.", false, ch, nullptr,
    tVictim, TO_CHAR);
  act(
    "You feel a malevolent psionic power emanating from $n towards you, but it "
    "quickly dissipates.",
    true, ch, nullptr, tVictim, TO_VICT);
}

int TBeing::doPsiblast(const char* tString) {
  // decreases int/wis/foc
  TBeing* tVictim = nullptr;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSI_BLAST, tString)))
    return false;

  int bKnown = getSkillValue(SKILL_PSI_BLAST);
  int tDamage = 0;

  if (bSuccess(bKnown, SKILL_PSI_BLAST)) {
    act("You send a blast of psionic power towards $N!", false, this, nullptr,
      tVictim, TO_CHAR);

    act("A look of shocked pain appears on $N's face.", true, this, nullptr,
      tVictim, TO_CHAR);
    act("$n sends a blast of psionic power into your mind.", true, this, nullptr,
      tVictim, TO_VICT);
    act("A look of shocked pain appears on $N's face as $n glares at $M.", true,
      this, nullptr, tVictim, TO_NOTVICT);

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
        act("$N seems to have suffered a temporary disorientation.", true, this,
          nullptr, tVictim, TO_CHAR);
        act("You seem to be suffering from a temporary disorientation.", true,
          this, nullptr, tVictim, TO_VICT);
        act("$N seems to have suffered a temporary disorientation.", true, this,
          nullptr, tVictim, TO_NOTVICT);
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
    tVictim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  return true;
}

int TBeing::doMindthrust(const char* tString) {
  TBeing* tVictim = nullptr;

  if (!(tVictim = psiAttackChecks(this, SKILL_MIND_THRUST, tString)))
    return false;

  int bKnown = getSkillValue(SKILL_MIND_THRUST);
  int tDamage = 0;

  if (bSuccess(bKnown, SKILL_MIND_THRUST)) {
    act("You use your psionic powers to stab at $N's mind!", false, this, nullptr,
      tVictim, TO_CHAR);

    act("$N winces in pain.", true, this, nullptr, tVictim, TO_CHAR);
    act("$n squints at you, causing a sharp stabbing pain in your mind.", true,
      this, nullptr, tVictim, TO_VICT);
    act("$n squints at $N causing $M to wince suddenly.", true, this, nullptr,
      tVictim, TO_NOTVICT);
    tDamage = getSkillDam(tVictim, SKILL_MIND_THRUST,
      getSkillLevel(SKILL_MIND_THRUST), getAdvLearning(SKILL_MIND_THRUST));

    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), false);
    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), false);
    if (bSuccess(bKnown / 4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(3, 7), false);
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);
  addSkillLag(SKILL_MIND_THRUST, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  return true;
}

int TBeing::doPsycrush(const char* tString) {
  // blindness and/or flee
  TBeing* tVictim = nullptr;
  int doflee = 0;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSYCHIC_CRUSH, tString)))
    return false;

  int bKnown = getSkillValue(SKILL_PSYCHIC_CRUSH);
  int tDamage = 0;
  int level = getSkillLevel(SKILL_PSYCHIC_CRUSH);

  if (bSuccess(bKnown, SKILL_PSYCHIC_CRUSH)) {
    act("You reach out with your psionic powers and CRUSH $N's psyche!", false,
      this, nullptr, tVictim, TO_CHAR);

    if (bSuccess(bKnown / 4, SKILL_PSYCHIC_CRUSH) &&
        !tVictim->affectedBySpell(SPELL_BLINDNESS) &&
        !tVictim->isAffected(AFF_TRUE_SIGHT) &&
        !tVictim->isAffected(AFF_CLARITY) &&
        !isNotPowerful(tVictim, level, SPELL_BLINDNESS, SILENT_YES)) {
      act("$N's eyes open wide in shock.", true, this, nullptr, tVictim, TO_CHAR);
      act(
        "$n's psychic crush is too much for you to bear and your vision goes "
        "blank.",
        true, this, nullptr, tVictim, TO_VICT);
      act("$N's eyes open wide in shock as $n glares at $m.", true, this, nullptr,
        tVictim, TO_NOTVICT);

      // very short duration
      int duration = durationModify(SKILL_PSYCHIC_CRUSH, Pulse::COMBAT * 2);

      tVictim->rawBlind(level, duration, SAVE_NO);

    } else {
      act("$N winces in pain.", true, this, nullptr, tVictim, TO_CHAR);
      act("$n crushes your psyche.", true, this, nullptr, tVictim, TO_VICT);
      act("$N winces in pain as $n glares at $m.", true, this, nullptr, tVictim,
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
    tVictim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (doflee && tVictim) {
    act("An overwhelming sense of panic causes you to flee.", true, this, nullptr,
      tVictim, TO_VICT);
    tVictim->doFlee("");
  }

  return true;
}

int kwaveDamage(TBeing* caster, TBeing* victim) {
  int rc = victim->crashLanding(POSITION_SITTING);
  if (IS_SET_ONLY(rc, DELETE_VICT))
    return DELETE_VICT;

  float wt = combatRound(discArray[SKILL_KINETIC_WAVE]->lag);
  wt += 1;
  victim->addToWait((int)wt);

  if (victim->spelltask)
    victim->addToDistracted(1, false);

  int damage = caster->getSkillDam(victim, SKILL_KINETIC_WAVE,
    caster->getSkillLevel(SKILL_KINETIC_WAVE),
    caster->getAdvLearning(SKILL_KINETIC_WAVE));
  return caster->reconcileDamage(victim, damage, SKILL_KINETIC_WAVE);
}

int TBeing::doKwave(const char* tString) {
  TBeing* tVictim = nullptr;
  int rc = 0;

  if (!(tVictim = psiAttackChecks(this, SKILL_KINETIC_WAVE, tString)))
    return false;

  int successfulSkill = bSuccess(SKILL_KINETIC_WAVE);
  int successfulHit = specialAttack(tVictim, SKILL_KINETIC_WAVE);

  if (successfulSkill &&
      (successfulHit == COMPLETE_SUCCESS || successfulHit == PARTIAL_SUCCESS)) {
    act("You set loose a wave of kinetic force at $N!", false, this, nullptr,
      tVictim, TO_CHAR);

    if (successfulHit == COMPLETE_SUCCESS) {
      if (tVictim->riding) {
        act("You knock $N off $p.", false, this, tVictim->riding, tVictim,
          TO_CHAR);
        act("$n knocks $N off $p.", false, this, tVictim->riding, tVictim,
          TO_NOTVICT);
        act("$n knocks you off $p.", false, this, tVictim->riding, tVictim,
          TO_VICT);
        tVictim->dismount(POSITION_STANDING);
      }

      act("$n sends $N sprawling with a kinetic force wave!", false, this, 0,
        tVictim, TO_NOTVICT);
      act("You send $N sprawling.", false, this, 0, tVictim, TO_CHAR);
      act("You tumble as $n knocks you over with a kinetic wave.", false, this,
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
      tVictim = nullptr;
    }
  } else {
    psiAttackFailMsg(this, tVictim);
    rc = reconcileDamage(tVictim, 0, SKILL_KINETIC_WAVE);
  }

  addSkillLag(SKILL_KINETIC_WAVE, rc);

  return true;
}

int TBeing::doPsidrain(const char* tString) {
  TBeing* tVictim = nullptr;

  if (!(tVictim = psiAttackChecks(this, SKILL_PSIDRAIN, tString)))
    return false;

  int successfulSkill = bSuccess(SKILL_PSIDRAIN);

  if (!successfulSkill) {
    sendTo("You fail your attempt to drain your victim.\n\r");
    return false;
  }

  // non-mindflayer race
  if (getRace() != RACE_MFLAYER && !isImmortal()) {
    // Adding a lockout
    if (tVictim->affectedBySpell(SKILL_PSIDRAIN)) {
      sendTo("You cannot psionically drain that target again so soon.\n\r");
      return false;
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
      true, this, nullptr, tVictim, TO_CHAR);
    colorAct(COLOR_SPELLS,
      "$n mentally drains you, siphoning off your <Y>energy <z>with $s mind!",
      true, this, nullptr, tVictim, TO_VICT);
    colorAct(COLOR_SPELLS,
      "$n mentally drains $N, siphoning off $S <Y>energy <z>with $s mind!",
      true, this, nullptr, tVictim, TO_NOTVICT);

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
      tVictim = nullptr;
      REM_DELETE(rc, DELETE_VICT);
    }

    return true;
  } else {
    // check incap or mortal etc
    if (tVictim->getPosition() > POSITION_INCAP) {
      sendTo("You can only drain incapacitated victims.\n\r");
      return false;
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
      true, this, nullptr, tVictim, TO_CHAR);
    colorAct(COLOR_SPELLS,
      "<k>$n wraps $s tentacles around your head and begins to devour your "
      "energy.<1>",
      true, this, nullptr, tVictim, TO_VICT);
    colorAct(COLOR_SPELLS,
      "<k>$n wraps $s tentacles around $N's head and begins to devour $S "
      "energy.<1>",
      true, this, nullptr, tVictim, TO_NOTVICT);

    int rc = reconcileDamage(tVictim, 100, SKILL_PSIDRAIN);
    addSkillLag(SKILL_PSIDRAIN, rc);

    if (IS_SET_ONLY(rc, DELETE_VICT)) {
      delete tVictim;
      tVictim = nullptr;
      REM_DELETE(rc, DELETE_VICT);
    } else {
      psiAttackFailMsg(this, tVictim);
    }

    return true;
  }
}

int TBeing::doDfold(const char* tString) {
  TBeing* tVictim = nullptr;
  int rc, location = 0;
  sstring sbuf;

  if (!doesKnowSkill(SKILL_DIMENSIONAL_FOLD)) {
    sendTo("You are not telepathic!\n\r");
    return false;
  }

  if (getMana() < discArray[SKILL_DIMENSIONAL_FOLD]->minMana) {
    sendTo("You don't have enough mana.\n\r");
    return false;
  }

  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo(
      "You can't use psionic powers until you are done focusing your "
      "mind.\n\r");
    return false;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo(
      "What a dumb master you have, charmed mobiles can't open dimensional "
      "folds.\n\r");
    return false;
  }

  // IF tstring is null or "home" then create a fold to the word of recall spot
  const char* home = "home";

  if (!tString || tString == nullptr || tString[0] == '\0' ||
      is_abbrev(tString, home)) {
    if (player.hometown)
      location = player.hometown;
    else
      location = Room::CS;
  } else {
    // parse to find victim
    if (!(tVictim = get_pc_world(this, tString, EXACT_YES, INFRA_NO, true))) {
      if (!(tVictim = get_pc_world(this, tString, EXACT_NO, INFRA_NO, true))) {
        sendTo(format("You fail to create a dimensional fold to '%s'\n\r") %
               tString);
        return false;
      }
    }
    // Valid targets?
    if (this == tVictim) {
      sendTo(
        "You can't open a dimensional fold to yourself. How would that even "
        "work?\n\r");
      return false;
    }

    if (tVictim->GetMaxLevel() > MAX_MORT && !isImmortal()) {
      sendTo(
        "Their mind is too powerful for you to open a dimensional fold "
        "to.\n\r");
      return false;
    }

    location = tVictim->in_room;
  }

  TRoom* rp = real_roomp(location);

  if (!rp) {
    vlogf(LOG_BUG, "Attempt to dimensional fold to a nullptr room.");
    return false;
  }

  if (!isImmortal() &&
      (rp->isRoomFlag(ROOM_PRIVATE) || rp->isRoomFlag(ROOM_HAVE_TO_WALK) ||
        zone_table[rp->getZoneNum()].enabled == false)) {
    act("Your mind can't seem to focus on that area.", false, this, 0, 0,
      TO_CHAR);
    return false;
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

    act("$n winces in concentration.", true, this, nullptr, nullptr, TO_ROOM);
    act(format("You wince as you focus on %s.") % rp->name, true, this, nullptr,
      nullptr, TO_CHAR);
    act("$p suddenly appears out of a swirling mist.", true, this, tmp_obj,
      nullptr, TO_ROOM);
    act("$p suddenly appears out of a swirling mist.", true, this, tmp_obj,
      nullptr, TO_CHAR);

    rp->sendTo(format("%s suddenly appears out of a swirling mist.\n\r") %
               next_tmp_obj->shortDescr.cap());

    rc = true;
  } else {
    sendTo("Your mind lacks the focus to create a dimensional fold.\n\r");
    rc = false;
  }

  reconcileMana(SKILL_DIMENSIONAL_FOLD, false);
  addSkillLag(SKILL_DIMENSIONAL_FOLD, rc);
  return rc;
}
