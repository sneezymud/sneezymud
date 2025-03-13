#include "disc_monk_meditation.h"
#include "handler.h"
#include "being.h"
#include "room.h"
#include "disc_cleric_cures.h"
#include "disc_cleric_aegis.h"
#include "obj_light.h"

int task_yoginsa(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj*) {
  int learn, wohlin_learn;
  int monk_level;

  if (!ch->canMeditate()) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      if (0 && ch->getMove() >= ch->moveLimit() &&
          ch->getHit() >= ch->hitLimit()) {
        ch->sendTo("Your body feels fully refreshed and restored.\n\r");
        ch->stopTask();
        return TRUE;
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
          return FALSE;
        }
      }
      ch->task->status = 0;
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop meditating.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops meditating.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_STAND:
      act("You stop meditating and stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops meditating and stands up.", FALSE, ch, 0, 0, TO_ROOM);
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
      return FALSE;  // eat the command
  }
  return TRUE;
}

static void chiLag(TBeing* ch, int tRc) {
  if (tRc == FALSE || IS_SET_DELETE(tRc, RET_STOP_PARSING))
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
  TObj* tObj = NULL;
  TBeing* tVictim = NULL;

  if (checkBusy())
    return FALSE;

  if (!doesKnowSkill(SKILL_CHI)) {
    sendTo("You lack the ability to chi.\n\r");
    return FALSE;
  }

  if (getMana() < 0) {
    sendTo("You lack the chi.\n\r");
    return FALSE;
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
    return FALSE;
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
      return FALSE;
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
      tVictim = NULL;
    } else if (tObj) {
      delete tObj;
      tObj = NULL;
    }

    REM_DELETE(tRc, DELETE_VICT);
  }
  if (IS_SET_DELETE(tRc, DELETE_THIS))
    return DELETE_THIS;

  return tRc;
}