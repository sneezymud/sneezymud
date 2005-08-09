//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "obj_base_clothing.h"

bool TBeing::canDisarm(TBeing *victim, silentTypeT silent)
{
  switch (race->getBodyType()) {
    case BODY_PIERCER:
    case BODY_MOSS:
    case BODY_KUOTOA:
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_LAMMASU:
    case BODY_WYVERN:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_CHIMERA:
    case BODY_SNAKE:
    case BODY_NAGA:
    case BODY_ORB:
    case BODY_VEGGIE:
    case BODY_LION:
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
    case BODY_PIG:
    case BODY_FOUR_HOOF:
    case BODY_ELEPHANT:
    case BODY_BAANTA:
    case BODY_AMPHIBEAN:
    case BODY_FROG:
    case BODY_MIMIC:
    case BODY_WYVELIN:
    case BODY_FISH:
    case BODY_TREE:
    case BODY_SLIME:
      if (!silent)
        sendTo("You have the wrong bodyform for grappling.\n\r");
      return FALSE;
    default:
      break;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (getCombatMode() == ATTACK_BERSERK) {
    if (!silent)
      sendTo("You are berserking! You can't focus enough to disarm anyone!\n\r ");
    return FALSE;
  }

  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return FALSE;
  }

  if (riding) {
    if (!silent)
      sendTo("Yeah... right... while mounted.\n\r");
    return FALSE;
  }
  if (victim->isFlying() && (victim->fight() != this)) {
    if (!silent)
      sendTo("You can only disarm fliers that are fighting you.\n\r");
    return FALSE;
  }
  if (!victim->heldInPrimHand() && !victim->heldInSecHand()) {
    if (!silent)
      act("$N is not wielding anything.",FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  }
  if (isHumanoid()) {
    if (bothArmsHurt()) {
      if (!silent)
        act("You need working arms to disarm!", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
  }
#if 0
  if (!equipment[getPrimaryHold()]) {
    sendTo("Your primary hand must be FREE in order to attempt a disarm!\n\r");
    return FALSE;
  }
#endif

  return TRUE;
}


// uses the psionic skill telekinesis to automatically retrieve a disarmed wep
// victim is the disarmee, ie the one with the telekinesis skill
bool trytelekinesis(TBeing *caster, TBeing *victim, TObj *obj){
  if(!victim->doesKnowSkill(SKILL_TELEKINESIS)){
    return FALSE;
  }

  if(!victim->bSuccess(SKILL_TELEKINESIS)){
    act("You try to retrieve $p using telekinesis, but it is too difficult.", 
	FALSE, caster, obj, victim, TO_VICT, ANSI_CYAN);
    act("$N furrows $s brow for a moment, but nothing happens.",
	FALSE, caster, obj, victim, TO_NOTVICT, ANSI_NORMAL);
    act("$N furrows $s brow for a moment, but nothing happens.",
	FALSE, caster, obj, victim, TO_CHAR, ANSI_NORMAL);
  } else {
    act("You catch $p in mid-air with the powers of your mind and return it to your grasp!",
	FALSE, caster, obj, victim, TO_VICT, ANSI_CYAN);
    act("$N's $p stops in mid-air, then flies back to his hand!",
	FALSE, caster, obj, victim, TO_NOTVICT, ANSI_CYAN);
    act("$N's $p stops in mid-air, then flies back to his hand!",
	FALSE, caster, obj, victim, TO_CHAR, ANSI_CYAN);
    return TRUE;
  }

  // shouldn't get here
  return FALSE;
}

static int disarm(TBeing * caster, TBeing * victim, spellNumT skill) 
{
  int percent;
  int level, i = 0;

  if (!caster->canDisarm(victim, SILENT_NO))
    return FALSE;

  const int disarm_move = 20;
  if (caster->getMove() < disarm_move) {
    caster->sendTo("You are too tired to attempt a disarm maneuver!\n\r");
    return FALSE;
  }
  caster->addToMove(-disarm_move);

  level = caster->getSkillLevel(skill);

  int bKnown = caster->getSkillValue(skill);
//  int level2  = victim->getSkillLevel(skill);

  if (caster->isNotPowerful(victim, level, skill, SILENT_YES) ) {
// wtf is this doing here - Maror
 //   ||  !victim->isNotPowerful(caster, level2, skill, SILENT_YES)) {
    act("You try to disarm $N, but fail miserably.",
           TRUE, caster, 0, victim, TO_CHAR);
    if (caster->isHumanoid())
      act("$n does a nifty fighting move, but then falls on $s butt.",
           TRUE, caster, 0, 0, TO_ROOM);
    else {
      act("$n lunges at you, but fails to accomplish anything.", 
              TRUE, caster, 0, victim, TO_VICT);
      act("$n lunges at $N, but fails to accomplish anything.",
              TRUE, caster, 0, victim, TO_NOTVICT);
    }
    caster->setPosition(POSITION_SITTING);
    if (dynamic_cast<TMonster *>(victim) && victim->awake() && !victim->fight()) 
      caster->reconcileDamage(victim, 0, skill);;

    return TRUE;
  }

  percent = 0;
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;

  // if my hands are empty, make it easy  
  if (!caster->heldInPrimHand() &&
      caster->isHumanoid())
    percent += 10;

  // if i am an equipped monk, make it tough
  if (caster->heldInPrimHand() && caster->hasClass(CLASS_MONK))
    percent -= 10;

  i = caster->specialAttack(victim,skill);
  if (i && bKnown >= 0 && i != GUARANTEED_FAILURE &&
      caster->bSuccess(bKnown + percent, skill)) {
    TObj * obj = NULL;
    bool isobjprim=TRUE; // is the disarmed object the primary hand object?

    if (!(obj=dynamic_cast<TObj *>(victim->heldInPrimHand()))){
      obj = dynamic_cast<TObj *>(victim->heldInSecHand());
      isobjprim=FALSE;
    }
  
    TBaseClothing *shield = dynamic_cast<TBaseClothing *>(obj);
    if (!isobjprim && shield && shield->isShield() && !::number(0,2)) {
      act("You catch the edge of $p's $N but fail to disarm it.",
          FALSE, caster, victim, victim->heldInSecHand(), TO_CHAR);
      act("$n catches the edge of $p's $N but fails to disarm it.",
          FALSE, caster, victim, victim->heldInSecHand(), TO_ROOM);
      caster->reconcileDamage(victim, 0, skill);;
    } else if (obj) {
      act("You attempt to disarm $N.", TRUE, caster, 0, victim, TO_CHAR);
      if (caster->isHumanoid())
        act("$n makes an impressive fighting move.", TRUE, caster, 0, 0, TO_ROOM);
      else {
        act("$n lunges at $N!",
             TRUE, caster, 0, victim, TO_NOTVICT);
        act("$n lunges at you!",
             TRUE, caster, 0, victim, TO_VICT);
      }
      if(victim->doesKnowSkill(SKILL_WEAPON_RETENTION) &&
	 (victim->bSuccess(SKILL_WEAPON_RETENTION))){
	act("You try to disarm $N, but are easily countered..", 
	    FALSE, caster, obj, victim, TO_CHAR);
	act("$n tries to disarm you, but is easily countered.",
	    FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
	act("$n tries to disarm $N, but is easily countered.",
	    FALSE, caster, obj, victim, TO_NOTVICT);
      } else if (victim->doesKnowSkill(SKILL_TRANCE_OF_BLADES) && (victim->task) &&
	  (victim->task->task == TASK_TRANCE_OF_BLADES)) {
	int val = (int)(((float) victim->getSkillValue(SKILL_TRANCE_OF_BLADES) * 0.70) + 30);
	if (victim->bSuccess(val, SKILL_TRANCE_OF_BLADES)) {
	  act("You try to make it past $N's defensive trance, but get beaten back!", FALSE, caster, obj, victim, TO_CHAR);
	  act("$n fails to make it past your defensive trance, and you beat $m back!", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
	  act("$n fails to make it past $N's defensive trance, and gets beaten back!", FALSE, caster, obj, victim, TO_NOTVICT);
	}
      } else {
	act("You send $p flying from $N's grasp.", FALSE, caster, obj, victim, TO_CHAR);
	act("$p flies from your grasp.", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
	act("$p flies from $N's grasp.", FALSE, caster, obj, victim, TO_NOTVICT);
	if(!trytelekinesis(caster, victim, obj)){
	  if(isobjprim){
	    victim->unequip(victim->getPrimaryHold());
	  } else {
	    victim->unequip(victim->getSecondaryHold());
	  }
	  
	  *victim->roomp += *obj;
	  victim->logItem(obj, CMD_DISARM);
	  victim->doSave(SILENT_YES);
	}
      }      
    } else {
      act("You try to disarm $N, but $E doesn't have a weapon.", TRUE, caster, 0, victim, TO_CHAR);
      act("$n makes an impressive fighting move, but does little more.", TRUE, caster, 0, 0, TO_ROOM);
    }
    caster->reconcileDamage(victim, 0, skill);
    
    victim->addToWait(combatRound(1));
    caster->reconcileHurt(victim, 0.01);
    
  } else {
    act("You try to disarm $N, but fail miserably, falling down in the process.", TRUE, caster, 0, victim, TO_CHAR, ANSI_YELLOW);
    act("$n does a nifty fighting move, but then falls on $s butt.", TRUE, caster, 0, 0, TO_ROOM);
    caster->setPosition(POSITION_SITTING);
    caster->reconcileDamage(victim, 0, skill);;
  }
  return TRUE;
}


int TBeing::doDisarm(sstring argument, TThing *v) 
{
  sstring v_name;
  TBeing * victim = NULL;
  int rc;

  spellNumT skill = getSkillNum(SKILL_DISARM);
  

  if (checkBusy()) {
    return FALSE;
  }
  one_argument(argument, v_name);
  if (!v) {
    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        if (argument.empty()) {
          sendTo("Syntax: disarm <person | item>\n\r");
          return FALSE;
        } else {
          rc = disarmTrap(argument.c_str(), NULL);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
          return FALSE;
        }
      }
    }
  } else {
    // v is either a being or an obj, unknown at this point
    victim = dynamic_cast<TBeing *>(v);

    if (!victim) {
      TObj *to = dynamic_cast<TObj *>(v);
      if (to) {
        rc = disarmTrap(argument.c_str(), to);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return FALSE;
      }      
    }      
  }
  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about how to disarm someone.\n\r");
    return FALSE;
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = disarm(this, victim, skill);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  if (rc)
    addSkillLag(skill, rc);

  return TRUE;
}

