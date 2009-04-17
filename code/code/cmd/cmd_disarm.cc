//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "extern.h"
#include "being.h"
#include "combat.h"
#include "obj_base_clothing.h"
#include "monster.h"
#include "skills.h"

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
  if (isHumanoid()) {
    if (bothArmsHurt()) {
      if (!silent)
        act("You need working arms to disarm!", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
  }
  if (affectedBySpell(SPELL_FUMBLE)) {
    if (!silent)
      act("You are fumbling about too much to disarm!", FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}


// uses the psionic skill telekinesis to automatically retrieve a disarmed wep
// victim is the disarmee, ie the one with the telekinesis skill
bool trytelekinesis(TBeing *caster, TBeing *victim, TObj *obj, bool success){

  if(success){
    act("You catch $p in mid-air with the powers of your mind and return it to your grasp!",
	    FALSE, caster, obj, victim, TO_VICT, ANSI_CYAN);
    act("$N's $p stops in mid-air, then flies back to his hand!",
	    FALSE, caster, obj, victim, TO_NOTVICT, ANSI_CYAN);
    act("$N's $p stops in mid-air, then flies back to his hand!",
	    FALSE, caster, obj, victim, TO_CHAR, ANSI_CYAN);
    return TRUE;
  }

  act("You try to retrieve $p using telekinesis, but it is too difficult.", 
    FALSE, caster, obj, victim, TO_VICT, ANSI_CYAN);
  act("$N furrows $s brow for a moment, but nothing happens.",
    FALSE, caster, obj, victim, TO_NOTVICT, ANSI_NORMAL);
  act("$N furrows $s brow for a moment, but nothing happens.",
	  FALSE, caster, obj, victim, TO_CHAR, ANSI_NORMAL);
  return FALSE;
}

static int disarm(TBeing * caster, TBeing * victim, spellNumT skill) 
{
  int percent = 0;
  int level = caster->getSkillLevel(skill);
  affectedData af;
  int bKnown = caster->getSkillValue(skill);
  const int disarm_move = 20;
  wearSlotT worn = WEAR_NOWHERE;
  TObj * obj = NULL;

  if (!caster->canDisarm(victim, SILENT_NO))
    return FALSE;

  if (caster->getMove() < disarm_move) {
    caster->sendTo("You are too tired to attempt a disarm maneuver!\n\r");
    return FALSE;
  }

  // apply 'fumbling' affect, which is -1 level's worth of tohit, bonus goes to duration
  af.type = SPELL_FUMBLE;
  af.level = level;
  af.location = APPLY_HITROLL;
  af.modifier = -1;
  af.duration = 5;

  caster->addToMove(-disarm_move);

  // fail to overcome target save
  if (caster->isNotPowerful(victim, level, skill, SILENT_YES))
  {
    act("You try to disarm $N, but fail miserably, causing you to fumble about.", TRUE, caster, 0, victim, TO_CHAR);
    if (caster->isHumanoid())
    {
      act("$n does a nifty fighting move, but then fumbles about looking foolish.", TRUE, caster, 0, 0, TO_ROOM);
    }
    else
    {
      act("$n lunges at you, but fails to accomplish anything.", TRUE, caster, 0, victim, TO_VICT);
      act("$n lunges at $N, but fails to accomplish anything.", TRUE, caster, 0, victim, TO_NOTVICT);
    }

    caster->affectJoin2(&af, joinFlagUpdateDur);
    if (dynamic_cast<TMonster *>(victim) && victim->awake() && !victim->fight()) 
      caster->reconcileDamage(victim, 0, skill);

    return TRUE;
  }

  // agility vs dex for a bonus towards success
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;

  // execute the special attack
  int attack = caster->specialAttack(victim, skill);
  if (!attack || bKnown < 0 || attack == GUARANTEED_FAILURE || !caster->bSuccess(bKnown + percent, skill))
  {
    act("You try to disarm $N but fail miserably, causing you to fumble about.", TRUE, caster, 0, victim, TO_CHAR, ANSI_YELLOW);
    act("$n does a nifty fighting move, but then fumbles about looking foolish.", TRUE, caster, 0, 0, TO_ROOM);
    caster->affectJoin2(&af, joinFlagUpdateDur);
    caster->reconcileDamage(victim, 0, skill);
    return TRUE;
  }

  // bonus attribute: agi
  af.duration += caster->plotStat(STAT_CURRENT, STAT_AGI, 0, 2, 0);

  // apply affect
  act("You attempt to disarm $N.", TRUE, caster, 0, victim, TO_CHAR);
  if (caster->isHumanoid())
    act("$n makes an impressive fighting move.", TRUE, caster, 0, 0, TO_ROOM);
  else
  {
    act("$n lunges at $N!", TRUE, caster, 0, victim, TO_NOTVICT);
    act("$n lunges at you!", TRUE, caster, 0, victim, TO_VICT);
  }

  // affect
  victim->affectJoin2(&af, joinFlagUpdateDur);
  caster->reconcileDamage(victim, 0, skill);
  caster->reconcileHurt(victim, 0.01);
  act("Your skillful attack causes $N to fumble.", FALSE, caster, obj, victim, TO_CHAR);
  act("$n's skillful attack causes you to fumble.", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
  act("$n's skillful attack causes $N to fumble.", FALSE, caster, obj, victim, TO_NOTVICT);

  // find the object to disarm
  if (victim->heldInPrimHand())
    obj = dynamic_cast<TObj *>(victim->equipment[worn = victim->getPrimaryHold()]);
  if (!obj && victim->heldInSecHand())
    obj = dynamic_cast<TObj *>(victim->equipment[worn = victim->getSecondaryHold()]);

  // no object, just stop
  if (!obj)
    return TRUE;

  // check anti-disarm skills here to allow skills to raise without requiring attacker to crit
  bool retained = victim->doesKnowSkill(SKILL_WEAPON_RETENTION) && victim->bSuccess(SKILL_WEAPON_RETENTION);
  int trancePower = (int)(((float) victim->getSkillValue(SKILL_TRANCE_OF_BLADES) * 0.70) + 30);
  bool isTranced = victim->doesKnowSkill(SKILL_TRANCE_OF_BLADES) && victim->task && victim->task->task == TASK_TRANCE_OF_BLADES;
  bool tranceBack = isTranced && victim->bSuccess(trancePower, SKILL_TRANCE_OF_BLADES);
  bool tryTelekenisis = victim->doesKnowSkill(SKILL_TELEKINESIS);
  bool telekined = tryTelekenisis && victim->bSuccess(SKILL_TELEKINESIS);

  // end here if we didnt crit - no actual disarm
  if (critSuccess(caster, skill) == CRIT_S_NONE)
    return TRUE;

  // crit affect
  act("", FALSE, caster, obj, victim, TO_CHAR);
  act("", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
  act("", FALSE, caster, obj, victim, TO_NOTVICT);
  af.duration *= 2;
  victim->affectJoin2(&af, joinFlagUpdateDur);
  act("You really caught $N off guard!", FALSE, caster, obj, victim, TO_CHAR);
  act("$n really caught you off guard!", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
  act("$n really caught $N off guard!", FALSE, caster, obj, victim, TO_NOTVICT);

  // make it harder to disarm shields on a crit
  TBaseClothing *shield = dynamic_cast<TBaseClothing *>(obj);
  if (shield && shield->isShield() && !::number(0,2))
  {
    act("You catch the edge of $p's $N but fail to disarm it.", FALSE, caster, victim, shield, TO_CHAR);
    act("$n catches the edge of $p's $N but fails to disarm it.", FALSE, caster, victim, shield, TO_ROOM);
    return TRUE;
  }

  // allow weapon retention skill to counter a disarm
  if (retained)
  {
    act("You try to disarm $N, but are easily countered.", FALSE, caster, obj, victim, TO_CHAR);
    act("$n tries to disarm you, but is easily countered.", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
    act("$n tries to disarm $N, but is easily countered.", FALSE, caster, obj, victim, TO_NOTVICT);
    return TRUE;
  }

  // allow trance to [homo]erotically "beat them back"
  if (tranceBack)
  {
    act("You try to make it past $N's defensive trance, but get beaten back!", FALSE, caster, obj, victim, TO_CHAR);
    act("$n fails to make it past your defensive trance, and you beat $m back!", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
    act("$n fails to make it past $N's defensive trance, and gets beaten back!", FALSE, caster, obj, victim, TO_NOTVICT);
    return TRUE;
  }

  // disarm crit success
  act("You send $p flying from $N's grasp.", FALSE, caster, obj, victim, TO_CHAR);
  act("$p flies from your grasp.", FALSE, caster, obj, victim, TO_VICT, ANSI_RED);
  act("$p flies from $N's grasp.", FALSE, caster, obj, victim, TO_NOTVICT);

  // presto! re-armed with telekenisis
  if (tryTelekenisis && trytelekinesis(caster, victim, obj, telekined))
    return TRUE;

  victim->unequip(worn);
  *victim->roomp += *obj;
  victim->logItem(obj, CMD_DISARM);
  victim->doSave(SILENT_YES); 
  return TRUE;
}


int TBeing::doDisarm(sstring argument, TThing *v) 
{
  TObj *to = NULL;
  TBeing * victim = fight();
  int rc;
  spellNumT skill = getSkillNum(SKILL_DISARM);
  sstring v_name;
  one_argument(argument, v_name);

  if (checkBusy())
    return FALSE;

  // get target
  if (v)
    victim = dynamic_cast<TBeing *>(v);
  else if (!v_name.empty())
    victim = get_char_room_vis(this, v_name);     

  // check if target was meant to be obj
  if (!victim && v)
    to = dynamic_cast<TObj *>(v);

  // if no target we are either getting a trap or bad syntax
  if (!victim)
  {
    if (argument.empty())
    {
      sendTo("Syntax: disarm <person | item>\n\r");
      return 0;
    }
    return disarmTrap(argument.c_str(), to) & DELETE_THIS; // only return 0 or delete
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

