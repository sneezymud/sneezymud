//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_nature.h"
#include "obj_magic_item.h"

struct TransformLimbType TransformLimbList[LAST_TRANSFORM_LIMB] =
{
  {"hands", 6, 20, "bear claws", WEAR_HAND_R, AFFECT_TRANSFORMED_HANDS, DISC_RANGER},
  {"arms", 30, 75,"falcon wings", WEAR_ARM_R, AFFECT_TRANSFORMED_ARMS,
DISC_ANIMAL},
  {"legs", 20, 15, "a dolphin's tail", WEAR_LEGS_R, AFFECT_TRANSFORMED_LEGS, DISC_ANIMAL},
  {"neck", 15, 40, "some fish gills", WEAR_NECK, AFFECT_TRANSFORMED_NECK, DISC_ANIMAL},
  {"head", 12, 60, "an eagle's head", WEAR_HEAD, AFFECT_TRANSFORMED_HEAD, DISC_RANGER},
  {"all", 1, 1, "all your limbs", MAX_WEAR, TYPE_UNDEFINED, DISC_RANGER}
};

int transformLimb(TBeing * caster, const char * buffer, int level, byte bKnown)
{
  int ret;
  bool multi = TRUE;
  wearSlotT limb = WEAR_NOWHERE;
  affectedData aff;
  affectedData aff2;
  bool two_affects = FALSE;
  int i;
  char newl[20];
  char old[20];
  char buf[256];

  if (caster->affectedBySpell(SPELL_POLYMORPH)) {
    caster->sendTo("You can't transform while polymorphed.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  // failure = scrapped item.  no item damage allowed in arena
  // this is to block problem in doTransformDrop()
  if (caster->roomp && caster->roomp->isRoomFlag(ROOM_ARENA)) {
    act("A magic power prevents anything from happening here.",
         FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.",
         TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FALSE;
  }

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > level) {
        return SPELL_SAVE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > caster->getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        return SPELL_SAVE;
      }
      break;
    }
  }

  if (i >= LAST_TRANSFORM_LIMB) {
    caster->sendTo("Couldn't find any such limb to transform.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  
  if (limb == MAX_WEAR) {
    caster->sendTo("You can't transform all of your limbs.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (!caster->isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  aff.type = SKILL_TRANSFORM_LIMB;
  aff.location = APPLY_NONE;
  aff.duration = (combatRound((level / 5) + 2));
  aff.bitvector = 0;
  aff.modifier = 0; 

  aff2.type = SKILL_TRANSFORM_LIMB;
  aff2.location = APPLY_NONE;
  aff2.duration = combatRound(((level / 5) + 2));
  aff2.bitvector = 0;
  aff2.modifier = 0;

  switch (limb) {
    case WEAR_NECK:
      aff.type = AFFECT_TRANSFORMED_NECK;
      aff.bitvector = AFF_WATERBREATH | AFF_SILENT;
      multi = FALSE;
      break;
    case WEAR_HEAD:
      aff.type = AFFECT_TRANSFORMED_HEAD;
      aff.bitvector = AFF_SILENT | AFF_TRUE_SIGHT;
      multi = FALSE;
      break;
    case WEAR_HAND_R:
      aff.type = AFFECT_TRANSFORMED_HANDS;
      aff.location = APPLY_DAMROLL;

      // this prevents weapon use, so we want the effect to sort of make
      // up for that fact.  But since NPCs don't use weapons, and their
      // barehand dam is naturally high, lets not goose it for them too much
      if (caster->isPc())
        aff.modifier = 2 + (level / 4);
      else
        aff.modifier = 1 + (level/17);

      aff2.type = AFFECT_TRANSFORMED_HANDS;
      aff2.location = APPLY_SPELL;
      aff2.modifier = SKILL_CLIMB;
      aff2.modifier2 = 50;
      two_affects = TRUE;
      break;
    case WEAR_ARM_R:
      aff.type = AFFECT_TRANSFORMED_ARMS;
      aff.bitvector = AFF_FLYING;
      break;
    case WEAR_LEGS_R:
      aff.type = AFFECT_TRANSFORMED_LEGS;
      aff.location = APPLY_SPELL;
      aff.modifier = SKILL_SWIM;
      aff.modifier2 = 50;
      break;
    default:
      vlogf(LOG_BUG, "Bad limb case in TRANSFORM_LIMB");
      caster->sendTo("Bug in your limbs, tell a god and put in bug file.\n\r");
      return FALSE;
  }

  if (bSuccess(caster, bKnown, caster->getPerc(), SKILL_TRANSFORM_LIMB)) {
    switch (critSuccess(caster, SKILL_TRANSFORM_LIMB)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SKILL_TRANSFORM_LIMB);
        aff.duration *= 2;
        aff2.duration *= 2;
        ret = SPELL_CRIT_SUCCESS;
        sprintf(newl, "%s", TransformLimbList[i].newName);
        sprintf(old, "%s", TransformLimbList[i].name);
        if (multi)
          sprintf(buf, "Your %s glow as they transform into %s!", old, newl);
        else
          sprintf(buf, "Your %s glows as it transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_CHAR);
        if (multi)
          sprintf(buf, "$n's %s liquify and then transform into %s!", old, newl);
        else
          sprintf(buf, "$n's %s liquifies and then transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_ROOM);
        break;
      default:
        sprintf(newl, "%s", TransformLimbList[i].newName);
        sprintf(old, "%s", TransformLimbList[i].name);
        if (multi)
          sprintf(buf, "Your %s tingle as they transform into %s!", old, newl); 
        else 
          sprintf(buf, "Your %s tingles as it transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_CHAR);
        if (multi)
          sprintf(buf, "$n's %s liquify and then transform into %s!", old, newl);
        else
          sprintf(buf, "$n's %s liquifies and then transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_ROOM);
        ret = SPELL_SUCCESS;
        break;
    }
    caster->affectTo(&aff);
    if (two_affects)
      caster->affectTo(&aff2);

    caster->makeLimbTransformed(caster, limb, TRUE);
    return ret;
  } else {  
    switch (critFail(caster, SKILL_TRANSFORM_LIMB)) {  
      case CRIT_F_HITOTHER:
        CF(SKILL_TRANSFORM_LIMB);
        caster->rawBleed(limb, (level * 3) + 100, SILENT_NO, CHECK_IMMUNITY_YES);
        return SPELL_CRIT_FAIL;
      default:
        return SPELL_FAIL;
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    }
  }
}

int vampireTransform(TBeing *ch)
{
  TMonster *mob;

  if (!ch->isPc() || IS_SET(ch->specials.act, ACT_POLYSELF) ||
      ch->polyed != POLY_TYPE_NONE){
    act("You are already transformed into another shape.",
	TRUE, ch, NULL, NULL, TO_CHAR);
    return FALSE;
  }
  
  if (!(mob = read_mobile(13749, VIRTUAL))) {
    ch->sendTo("It didn't seem to work.\n\r");
    return FALSE;
  }
  thing_to_room(mob,ROOM_VOID);
  mob->swapToStrung();
  

  act("You use your dark powers to transform into $N.", 
       TRUE, ch, NULL, mob, TO_CHAR);
  act("$n transforms into $N.",
       TRUE, ch, NULL, mob, TO_ROOM);

  DisguiseStuff(ch, mob);
  
  --(*mob);
  *ch->roomp += *mob;
  --(*ch);
  thing_to_room(ch, ROOM_POLY_STORAGE);
  
  // stop following whoever you are following.
  if (ch->master)
    ch->stopFollower(TRUE);
  
  for(int tmpnum = 1; tmpnum < MAX_TOG_INDEX; tmpnum++) {
    if (ch->hasQuestBit(tmpnum))
      mob->setQuestBit(tmpnum);
  }
  
  mob->specials.affectedBy = ch->specials.affectedBy;
  
  
  // switch ch into mobile 
  ch->desc->character = mob;
  ch->desc->original = dynamic_cast<TPerson *>(ch);

  mob->desc = ch->desc;
  ch->desc = NULL;
  ch->polyed = POLY_TYPE_DISGUISE;

  SET_BIT(mob->specials.act, ACT_DISGUISED);
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
  REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
  REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

  sstring tStNewNameList(mob->name);
  
  tStNewNameList += " [";
  tStNewNameList += ch->getNameNOC(ch);
  tStNewNameList += "]";
  
  delete [] mob->name;
  mob->name = mud_str_dup(tStNewNameList);
  
  mob->setSex(ch->getSex());
  mob->setHeight(ch->getHeight());
  mob->setWeight(ch->getWeight());

  for (statTypeT tStat = MIN_STAT; tStat < MAX_STATS; tStat++) {
    mob->setStat(STAT_CURRENT, tStat, ch->getStat(STAT_CURRENT, tStat));
  }

  return TRUE;
}


int TBeing::doTransform(const char *argument) 
{
  int i = 0, bKnown = 0;
  wearSlotT limb = WEAR_NOWHERE;
  int level, ret = 0;
  int rc = 0;
  char buffer[256];

  if (!doesKnowSkill(SKILL_TRANSFORM_LIMB)) {
    if(isVampire())
      return vampireTransform(this);

    sendTo("You know nothing about transforming your limbs.\n\r");
    return FALSE;
  }

  strcpy(buffer, argument);

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > getSkillLevel(SKILL_TRANSFORM_LIMB)) {
        sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      break;
    }
  }
  if (i >= LAST_TRANSFORM_LIMB) {
    if(isVampire())
      return vampireTransform(this);

    sendTo("Couldn't find any such limb to transform.\n\r");
    return FALSE;
  }

  if (limb == MAX_WEAR) {
    sendTo("You can't transform all of your limbs.\n\r");
    return FALSE;
  }

  if (!isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, this, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  level = getSkillLevel(SKILL_TRANSFORM_LIMB);
  bKnown = getSkillValue(SKILL_TRANSFORM_LIMB);

  ret=transformLimb(this, buffer, level, bKnown);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Something went wrong with the magic.",
          FALSE, this, 0, NULL, TO_CHAR);
      act("You feel your own limb open and your blood start to drain!",
          FALSE, this, 0, NULL, TO_CHAR);
      act("Something went wrong with $n's magic.",
          FALSE, this, 0, NULL, TO_ROOM);
      act("$n seems to have caused $s limbs to start bleeding!",
          FALSE, this, 0, NULL, TO_ROOM);
  } else if (ret==SPELL_SAVE) {
      act("You are not powerful enough to transform that limb.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
      act("You fail to transform your limbs.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  } else {
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;

}

int transformLimb(TBeing * caster, const char * buffer)
{
  taskDiffT diff;
  int i;
  wearSlotT limb = WEAR_NOWHERE;

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > caster->getSkillLevel(SKILL_TRANSFORM_LIMB)) {
        caster->sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > caster->getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        caster->sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      break;
    }
  }
  if (i >= LAST_TRANSFORM_LIMB) {
    caster->sendTo("Couldn't find any such limb to transform.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (limb == MAX_WEAR) {
    caster->sendTo("You can't transform all of your limbs.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (!caster->isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

    if (!bPassMageChecks(caster, SKILL_TRANSFORM_LIMB, caster))
      return FALSE;

    lag_t rounds = discArray[SKILL_TRANSFORM_LIMB]->lag;
    diff = discArray[SKILL_TRANSFORM_LIMB]->task;

    
    start_cast(caster, NULL, NULL, caster->roomp, SKILL_TRANSFORM_LIMB, diff, 1, buffer, rounds, caster->in_room, 0, 0,TRUE, 0);
    return FALSE;
}

int castTransformLimb(TBeing * caster)
{
  int level, ret;
  int rc = 0;

  level = caster->getSkillLevel(SKILL_TRANSFORM_LIMB);
  int bKnown = caster->getSkillValue(SKILL_TRANSFORM_LIMB);

  ret=transformLimb(caster, caster->spelltask->orig_arg, level, bKnown);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Something went wrong with your spell.",
          FALSE, caster, 0, NULL, TO_CHAR);
      act("You feel your own limb open and your blood start to drain!",
          FALSE, caster, 0, NULL, TO_CHAR);
      act("Something went wrong with $n's spell.",
          FALSE, caster, 0, NULL, TO_ROOM);
      act("$n seems to have caused $s limbs to start bleeding!",
          FALSE, caster, 0, NULL, TO_ROOM);
  } else if (ret==SPELL_SAVE) {
      act("You are not powerful enough to transform that limb.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
      act("You fail to transform your limbs.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  } else {
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  }

  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int barkskin(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;

  if (victim->isPlayerAction(PLR_SOLOQUEST) && (victim != caster) &&
      !caster->isImmortal() && caster->isPc()) {
    act("$N is on a quest, you can't invoke barkskin on $M!",
      FALSE, caster, NULL, victim, TO_CHAR); 

    return FALSE;
  }
  if (victim->isPlayerAction(PLR_GRPQUEST) && (victim != caster) &&
          !caster->isImmortal() && caster->isPc() && !caster->isPlayerAction(PLR_GRPQUEST)) {
    act("$N is on a group quest you aren't on!  No help allowed!",
      FALSE, caster, NULL, victim, TO_CHAR);

    return FALSE;
  }

  aff.type = SKILL_BARKSKIN;
  aff.location = APPLY_ARMOR;
  aff.duration = max(min(level/2, 25), 1) * UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = -90;

  if (bSuccess(caster, bKnown, caster->getPerc(), SKILL_BARKSKIN)) {
    if (critSuccess(caster, SKILL_BARKSKIN)) {
      CS(SKILL_BARKSKIN);
      aff.modifier *= 2;
      aff.duration *= 2;
    }

    if (caster != victim)
      aff.modifier = -10;

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      return SPELL_FALSE;
    }

    act("Your skin turns into an extremely hard, oak-like bark.", 
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$n's skin turns into an extremely hard, oak-like bark.", 
        TRUE, victim, NULL, NULL, TO_ROOM);

    caster->reconcileHelp(victim, discArray[SKILL_BARKSKIN]->alignMod);
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SKILL_BARKSKIN)) {
      CF(SKILL_BARKSKIN);
      act("Your skin turns to hard bark, but then softens considerably!", FALSE, victim, NULL, NULL, TO_CHAR);
      act("$n's skin turns to hard bark, but then seems to soften.", TRUE, victim, NULL, NULL, TO_ROOM);
      aff.modifier = +20;
      caster->affectTo(&aff);
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    }
    return SPELL_FAIL;
  }
}

int barkskin(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int rc = 0;

  int ret=barkskin(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int TBeing::doBarkskin(const char *argument)
{
  int rc = 0;
  TBeing *victim = NULL;
  char namebuf[256];

  if (!doesKnowSkill(SKILL_BARKSKIN)) {
    sendTo("You know nothing about barkskin.\n\r");
    return FALSE;
  }

  if (!argument) {
    victim = this;
  } else {
    strcpy(namebuf, argument);
    if (!(victim = get_char_room_vis(this, namebuf))) {
      sendTo("Apply barkskin to what?\n\r");
      return FALSE;
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  int level = getSkillLevel(SKILL_BARKSKIN);
  int bKnown = getSkillValue(SKILL_BARKSKIN);

  // not technically a spell, but needs a component anyway
  if (!useComponent(findComponent(SKILL_BARKSKIN), victim, CHECK_ONLY_NO))
    return FALSE;

  int ret=barkskin(this,victim,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// this is the old entry point for barskin (as a spell)
// it is still needed for mob-casting
int barkskin(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SKILL_BARKSKIN, victim))
    return FALSE;

  lag_t rounds = discArray[SKILL_BARKSKIN]->lag;
  taskDiffT diff = discArray[SKILL_BARKSKIN]->task;

  start_cast(caster, victim, NULL, caster->roomp, SKILL_BARKSKIN, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;
}

int castBarkskin(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  int level = caster->getSkillLevel(SKILL_BARKSKIN);
  int bKnown = caster->getSkillValue(SKILL_BARKSKIN);

  int ret=barkskin(caster,victim,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int rootControl(TBeing *caster, TBeing *victim, TMagicItem *tObj)
{
  int tLevel = tObj->getMagicLevel(),
    tKnown = tObj->getMagicLearnedness(),
    tReturn = 0;

  tReturn = rootControl(caster, victim, 0, tLevel, tKnown);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}

int rootControl(TBeing * caster, TBeing * victim, int, int dam, byte bKnown)
{
  dam = max(1,dam);
 
  if (!victim) {
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (victim->getPosition() <= POSITION_SLEEPING) {
    act("$N seems to already be on the $g!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (caster->roomp->notRangerLandSector() && !caster->roomp->isForestSector()) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    caster->sendTo("You need to be in nature or on land to cast this spell!\n\r");
    return SPELL_FAIL;
  }
  caster->reconcileHurt(victim,discArray[SPELL_ROOT_CONTROL]->alignMod);
 
  act("Large tree roots start to form in front of $N.", FALSE, caster , NULL, victim, TO_CHAR);
  act("Large tree roots start to form in front of $N.", TRUE, caster , NULL, victim, TO_NOTVICT);
  act("Large tree roots start to form in front of you.", TRUE, caster , NULL, victim, TO_VICT);

  if (bSuccess(caster,bKnown,SPELL_ROOT_CONTROL)) {
    act("$n commands the tree roots to trip $N, causing $M to fall to the $g.", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n commands the tree roots to trip you!", FALSE, caster, NULL, victim, TO_VICT);
    act("You command the tree roots to trip $N, causing $M to fall to the $g.", FALSE, caster, NULL, victim, TO_CHAR);
    if (victim->isLucky(caster->spellLuckModifier(SPELL_ROOT_CONTROL))) {
      SV(SPELL_ROOT_CONTROL);
      dam /= 2;
    } else if (critSuccess(caster, SPELL_ROOT_CONTROL) == CRIT_S_DOUBLE) {
      CS(SPELL_ROOT_CONTROL);
      dam *= 2;
    }
    victim->setPosition(POSITION_SITTING);
    victim->addToWait(combatRound(1));
    if (caster->reconcileDamage(victim, dam, SPELL_ROOT_CONTROL) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_ROOT_CONTROL) == CRIT_F_HITSELF) {
      CF(SPELL_ROOT_CONTROL);
      act("The roots turn on you, and cause you to trip!", FALSE, caster, NULL, victim, TO_CHAR);
      act("The roots that $n summoned turn against $m and cause $m to trip!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("The roots $n summoned to aid $m, trip $m instead!", FALSE, caster, NULL, victim, TO_VICT);
      caster->setPosition(POSITION_SITTING);
      if (caster->reconcileDamage(caster, dam, SPELL_ROOT_CONTROL) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_CRIT_FAIL;
    } else {
      act("The roots you summoned ignore you!", FALSE, caster, NULL, victim, TO_CHAR);
      act("The roots that $n summoned ignore $s commands.", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("The roots ignore $n's request to trip you.", FALSE, caster, NULL, caster, TO_VICT);
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_FAIL;
    }
  }
}
 
int rootControl(TBeing * caster, TBeing * victim)
{
  int ret;
  int rc = 0;
 
  if (!bPassMageChecks(caster, SPELL_ROOT_CONTROL, victim))
    return FALSE;

  int level = caster->getSkillLevel(SPELL_ROOT_CONTROL);
  int bKnown = caster->getSkillValue(SPELL_ROOT_CONTROL);
  int dam = caster->getSkillDam(victim, SPELL_ROOT_CONTROL, level, caster->getAdvLearning(SPELL_ROOT_CONTROL));
 
  ret=rootControl(caster,victim,level,dam,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}
 
// LIVING VINES

int livingVines(TBeing *caster, TBeing *victim,int level, byte bKnown)
{
  affectedData aff1, aff2;
 
  if (caster->isNotPowerful(victim, level, SPELL_LIVING_VINES, SILENT_NO)) {
    return SPELL_FAIL;
  }
  if (caster->roomp->notRangerLandSector() && !caster->roomp->isForestSector()) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    caster->sendTo("You need to be in nature or on land to cast this spell!\n\r");
    return SPELL_FAIL;
  }
 
  caster->reconcileHurt(victim, discArray[SPELL_LIVING_VINES]->alignMod);
 
  aff1.type = SPELL_LIVING_VINES;
  aff1.level = level;
  aff1.bitvector = AFF_WEB;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = (level / 2) + 5;
  aff1.duration = level * UPDATES_PER_MUDHOUR;
 
  aff2.type = SPELL_LIVING_VINES;
  aff2.level = level;
  aff2.bitvector = AFF_WEB;
  aff2.location = APPLY_SPELL_HITROLL;
  aff2.modifier = (-level*2);
  aff2.duration = level * UPDATES_PER_MUDHOUR;
   
  if (bSuccess(caster, bKnown, SPELL_LIVING_VINES)) {
    act("$n summons the vines to hold $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You command the vines to trap $N!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n summons the vines to ensnare you!", FALSE, caster, NULL, victim, TO_VICT);
    switch(critSuccess(caster, SPELL_LIVING_VINES)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_LIVING_VINES);
        aff1.duration *= 2;
        aff2.modifier *= 2;
        break;
      default:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_LIVING_VINES))) {
          SV(SPELL_LIVING_VINES);
          aff1.duration /= 2;
          aff2.modifier /= 2;
        }
    }
    victim->affectTo(&aff1);
    victim->affectTo(&aff2);
    caster->reconcileDamage(victim, 0,SPELL_LIVING_VINES);
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_LIVING_VINES)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_LIVING_VINES);
        act("$n traps $mself with the vines %s summoned!", FALSE, caster, NULL, NULL, TO_ROOM);
        act("Oops! You trap yourself in the vines you summoned!", FALSE, caster, NULL, victim, TO_CHAR);
        act("Hey, $n was trying to trap you with the vines %s summoned!", FALSE, caster, NULL, victim, TO_VICT);
        caster->affectTo(&aff1);
        caster->affectTo(&aff2);
        caster->reconcileDamage(victim, 0,SPELL_LIVING_VINES);
        return SPELL_CRIT_FAIL;
        break;
      default:
        break;
    }
    caster->sendTo("Nothing seems to happen.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}
 
void livingVines(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  livingVines(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}
 
void livingVines(TBeing *caster, TBeing *victim)
{
int ret,level;
 
  if (!bPassMageChecks(caster, SPELL_LIVING_VINES, victim))
    return;
 
  level = caster->getSkillLevel(SPELL_LIVING_VINES);
  int bKnown = caster->getSkillValue(SPELL_LIVING_VINES);
 
  if ((ret=livingVines(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
}

// END LIVING VINES
// TREE WALK
 
int TObj::treeMe(TBeing *, const char *, int, int *)
{
  return FALSE;
}

int treeWalk(TBeing * caster, const char * arg, int, byte bKnown)
{
  TBeing *ch = NULL;
  TObj *o;
  TRoom *rp = NULL;
  TThing *t, *t2, *t3;
  int rc;
  int numx, j = 1;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  act("You reach into the Sydarthae, in search of the life force of a powerful tree.", FALSE, caster, 0, 0, TO_CHAR);
  act("$n enters a trance.", FALSE, caster, 0, 0, TO_ROOM);


  for (;arg && *arg && isspace(*arg); arg++);

  if (bSuccess(caster,bKnown,SPELL_TREE_WALK)) {
    strcpy(tmpname, arg);
    tmp = tmpname;

    if (!(numx = get_number(&tmp)))
      numx = 1;

    for (o = object_list; o; o = o->next) {
      if (o->treeMe(caster, tmp, numx, &j)) {
        rp = o->roomp;
        if (rp)
          break;
      }
    }
    if (!o) {
      for (ch = character_list; ch; ch = ch->next) {
        if (ch->getRace() != RACE_TREE)
          continue;
        if (isname(tmp, ch->name)) {
          if (j >= numx) {
            rp = ch->roomp;
            if (rp) {
              act("You locate $N, and form a magical anchor between $M and you.", 
                    FALSE, caster, 0, ch, TO_CHAR);
              break;
            }
          }
          j++;
        }
      }
    }
    if (!o && !ch) {
      act("You fail to find any lifeforce by that name.", 
             FALSE, caster, 0, 0, TO_CHAR);
      act("$n snaps out of $s trance.", FALSE, caster, 0, 0, TO_ROOM);
      return SPELL_SUCCESS;
    }

    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (!tbt)
        continue;
      if (tbt->inGroup(*caster)) {
        act("A mystic force thrusts you into the Sydarthae, and out the otherside.",
           FALSE, tbt, 0, 0, TO_CHAR);
        act("A mystic force yanks $n into somewhere unknown.",
           FALSE, caster, 0, 0, TO_ROOM);

        while ((t3 = tbt->rider)) {
          TBeing *tb = dynamic_cast<TBeing *>(t3);
          if (tb) {
            rc = tb->fallOffMount(t, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }
          } else {
            t3->dismount(POSITION_DEAD);
          }
        }

        if (tbt->riding) {
          
          rc = tbt->fallOffMount(tbt->riding, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tbt;
            tbt = NULL;
          }
        }

        --(*tbt);
        *rp += *tbt;

        act("$n shimmers into existence.", FALSE, tbt, NULL, NULL, TO_ROOM);
        act("You shimmer into existence.", FALSE, tbt, NULL, NULL, TO_CHAR);

        tbt->doLook("", CMD_LOOK);

        rc = tbt->genericMovedIntoRoom(rp, -1);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          if (tbt != caster) {
            delete tbt;
            tbt = NULL;
          } else {
            return SPELL_SUCCESS + CASTER_DEAD;
          }
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    act("You fail to detect a life force strong enough to anchor yourself with.", FALSE, caster, 0, 0, TO_CHAR);
    act("$n snaps out of $s trance.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

int treeWalk(TBeing * caster, const char * arg)
{
  int ret,level;
 
  if (!bPassMageChecks(caster, SPELL_TREE_WALK, NULL))
    return FALSE;

  if (caster->roomp->isFlyingSector()) {
    caster->sendTo("You are unable to break through the magic.");
    return FALSE;
  }

  if (caster->fight()) {
    caster->sendTo("You are unable to commune with nature while fighting.");
    return FALSE;
  }
 
  level = caster->getSkillLevel(SPELL_TREE_WALK);
  int bKnown = caster->getSkillValue(SPELL_TREE_WALK);
 
  ret=treeWalk(caster,arg,level,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }

  if (IS_SET(ret, CASTER_DEAD))
    return DELETE_THIS;
  return FALSE;
}

// END TREE WALK

int TBeing::doEarthmaw(const char *argument)
{
  if (!doesKnowSkill(SPELL_EARTHMAW)) {
    sendTo("You do no know the secrets of the earthmaw spell.\n\r");
    return FALSE;
  }
  if (this->roomp->notRangerLandSector()) {
    sendTo("You must be in a wilderness landscape for the earthmaw spell to be effective!\n\r");
    return FALSE;
  }
  char    tTarget[256];
  TObj   *tObj    = NULL;
  TBeing *victim = NULL;
  TBeing *horsie = NULL;


  if (checkBusy(NULL))
    return FALSE;


  if (getMana() < 0) {
    sendTo("You lack the mana to split the earth.\n\r");
    return FALSE;
  }

  if (argument && *argument) {
    strcpy(tTarget, argument);
    generic_find(tTarget, FIND_CHAR_ROOM, this, &victim, &tObj);
  } else {
    if (!fight()) {
      sendTo("Who do you want to call the earthmaw upon?\n\r");
      return FALSE;
    } else
      victim = fight();
  }

  if (victim == NULL) {
    sendTo("There is no one by that name here.\n\r");
    return FALSE;
  } else if (victim == this) {
    this->sendTo("Do you really want to call the earthmaw upon yourself??\n\r");
    return FALSE;
  } else if (victim->isFlying()) {
    sendTo("You cannot call the earthmaw upon someone in the air.");
    return FALSE;
  }

  int lev = getSkillLevel(SPELL_EARTHMAW);
  int bKnown= getSkillValue(SPELL_EARTHMAW);

  int dam = getSkillDam(victim, SPELL_EARTHMAW, lev, getAdvLearning(SPELL_EARTHMAW));


  if (!useComponent(findComponent(SPELL_EARTHMAW), this, CHECK_ONLY_NO))
    return FALSE;

  addToWait((int)combatRound(discArray[SPELL_EARTHMAW]->lag));
  reconcileHurt(victim,discArray[SPELL_EARTHMAW]->alignMod);
  
  if (bSuccess(this, bKnown, SPELL_EARTHMAW)) {
    
    if (critSuccess(this, SPELL_EARTHMAW) == CRIT_S_DOUBLE) {
      CS(SPELL_EARTHMAW);
      dam *= 2;
      act("<Y>An incredibly large fissure opens up in the ground below you!<1>", FALSE, this, NULL, victim, TO_VICT);
      act("<Y>An incredibly large fissure opens up in the ground below $N<Y><o>!<1>", FALSE, this, NULL, victim, TO_NOTVICT);
      act("<Y>An incredibly large fissure opens up in the ground below $N<Y><o>!<1>", FALSE, this, NULL, victim, TO_CHAR);
    } else {
      act("<o>A large fissure opens up in the ground below you!<1>", FALSE, this, NULL, victim, TO_VICT);
      act("<o>A large fissure opens up in the ground below $N<1><o>!<1>", FALSE, this, NULL, victim, TO_NOTVICT);
      act("<o>A large fissure opens up in the ground below $N<1><o>!<1>", FALSE, this, NULL, victim, TO_CHAR);
    }

    if ((horsie = dynamic_cast<TBeing *> (victim->riding))) {
      act("$N collapses beneath $n as the $g gives way!",
          TRUE, victim, 0, horsie, TO_ROOM);
      act("$N collapses beneath you as the $g gives way!",
          TRUE, victim, 0, horsie, TO_CHAR);
      victim->fallOffMount(victim->riding, POSITION_SITTING);

      act("<o>$N<1><o> tumbles into the fissure!<1>", FALSE, this, NULL, horsie, TO_CHAR);
      act("<o>$N<1><o> tumbles into the fissure!<1>", FALSE, this, NULL, horsie, TO_NOTVICT);
      act("<o>You tumble into the fissure!<1>", FALSE, this, NULL, horsie, TO_VICT);

    }
    
    act("<o>$N<1><o> tumbles into the fissure, which collapses on top of $m!<1>" , FALSE, this, NULL, victim, TO_CHAR);
    act("<o>$N<1><o> tumbles into the fissure, which collapses on top of $m!<1>", FALSE, this, NULL, victim, TO_NOTVICT);
    act("<o>You tumble into the fissure, which collapses on top of you!<1>", FALSE, this, NULL, victim, TO_VICT);
    
    if (horsie && this->reconcileDamage(horsie, dam, SPELL_EARTHMAW) == -1) {
      delete horsie;
    }
    if (this->reconcileDamage(victim, dam, SPELL_EARTHMAW) == -1) {
      delete victim;
      return SPELL_SUCCESS + VICTIM_DEAD + DELETE_VICT;
    }
    return SPELL_SUCCESS;
    
  } else {
    
    act("You fail to call the earthmaw.", FALSE, this, NULL, victim, TO_CHAR);
    act("$n fails to call the earthmaw.", FALSE, this, NULL, victim, TO_ROOM);
    
    return SPELL_FAIL;
  }
  
  
  return SPELL_FAIL;
}

