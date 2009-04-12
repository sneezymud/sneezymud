//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   File containing all skills for spell casters, and classes with
//    casting derivatives. - Russ Russell
//
///////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "combat.h"

int TBeing::doTurn(const char *argument, TBeing *vict)
{
  TBeing *victim;
  char caName[256];
  int percent;

  if (GetMaxLevel() <= MAX_MORT) {
    // turn bracket
    doNotHere();
    return FALSE;
  }

  // turn undead
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  strcpy(caName, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, caName))) {
      if (!(victim = fight())) {
        sendTo("Attempt to turn which minion of the darkness?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (victim == this) {
    sendTo("Now that would be quite pointless...\n\r");
    return FALSE;
  }
  if (!(victim->isUndead() || victim->isDiabolic())) {
    sendTo("They are not a minion of the darkness!\n\r");
    return FALSE;
  }
 /* Russ, check here if cleric has a symbol, and have them grope
    around if they can't find it etc.  Basically, copy it outta
    the cleric spells. */
  percent = ::number(1, 101);
#if 0
  percent = percent - (getWis() * 2) + (victim->getWis() * 2) -
      (getLevel(SHAMAN_LEVEL_IND)) + (victim->GetMaxLevel());
#endif

  if (victim->isDiabolic())
    percent += 50;
  if ((percent > getSkillValue(SKILL_TURN) || (victim->GetMaxLevel() > 50)) &&
      (GetMaxLevel() <= 50)) {
    if (victim->getPosition() > POSITION_DEAD) {
      act("$n attempts to confuse $N, but fails.", FALSE, this, 0, victim, TO_NOTVICT);
      act("You attempt to confuse $N, but fail.", FALSE, this, 0, victim, TO_CHAR);
      act("$n has just tried to confuse you!", FALSE, this, 0, victim, TO_VICT);
      reconcileDamage(victim, 0, SKILL_TURN);
    }
  } else {
    act("$n tries to confuse $N.", FALSE, this, 0, victim, TO_NOTVICT);
    act("$n tries to confuse you.", FALSE, this, 0, victim, TO_VICT);
    act("You try to confuse $N.", FALSE, this, 0, victim, TO_CHAR);
#if 0
    if (isGood()) {
      act("$n mind focus sends $N reeling backwards in screams of agony!", FALSE, this, 0, victim, TO_NOTVICT);
      act("You scream in utter agony as $n's mind penetrates your soul!", FALSE, this, 0, victim, TO_VICT);
      act("The loa send $N reeling backwards!", FALSE, this, 0, victim, TO_CHAR);

      if ((percent < 5) || (GetMaxLevel() > 50)) {
	act("$N is blasted into oblivion!", FALSE, this, 0, victim, TO_NOTVICT);
	act("$N is blasted into oblivion!", FALSE, this, 0, victim, TO_VICT);
	act("$N is blasted into oblivion!", FALSE, this, 0, victim, TO_CHAR);
	victim->rawKill(SKILL_TURN, this);
        if (vict)
          return DELETE_VICT;
        delete victim;
        victim = NULL;
	return FALSE;
      } else {
	if (percent < 10) {
	  act("$N is blinded!", FALSE, this, 0, victim, TO_NOTVICT);
	  act("$N is blinded!", FALSE, this, 0, victim, TO_VICT);
	  act("$N is blinded!", FALSE, this, 0, victim, TO_CHAR);

          victim->rawBlind(GetMaxLevel(), GetMaxLevel() * UPDATES_PER_MUDHOUR, SAVE_YES);

	  if (reconcileDamage(victim, (4 * getLevel(SHAMAN_LEVEL_IND)),DMG_DISINT) == -1) {
            if (vict)
              return DELETE_VICT;
            delete victim;
            victim = NULL
	    return FALSE;
          }
          if (!victim->isLucky(spellLuckModifier(SKILL_TURN)))
	    victim->addFeared(this);
	} else {
	  if (percent < 20) {
	    act("$N is stunned!", FALSE, this, 0, victim, TO_NOTVICT);
	    act("$N is stunned!", FALSE, this, 0, victim, TO_VICT);
	    act("$N is stunned!", FALSE, this, 0, victim, TO_CHAR);
	    victim->setPosition(POSITION_STUNNED);
	    if (reconcileDamage(victim, (2 * getLevel(SHAMAN_LEVEL_IND)),SKILL_TURN) == -1) {
              if (vict)
                return DELETE_VICT;
              delete victim;
              victim = NULL
              return FALSE;
            }

            if (!victim->isLucky(spellLuckModifier(SKILL_TURN)))
              victim->addFeared(this);
	  } else {
	    if (reconcileDamage(victim, (getLevel(CLERIC_LEVEL_IND)),SKILL_TURN) == -1) {
              if (vict)
                return DELETE_VICT;
              delete victim;
              victim = NULL
              return FALSE;
            }
            if (!victim->isLucky(spellLuckModifier(SKILL_TURN)))
              victim->addFeared(this);
	  }
	}
      }
    } else {
      if (isEvil()) {
	act("$N becomes enthralled by $n!", FALSE, this, 0, victim, TO_NOTVICT);
	act("You are helpless but to submit to $n!", FALSE, this, 0, victim, TO_VICT);
	act("$N becomes enthralled by you!", FALSE, this, 0, victim, TO_CHAR);
      }
    }
#endif
  }
  addToWait(combatRound(5));
  return FALSE;
}

void TBeing::doPenance()
{

  if ((roomp->isWaterSector() || roomp->isUnderwaterSector()) && !isAquatic()) {
    sendTo("Trying to tread water really messes up your ability to repent.\n\r");
    return;
  }
  if(!isImmortal()) {
    if (getCond(DRUNK) > 0) {
      sendTo("Drunkeness really messes up your ability to perform penance.\n\r");
      return;
    }
    if (!getCond(FULL)) {
      sendTo("You are too hungry to perform penance.\n\r");
      return;
    }

    if (!getCond(THIRST)) {
      sendTo("You are too thirsty to perform penance.\n\r");
      return;
    }
  }

  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You are a little too keyed up at the moment to properly focus.\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_PENANCE)) {
    sendTo("You really don't know anything about repenting.\n\r");
    return;
  }
  if (riding && dynamic_cast<TBeing *>(riding)) {
    sendTo("It is impossible to be repentive while mounted!\n\r");
    return;
  }
  if (riding && dynamic_cast<TObj *>(riding)) {
    // penance sets position so don't allow this
    sendTo("You aren't in the proper position, so you stand up.\n\r");
    doStand();
    // fall through
  }
  if (getPosition() == POSITION_FIGHTING) {
    sendTo("It is impossible to be repentive while fighting!\n\r");
    return;
  }
  if (task && getPosition() <= POSITION_SITTING)
    stopTask();

  sendTo("You rest and begin to chant.\n\r");
  act("$n sits down in a position of penance.", TRUE, this, 0, 0, TO_ROOM);
  setPosition(POSITION_RESTING);
  start_task(this, NULL, NULL, TASK_PENANCE, "", 0, in_room, 1, 0, 40);
}

void TBeing::doMeditate()
{
  if ((roomp->isWaterSector() || roomp->isUnderwaterSector()) && !isAquatic()) {
    sendTo("Trying to tread water really messes up your ability to meditate.\n\r");
    return;
  }
  if(!isImmortal()) {
    if (getCond(DRUNK) > 0) {
      sendTo("Drunkenness really messes up your ability to meditate.\n\r");
     return;
    }
    if (!getCond(FULL)) {
      sendTo("You are too hungry to meditate.\n\r"); 
      return;
    }
    if (!getCond(THIRST)) {
      sendTo("You are too thirsty to meditate.\n\r");
      return;
    }
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You are a little too keyed up at the moment to properly focus.\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_MEDITATE)) {
    sendTo("You really don't know anything about meditating.\n\r");
    return;
  }
  if (riding && dynamic_cast<TBeing *>(riding)) {
    sendTo("It is impossible to meditate while mounted!\n\r");
    return;
  }
  if (riding && dynamic_cast<TObj *>(riding)) {
    // meditate sets position so don't allow this
    sendTo("You aren't in the proper position, so you stand up.\n\r");
    doStand();
    // fall through
  }
  if (fight()) {
    sendTo("It is impossible to meditate while fighting!\n\r");
    return;
  }
  if (task) {
    if (task->task == TASK_MEDITATE) {
      sendTo("You sink deeper into your meditation.\n\r");
      return;
    } else if (getPosition() <= POSITION_SITTING) 
      stopTask();
  }
  sendTo("You rest and begin to meditate.\n\r");
  act("$n sits down in a position of meditation.", TRUE, this, 0, 0, TO_ROOM);
  setPosition(POSITION_SITTING);

  start_task(this, NULL, NULL, TASK_MEDITATE, "", 0, in_room, 1, 0, 40);
}

void TBeing::doYoginsa()
{
  if ((roomp->isWaterSector() || roomp->isUnderwaterSector()) && !isAquatic()) {
    sendTo("Trying to tread water really messes up your ability to meditate.\n\r");
    return;
  }
  if(!isImmortal()) {
    if (getCond(DRUNK) > 0) {
      sendTo("Drunkenness really messes up your ability to meditate.\n\r");
     return;
    }
    // hunger thirst checks ignored
  }
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You are a little too keyed up at the moment to properly focus.\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_YOGINSA)) {
    sendTo("You really don't know anything about meditating.\n\r");
    return;
  }
  if (riding && dynamic_cast<TBeing *>(riding)) {
    sendTo("It is impossible to meditate while mounted!\n\r");
    return;
  }
  if (riding && dynamic_cast<TObj *>(riding)) {
    // meditate sets position so don't allow this
    sendTo("You aren't in the proper position, so you stand up.\n\r");
    doStand();
    // fall through
  }
  if (fight()) {
    sendTo("It is impossible to meditate while fighting!\n\r");
    return;
  }
  if (task) {
    if (task->task == TASK_YOGINSA) {
      sendTo("You meditate harder and harder.\n\r");
      return;
    } else if (getPosition() <= POSITION_SITTING) 
      stopTask();
  }
  sendTo("You relax and begin meditating.\n\r");
  act("$n sits down and begins meditating.", TRUE, this, 0, 0, TO_ROOM);
  setPosition(POSITION_SITTING);

  start_task(this, NULL, NULL, TASK_YOGINSA, "", 0, in_room, 1, 0, 40);
}

void TBeing::loseSneak()
{
  if (affectedBySpell(SKILL_SNEAK)) {
    affectFrom(SKILL_SNEAK);
    sendTo("You are no longer sneaky.\n\r");
  }

  if (checkForSkillAttempt(SKILL_SNEAK)) {
    removeSkillAttempt(SKILL_SNEAK);
    sendTo("You no longer attempt to be sneaky.\n\r");
  }

  // don't remove a generalized AFF_SNEAK since this is a character trait
}

