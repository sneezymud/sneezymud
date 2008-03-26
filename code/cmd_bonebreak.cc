#include "stdsneezy.h"
#include "combat.h"
#include "disc_monk.h"
#include "disc_leverage.h"

int TBeing::doBoneBreak(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *v;

  if (checkBusy(NULL)) {
    return FALSE;
  }
  if (getPosition() == POSITION_CRAWLING) {
    sendTo("You can't bone break while crawling.\n\r");
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_BONEBREAK)) {
    sendTo("You know nothing about breaking bones.\n\r");
    return FALSE;
  }
  
  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, argument))) {
      if (!(v = fight())) {
        sendTo("Break whose bones?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*v)) {
    sendTo("That person doesn't seem to be around.\n\r");
    return FALSE;
  }
  rc = bonebreak(this, v);
  addSkillLag(SKILL_BONEBREAK, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete v;
    v = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;

}

int bonebreakMiss(TBeing *c, TBeing *v, int type)
{
    switch(type){
      case 0:{ // normal
	act("$n attempts to get $N into a bone breaking hold, but fails.", 
	    FALSE, c, 0, v, TO_NOTVICT);
	act("$N avoids your puny attempt to get $M into a bone breaking hold.", 
	    FALSE, c, 0, v, TO_CHAR);
	act("$n tries get you into a bone breaking hold, but you avoid it.", 
	    FALSE, c, 0, v, TO_VICT);
      } break;
      case 1:{ // monk counter move
	act("$n tries to grapple $N, but $E cleverly avoids $s moves.", 
	    FALSE, c, 0, v, TO_NOTVICT);
	act("$N cleverly avoids your attempt to get $M into a bone breaking hold.",
	    FALSE, c, 0, v, TO_CHAR);
	act("$n tries grapple you into a bone breaking hold but you avoid it.", 
	    FALSE, c, 0, v, TO_VICT);    
      } break;
    }

  c->reconcileDamage(v, 0,SKILL_BONEBREAK);
  return TRUE;
}


int bonebreakHit(TBeing *c, TBeing *victim)
{
  wearSlotT slot;
  int dam=::number(1,c->GetMaxLevel()), rc;
  char limb[256], buf[256];

  // find a suitable bone to break 
  for (slot = (wearSlotT) number(MIN_WEAR, MAX_WEAR - 1);; 
       slot = (wearSlotT) number(MIN_WEAR, MAX_WEAR - 1)) {
    if (notBreakSlot(slot, false))
      continue;
    if (!victim->slotChance(slot) || victim->isLimbFlags(slot, PART_BROKEN))
      continue;
    if(!victim->hasPart(slot))
      continue;
    break;
  }

  if (victim->isImmune(IMMUNE_BONE_COND, slot, c->GetMaxLevel())) {
    act("You grab ahold of $N but you just can't seem to break any bones.",
	FALSE, c, 0, victim, TO_CHAR);
    act("$n grabs ahold of you, but is incapable of breaking your bones.", 
	FALSE, c, 0, victim, TO_VICT);
    act("$n grabs ahold of $N, but doesn't seem to be able to break any bones.", 
	FALSE, c, 0, victim, TO_NOTVICT);

    if ((rc = c->reconcileDamage(victim, 0,SKILL_BONEBREAK)) == -1)
      return DELETE_VICT;    
    return TRUE; // lag them anyway
  }


  sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
  switch(critSuccess(c, SKILL_BONEBREAK)) {
    case CRIT_S_KILL: 
      sprintf(buf, "You yell out a mighty warcry and rip $N's %s completely off!", limb);
      act(buf, FALSE, c, 0, victim, TO_CHAR);
      sprintf(buf, "$n yells out a mighty warcry and rips your %s completely off!", limb);
      act(buf, FALSE, c, 0, victim, TO_VICT);
      sprintf(buf, "$n yells out a mighty warcry and rips $N's %s completely off!", limb);
      act(buf, FALSE, c, 0, victim, TO_NOTVICT);

      victim->makePartMissing(slot, FALSE);
      victim->rawBleed(slot, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
      if (c->desc)
	c->desc->career.crit_sev_limbs++;
      if (victim->desc)
	victim->desc->career.crit_sev_limbs_suff++;

      break;
    default:
      sprintf(buf, "You grab onto $N's %s and break it!", limb);
      act(buf, FALSE, c, 0, victim, TO_CHAR);
      sprintf(buf, "$n grabs your %s and breaks it.  Ohh the pain!", limb);
      act(buf, FALSE, c, 0, victim, TO_VICT);
      sprintf(buf, "You hear a muffled SNAP as $n grabs $N's %s and breaks it!", limb);
      act(buf, FALSE, c, 0, victim, TO_NOTVICT);
      victim->addToLimbFlags(slot, PART_BROKEN);
  }

  victim->dropWeapon(slot);

  if ((rc = c->reconcileDamage(victim, dam, SKILL_BONEBREAK)) == -1)
    return DELETE_VICT;

  return TRUE;
}

int bonebreak(TBeing *caster, TBeing *victim)
{
  int percent;
  int level, i = 0, found=0, ok=0;
  wearSlotT slot;
  int rc = 0;
  const int BONEBREAK_MOVE   = 15;
  
  if (caster->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (victim == caster) {
    caster->sendTo("You consider breaking your own bones, but can't determine how to get leverage...\n\r");
    return FALSE;
  }
  if (caster->riding) {
    caster->sendTo("You would fall off your mount if you tried that!\n\r");
    return FALSE;
  }
  if (caster->noHarmCheck(victim))
    return FALSE;
  
  if ((victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) &&
       caster->GetMaxLevel()<victim->GetMaxLevel()){
    caster->sendTo("Attacking an immortal wouldn't be very smart.\n\r");
    return FALSE;
  }
  if (caster->getMove() < BONEBREAK_MOVE) {
    caster->sendTo("You lack the vitality.\n\r");
    return FALSE;
  }

  if (victim->isFlying()) {
    caster->sendTo("You can't get ahold of something that is flying.\n\r");
    return FALSE;
  }
  if(victim->raceHasNoBones()){
    caster->sendTo("You can't seem to locate anything worth breaking.\n\r");
    return FALSE;
  }

  if (caster->attackers > 4) {
    caster->sendTo("There's no room to bone break!\n\r");
    return FALSE;
  }
  if (victim->attackers > 4) {
    caster->sendTo("You can't get close enough to them to break their bones!\n\r");
    return FALSE;
  }

  // check whether or not there is a bone left to break 
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))
      continue;
    if (!victim->slotChance(slot))
      continue;
    if(victim->isLimbFlags(slot, PART_BROKEN)){
       ++found;
       continue;
    }
    if(!victim->hasPart(slot))
      continue;
    ok++;
  }
  if (!ok) {
    caster->sendTo("You've already broken everything you can.\n\r");
    return FALSE;
  }

  percent = ((10 - (victim->getArmor() / 10)) << 1);
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;
  if(found>0)  // make it harder for each bone already broken
    percent/=(found*found);

  if(caster->GetMaxLevel()<victim->GetMaxLevel())
    percent/=(victim->GetMaxLevel()-caster->GetMaxLevel());


  level = caster->getSkillLevel(SKILL_BONEBREAK);
  int bKnown = caster->getSkillValue(SKILL_BONEBREAK);
  
  caster->addToMove(-(BONEBREAK_MOVE));

  if (caster->bSuccess(bKnown + percent, SKILL_BONEBREAK) &&
      (i = caster->specialAttack(victim,SKILL_BONEBREAK)) &&
      i != GUARANTEED_FAILURE &&
      percent < bKnown &&
      !caster->isNotPowerful(victim, level, SKILL_BONEBREAK, SILENT_YES)) {

    if (victim->canCounterMove(bKnown/3)) {
      SV(SKILL_BONEBREAK);
      rc = bonebreakMiss(caster, victim, 1);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
	return rc;
      return TRUE;
    }

    return (bonebreakHit(caster, victim));
  } else {
    rc = bonebreakMiss(caster, victim, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  return TRUE;
}

