//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "task_skulk.cc" - Task implementation for the Skulk skill
//
//////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "handler.h"
#include "being.h"
#include "skills.h"
#include "spelltask.h"
#include "disc_thief_stealth.h"

void stop_skulk(TBeing* ch) {
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop skulking.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops skulking.", FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

int task_skulk(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*, TObj*) {
  int learning, pulseRate, timeReduction, duration, bonus, cbsBonus;
  affectedData aff;
  
  // Basic sanity check
  if (ch->isLinkdead() || (ch->in_room < 0)) {
    stop_skulk(ch);
    return FALSE;
  }
  
  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      // Calculate pulse rate based on skill level
      learning = ch->getSkillValue(SKILL_SKULK);
      pulseRate = Pulse::MOBACT * (5 - (learning / 25)); // 1-5 times MOBACT
      pulseRate = max(Pulse::MOBACT, pulseRate); // Minimum of 1x MOBACT
      
      ch->task->calcNextUpdate(pulse, pulseRate);
      
      // Fatigue check - skulking is tiring. Drain scales inversely with
      // skill: a master skulker (5 -> 1) is more efficient.
      ch->addToMove(-skulkMoveCost(learning));
      if (ch->getMove() < 5) {
        ch->sendTo("You are too tired to continue skulking.\n\r");
        act("$n looks exhausted and stops skulking.", TRUE, ch, 0, 0, TO_ROOM);
        ch->stopTask();
        return FALSE;
      }
      
      // Check for success on each pulse
      if (ch->bSuccess(SKILL_SKULK)) {
        CS(SKILL_SKULK);
        
        // More skilled characters progress faster
        timeReduction = 1;
        if (learning > 50) timeReduction++;
        if (learning > 75) timeReduction++;
        
        ch->task->timeLeft -= timeReduction;
        
        if (ch->task->timeLeft <= 0) {
          // Task complete - apply the skulk effect
          
          // Duration based on skill level
          duration = (3 + (learning / 20)) * Pulse::UPDATES_PER_MUDHOUR;
          
          // Bonus based on skill level (1-9)
          bonus = min(9, max(1, learning / 12));
          
          // CBS bonus based on skill level (1-5)
          cbsBonus = min(5, max(1, learning / 20));
          
          // Apply DEX bonus
          aff.type = SKILL_SKULK;
          aff.level = ch->getSkillLevel(SKILL_SKULK);
          aff.duration = duration;
          aff.modifier = bonus;
          aff.location = APPLY_DEX;
          aff.bitvector = 0;
          ch->affectTo(&aff);
          
          // Apply PER bonus
          aff.location = APPLY_PER;
          ch->affectTo(&aff);
          
          // Apply CBS (Can Be Seen) bonus
          aff.modifier = cbsBonus;
          aff.location = APPLY_CAN_BE_SEEN;
          ch->affectTo(&aff);
          
          ch->sendTo("You fade into the shadows as you begin to skulk!\n\r");
          act("$n's movements become incredibly stealthy.", TRUE, ch, 0, 0, TO_ROOM);
          
          ch->stopTask();
          return TRUE;
        } else {
          // Progress messages
          if (ch->task->timeLeft > 5) {
            act("You crouch low to the ground to avoid detection.", FALSE, ch, 0, 0, TO_CHAR);
          } else if (ch->task->timeLeft > 2) {
            act("Your movements are becoming more stealthy.", FALSE, ch, 0, 0, TO_CHAR);
          } else {
            act("Your presence is barely perceptible.", FALSE, ch, 0, 0, TO_CHAR);
          }
        }
      } else {
        CF(SKILL_SKULK);
        // No time reduction on failure. Thin messages to every other
        // failure so low-skill build-up isn't spammy. task->status doubles
        // as the failure counter — once it crosses the cap we abort the
        // attempt rather than draining move forever.
        if ((ch->task->status++ % 2) == 0)
          act("You stumble slightly, but recover your stance.", false, ch, 0, 0, TO_CHAR);

        constexpr int kSkulkMaxFailures = 10;
        if (ch->task->status >= kSkulkMaxFailures) {
          ch->sendTo("You give up trying to find a stealthy posture.\n\r");
          act("$n abandons $s skulking attempt.", true, ch, 0, 0, TO_ROOM);
          ch->stopTask();
          return FALSE;
        }
      }
      return FALSE;
      
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop your skulking.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops skulking.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
      
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't continue skulking while under attack!\n\r");
      stop_skulk(ch);
      break;
      
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;
  }
  return FALSE;
}
