//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "obj_base_weapon.h"
#include "being.h"

void stop_trance_of_blades(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You suddenly snap out of your trance.",
	FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
    act("$n suddenly snaps out of $s trance.",
	FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

//void TBaseWeapon::tranceOfBladesPulse(TBeing *ch, TThing *)
//  {
//  stop_trance_of_blades(ch);
//  return;
//  }


int task_trance_of_blades(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TThing *o = NULL;
  int chance = 0;
  // sanity check
  if (ch->isLinkdead() ||
      (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_trance_of_blades(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    ch->task->calcNextUpdate(pulse, PULSE_MOBACT);
    if (!(o = ch->heldInPrimHand())) {
      act("Loss of your weapon causes you to snap out of your trance.",
          FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
      act("Loss of $s weapon causes $n to snap out of $s trance.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      ch->addToWait(combatRound(2));
      ch->cantHit += ch->loseRound(1);
      return FALSE;
    }   
    if (ch->getCombatMode() == ATTACK_BERSERK) {
      act("Berserking causes you to snap out of your trance.",
          FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
      act("Berserking causes $n to snap out of $s trance.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      ch->addToWait(combatRound(2));
      ch->cantHit += ch->loseRound(1);
      return FALSE;
    }   
    if (!ch->canUseArm(HAND_PRIMARY)) {
      act("Your injured arm causes you to snap out of your trance.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n's injured arm causes $m to snap out of $s trance.",
          FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      ch->addToWait(combatRound(2));
      ch->cantHit += ch->loseRound(1);
      return FALSE;
    }
    if (ch->getMove() < 30) {
      act("Your fatigue causes you to snap out of your trance.",
	  FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
      act("$n's fatigue causes $m to snap out of $s trance.",
	  FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      ch->addToWait(combatRound(2));
      ch->cantHit += ch->loseRound(1);
      return FALSE;      
    }
    ch->addToMove(-10);
    chance = 150 - ch->getSkillValue(SKILL_TRANCE_OF_BLADES);
    if (!(ch->attackers)) chance *= 2;
    chance = (int)((float) chance * ch->plotStat(STAT_CURRENT, STAT_FOC, 1.25, 0.80, 1.00));
    if (chance > ::number(0,999)) {
      act("Your concentration has been lost, and you snap out of your defensive trance.",
	  FALSE, ch, 0, 0, TO_CHAR, ANSI_YELLOW);
      act("$n loses $s concentration and snaps out of $s defensive trance.",
	  FALSE, ch, 0, 0, TO_ROOM, ANSI_YELLOW);
      ch->stopTask();
      ch->addToWait(combatRound(2));
      ch->cantHit += ch->loseRound(1);
      return FALSE;
    }
    if (!(::number(0,2)) || (ch->attackers))
      act("Your focus is good and you are able to maintain your defensive trance.",
	  FALSE, ch, 0, 0, TO_CHAR);
    return FALSE;
  case CMD_ABORT:
  case CMD_STOP:
    act("You slowly come out of your trance.",
	FALSE, ch, o, 0, TO_CHAR);
    act("$n slowly comes out of $s trance.", FALSE, ch, o, 0, TO_ROOM);
    ch->stopTask();
    ch->addToWait(combatRound(2));
    ch->cantHit += ch->loseRound(1);
    break;
  case CMD_TASK_FIGHTING:
    //    act("Your $o becomes a blur as you concentrate on your defensive trance.", FALSE, ch, o, 0, TO_CHAR);
    //    act("$n's $o becomes a blur as $e concentrates on $s defensive trance.", FALSE, ch, o, 0, TO_ROOM);
    break;
  default:
    if (cmd < MAX_CMD_LIST)
      warn_busy(ch);
    break;                    // eat the command
  }
  return TRUE;
}

