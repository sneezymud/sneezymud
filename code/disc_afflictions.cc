#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_afflictions.h"
#include "obj_magic_item.h"

// returns DELETE_VICT
static int injureLimbs(TBeing *ch, TBeing *victim, spellNumT skill, int level, int adv_learn)
{
  // on some prayer failures, we don't want to not do anything so we
  // create this anti-salve routine.
  // damage a bunch of limbs
  int dam = ch->getSkillDam(victim, skill, level, adv_learn);
  dam = max(dam, 10);

  act("$d aids you to a lesser extent, damaging $N's limbs!",
          false, ch, 0, victim, TO_CHAR);
  act("$d blasts you, damaging your limbs!",
          false, ch, 0, victim, TO_VICT);
  act("$d blasts $N, damaging $S limbs!",
          false, ch, 0, victim, TO_NOTVICT);

  int num = 0;
  wearSlotT slot;
  for (num = 0; num < dam; num++) {
    int num2 = 0;  // prevent endless loops
    for (slot = MIN_WEAR; slot < MAX_WEAR && num2 < 10; slot++) {
      num2++;
      if (!victim->slotChance(slot))
        continue;
      if (!victim->getCurLimbHealth(slot))
        continue;
      int rc = victim->hurtLimb(1, slot);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    }    
  }
  return FALSE;
}
 
static bool checkForTorment(const TBeing *ch)
{
  affectedData *hjp;

  for (hjp = ch->affected; hjp; hjp = hjp->next) {
    if (hjp->type == AFFECT_SKILL_ATTEMPT) {
      if (hjp->modifier == SPELL_WITHER_LIMB)
        return TRUE;
      if (hjp->modifier == SPELL_BONE_BREAKER)
        return TRUE;
      if (hjp->modifier == SPELL_PARALYZE_LIMB)
        return TRUE;
      if (hjp->modifier == SPELL_NUMB)
        return TRUE;
      if (hjp->modifier == SPELL_NUMB_DEIKHAN)
        return TRUE;
    }
  }
  return FALSE;
}

static void addTorment(TBeing * victim, spellNumT spell)
{
  affectedData hjp;
  hjp.type = AFFECT_SKILL_ATTEMPT;
  hjp.modifier = spell;
  hjp.duration = 30 * UPDATES_PER_MUDHOUR;

  victim->affectTo(&hjp);
}

int harm(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  level = min(level, 70);

  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    switch(critSuccess(caster, spell)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(spell);
        dam *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (victim->isLucky(caster->spellLuckModifier(spell))) {
      SV(spell);
      dam /= 2;
    }

    act("$N buckles from the pain!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N buckles from the pain!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You buckle from the pain! You could swear you're about to die!",
                  FALSE, caster, NULL, victim, TO_VICT);
    if (caster->reconcileDamage(victim, dam, spell) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        act("Something goes tragically wrong and $n buckles in pain!",
                FALSE, caster, NULL, NULL, TO_ROOM);
        act("Something goes tragically wrong and you buckle in pain!",
                FALSE, caster, NULL, NULL, TO_CHAR);
        act("That pain was intended for you!",
                FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, spell) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        caster->deityIgnore();
        act("That pain was intended for you!",
                FALSE, caster, NULL, victim, TO_VICT);
        break;
    }
    return SPELL_FAIL;
  }
}

int harm(TBeing * caster, TBeing * victim, TMagicItem *obj, spellNumT spell)
{
  int ret, rc = 0;

  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  ret=harm(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harm(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_HARM);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);

  if (caster != victim) {
    act("You invoke an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_CHAR);
    act("$n invokes an injurious prayer against you.",
          FALSE, caster, 0, victim, TO_VICT);
    act("$n invokes an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_NOTVICT);
  } else {
    act("You invoke an injurious prayer against yourself.",
          FALSE, caster, 0, 0, TO_CHAR);
    act("$n invokes an injurious prayer against $mself.",
          FALSE, caster, 0, 0, TO_ROOM);
  }

  int ret=harm(caster,victim,level,caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int poison(TBeing * caster, TObj * obj, int, byte bKnown, spellNumT spell)
{
  int rc;

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    rc = obj->poisonObject();
    if (rc) {
      return SPELL_SUCCESS;
    }
    caster->sendTo("You can't poison non-consummables.\n\r");
    return SPELL_FAIL;
  } else {
    return SPELL_FAIL;
  }
}

void poison(TBeing * caster, TObj * obj)
{
  spellNumT spell = caster->getSkillNum(SPELL_POISON);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=poison(caster,obj,level,caster->getSkillValue(spell), spell);
  if (ret == SPELL_SUCCESS) {
    act("You succeed in poisoning $p.", TRUE, caster, obj, 0, TO_CHAR);
    caster->deityIgnore(SILENT_YES);  // intentional  :)
    vlogf(LOG_MISC, fmt("%s poisoned %s.") %  caster->getName() % obj->getName());
  } else {
    caster->deityIgnore();
  }
}

int poison(TBeing * caster, TObj * target, TMagicItem *obj, spellNumT spell)
{
  int ret;

  ret=poison(caster,target,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (ret == SPELL_SUCCESS) {
    act("You succeed in poisoning $p.", TRUE, caster, obj, 0, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    vlogf(LOG_MISC, fmt("%s poisoned %s.") %  caster->getName() % obj->getName());
  } else {
    caster->deityIgnore();
  }
  return FALSE;
}

int poison(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell)
{
  affectedData aff, aff2;

  if (victim->isImmune(IMMUNE_POISON)) {
    act("Your prayer seems to have no affect on $N!", 
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to poison you. Luckily you are immune.", 
        FALSE, caster, NULL, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (victim->affectedBySpell(SPELL_POISON) ||
      victim->affectedBySpell(SPELL_POISON_DEIKHAN)) {
    act("You can't poison $N. $E is already poisoned!", 
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to poison you again!", 
        FALSE, caster, NULL, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (caster->isNotPowerful(victim, level, spell, SILENT_YES)) {
    act("Your prayer seems to have little or no affect on $N!", 
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to poison you. Lucky $e was so weak.", 
        FALSE, caster, NULL, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  aff.type = SPELL_POISON;
  aff.level = level;
  aff.duration = (2 + level/10) * UPDATES_PER_MUDHOUR;
  aff.modifier = -20;
  aff.location = APPLY_STR;
  aff.bitvector = AFF_POISON;
  aff.duration = (int) (caster->percModifier() * aff.duration);

  // we'll be tweaking duration, so we'll set it at the end
  aff2.type = AFFECT_DISEASE;
  aff2.level = 0;
  aff2.modifier = DISEASE_POISON;
  aff2.location = APPLY_NONE;
  aff2.bitvector = AFF_POISON;

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    if (victim->isLucky(caster->spellLuckModifier(spell))) {
      SV(spell);
      aff.duration /= 2;
    }
    switch (critSuccess(caster, spell)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(spell);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
 
    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_POISON));
    aff.duration /= 100;

    aff2.duration = aff.duration;
    victim->affectTo(&aff);
    victim->affectTo(&aff2);
    disease_start(victim, &aff2);

    return SPELL_SUCCESS;
  } else {
    switch(critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);

        //we've made raw immunity check, but allow it to reduce effects too
        aff.duration *= (100 - caster->getImmunity(IMMUNE_POISON));
        aff.duration /= 100;

        caster->affectTo(&aff);

        aff2.duration = aff.duration;
        caster->affectTo(&aff2);

        disease_start(caster, &aff2);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    return SPELL_FAIL;
  }
}

void poison(TBeing * caster, TBeing * victim)
{
  char buf[256], gender_desc[10];

  spellNumT spell = caster->getSkillNum(SPELL_POISON);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=poison(caster,victim,level,caster->getSkillValue(spell), spell);

  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster != victim) {
      sprintf(gender_desc, "%s", (!caster->getSex() ? "eunic" : (caster->getSex() == 1 ? "bastard" : "bitch")));
      sprintf(buf, "That %s $n just poisoned $N!", gender_desc);
      act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
      sprintf(buf, "That %s $n just poisoned you!", gender_desc);
      act(buf, FALSE, caster, NULL, victim, TO_VICT);
      act("You fill $N's veins with poison!", FALSE, caster, NULL, victim, TO_CHAR);
    } else {
      caster->sendTo("Way to go!  You've poisoned yourself!!\n\r");
      act("$n looks sheepish as $e realizes $e just poisoned $mself.",
          TRUE, caster, 0, 0, TO_ROOM);
    }
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster != victim) {
      sprintf(gender_desc, "%s", (!caster->getSex() ? "eunic" : (caster->getSex() == 1 ? "bastard" : "bitch")));
      sprintf(buf, "That %s $n just poisoned $N seriously!", gender_desc);
      act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
      sprintf(buf, "That %s $n just poisoned you seriously!", gender_desc);
      act(buf, FALSE, caster, NULL, victim, TO_VICT);
      act("You fill $N's veins with a serious poison!", FALSE, caster, NULL, victim, TO_CHAR);
    } else {
      act("Way to go!  You've poisoned yourself badly!", 
          FALSE, caster, NULL, NULL, TO_VICT);
      act("$n looks sheepish as $e realizes $e just poisoned $mself seriously.",          TRUE, caster, 0, 0, TO_ROOM);
    }
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("$n seems to have poisoned $mself!", 
          FALSE, caster, NULL, caster, TO_NOTVICT);
    act("Oh no! You seem to have poisoned yourself!", 
          FALSE, caster, NULL, NULL, TO_CHAR);
    act("That poison was intended for you!", 
          FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_FAIL)) {
    caster->sendTo("You fail to poison your enemy.\n\r");
    act("$n just tried to poison you!", FALSE, caster, NULL, victim, TO_VICT);
    caster->deityIgnore();
  } else {
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

int poison(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;

  ret=poison(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster == victim) {
      caster->sendTo("Don't you feel stupid for poisoning yourself.\n\r");
      act("$n looks sheepish as $e realizes $e just poisoned $mself.",
         TRUE, caster, 0, 0, TO_ROOM);
    } else {
      act("Poison courses through your veins!",
          FALSE, victim, 0, 0, TO_CHAR);
      act("Poison courses through $n's veins!",
          FALSE, victim, 0, 0, TO_ROOM);

    }
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster == victim) {
      caster->sendTo("Don't you feel stupid for poisoning yourself.\n\r");
      act("$n looks sheepish as $e realizes $e just poisoned $mself.",
         TRUE, caster, 0, 0, TO_ROOM);
    } else {
      act("Poison courses through your veins!",
          FALSE, victim, 0, 0, TO_CHAR);
      act("Poison courses through $n's veins!",
          FALSE, victim, 0, 0, TO_ROOM);

    }
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    caster->sendTo("Oops!  it backfired on you and now you're poisoned!\n\r");
    act("Oops!  it backfired on you and now you're poisoned!", 
          TRUE, caster, 0, victim, TO_CHAR);
    act("$n just tried to poison you!", 
          TRUE, caster, 0, victim, TO_VICT);
    act("$n's action backfired on $m and now $e is poisoned.",
          TRUE, caster, 0, victim, TO_NOTVICT);
  } else if (IS_SET(ret, SPELL_FAIL)){
    act("Nothing seems to happen. You fail to cast poison successfully.", 
          TRUE, caster, 0, 0, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
  } else {
    // where various falses return that we dont want messages
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return FALSE;  
}

int blindness(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  int ret = 0;

  if (victim->affectedBySpell(SPELL_BLINDNESS)) {
    act("You can't blind $N since $E is already blind!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to blind you again!", 0, caster, NULL, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }
  if (victim->isAffected(AFF_TRUE_SIGHT)) {
    act("$N's vision is much too clear to be blinded!",FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }
  if (caster->isNotPowerful(victim, level, SPELL_BLINDNESS, SILENT_YES)) {
    act("You can't blind $N!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to blind you!", FALSE, caster, NULL, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_BLINDNESS)) {
    caster->reconcileHurt(victim, discArray[SPELL_BLINDNESS]->alignMod);

    ret=SPELL_SUCCESS;
    switch (critSuccess(caster, SPELL_BLINDNESS)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLINDNESS);
        ret+=SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (victim->isLucky(caster->spellLuckModifier(SPELL_BLINDNESS))) {
      SV(SPELL_BLINDNESS);
      ret+=SPELL_SAVE;
    }

    return ret;
  } else {
    ret=SPELL_FAIL;

    switch(critFail(caster, SPELL_BLINDNESS)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_BLINDNESS);
        ret+=SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    return ret;
  }
}

void blindness(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret;
  int duration, level = obj->getMagicLevel();

  ret=blindness(caster,victim,level,obj->getMagicLearnedness());

  duration = (obj->getMagicLevel()/10 + 1) * UPDATES_PER_MUDHOUR;
  saveTypeT save = SAVE_NO;
  if (IS_SET(ret, SPELL_SAVE)) 
    save = SAVE_YES;

  if (IS_SET(ret, SPELL_SUCCESS) && !IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster != victim) {
      act("A thin cloudy film appears over $N's eyes.",
              FALSE, caster, NULL, victim, TO_CHAR);
      act("A thin cloudy film appears over $N's eyes.",
              FALSE, caster, NULL, victim, TO_NOTVICT);
      act("A thin cloudy film appears over your eyes.",
              FALSE, caster, NULL, victim, TO_VICT);
      act("$n has used $p to blind $N -- $E's virtually helpless!",
              FALSE, caster, obj, victim, TO_NOTVICT);
      act("You have used $p to blind $N -- $E's virtually helpless!",
              FALSE, caster, obj, victim, TO_CHAR);
      act("$n has used $p to blind you! You're virtually helpless!",
              FALSE, caster, obj, victim, TO_VICT);
    } else {
      act("A thin cloudy film appears over your eyes.",
              FALSE, caster, NULL, NULL, TO_CHAR);
      act("A thin cloudy film appears over $n's eyes.",
              FALSE, caster, NULL, NULL, TO_ROOM);
      act("You have used $p to blind yourself -- you're virtually helpless!",
              FALSE, caster, obj, NULL, TO_CHAR);
      act("$n has used $p to blind $mself -- $e's virtually helpless!",
              FALSE, caster, obj, NULL, TO_ROOM);
    }
    victim->rawBlind(level, duration, save);
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster != victim) {
      act("A dense dark film appears over $N's eyes.",
          TRUE, caster, NULL, victim, TO_CHAR);
      act("A dense dark film appears over $N's eyes.",
              TRUE, caster, NULL, victim, TO_NOTVICT);
      act("A dense dark film appears over your eyes.",
              FALSE, caster, NULL, victim, TO_VICT);
      act("$n has used $p to blind $N -- $E's virtually helpless!",
              TRUE, caster, obj, victim, TO_NOTVICT);
      act("You have used $p to blind $N -- $E's virtually helpless!",
              FALSE, caster, obj, victim, TO_CHAR);
      act("$n has used $p to blind you! You're virtually helpless!",
              FALSE, caster, obj, victim, TO_VICT);
    } else {
      act("A dense dark film appears over your eyes.",
          TRUE, caster, NULL, NULL, TO_CHAR);
      act("A dense dark film appears over $n's eyes.",
              TRUE, caster, NULL, NULL, TO_ROOM);
      act("You have used $p to blind yourself -- you're virtually helpless!",
              FALSE, caster, obj, NULL, TO_CHAR);
      act("$n has used $p to blind $mself -- $e's virtually helpless!",
              TRUE, caster, obj, NULL, TO_ROOM);
    }
    duration *= 2;
    victim->rawBlind(level, duration, save);
  } else if (IS_SET(ret, SPELL_FAIL)) {
    act("Poor $n...can't even figure out how to use $p right!",
            FALSE, caster, obj, victim, TO_NOTVICT);
    act("Hmm, $N doesn't seem to be blinded by $p.",
            TRUE, caster, obj, victim, TO_CHAR);
    act("$n was trying to harm you with $p !",
            TRUE, caster, obj, victim, TO_VICT);

    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
      act("$n seems to have blinded $mself with $p!",
            FALSE, caster, obj, NULL, TO_ROOM);
      act("Ack, you seem to have blinded yourself with $p.",
            FALSE, caster, obj, NULL, TO_CHAR);
      caster->rawBlind(level, duration, SAVE_YES);
    }
  } else if (IS_SET(ret, SPELL_FALSE)) {
  }

  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

void blindness(TBeing * caster, TBeing * victim)
{
  if (!bPassClericChecks(caster,SPELL_BLINDNESS))
    return;

  int level = caster->getSkillLevel(SPELL_BLINDNESS);

  int ret=blindness(caster,victim,level,caster->getSkillValue(SPELL_BLINDNESS));

  int duration = (level/10 + 1) * UPDATES_PER_MUDHOUR;
  duration = (int) (caster->percModifier() * duration);

  saveTypeT save = SAVE_NO;
  if (IS_SET(ret, SPELL_SAVE))
    save = SAVE_YES;
  
  if (IS_SET(ret, SPELL_SUCCESS) && !IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    act("A thin cloudy film appears over $N's eyes.",
        TRUE, caster, NULL, victim, TO_CHAR);
    act("A thin cloudy film appears over $N's eyes.",
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("A thin cloudy film appears over your eyes.",
        FALSE, caster, NULL, victim, TO_VICT);
    act("$n has blinded $N -- $E's virtually helpless!",
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("You have blinded $N -- $E's virtually helpless!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n has blinded you! You're virtually helpless!",
        FALSE, caster, NULL, victim, TO_VICT);
    victim->rawBlind(level, duration, save);
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    act("A dense dark film appears over $N's eyes.",
        TRUE, caster, NULL, victim, TO_CHAR);
    act("A dense dark film appears over $N's eyes.",
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("A dense dark film appears over your eyes.",
        FALSE, caster, NULL, victim, TO_VICT);
    act("$n has blinded $N -- $E's virtually helpless!",
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("You have blinded $N -- $E's virtually helpless!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n has blinded you! You're virtually helpless!",
        FALSE, caster, NULL, victim, TO_VICT);
    duration *= 2;
    victim->rawBlind(level, duration, save);
  } else if (IS_SET(ret, SPELL_FAIL)) {
    act("Poor $n...can't even invoke a little prayer!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Hmm, your attempt to blind $N failed.",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n was trying to harm you with a prayer!",
        FALSE, caster, NULL, victim, TO_VICT);

    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
      act("$n seems to have blinded $mself!",
          FALSE, caster, NULL, NULL, TO_ROOM);
      act("Ack, you seem to have blinded yourself.",
          FALSE, caster, NULL, NULL, TO_CHAR);
      caster->rawBlind(level, duration, SAVE_YES);
    }
  }
  if (IS_SET(ret, SPELL_FALSE)) {
  }

  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

int harmLight(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  level = min(level, 10);

  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    switch (critSuccess(caster, spell)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(spell);
        dam *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    act("$n is lightly hurt!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You are lightly hurt!", FALSE, victim, NULL, NULL, TO_CHAR);
    if (caster->reconcileDamage(victim, dam, spell) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch(critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        act("Something goes tragically wrong and you have lightly hurt yourself!",
               FALSE, caster, NULL, NULL, TO_CHAR);
        act("Something goes tragically wrong and $n has lightly hurt $mself!",
               FALSE, caster, NULL, NULL, TO_ROOM);
        act("$n's injury was intended for you!",
               FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, spell) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        act("That pain was intended for you!",
                FALSE, caster, NULL, victim, TO_VICT);
        caster->deityIgnore();
        break;
    }
    return SPELL_FAIL;
  }
}

int harmLight(TBeing * caster, TBeing * victim, TMagicItem *obj, spellNumT spell)
{
  int ret, rc = 0;

  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  ret=harmLight(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harmLight(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_HARM_LIGHT);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);

  if (caster != victim) {
    act("You invoke an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_CHAR);
    act("$n invokes an injurious prayer against you.",
          FALSE, caster, 0, victim, TO_VICT);
    act("$n invokes an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_NOTVICT);
  } else {
    act("You invoke an injurious prayer against yourself.",
          FALSE, caster, 0, 0, TO_CHAR);
    act("$n invokes an injurious prayer against $mself.",
          FALSE, caster, 0, 0, TO_ROOM);
  }
  int ret=harmLight(caster,victim,level,caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret,SPELL_CRIT_FAIL)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harmCritical(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    return SPELL_FAIL;
  }

  level = min(level, 45);

  caster->reconcileHurt(victim, discArray[spell]->alignMod);
  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    if (critSuccess(caster, spell)) {
      CS(spell);
      dam *= 2;
    }

    act("$n is critically hurt!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You are critically hurt!", FALSE, victim, NULL, NULL, TO_CHAR);
    if (caster->reconcileDamage(victim, dam, spell) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, spell) != CRIT_F_NONE) {
      CF(spell);
      act("Something goes tragically wrong and you have critically hurt yourself!",
             FALSE, caster, NULL, NULL, TO_CHAR);
      act("Something goes tragically wrong and $n has critically hurt $mself!",
               FALSE, caster, NULL, NULL, TO_ROOM);
      act("$n's injury was intended for you!",
               FALSE, caster, NULL, victim, TO_VICT);
      if (caster->reconcileDamage(caster, dam, spell) == -1)
        return SPELL_SUCCESS + CASTER_DEAD;
      return SPELL_SUCCESS;
    } else {
      caster->deityIgnore();
      act("That pain was intended for you!",
                FALSE, caster, NULL, victim, TO_VICT);
      return SPELL_FAIL;
    }
  }
}

int harmCritical(TBeing * caster, TBeing * victim, TMagicItem *obj, spellNumT spell)
{
  int ret, rc = 0;

  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  ret=harmCritical(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harmCritical(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_HARM_CRITICAL);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);

  if (caster != victim) {
    act("You invoke an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_CHAR);
    act("$n invokes an injurious prayer against you.",
          FALSE, caster, 0, victim, TO_VICT);
    act("$n invokes an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_NOTVICT);
  } else {
    act("You invoke an injurious prayer against yourself.",
          FALSE, caster, 0, 0, TO_CHAR);
    act("$n invokes an injurious prayer against $mself.",
          FALSE, caster, 0, 0, TO_ROOM);
  }
  int ret=harmCritical(caster,victim,level,caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret,SPELL_CRIT_FAIL)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harmSerious(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    return SPELL_FAIL;
  }

  level = min(level, 25);

  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    if (critSuccess(caster, spell)) {
      CS(spell);
      dam *= 2;
    }

    act("$n is seriously hurt!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You are seriously hurt!", FALSE, victim, NULL, NULL, TO_CHAR);
    if (caster->reconcileDamage(victim, dam, spell) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, spell) != CRIT_F_NONE) {
      CF(spell);
      act("Something goes tragically wrong and you have seriously hurt yourself!",
             FALSE, caster, NULL, NULL, TO_CHAR);
      act("Something goes tragically wrong and $n has seriously hurt $mself!",
               FALSE, caster, NULL, NULL, TO_ROOM);
      act("$n's injury was intended for you!",
               FALSE, caster, NULL, victim, TO_VICT);
      if (caster->reconcileDamage(caster, dam, spell) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    } else {
      caster->deityIgnore();
      act("That pain was intended for you!",
                FALSE, caster, NULL, victim, TO_VICT);
    }
    return SPELL_FAIL;
  }
}

int harmSerious(TBeing * caster, TBeing * victim, TMagicItem *obj, spellNumT spell)
{
  int ret, rc = 0;

  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  ret=harmSerious(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int harmSerious(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_HARM_SERIOUS);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);

  if (caster != victim) {
    act("You invoke an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_CHAR);
    act("$n invokes an injurious prayer against you.",
          FALSE, caster, 0, victim, TO_VICT);
    act("$n invokes an injurious prayer against $N.",
          FALSE, caster, 0, victim, TO_NOTVICT);
  } else {
    act("You invoke an injurious prayer against yourself.",
          FALSE, caster, 0, 0, TO_CHAR);
    act("$n invokes an injurious prayer against $mself.",
          FALSE, caster, 0, 0, TO_ROOM);
  }
  int ret=harmSerious(caster,victim,level,caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret,SPELL_CRIT_FAIL)) {
  } else {
  } 
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int paralyze(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  int save1, ret;
  affectedData aff;

  if (victim->isUndead()) {
    act("The Undead completely ignore the affects of paralyzation.",
         FALSE,caster,0,0,TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (victim->isImmune(IMMUNE_PARALYSIS)) {
    act("Your prayer seems to have no effect on $N!",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    victim->failPara(caster);
    return SPELL_FALSE;
  }

  if (caster->isNotPowerful(victim, level, SPELL_PARALYZE, SILENT_YES)) {
    act("$N's movements stop for a split second but $E recovers quickly!", 
        FALSE, caster, NULL, victim, TO_CHAR);      
    act("$N's movements stop for a split second but $E recovers quickly!", 
        FALSE, caster, NULL, victim, TO_NOTVICT);
    victim->failPara(caster);
    return SPELL_FALSE;
  }

  if (victim->isAffected(AFF_PARALYSIS)) {
    act("Someone must really want you to be paralyzed!", 
        FALSE, caster, NULL, victim, TO_VICT);
    act("Try paralyzing someone who can still move.", 
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FALSE;
  }

  if (victim->isImmune(IMMUNE_PARALYSIS)) {
    act("Your prayer seems to have little or no effect on $N!",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    victim->failPara(caster);
    return SPELL_FALSE;
  }

  aff.type = SPELL_PARALYZE;
  aff.level = level;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_PARALYSIS;
  aff.modifier = 0;

  // balance notes, each "duration" is a complete round out of action
  // to compare to other spell damages, we assume mob does 1.20 * lev
  // dam per round
  // clerics ought to be doing 1.60 * lev dam per round.
  // theoretically, that means paralyze ought to be 4/3 * difficulty
  // modifier of 100/60 = 20/9 = 2.25
  // anyway, lets bump it up to 3 rounds
  aff.duration = 3;

  save1 = victim->isLucky(caster->spellLuckModifier(SPELL_PARALYZE));

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_PARALYZE)) {
    caster->reconcileHurt(victim, discArray[SPELL_PARALYZE]->alignMod);
    if (save1) {
      SV(SPELL_PARALYZE);
      return SPELL_SAVE;
    }

    ret = SPELL_SUCCESS;
    switch(critSuccess(caster, SPELL_PARALYZE)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_PARALYZE);
        aff.duration *= 2;
        ret = SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        break;
    }

    //we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_PARALYSIS));
    aff.duration /= 100;

    victim->affectTo(&aff);

    if (victim->riding && dynamic_cast<TBeing *>(victim->riding))
      victim->fallOffMount(victim->riding, POSITION_STUNNED);

    victim->setPosition(POSITION_STUNNED);
    return ret;
  } else {
    switch (critFail(caster, SPELL_PARALYZE)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_PARALYZE);
        if (!caster->isImmune(IMMUNE_PARALYSIS)) {
          // we've made raw immunity check, but allow it to reduce effects too
          aff.duration *= (100 - caster->getImmunity(IMMUNE_PARALYSIS));
          aff.duration /= 100;

          caster->affectTo(&aff);
          caster->setPosition(POSITION_STUNNED);
          return SPELL_CRIT_FAIL;
        } else {
        return SPELL_FAIL;
        }
        break;
      case CRIT_F_NONE:
        break;
    }
    return SPELL_FAIL;
  }
}

void paralyze(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret;

  ret = paralyze(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("A thin cloud of icy air has surrounded $N!", 
            FALSE, caster, NULL, victim, TO_NOTVICT);
    act("A thin cloud of icy air has you surrounded you!",           
            FALSE, caster, NULL, victim, TO_VICT);
    act("$N falls down!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N falls down!",
            FALSE, caster, NULL, victim, TO_CHAR);
    act("$N stops moving -- $n has paralyzed $M!", 
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N stops moving -- you have paralyzed $M!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("You fall down!\n\rYour limbs freeze in place -- $n has paralyzed you!",
           FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_SAVE)) {
    act("A thin cloud of icy air has surrounded $N!", 
            FALSE, caster, NULL, victim, TO_NOTVICT);
    act("A thin cloud of icy air has you surrounded you!",           
            FALSE, caster, NULL, victim, TO_VICT);
    act("$d takes pity on $m, and $e breaks free of the paralysis!",
        FALSE, victim, NULL, NULL, TO_ROOM);
    act("$d takes pity on you, and you break free of the paralysis!",
        FALSE, victim, NULL, NULL, TO_CHAR);
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    act("A large icy white cloud of air has totally encased $N!",
            FALSE, caster, NULL, victim, TO_NOTVICT);
    act("A large icy white cloud of air has totally encased you!",
            FALSE, caster, NULL, victim, TO_VICT);
    act("$N falls down!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N falls down!",
            FALSE, caster, NULL, victim, TO_CHAR);
    act("$N stops moving -- $n has paralyzed $M!", 
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N stops moving -- you have paralyzed $M!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("You fall down!\n\rYour limbs freeze in place -- $n has paralyzed you!", 
        FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_FAIL)) {
    act("Your prayer fails and you are unable to paralyze $N.",
            FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("$n just paralyzed $mself!", 
            FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Ack! The affects of the prayer have misfired.",
            FALSE, caster, NULL, victim, TO_CHAR);
    act("Your limbs freeze in place!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("Hey, $n was trying to paralyze you but got $mself instead!", 
            FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_FALSE)) {
  }

  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

void paralyze(TBeing * caster, TBeing * victim)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_PARALYZE))
    return;

  level = caster->getSkillLevel(SPELL_PARALYZE);

  ret=paralyze(caster,victim,level,caster->getSkillValue(SPELL_PARALYZE));
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("A thin cloud of icy air surrounds $n!",
        FALSE, victim, NULL, NULL, TO_ROOM);
    act("A thin cloud of icy air surrounds you!",
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$N falls down!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N falls down!",
            FALSE, caster, NULL, victim, TO_CHAR);
    act("$N stops moving -- $n has paralyzed $M!", 
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N stops moving -- you have paralyzed $M!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("You fall down!\n\rYour limbs freeze in place -- $n has paralyzed you!",        FALSE, caster, NULL, victim, TO_VICT);
  }
  if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    act("A large icy white cloud of air totally encases $n!",
        FALSE, victim, NULL, NULL, TO_ROOM);
    act("A large icy white cloud of air totally encases you!",
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$N falls down!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N falls down!",
            FALSE, caster, NULL, victim, TO_CHAR);
    act("$N stops moving -- $n has paralyzed $M!", 
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N stops moving -- you have paralyzed $M!", 
            FALSE, caster, NULL, victim, TO_CHAR);
    act("You fall down!\n\rYour limbs freeze in place -- $n has paralyzed you!", 
         FALSE, caster, NULL, victim, TO_VICT);
  }
  if (IS_SET(ret, SPELL_SAVE)) {
    act("A thin cloud of icy air surrounds $n!",
        FALSE, victim, NULL, NULL, TO_ROOM);
    act("A thin cloud of icy air surrounds you!",
        FALSE, victim, NULL, NULL, TO_CHAR);
    act("$d takes pity on $m, and $e breaks free of the paralysis!",
        FALSE, victim, NULL, NULL, TO_ROOM);
    act("$d takes pity on you, and you break free of the paralysis!",
        FALSE, victim, NULL, NULL, TO_CHAR);
  }
  if (IS_SET(ret, SPELL_FAIL)) {
    act("You prayer fails and you are unable to paralyze $N.",
          FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
  } 
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("$n just paralyzed $mself!",
        FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Ack! Your prayer affects misfired on you.",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("Your limbs freeze in place!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("Hey, $n was trying to paralyze you but got $mself instead!",
        FALSE, caster, NULL, victim, TO_VICT);
  }
  if (IS_SET(ret, SPELL_FALSE)) {
  } 

  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

bool notBreakSlot(wearSlotT slot, bool avoid)
{
  if (slot == WEAR_HEAD || slot == WEAR_NECK ||
          slot == WEAR_BACK || slot == WEAR_BODY ||
          slot == WEAR_WAISTE ||
          slot == HOLD_RIGHT || slot == HOLD_LEFT)
    return true;
  else if (!avoid)
    return false;
  else {
    switch (slot) {
      case WEAR_ARM_R:
      case WEAR_ARM_L:
      case WEAR_WRIST_R:
      case WEAR_WRIST_L:
      case WEAR_HAND_R:
      case WEAR_HAND_L:
      case WEAR_FINGER_R:
      case WEAR_FINGER_L:
        // avoid to great extent
        if (::number(0,9))
          return true;
        return false;
      default:
        return false;
    }
  }
}

bool TBeing::canBoneBreak(TBeing *victim, silentTypeT silent)
{
  if (victim->raceHasNoBones()) {
    if (!silent) {
      act("$N has no bones worth breaking.", FALSE, this, 0, victim, TO_CHAR);
      deityIgnore(SILENT_YES);
    }
    return false;
  }

  if (checkForTorment(victim)) {
    if (!silent) {
      act("$d refuses to torment $N any further.",
           false, this, 0, victim, TO_CHAR);
      deityIgnore(SILENT_YES);
    }
    return false;
  }

  bool found = FALSE;
  bool ok = FALSE;
  wearSlotT slot;

  // check whether or not there is a bone left to break 
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))
      continue;
    if (!victim->slotChance(slot))
      continue;
    found |= victim->isLimbFlags(slot, PART_BROKEN);
    ok = TRUE;
  }
  if (!ok) {
    // has no unbroken limbs left
    if (!silent) {
      act("Belatedly, you realize you can not break any more limbs on $N!", FALSE, this, NULL, victim, TO_CHAR);
      act("Your prayer goes for naught.", FALSE, this, NULL, victim, TO_CHAR);
      deityIgnore(SILENT_YES);
    }
    return false;
  }
  if (found) {
    if (!silent) {
      act("$d refuses to let you break another of $N's limbs!",
            FALSE, this, NULL, victim, TO_CHAR);
      act("Your attack goes for naught.",
            FALSE, this, NULL, victim, TO_CHAR);
      deityIgnore(SILENT_YES);
    }
    return false;
  }

  return true;
}

int boneBreaker(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  char buf[256], limb[256];
  int ret;
  wearSlotT slot;

  if (!caster->canBoneBreak(victim, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FALSE;
  }

  if (caster->isNotPowerful(victim, level, SPELL_BONE_BREAKER, SILENT_YES)) {
    act("You see a glow around $N's limbs but it disappears fast.",
            FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FALSE;
  }

  if (victim->isImmune(IMMUNE_BONE_COND)) {
    act("You see a glow around $N's limbs but it has no effect.", 
           FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FALSE;
  }

  int dam = number(1, (level / 5));
  dam = (int) (caster->percModifier() * dam);
  dam = max(1,dam);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_BONE_BREAKER)) {
    addTorment(victim, SPELL_BONE_BREAKER);

    caster->reconcileHurt(victim, discArray[SPELL_BONE_BREAKER]->alignMod);

    // find a suitable bone to break 
    for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
      if (notBreakSlot(slot, true))
        continue;
      if (!victim->slotChance(slot) || victim->isLimbFlags(slot, PART_BROKEN))
        continue;
      break;
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->addToLimbFlags(slot, PART_BROKEN);
    victim->sendTo("You hear a muffled SNAP!\n\r");
    sprintf(buf, "Extreme pain shoots through your %s!\n\rYour %s has been broken and is now useless!", limb, limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);

    if (victim->awake())
      sprintf(buf, "You hear a muffled SNAP as $n clutches $s %s in extreme pain!", limb);
    else
      sprintf(buf, "You hear a muffled SNAP as $n's %s is shattered!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);

    ret = SPELL_SUCCESS;
    switch(critSuccess(caster, SPELL_BONE_BREAKER)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BONE_BREAKER);
          dam *= 2;
        if (!victim->isImmune(IMMUNE_PARALYSIS)) {
          victim->addToLimbFlags(slot, PART_PARALYZED);
          sprintf(buf, "In fact, you can't even move your %s! You fear it may be paralyzed!", limb);
          act(buf, FALSE, caster, NULL, victim, TO_VICT);
          sprintf(buf, "$n's %s is hanging there uselessly! It's been paralyzed as well!", limb);
          act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
          ret = SPELL_CRIT_SUCCESS;
        }
        break;
      case CRIT_S_NONE:
        break;
    } 

    victim->dropWeapon(slot);
    if (caster->reconcileDamage(victim, dam, SPELL_BONE_BREAKER) == -1)
      ret += VICTIM_DEAD;

    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }

    return ret;
  } else {
    switch (critFail(caster, SPELL_BONE_BREAKER)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_BONE_BREAKER);
        if (!caster->isImmune(IMMUNE_BONE_COND)) {
          for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
            if (notBreakSlot(slot, false))
              continue;
            if (!caster->slotChance(slot) ||
              caster->isLimbFlags(slot, PART_BROKEN))
              continue;
            break;
          }
            
          act("Your prayer backfires on you!", FALSE, caster, NULL, NULL, TO_CHAR);
          sprintf(limb, "%s", caster->describeBodySlot(slot).c_str());
          caster->addToLimbFlags(slot, PART_BROKEN);
          sprintf(buf, "Extreme pain shoots through your %s!\n\rYour %s has been broken!", limb, limb);
          act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
          sprintf(buf, "You hear a muffled SNAP as $n clutches $s %s in extreme pain!", limb);
          act(buf, FALSE, caster, NULL, NULL, TO_ROOM);

          if (!victim->isPc()) {
            dynamic_cast<TMonster *>(victim)->addHated(caster);
          }
          return SPELL_CRIT_FAIL;
        } else {
          if (!victim->isPc()) {
            dynamic_cast<TMonster *>(victim)->addHated(caster);
          }
          break;
        }
      case CRIT_F_NONE:
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        break;
    }

    // turn some "failures" into successes
    if (!::number(0,2)) {
      SV(SPELL_BONE_BREAKER);
      int rc = injureLimbs(caster, victim, SPELL_BONE_BREAKER, level, adv_learn);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return SPELL_FAIL | VICTIM_DEAD;

      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_FAIL;
    }

    return SPELL_FAIL;
  }
}

int boneBreaker(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=boneBreaker(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
    act("$p fails to break any of $N's limbs!", 
              FALSE, caster, obj, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
  } else {
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int boneBreaker(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_BONE_BREAKER))
    return FALSE;

  level = caster->getSkillLevel(SPELL_BONE_BREAKER);

  ret=boneBreaker(caster,victim,level,caster->getSkillValue(SPELL_BONE_BREAKER), caster->getAdvLearning(SPELL_BONE_BREAKER));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
    act("Your prayer fails to break any of $N's limbs!", 
           FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
  } else {
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

bool notBleedSlot(wearSlotT slot)
{
  return (slot == HOLD_RIGHT || slot == HOLD_LEFT);
}

int bleed(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  char buf[256], limb[256];
  int ret;
  bool found = 0;
  wearSlotT slot;
  int duration;

  if (victim->isUndead()) {
    act("Undead tend not to have beating hearts, so can't bleed.", 
        FALSE, caster,0,0,TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return FALSE;
  }
  if (victim->isImmune(IMMUNE_BLEED)) {
    act("Your prayer seems to have no effect on $N!",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return FALSE;
  }
  if (caster->isNotPowerful(victim, level, SPELL_BLEED, SILENT_YES) ||
      victim->isLucky(caster->spellLuckModifier(SPELL_BLEED))) {
    act("A small welt appears on $n but it subsides quickly.",
        TRUE, victim, 0,0,TO_ROOM);
    act("A small welt appears on your body but it subsides quickly.",
        TRUE, victim, 0,0,TO_CHAR);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return FALSE; 
  }
  caster->reconcileHurt(victim, discArray[SPELL_BLEED]->alignMod);

  // check whether or not there is a slot left to bleed 
  // Added check for paralyzed limb so only one limb can be
  // bled on each mob to stop over abuse of this spell - Russ 11/03/96
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBleedSlot(slot))
      continue;
    if (!victim->slotChance(slot))
      continue;
    if ((found = (victim->isLimbFlags(slot, PART_BLEEDING)))) {
      break;
    }
  }
  if (slot < MAX_WEAR) {
    act("$N is already bleeding!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Your prayer goes for naught.", FALSE, caster, NULL, victim, TO_CHAR);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  int dam = 0;

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_BLEED)) {
    // find a suitable slot to bleed 
    for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
      if (notBleedSlot(slot))
        continue;
      if (!victim->slotChance(slot))
        continue;
      if (victim->isLimbFlags(slot, PART_BLEEDING))
        continue;
      break;
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());

    sprintf(buf, "A gaping gash opens up on your %s!\n\rBright red blood begins to course out!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "The flesh on $n's %s opens up and begins to bleed profusely!",
 limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);

    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    ret = SPELL_SUCCESS;
    duration = (level * 3) + 100;
    victim->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_NO);

    switch(critSuccess(caster, SPELL_BLEED)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLEED);
        victim->rawInfect(slot, duration, SILENT_NO, CHECK_IMMUNITY_YES);
        ret = SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_BLEED) == -1)
      ret +=  VICTIM_DEAD;
    return ret;
  } else {
    switch (critFail(caster, SPELL_BLEED)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_BLEED);
        for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
          if (notBleedSlot(slot))
            continue;
          if (!caster->slotChance(slot))
            continue;
          if (caster->isLimbFlags(slot, PART_BLEEDING))
            continue;
          break;
        }

        sprintf(limb, "%s", caster->describeBodySlot(slot).c_str());

        duration = (level * 3) + 100;
        duration = (int) (caster->percModifier() * duration);
        caster->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_YES);
        if (caster->reconcileDamage(caster, dam, SPELL_BLEED) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        return SPELL_FAIL;
        break;
    }
    return SPELL_FAIL;
  }
}

int bleed(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=bleed(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
      act("Oh no! There are living creatures in the wound! It's infected!",               FALSE, caster, NULL, victim, TO_VICT);
      act("What luck! $p also managed to infect it!", 
          FALSE, caster, obj, NULL, TO_CHAR);
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else {
    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
      act("You scream in agony as $p turns on you starting your limb to bleeding.", TRUE, caster, obj, 0, TO_CHAR);
   act("$n grimaces in surprise as $e skin rends and $e starts to bleed.", TRUE, caster, 0, 0, TO_ROOM);

    } else if (IS_SET(ret, SPELL_FAIL)) {
      act("$p fails to make $N bleed!",
          FALSE, caster, obj, victim, TO_CHAR);
      caster->deityIgnore(SILENT_YES);
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int bleed(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_BLEED))
    return FALSE;

  level = caster->getSkillLevel(SPELL_BLEED);

  ret=bleed(caster,victim,level,caster->getSkillValue(SPELL_BLEED));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    act("Oh no! There are living creatures in the wound! It's infected!",
         FALSE, caster, NULL, victim, TO_VICT);
    act("What luck! Your spell also managed to infect it!",
        FALSE, caster, NULL, NULL, TO_CHAR);
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else {
    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
      caster->sendTo("You scream in agony as your own prayer turns on you starting your limb to bleeding.\n\r");
      act("$n grimaces in surprise as $e skin rends and $e starts to bleed.", TRUE, caster, 0, 0, TO_ROOM);
    } else if (IS_SET(ret, SPELL_FAIL)) {
      act("Your prayer fails to make $N bleed!",
           FALSE, caster, NULL, victim, TO_CHAR);
      caster->deityIgnore(SILENT_YES);
    } else {
    }
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

bool TBeing::canWither(TBeing *victim, silentTypeT silent)
{
  if (checkForTorment(victim)) {
    act("$d refuses to torment $N any further.",
           false, this, 0, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }

  if (victim->isUndead()) {
    act("You can't wither the undead.", FALSE, this,0,0,TO_CHAR);
    act("The undead tend to be immune to withering.", FALSE, this,0,0,TO_ROOM);
    return false;
  }

  bool found = false;
  bool ok = false;
  wearSlotT slot;

  // check whether or not there is a limb left to wither 
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))  // same ones, right?
      continue;
    if (!victim->slotChance(slot))
      continue;
    found |= (victim->isLimbFlags(slot, PART_USELESS));
    ok = TRUE;
  }
  if (!ok) {
    act("Belatedly, you realize $N doesn't have a limb left to wither!", FALSE, this, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.", FALSE, this, NULL, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }
  if (found) {
    act("$d refuses to let you wither another of $N's limbs!",
          FALSE, this, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.",
          FALSE, this, NULL, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }

  return true;
}

int witherLimb(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  char buf[256], limb[256];
  wearSlotT slot;

  if (!caster->canWither(victim, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (caster->isNotPowerful(victim, level, SPELL_WITHER_LIMB, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_WITHER_LIMB]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_WITHER_LIMB)) {
    addTorment(victim, SPELL_WITHER_LIMB);

    // find a suitable limb to wither 
    for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
      if (notBreakSlot(slot, true))
        continue;
      if (!victim->slotChance(slot))
        continue;
      if (victim->isLimbFlags(slot, PART_USELESS))
        continue;
      break;
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->addToLimbFlags(slot, PART_USELESS);
    sprintf(buf, "Suddenly the strength starts to drains from your %s!\n\rIt's withering up into a useless twig!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "You watch in horror as $n's %s withers up into a useless twig!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);

    victim->dropWeapon(slot);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }

    return SPELL_SUCCESS;
  } else {
    // turn some "failures" into successes
    if (!::number(0,2)) {
      SV(SPELL_WITHER_LIMB);
      int rc = injureLimbs(caster, victim, SPELL_WITHER_LIMB, level, adv_learn);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return SPELL_FAIL | VICTIM_DEAD;
      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_FAIL;
    }
    caster->deityIgnore();
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }
}

int witherLimb(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  int rc = witherLimb(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  int retCode = 0;

  if (IS_SET(rc, VICTIM_DEAD))
    ADD_DELETE(retCode, DELETE_VICT);
  return retCode;
}

int witherLimb(TBeing * caster, TBeing * victim)
{
  int level;

  if (!bPassClericChecks(caster,SPELL_WITHER_LIMB))
    return FALSE;

  level = caster->getSkillLevel(SPELL_WITHER_LIMB);

  int ret=witherLimb(caster,victim,level,caster->getSkillValue(SPELL_WITHER_LIMB), caster->getAdvLearning(SPELL_WITHER_LIMB));
  int retCode = 0;

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(retCode, DELETE_VICT);
  return retCode;
}

// returns 0 if no slots exist
// returns 2 if slot exists, and no paralyzed limbs
// returns 3 if slot exists, and has a paralyzed limb
int TBeing::canBeParalyzeLimbed()
{
  // check whether or not there is a limb left to paralyze 
  // Added check for paralyzed limb so only one limb can be
  // paralyzed on each mob to stop over abuse of this spell - Russ 10/21/96
  int ok = 0;
  wearSlotT slot;
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))  // same ones, right?
      continue;
    if (!slotChance(slot))
      continue;
    if (isLimbFlags(slot, PART_PARALYZED))
      SET_BIT(ok, 1);
    SET_BIT(ok, 2);
  }
  return ok;
}

bool TBeing::canParalyzeLimb(TBeing *victim, silentTypeT silent)
{
  if (victim->isUndead()) {
    act("Undead completely ignore the affects of paralyzation.", 
         FALSE, this, 0, 0, TO_ROOM);
    deityIgnore(SILENT_YES);
    return false;
  }

  if (checkForTorment(victim)) {
    act("$d refuses to torment $N any further.",
           false, this, 0, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }

  int rc = victim->canBeParalyzeLimbed();
  if (!rc) {
    act("$N has no limbs you can paralyze!",
         FALSE, this, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.", FALSE, this, NULL, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }
  if (IS_SET(rc, 1)) {
    act("$d refuses to let you paralyze another of $N's limbs!",
          FALSE, this, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.", FALSE, this, NULL, victim, TO_CHAR);
    deityIgnore(SILENT_YES);
    return false;
  }

  return true;
}

int paralyzeLimb(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  char buf[256], limb[256];
  wearSlotT slot;

  if (!caster->canParalyzeLimb(victim, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (caster->isNotPowerful(victim, level, SPELL_PARALYZE_LIMB, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (victim->isImmune(IMMUNE_PARALYSIS)) {
    caster->deityIgnore();
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_PARALYZE_LIMB]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(),SPELL_PARALYZE_LIMB)) {
    addTorment(victim, SPELL_PARALYZE_LIMB);

    // find a suitable limb to paralyze 
    for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
      if (notBreakSlot(slot, true))  // same ones, right?
        continue;
      if (!victim->slotChance(slot))
        continue;
      if (victim->isLimbFlags(slot, PART_PARALYZED))
        continue;
      break;
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    victim->addToLimbFlags(slot, PART_PARALYZED);
    sprintf(buf, "Suddenly the feeling disappears from your %s!\n\rIt's been paralyzed! You can't use your %s!", limb, limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "$n's %s has been paralyzed!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
  
    victim->dropWeapon(slot);

    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }

    return SPELL_SUCCESS;
  } else {
    // turn some "failures" into successes
    if (!::number(0,2)) {
      SV(SPELL_PARALYZE_LIMB);
      int rc = injureLimbs(caster, victim, SPELL_PARALYZE_LIMB, level, adv_learn);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return SPELL_FAIL | VICTIM_DEAD;
      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_FAIL;
    }

    caster->deityIgnore();
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }
}

int paralyzeLimb(TBeing *caster, TBeing *victim, TMagicItem *obj) 
{
  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);

  int ret = paralyzeLimb(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  int retCode = 0;

  if (IS_SET_DELETE(ret, VICTIM_DEAD))
    ADD_DELETE(retCode, VICTIM_DEAD);

  return retCode;
}


int paralyzeLimb(TBeing * caster, TBeing * v)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_PARALYZE_LIMB))
    return FALSE;

  level = caster->getSkillLevel(SPELL_PARALYZE_LIMB);

  ret=paralyzeLimb(caster,v,level,caster->getSkillValue(SPELL_PARALYZE_LIMB), caster->getAdvLearning(SPELL_PARALYZE_LIMB));
  if (ret == SPELL_SUCCESS) {
  } else {
    caster->deityIgnore();
  }
  int retCode = 0;

  if (IS_SET_DELETE(ret, VICTIM_DEAD))
    ADD_DELETE(retCode, VICTIM_DEAD);

  return retCode;
}

int numb(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  char buf[256], limb[256];
  wearSlotT slot;
  affectedData aff;

  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    return SPELL_FAIL;
  }

  if (victim->isUndead()) {
    act("Being undead, they can't get much more numb.", FALSE, caster,0,0,TO_CHAR);
    act("Being undead, they can't get much more numb.", FALSE,caster,0,0,TO_ROOM);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (victim->isImmune(IMMUNE_PARALYSIS)) {
    caster->deityIgnore();
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (checkForTorment(victim)) {
    act("$d refuses to torment $N any further.",
           false, caster, 0, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  // check whether or not there is a limb left to paralyze 
  int found, ok;
  found = ok = FALSE;
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBreakSlot(slot, false))  // same ones, right?
      continue;
    if (!victim->slotChance(slot))
      continue;
    found |= victim->isLimbFlags(slot, PART_PARALYZED);
    ok = TRUE;
  }

  if (!ok) {
    act("Belatedly, you realize $N doesn't have a limb left to paralyze!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.", FALSE, caster, NULL, victim, TO_CHAR);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }
  if (found) {
    act("$d refuses to let you paralyze another of $N's limbs!",
          FALSE, caster, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.",
          FALSE, caster, NULL, victim, TO_CHAR);
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    caster->reconcileHurt(victim, discArray[spell]->alignMod);
    addTorment(victim, spell);

    // find a suitable limb to paralyze 
    for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
      if (notBreakSlot(slot, true))  // same ones, right?
        continue;
      if (!victim->slotChance(slot))
        continue;
      if (victim->isLimbFlags(slot, PART_PARALYZED))
        continue;
      break;
    }

    sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
    sprintf(buf, "Suddenly the feeling disappears from your %s!\n\rIt's been paralyzed! You can't use your %s!", limb, limb);
    act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
    sprintf(buf, "$n's %s has been paralyzed!", limb);
    act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
  
    aff.type = AFFECT_DISEASE; 
    aff.level = slot;
    aff.duration = (level * 3) + 100; 
    aff.modifier = DISEASE_NUMB; 
    aff.location = APPLY_NONE;
    aff.bitvector = 0;

    aff.duration = (int) (caster->percModifier() * aff.duration);
    if (critSuccess(caster, spell)) {
      CS(spell);
      aff.duration *= 2;
    }

    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_PARALYSIS));
    aff.duration /= 100;

    victim->affectTo(&aff);                                                  
    disease_start(victim, &aff); 

    victim->dropWeapon(slot);

    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_SUCCESS;
  } else {
    // turn some "failures" into successes
    if (!::number(0,2)) {
      SV(spell);
      int rc = injureLimbs(caster, victim, spell, level, adv_learn);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return SPELL_FAIL | VICTIM_DEAD;
      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_FAIL;
    }

    caster->deityIgnore();
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }
}

int numb(TBeing *caster, TBeing *victim, TMagicItem *obj, spellNumT spell)
{
  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);
 
  int rc = numb(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  int retCode = 0;

  if (IS_SET(rc, VICTIM_DEAD))
    ADD_DELETE(retCode, DELETE_VICT);
  return retCode;
}

int numb(TBeing *caster, TBeing *victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_NUMB);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);

  int ret=numb(caster,victim,level,caster->getSkillValue(spell), spell, caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  int retCode = 0;

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(retCode, DELETE_VICT);
  return retCode;
}

int disease(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  if (victim->isImmune(IMMUNE_DISEASE)) {
    act("$N shakes off the effects as if immune.",
        FALSE, caster, 0, victim, TO_CHAR);
    act("You shake off the effects of that disease-spewing $n.",
        FALSE, caster, 0, victim, TO_VICT);
    caster->deityIgnore(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_DISEASE)){
    caster->reconcileHurt(victim, discArray[SPELL_DISEASE]->alignMod);

    if(!genericDisease(victim, level)){
      act("$d refuses to disease $N any more.",
	  FALSE, caster, 0, victim, TO_CHAR);
      act("$d refuses to disease you any more.",
	  FALSE, caster, 0, victim, TO_VICT);
      act("$d refuses to disease $N any more.",
	  FALSE, caster, 0, victim, TO_NOTVICT);

      return SPELL_FAIL;
    } else {
      act("$d brings forth diseased vermin to impair $N.",
	  FALSE, caster, 0, victim, TO_CHAR);
      act("$d brings forth diseased vermin to impair you.",
	  FALSE, caster, 0, victim, TO_VICT);
      act("$d brings forth diseased vermin to impair $N.",
	  FALSE, caster, 0, victim, TO_NOTVICT);

      return SPELL_SUCCESS;
    }
  } else {
    caster->deityIgnore();
    return SPELL_FAIL;
  }
}

void disease(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);
 
  disease(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
 
  return;
}

void disease(TBeing *caster, TBeing *victim)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_DISEASE))
    return;

  level = caster->getSkillLevel(SPELL_DISEASE);

  ret=disease(caster,victim,level,caster->getSkillValue(SPELL_DISEASE));
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

int infect(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell)
{
  char buf[256], limb[256];
  bool found = FALSE;
  wearSlotT slot;
  int duration;

  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  if (victim->isUndead()) {
    act("Seeing they are undead, infectation is the least of their worries.", FALSE, caster,0,0,TO_CHAR);
    act("The undead tend to be immune to infection.", FALSE,caster,0,0,TO_ROOM);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  // check whether or not there is a slot left to infect 
  // Added check for infected limb so only one limb can
  // be infected on each mob to stop over abuse of this spell - Russ 11/06/96
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (notBleedSlot(slot))  // same ones, right?
      continue;
    if (!victim->slotChance(slot))
      continue;
    if ((found = (victim->isLimbFlags(slot, PART_INFECTED)))) {
      break;
    }
  }
  if (slot < MAX_WEAR) {
    act("$N is already infected! Don't get too near $M!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("Your attack goes for naught.", FALSE, caster, NULL, NULL, TO_CHAR);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  int dam = 0;

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    caster->reconcileHurt(victim, discArray[spell]->alignMod);

    if (!victim->isLucky(caster->spellLuckModifier(spell))) {
      // find a suitable slot to infect 
      for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
        if (notBleedSlot(slot))  // same ones, right?
          continue;
        if (!victim->slotChance(slot))
          continue;
        if (victim->isLimbFlags(slot, PART_INFECTED))
          continue;
        break;
      }

      sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());

      duration = (level * 3) + 100;
      duration = (int) (caster->percModifier() * duration);

      if (!victim->isLimbFlags(slot, PART_BLEEDING)) {
        if (victim->rawBleed(slot, duration, SILENT_YES, CHECK_IMMUNITY_YES)) {
  
          sprintf(buf, "A small cut opens up on your %s!\n\rBright red blood begins to spit out!", limb);
          act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
          sprintf(buf, "The flesh on $n's %s cracks open and begins to bleed!", limb);
          act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
        }
      }
      if (!victim->isLimbFlags(slot, PART_INFECTED)) {
        if (victim->rawInfect(slot, duration, SILENT_YES, CHECK_IMMUNITY_YES)) {
    
          sprintf(buf, "Magical green germs materialize in the bloody wound on your %s!\nIt's infected! You'd better get that cleaned up as soon as possible!", limb);
          act(buf, FALSE, victim, NULL, NULL, TO_CHAR);
          sprintf(buf, "$n looks at the bloody wound on $s %s and screams in terror!\nYou can virtually see the little green germs infesting it!", limb);
          act(buf, FALSE, victim, NULL, NULL, TO_ROOM);
        }
      }
      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_SUCCESS;
    } else {
      // Victim passed luck/save
      if (!victim->isPc()) {
        dynamic_cast<TMonster *>(victim)->addHated(caster);
      }
      return SPELL_FAIL;
    }
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        for (slot = pickRandomLimb(); ; slot = pickRandomLimb()) {
          if (notBleedSlot(slot))  // same ones, right?
            continue;
          if (!caster->slotChance(slot))
            continue;
          if (caster->isLimbFlags(slot, PART_INFECTED))
            continue;
          break;
        }
  
       sprintf(limb, "%s", caster->describeBodySlot(slot).c_str());

        duration = (level * 3) + 100;
        duration = (int) (caster->percModifier() * duration);

        if (caster->rawInfect(slot, duration, SILENT_YES, CHECK_IMMUNITY_YES)) {

          caster->sendTo("You scream in agony as your own attack turns on you infecting your limb!\n\r");
          act("$n grimaces in surprise as $e skin becomes infected!", 
             TRUE, caster, 0, 0, TO_ROOM);
        }
        if (caster->reconcileDamage(caster, dam, spell) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->sendTo("You fail to harm your victim.\n\r");
    caster->deityIgnore(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }
}

void infect(TBeing *caster, TBeing *victim, TMagicItem *obj, spellNumT spell) 
{
  act("$p invokes an injurious prayer against you.",
        FALSE, victim, obj, 0, TO_CHAR);
  act("$p invokes an injurious prayer against $n.",
        FALSE, victim, obj, 0, TO_ROOM);
 
  infect(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
 
  return;
}

void infect(TBeing * caster, TBeing * victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_INFECT);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);

  int ret=infect(caster,victim,level,caster->getSkillValue(spell), spell);
  if (ret== SPELL_SUCCESS) {
  }
}
