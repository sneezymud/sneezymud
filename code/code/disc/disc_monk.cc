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
#include "combat.h"
#include "disc_monk.h"
#include "disc_cures.h"
#include "disc_aegis.h"
#include "obj_light.h"

int TBeing::doLeap(const sstring& arg) {
  int rc;

  if (checkBusy()) {
    return false;
  }

  if (arg.empty()) {
    sendTo("Which way do you want to leap?\n\r");
    return false;
  }

  if (fight()) {
    sendTo("You can't leap away while fighting!\n\r");
    return false;
  }

  if (!doesKnowSkill(SKILL_CATLEAP)) {
    sendTo("You do not know the secrets of cat-like leaping.\n\r");
    return false;
  }

  if (roomp->isFallSector()) {
    sendTo("There's no ground beneath you to leap off of here!\n\r");
    return false;
  }

  if (getMove() < 15) {
    sendTo("You're too tired to be jumping around.\n\r");
    return false;
  }

  dirTypeT dir = getDirFromChar(arg);
  if (dir == -1 || !exitDir(dir)) {
    sendTo("You can't go that way.\n\r");
    return false;
  }

  bool was_flying = IS_SET(specials.affectedBy, AFF_FLYING);

  // make them fly
  SET_BIT(specials.affectedBy, AFF_FLYING);
  setPosition(POSITION_FLYING);

  act("You leap into the air!", false, this, 0, 0, TO_CHAR);
  act("$n takes a great leap into the air!", false, this, 0, 0, TO_ROOM);
  addToMove(-15);

  if (!bSuccess(SKILL_CATLEAP)) {
    act("You don't make it very far.", false, this, 0, 0, TO_CHAR);
    act("$n doesn't make it very far.", false, this, 0, 0, TO_ROOM);
    rc = crashLanding(POSITION_SITTING);
  } else {
    rc = doMove(getDirFromChar(arg));
  }

  if (!was_flying)
    REMOVE_BIT(specials.affectedBy, AFF_FLYING);
  return rc;
}

int task_yoginsa(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj*) {
  int learn, wohlin_learn;
  int monk_level;

  if (!ch->canMeditate()) {
    ch->stopTask();
    return false;
  }
  if (ch->utilityTaskCommand(cmd))
    return false;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      if (0 && ch->getMove() >= ch->moveLimit() &&
          ch->getHit() >= ch->hitLimit()) {
        ch->sendTo("Your body feels fully refreshed and restored.\n\r");
        ch->stopTask();
        return true;
      }
      ch->task->calcNextUpdate(pulse, 4 * Pulse::MOBACT);
      if (!ch->task->status) {
        if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
          ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_YOGINSA,
            15);
          if (ch->doesKnowSkill(SKILL_WOHLIN))
            ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_WOHLIN,
              15);

          learn = ch->getSkillValue(SKILL_YOGINSA);
          wohlin_learn = ch->getSkillValue(SKILL_WOHLIN);
          monk_level = ch->getLevel(MONK_LEVEL_IND);

          if (ch->bSuccess(learn, SKILL_YOGINSA) &&
              (::number(1, 100) < (70 + (wohlin_learn / 4)))) {
            // this artifical roll to check for a success is so we can slowly
            // phase out the speed of hp recover without causing a ruckus.
            // lower the .85 lower down and raise the 80 above, keeping the
            // product of the two close to .65 (or whatever
            // stats.damage_modifier is)
            ch->sendTo(
              format("%sMeditating refreshes your inner harmonies!%s\n\r") %
              ch->green() % ch->norm());
            ch->setHit(
              min(ch->getHit() + max(2, (int)(((double)ch->hitGain()) * (.80))),
                (int)ch->hitLimit()));
            ch->setMove(
              min(ch->getMove() + ch->moveGain() / 2, (int)ch->moveLimit()));
            ch->setMana(
              min(ch->getMana() + ch->manaGain() / 2, (int)ch->manaLimit()));

            // salve 20
            if (wohlin_learn > 20 && ::number(0, 100) <= (wohlin_learn - 20)) {
              salve(ch, ch, monk_level * wohlin_learn / 400, 0, SKILL_WOHLIN);
            }

            // cure poison 35
            if (wohlin_learn > 35 && ::number(0, 100) <= (wohlin_learn - 35)) {
              curePoison(ch, ch, monk_level * wohlin_learn / 200, 0,
                SKILL_WOHLIN);
            }

            // sterilize 50
            if (wohlin_learn > 50 && ::number(0, 100) <= (wohlin_learn - 50)) {
              sterilize(ch, ch, 0, 0, SKILL_WOHLIN);
            }

            // cure disease 60
            if (wohlin_learn > 60 && ::number(0, 100) <= (wohlin_learn - 60)) {
              cureDisease(ch, ch, 0, 0, SKILL_WOHLIN);
            }

            // clot 75
            if (wohlin_learn > 75 && ::number(0, 100) <= (wohlin_learn - 75)) {
              clot(ch, ch, 0, 0, SKILL_WOHLIN);
            }

            // reduce hunger/thirst 90
            // bumped these up a bit - Maror Feb 2004
            if (wohlin_learn > 90 && ::number(0, 100) <= (wohlin_learn - 90)) {
              if (ch->getCond(THIRST) <= 5) {
                ch->sendTo("You don't feel quite so thirsty.\n\r");
                ch->gainCondition(THIRST, 4);
              }
              if (ch->getCond(FULL) <= 5) {
                ch->sendTo("You don't feel quite so hungry.\n\r");
                ch->gainCondition(FULL, 4);
              }
            }

            if (ch->ansi()) {
              ch->desc->updateScreenAnsi(CHANGED_HP);
              ch->desc->updateScreenAnsi(CHANGED_MOVE);
              ch->desc->updateScreenAnsi(CHANGED_MANA);
            } else if (ch->vt100()) {
              ch->desc->updateScreenVt100(CHANGED_HP);
              ch->desc->updateScreenVt100(CHANGED_MOVE);
              ch->desc->updateScreenVt100(CHANGED_MANA);
            }
          } else {
          }

        } else {
          ch->sendTo("A magical force in the room stops your meditation\n\r");
          ch->stopTask();
          return false;
        }
      }
      ch->task->status = 0;
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop meditating.", false, ch, 0, 0, TO_CHAR);
      act("$n stops meditating.", false, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_STAND:
      act("You stop meditating and stand up.", false, ch, 0, 0, TO_CHAR);
      act("$n stops meditating and stands up.", false, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_STANDING);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo(
        "You are unable to continue meditating while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST) {
        ch->sendTo("You break your focus...\n\r");
        ch->stopTask();
      }
      return false;  // eat the command
  }
  return true;
}

// this function is meant to be called from brawling commands so monks
// automatically springleap.  They can still force it thru doSpringleap
int TBeing::trySpringleap(TBeing* vict) {
  if (!doesKnowSkill(SKILL_SPRINGLEAP))
    return false;

  return doSpringleap("", false, vict);
}

int TBeing::doSpringleap(sstring argument, bool should_lag, TBeing* vict) {
  TBeing* victim;
  sstring name_buf;
  int rc;

  if (!doesKnowSkill(SKILL_SPRINGLEAP)) {
    sendTo("You don't know how.\n\r");
    return false;
  }
  one_argument(argument, name_buf);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Springleap at whom?\n\r");
        return false;
      }
#if 0
    } else if (!fight()) {
      sendTo("You are not able to initiate combat with a springleap.\n\r");
      return false;
#endif
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }
  rc = springleap(this, victim, should_lag);
  if (rc && should_lag)  // auto springleap doesn't lag
    addSkillLag(SKILL_SPRINGLEAP, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int springleap(TBeing* caster, TBeing* victim, bool should_lag) {
  int i, d = 0;
  int percent;
  spellNumT iSkill = SKILL_SPRINGLEAP;

  if (caster->checkPeaceful(
        "You feel too peaceful to contemplate violence.\n\r"))
    return false;

  if (!caster->doesKnowSkill(iSkill)) {
    caster->sendTo("You don't know how to do that!\n\r");
    return false;
  }

  if (caster->getPosition() > POSITION_SITTING) {
    caster->sendTo("You're not in position for that!\n\r");
    return false;
  }

  if (victim == caster) {
    caster->sendTo("Aren't we funny today...\n\r");
    return false;
  }

  if (caster->noHarmCheck(victim))
    return false;

  percent = 0;
  int bKnown = caster->getSkillValue(SKILL_SPRINGLEAP);

  act("$n does a really nifty move, and aims a leg towards $N.", false, caster,
    0, victim, TO_NOTVICT);
  act("You leap off the $g at $N.", false, caster, 0, victim, TO_CHAR);
  act("$n leaps off the $g at you.", false, caster, 0, victim, TO_VICT);
  caster->reconcileHurt(victim, 0.04);

  if (caster->bSuccess(bKnown + percent, SKILL_SPRINGLEAP)) {
    if ((i = caster->specialAttack(victim, SKILL_SPRINGLEAP)) ||
        (i == GUARANTEED_SUCCESS)) {
      if (victim->getPosition() > POSITION_DEAD) {
        if (!(d = caster->getActualDamage(victim, nullptr,
                caster->getSkillLevel(SKILL_SPRINGLEAP) >> 1, SKILL_KICK))) {
          act(
            "You attempt to kick $N but lose your balance and fall face down "
            "in some mud that has suddenly appeared.",
            false, caster, nullptr, victim, TO_CHAR);
          act(
            "When $n tries to kick you, you quickly make $m fall in some mud "
            "you create.",
            false, caster, nullptr, victim, TO_VICT);
          act("$n falls face down in some mud created by $N.", false, caster,
            nullptr, victim, TO_NOTVICT);
        } else if (caster->willKill(victim, d, SKILL_KICK, true)) {
          act("Your kick at $N's face splits $S head open.", false, caster,
            nullptr, victim, TO_CHAR);
          act("$n aims a kick at your face which splits your head in two.",
            false, caster, nullptr, victim, TO_VICT);
          act("$n neatly kicks $N's head into pieces.", false, caster, nullptr,
            victim, TO_NOTVICT);
          iSkill = DAMAGE_KICK_HEAD;
        } else {
          act("Your kick hits $N in the solar plexus.", false, caster, nullptr,
            victim, TO_CHAR);
          act("You're hit in the solar plexus, wow, this is breathtaking!",
            false, caster, nullptr, victim, TO_VICT);
          act("$n kicks $N in the solar plexus, $N is rendered breathless.",
            false, caster, nullptr, victim, TO_NOTVICT);
        }
      }
      if (caster->reconcileDamage(victim, d, iSkill) == -1)
        return DELETE_VICT;
    } else {
      act("You miss your kick at $N's groin, much to $S relief.", false, caster,
        nullptr, victim, TO_CHAR);
      act("$n misses a kick at your groin, you breathe lighter now.", false,
        caster, nullptr, victim, TO_VICT);
      act("$n misses a kick at $N's groin.", false, caster, nullptr, victim,
        TO_NOTVICT);
      caster->reconcileDamage(victim, 0, SKILL_SPRINGLEAP);
    }
    if (victim)
      victim->addToWait(combatRound(1));
  } else {
    if (victim->getPosition() > POSITION_DEAD) {
      caster->sendTo("You fall on your butt.\n\r");
      act("$n falls on $s butt.", false, caster, 0, 0, TO_ROOM);
      if (caster->reconcileDamage(victim, 0, SKILL_SPRINGLEAP) == -1)
        return DELETE_VICT;
    }
    return true;
  }
  caster->setPosition(POSITION_STANDING);
  caster->updatePos();
  return true;
}

bool TBeing::canCounterMove(int perc) {
  // perc is based on the person doing the move's skill
  // it is somewhat reduced based on arbitray rating of the skill's difficulty

  if (!doesKnowSkill(SKILL_COUNTER_MOVE))
    return false;

  if (!awake() || getPosition() < POSITION_CRAWLING)
    return false;

  int skill = getSkillValue(SKILL_COUNTER_MOVE);
  skill -= perc;

  if (eitherArmHurt())
    skill /= 2;
  if (eitherLegHurt())
    skill = (int)(skill * 0.75);

  skill = max(skill, 1);

  if (!bSuccess(skill, SKILL_COUNTER_MOVE))
    return false;

  return true;
}

int TBeing::doDodge() {
  sendTo("Dodging is not yet supported in this fashion.\n\r");
  return 0;
}

int TBeing::monkDodge(TBeing* v, TThing* weapon, int* dam, int w_type,
  wearSlotT part_hit) {
  char buf[256], type[16];

  // presumes monk is in appropriate position for dodging already

  if (!v->doesKnowSkill(SKILL_JIRIN))
    return false;

  // Balance notes: dodging is in some ways a replacement for Monk's
  // lack of AC
  // In theory, eq alone gets them hit 90% of time in fair fight
  // and "skills" is supposed to lower that to 78%
  // we've already passed through the hits() function when we get here
  // and monks have no intrinsic AC boost
  // So technically, we should be blocking 12/90 = 13.3% of damage
  w_type -= TYPE_HIT;

  // the higher amt is, the more things get blocked
  int amt = 100;

  if (::number(0, 999) >= amt)
    return false;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (v->bSuccess(SKILL_JIRIN)) {
    *dam = 0;

    switch (::number(0, 2)) {
      case 0:
        strcpy(type, "dodge");
        break;
      case 1:
        strcpy(type, "block");
        break;
      case 2:
        strcpy(type, "deflect");
        break;
    }

    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, false, this, 0, v, TO_VICT, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, false, this, 0, v, TO_CHAR, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, true, this, 0, v, TO_NOTVICT);

    return true;
  }
  return false;
}

static void chiLag(TBeing* ch, int tRc) {
  if (tRc == false || IS_SET_DELETE(tRc, RET_STOP_PARSING))
    return;

  ch->addSkillLag(SKILL_CHI, tRc);
}

int TBeing::doChi(const char* tString, TThing* tSucker) {
  // Require 25 in SKILL_CHI for 'chi self'
  // Require 50 in SKILL_CHI and 10 in:  -- for 'chi <person>'
  //   getDiscipline(DISC_MEDITATION_MONK)->getLearnedness() < 25) {
  // Require 100 in SKILL_CHI and 50 in <upper> for 'chi all'

  int tRc = 0;
  char tTarget[256];
  TObj* tObj = nullptr;
  TBeing* tVictim = nullptr;

  if (checkBusy())
    return false;

  if (!doesKnowSkill(SKILL_CHI)) {
    sendTo("You lack the ability to chi.\n\r");
    return false;
  }

  if (getMana() < 0) {
    sendTo("You lack the chi.\n\r");
    return false;
  }

  // Target string specified
  if (tString && *tString)
    strcpy(tTarget, tString);
  // Caster is fighting - default to whoever caster is fighting
  else if (fight()) {
    tVictim = fight();
  }
  // No target can be determined
  else {
    sendTo("Chi what or whom?\n\r");
    return false;
  }

  if (is_abbrev(tTarget, getName()))
    tRc = chiSelf();
  else if (!strcmp(tTarget, "all"))
    tRc = chiRoom();
  else if (tVictim)
    tRc = chiTarget(tVictim);
  else {
    generic_find(tTarget, FIND_CHAR_ROOM | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, this,
      &tVictim, &tObj);

    if (tObj)
      tRc = chiObject(tObj);
    else if (tVictim)
      tRc = chiTarget(tVictim);
    else {
      sendTo("Yes, good.  Use chi...on what or whom?\n\r");
      return false;
    }
  }

  chiLag(this, tRc);

  if (IS_SET_DELETE(tRc, RET_STOP_PARSING))
    REM_DELETE(tRc, RET_STOP_PARSING);

  if (IS_SET_DELETE(tRc, DELETE_VICT)) {
    //    vlogf(LOG_BUG, format("Passive Delete: %s/%s") %  (tVictim ? "tVictim"
    //    : "-") % (tObj ? "tObj" : "-"));

    if (tVictim) {
      delete tVictim;
      tVictim = nullptr;
    } else if (tObj) {
      delete tObj;
      tObj = nullptr;
    }

    REM_DELETE(tRc, DELETE_VICT);
  }
  if (IS_SET_DELETE(tRc, DELETE_THIS))
    return DELETE_THIS;

  return tRc;
}
