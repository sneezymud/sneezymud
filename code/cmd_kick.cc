//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"

bool TBeing::canKick(TBeing *victim, silentTypeT silent)
{
  if (checkBusy())
    return FALSE;

  if (affectedBySpell(AFFECT_TRANSFORMED_LEGS)) {
    if (!silent)
      sendTo("How do you expect to kick with your legs transformed.\n\r");
    return FALSE;
  }
  if (getPosition() == POSITION_CRAWLING) {
    if (!silent)
      sendTo("You can't kick while crawling.\n\r");
    return FALSE;
  }

  spellNumT skill = getSkillNum(SKILL_KICK);
  if (!doesKnowSkill(skill)) {
    if (!silent)
      sendTo("You know nothing about kicking.\n\r");
    return FALSE;
  }

  if (doesKnowSkill(SKILL_ADVANCED_KICKING)){
    if (!silent)
      sendTo("You are kicking constantly in melee, already.\n\r");
    return FALSE;
  }
  
  if (!hasLegs()) {
    if (!silent)
      sendTo("You need legs to kick.\n\r");
    return FALSE;
  }

  switch (getRace()) {
    case RACE_BIRD:
      // note, we intentionally do NOT trap ostriches, penguins here which
      // are allowed to kick
      if (!silent)
        sendTo("Bird's can't kick.\n\r");
      return false;
    default:
      break;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
  
  if (eitherLegHurt()) {
    if (!silent)
      sendTo("It's very hard to kick without the use of your legs!\n\r");
    return FALSE;
  }
  if (victim == this) {
    if (!silent)
      sendTo("You cannot kick yourself!\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully kick an immortal.\n\r");
    return FALSE;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) && (master == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, 
        this, 0, victim, TO_CHAR);
    return FALSE;
  }
  if (riding) {
    if (!silent)
      sendTo("You can't kick while mounted!\n\r");
    return FALSE;
  }
  if (isSwimming()) {
    if (!silent)
      sendTo("It's near impossible to kick while swimming.\n\r");
    return FALSE;
  }
  if (roomp && roomp->isUnderwaterSector()) {
    if (!silent)
      sendTo("The water around you totally impedes your kick!\n\r");
    return FALSE;
  }
  if (victim->isFlying()) {
    if (!silent)
      act("How can you kick $N when $E is flying?", 
          FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

enum kickSlotT {
  KICK_NONE,
  KICK_LEG,
  KICK_WAISTE,
  KICK_BODY,
  KICK_HEAD
};

static int kickMiss(TBeing *caster, TBeing *victim, kickSlotT slot, spellNumT skill)
{
  switch (slot)
  {
    case KICK_BODY:   // body
      act("$n misses a kick at $N's solar plexus.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You miss your kick at $N's solar plexus.", FALSE, caster, 0, victim, TO_CHAR);            
      act("Leaping backwards, you dodge $n's kick at your chest!", FALSE, caster, 0, victim, TO_VICT);            
      break;
    case KICK_WAISTE:   // waist/crotch
      act("$n misses a kick at $N's groin.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You miss your kick at $N's groin.", FALSE, caster, 0, victim, TO_CHAR);            
      act("Jumping out of the way, you avoid $n's kick at your crotch!", FALSE, caster, 0, victim, TO_VICT);            
      break;
    case KICK_HEAD:   // head
      act("$n misses a kick at $N's head.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You miss your kick at $N's head.", FALSE, caster, 0, victim, TO_CHAR);            
      act("Ducking quickly, you avoid $n's kick at your head!", FALSE, caster, 0, victim, TO_VICT);            
      break;
    case KICK_LEG:   // leg
    case KICK_NONE:   // leg
      act("$n misses a kick at $N.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You miss your kick at $N.", FALSE, caster, 0, victim, TO_CHAR);      
      if (!victim->riding)
        act("Rolling aside, you avoid $n's kick!", 
               FALSE, caster, 0, victim, TO_VICT);            
      else
        act("You avoid $n's kick!", 
               FALSE, caster, 0, victim, TO_VICT);            
      break;
  }

  if (caster->reconcileDamage(victim, 0,skill) == -1)
    return DELETE_VICT;
  
  return TRUE;
}

static int kickHit(TBeing *caster, TBeing *victim, int score, int level, spellNumT skill)
{
  int dam, limb_dam;
  spellNumT dam_type = skill;
  int hgt=caster->getHeight() ;
  kickSlotT slot_i;
  wearSlotT slot;
  int rc;

  // determine how high they can kick
  // tmp is a number of inches
  int tmp;

  // made this a little less reliant on dexterity and raised the 
  // overall height
  if (caster->hasClass(CLASS_MONK))
    tmp = caster->plotStat(STAT_CURRENT, STAT_DEX, 
      hgt * 50/100, hgt * 110/100, hgt * 70/100);
  else
    tmp = caster->plotStat(STAT_CURRENT, STAT_DEX,
      hgt * 30/100, hgt * 80/100, hgt * 50/100);
 
  // randomize hit location 
  tmp = ::number(0,tmp);

  // find possible corrosponding values on v 
  // legs are 0-40%, waist 41-60, chest 61-95, head 95+ 
  if (tmp < victim->getPartMinHeight(ITEM_WEAR_FEET)) {
    act("You strain to kick $N's feet, but are unable to reach $M.",
          FALSE, caster, 0, victim, TO_CHAR);
    act("$n misses a kick at your feet.",
          FALSE, caster, 0, victim, TO_VICT);
    act("$n misses a kick at $N's feet.",
          FALSE, caster, 0, victim, TO_NOTVICT);

    if (caster->reconcileDamage(victim, 0,skill) == -1)
      return DELETE_VICT;
  
    return TRUE;
  } else if (tmp < victim->getPartMinHeight(ITEM_WEAR_WAISTE))
    slot_i = KICK_LEG;   // shins
  else if (tmp < victim->getPartMinHeight(ITEM_WEAR_BODY))
    slot_i = KICK_WAISTE;   // waist
  else if (tmp < victim->getPartMinHeight(ITEM_WEAR_HEAD))
    slot_i = KICK_BODY;   // chest
  else
    slot_i = KICK_HEAD;   // head
  
  if (!victim->isHumanoid())
    slot_i = KICK_NONE;

  dam = caster->getSkillDam(victim, skill, level, caster->getAdvLearning(skill));

  TObj *item;
  switch (slot_i) {
    case KICK_WAISTE:
      limb_dam = (level/5)+1;
      act("$n kicks $N in the crotch, yowch!.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You're kicked in the crotch by $n.", FALSE, caster, 0, victim, TO_VICT); 
      act("Your kick hits $N in the crotch.", FALSE, caster, 0, victim, TO_CHAR);
      if (victim->getSex() == SEX_MALE) {
        if (!victim->equipment[WEAR_WAISTE] || 
            isname("belt", victim->equipment[WEAR_WAISTE]->name)) {
          // no equipment or just a belt
          victim->sendTo("Your voice just went up an octave.  Ouch!\n\r");
          dam += 1;
          victim->cantHit += victim->loseRound(0.25);
        } else 
          victim->sendTo(fmt("Good thing you were wearing your %s.\n\r") %
               fname(victim->equipment[WEAR_WAISTE]->name));
      }
      item = dynamic_cast<TObj *>(victim->equipment[WEAR_WAISTE]);
      if (!item) {
        rc = caster->damageLimb(victim, WEAR_WAISTE, NULL, &limb_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (caster->dentItem(victim, item, 1, caster->getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
      break;
    case KICK_BODY:
      act("$n kicks $N in the solar plexus.", FALSE, caster, 0, victim, TO_NOTVICT);            
      act("You're kicked in the solar plexus by $n.  Wow, this is breathtaking!", FALSE, caster, 0, victim, TO_VICT);            
      act("Your kick hits $N in the solar plexus.", FALSE, caster, 0, victim, TO_CHAR);
      dam += 1;
      dam_type = DAMAGE_KICK_SOLAR;
      break;
    case KICK_HEAD:
      act("$n gives $N a boot to the head!", FALSE, caster, 0, victim, TO_NOTVICT); 
      act("You're kicked in the head by $n!", FALSE, caster, 0, victim, TO_VICT);  
      act("You boot $N in the head.", FALSE, caster, 0, victim, TO_CHAR);    
      dam += 2;
      victim->sendTo("Who?  What?!?  Where am I?\n\r");
      victim->cantHit += victim->loseRound(0.5);
      victim->addToWait(combatRound(0.5));
      dam_type = DAMAGE_KICK_HEAD;
      break;            
    case KICK_LEG:
      limb_dam = (level/5)+1;
      act("$n kicks $N in the shin.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You're kicked in the shin by $n.   Ouch!",
            FALSE, caster, 0, victim, TO_VICT);                        
      act("Your kick hits $N in the shin.", FALSE, caster, 0, victim, TO_CHAR);
      slot = wearSlotT(::number(WEAR_LEGS_R, WEAR_LEGS_L));
      item = dynamic_cast<TObj *>(victim->equipment[slot]);
      if (!item) {
        rc = caster->damageLimb(victim, slot, NULL, &limb_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (caster->dentItem(victim, item, 1, caster->getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
      dam_type = DAMAGE_KICK_SHIN;
      break;
    case KICK_NONE:
      act("$n kicks $N in the side.", FALSE, caster, 0, victim, TO_NOTVICT);
      act("You're kicked in the side by $n.   Ouch!",
            FALSE, caster, 0, victim, TO_VICT);
      act("Your kick hits $N in the side.", FALSE, caster, 0, victim, TO_CHAR);
      dam_type = DAMAGE_KICK_SIDE;
      break;
  }

  item = dynamic_cast<TObj *>(caster->equipment[caster->getPrimaryFoot()]);
  if (item)
    if (item->isSpiked() || item->isObjStat(ITEM_SPIKED)) {
      act("The spikes on your $o sink into $N.", FALSE, caster, item, victim, TO_CHAR);
      act("The spikes on $n's $o sink into $N.", FALSE, caster, item, victim, TO_NOTVICT);
      act("The spikes on $n's $o sink into you.", FALSE, caster, item, victim, TO_VICT);

      if(caster->reconcileDamage(victim, (int)(dam*0.15), TYPE_STAB) == -1)
	return DELETE_VICT;
    }

  if (caster->reconcileDamage(victim, dam,dam_type) == -1)
    return DELETE_VICT;

  if (victim->spelltask) {
    if ((dam_type == DAMAGE_KICK_HEAD || dam_type == DAMAGE_KICK_SOLAR) &&
        caster->isLucky(levelLuckModifier(victim->GetMaxLevel()))) {
      if (victim->getPosition() < POSITION_STANDING) {
        if (!::number(0,2))
          victim->addToDistracted(2, FALSE);
        else
          victim->addToDistracted(1, FALSE);
      } else 
        victim->addToDistracted(1, FALSE);
    }
  }  
  return TRUE;
}

static int kick(TBeing *ch, TBeing *victim, spellNumT skill)
{
  int percent;
  int level, i = 0;
  int kick_move = dice(2,3);
  int rc = 0;
  
  if (!ch->canKick(victim, SILENT_NO))
    return FALSE;

  percent = -((10 - (victim->getArmor() / 100)));
  level = ch->getSkillLevel(skill);
  int bKnown = ch->getSkillValue(skill);

  if (ch->getMove() < 10) {
    ch->sendTo("You don't have the energy to make the kick!\n\r");
    return FALSE;
  }
  if (!ch->isImmortal()) 
    ch->addToMove(-kick_move);

  i = ch->specialAttack(victim, skill);
  // check bSuccess last so skill counters are OK
  if (i && (i != GUARANTEED_FAILURE) &&
      ch->bSuccess(bKnown, skill)) {
    if (victim->canCounterMove(bKnown/2)) {
      SV(skill);
      act("$N blocks your kick.", TRUE, ch, 0, victim, TO_CHAR, ANSI_RED);
      act("You block $n's kick.", TRUE, ch, 0, victim, TO_VICT, ANSI_ORANGE);
      act("$N blocks $n's kick.", TRUE, ch, 0, victim, TO_NOTVICT);
      return TRUE;
    }
    rc = kickHit(ch, victim, bKnown+percent, level, skill);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else {
    if (victim->getPosition() <= POSITION_SLEEPING) {
      rc = kickHit(ch, victim, bKnown+percent, level, skill);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_VICT;
    } else {
      kickMiss(ch, victim, KICK_LEG, skill);
    }
  }
  return TRUE;
}

int TBeing::doKick(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char namebuf[256];

  strcpy(namebuf, argument);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      if (!(victim = fight())) {
        sendTo("Kick whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  spellNumT skill = getSkillNum(SKILL_KICK);
  rc = kick(this,victim, skill);
  if (rc)
    addSkillLag(skill, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

