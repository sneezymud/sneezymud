/*******************************************************************
 *                                                                 *
 *                                                                 *
 *   disc_shaman_skunk.cc              Shaman Discipline Disc      *
 *                                                                 *
 *   SneezyMUD Development - All Rights Reserved                   *
 *                                                                 *
 *******************************************************************/
 
#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_skunk.h"
#include "obj_magic_item.h"

int deathMist(TBeing *caster, int level, byte bKnown)
{
  TBeing *tmp_victim;
  TThing *t, *t2;
  affectedData aff, aff2;
  int found = FALSE;

  if (caster->bSuccess(bKnown, SPELL_DEATH_MIST)) {
    act("<g>A misty cloud escapes your open mouth.<1>",
        FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n opens $s mouth and a chilling green mist pours out.",
        TRUE,caster,0,0,TO_ROOM,ANSI_GREEN);

    aff.type = SPELL_DEATH_MIST;
    aff.level = 30;
    aff.duration = (25) * UPDATES_PER_MUDHOUR;
    aff.modifier = -10;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_SYPHILIS;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 30;
    aff2.duration = (25) * UPDATES_PER_MUDHOUR;
    aff2.modifier = DISEASE_SYPHILIS;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_SYPHILIS;

    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if (caster != tmp_victim && !tmp_victim->isImmortal() &&
          !tmp_victim->isImmune(IMMUNE_DISEASE, WEAR_WAIST) &&
          !tmp_victim->isAffected(AFF_SYPHILIS)) {
	      if (caster->inGroup(*tmp_victim)) {
          if (!caster->bSuccess(bKnown, SPELL_DEATH_MIST)) {
            caster->reconcileHelp(tmp_victim,discArray[SPELL_DEATH_MIST]->alignMod);
            tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
            tmp_victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
            found = TRUE;
            act("You feel a stinging in your waist.",
                FALSE, caster, NULL, tmp_victim, TO_VICT);
            act("$n starts to look a little uncomfortable.",
                TRUE, tmp_victim, NULL, NULL, TO_ROOM);
          }
        } else {
          caster->reconcileHelp(tmp_victim,discArray[SPELL_DEATH_MIST]->alignMod);
          tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
          tmp_victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
          found = TRUE;
          act("You feel a stinging in your waist.",
              FALSE, caster, NULL, tmp_victim, TO_VICT);
          act("$n starts to look a little uncomfortable.",
              TRUE, tmp_victim, NULL, NULL, TO_ROOM);
        }
      } 
    }
    if (!caster->isAffected(AFF_SYPHILIS) &&
        !caster->bSuccess(bKnown, SPELL_DEATH_MIST) &&
        !caster->isImmune(IMMUNE_DISEASE, WEAR_WAIST) &&
        !caster->isImmortal()) {
      caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
      caster->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
      found = TRUE;
      act("You feel a stinging in your waist.",
          FALSE, caster, NULL, NULL, TO_CHAR);
      act("$n starts to look a little uncomfortable.",
          TRUE, caster, NULL, NULL, TO_ROOM);
    }
    if (!found) {
      caster->sendTo("Nothing in the vicinity appears to have been further discomforted by your ritual.\n\r");
    //  return SPELL_FAIL;
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int deathMist(TBeing *caster)
{
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DEATH_MIST, NULL))
    return TRUE;

  lag_t rounds = discArray[SPELL_DEATH_MIST]->lag;
  diff = discArray[SPELL_DEATH_MIST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_DEATH_MIST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castDeathMist(TBeing *caster)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_DEATH_MIST);
  int bKnown = caster->getSkillValue(SPELL_DEATH_MIST);

  if ((ret=deathMist(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}
// END DEATH MIST

int cleanse(TBeing *caster, TBeing * victim, int level, byte learn, spellNumT spell)
{
  char buf[256];
  affectedData aff;

  if (spell==SKILL_WOHLIN || caster->bSuccess(learn, caster->getPerc(), spell)) {
    if (victim->isAffected(AFF_SYPHILIS) || victim->affectedBySpell(SPELL_DEATH_MIST) || 
	victim->hasDisease(DISEASE_SYPHILIS)) {
      sprintf(buf, "You succeed in curing the syphilis in %s body.", (caster == victim) ? "your" : "$N's");
      act(buf, FALSE, caster, NULL, victim, TO_CHAR);
      if (caster != victim)
        act("With a sigh of relief you feel that your syphilis has been removed.", FALSE, caster, NULL, victim, TO_VICT);
      act("The syphilis in $N's body has been removed!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("The syphilis in your body has been removed! What a relief!", FALSE, victim, NULL, NULL, TO_CHAR);
      victim->affectFrom(SPELL_DEATH_MIST);
      victim->diseaseFrom(DISEASE_SYPHILIS);
      caster->reconcileHelp(victim,discArray[spell]->alignMod);
    } else {
      if(spell==SKILL_WOHLIN)
	return FALSE;
      act("Can't do this...noone here has syphilis.", FALSE, caster, NULL, NULL, TO_CHAR);
      caster->nothingHappens(SILENT_YES);
      return FALSE;
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        aff.type = SPELL_DEATH_MIST;
        aff.level = level;
        aff.duration = (aff.level << 1) * UPDATES_PER_MUDHOUR;
        aff.modifier = -10;
        aff.location = APPLY_STR;
        aff.bitvector = AFF_SYPHILIS;
        caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
        return SPELL_CRIT_FAIL;
      default:
        break;
    }
    return SPELL_FAIL;
  }
}

int cleanse(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_CLEANSE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CLEANSE]->lag;
  taskDiffT diff = discArray[SPELL_CLEANSE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CLEANSE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE; 
}

int castCleanse(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_CLEANSE);
  int bKnown = caster->getSkillValue(SPELL_CLEANSE);

  if ((ret=cleanse(caster,victim,level,bKnown,SPELL_CLEANSE)) == SPELL_SUCCESS) {
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

int cleanse(TBeing *caster, TBeing *victim, TMagicItem *tMagItem)
{
  int tRc = FALSE; 
  int tReturn;

  //  tReturn = cleanse(caster, victim, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);
  tReturn = cleanse(caster, victim, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), SPELL_CLEANSE);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

// LICH TOUCH

int lichTouch(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!",
             FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_LICH_TOUCH, level, adv_learn);
  caster->reconcileHurt(victim, discArray[SPELL_LICH_TOUCH]->alignMod);
  bool save = victim->isLucky(caster->spellLuckModifier(SPELL_LICH_TOUCH));
  int vit = dice(number(1,level),4);
  int lfmod = ::number(20,(level*5));
  int hpgain = ::number(30,(level*2));

  if (victim->getImmunity(IMMUNE_DRAIN) >= 100) {
    act("$N is immune to draining!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown,SPELL_LICH_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster, NULL, victim, TO_VICT);
    caster->addToLifeforce(lfmod);
    caster->addToHit(hpgain);
    caster->updatePos();
    TPerson *pers;
    switch (critSuccess(caster, SPELL_LICH_TOUCH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
        CS(SPELL_LICH_TOUCH);
        dam *=2;
        vit *=2;
        lfmod *= 2;
        break;
      case CRIT_S_KILL:
        CS(SPELL_LICH_TOUCH);
        pers = dynamic_cast<TPerson *>(victim);
        if (pers && !save) {
          pers->dropLevel(pers->bestClass());
          pers->setTitle(false);
          vlogf(LOG_MISC, fmt("%s just lost a level from a critical lich touch!") %  pers->getName());
        }
        break;
      case CRIT_S_NONE:
        break;
    }
    if (save) {
      SV(SPELL_LICH_TOUCH);
      dam /= 2;
      vit /= 2;
      lfmod /= 2;
    }
    if (!victim->isImmortal())
      victim->addToMove(-vit);
    if (caster->reconcileDamage(victim, dam, SPELL_LICH_TOUCH) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_LICH_TOUCH)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_LICH_TOUCH);
        act("$n's body glows a dark, evil-looking red!", 
               FALSE, caster, NULL, NULL, TO_ROOM);
        act("You sang the invokation incorrectly! The ancestors are EXTREMELY pissed!", 
               FALSE, caster, NULL, NULL, TO_CHAR);
        caster->addToMove(-vit);
        caster->addToLifeforce(0);
	caster->updatePos();
        dam /= 3;
        if (caster->reconcileDamage(caster, dam, SPELL_LICH_TOUCH) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int lichTouch(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_LICH_TOUCH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_LICH_TOUCH]->lag;
  taskDiffT diff = discArray[SPELL_LICH_TOUCH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LICH_TOUCH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 
0);

  return TRUE; 
}

int castLichTouch(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_LICH_TOUCH);
  int bKnown = caster->getSkillValue(SPELL_LICH_TOUCH);

  ret=lichTouch(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_LICH_TOUCH));
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int lichTouch(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = lichTouch(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}
// END LICH TOUCH

int cardiacStress(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("Gods dont have heart attacks, they don't have hearts!", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 80);

  int dam = caster->getSkillDam(victim, SPELL_CARDIAC_STRESS, level, adv_learn);

  caster->reconcileHurt(victim, discArray[SPELL_CARDIAC_STRESS]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_CARDIAC_STRESS)) {
    act("$N clutches $S chest and keels over in EXTREME pain!",
	FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED_BOLD);
    act("The stress on your heart is INTENSE!! You fall down from the pain!",
	FALSE, caster, NULL, victim, TO_VICT, ANSI_RED_BOLD);
    act("$n spits on $N who clutches at $S chest and keels over in EXTREME pain!",
	FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED_BOLD);
    switch(critSuccess(caster, SPELL_CARDIAC_STRESS)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CARDIAC_STRESS);
        dam <<= 1;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_CARDIAC_STRESS))) {
          SV(SPELL_CARDIAC_STRESS);
          dam /= 2;
        }
    }
    if (caster->reconcileDamage(victim, dam, SPELL_CARDIAC_STRESS) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_CARDIAC_STRESS)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        act("You screwed up the ritual and the loa make you pay for your mistake!",
	    FALSE, caster, NULL, 0, TO_CHAR, ANSI_RED_BOLD);
        act("$n clutches $s chest in EXTREME agony!",
	    FALSE, caster, NULL, 0, TO_ROOM, ANSI_RED_BOLD);
        if (caster->isLucky(caster->spellLuckModifier(SPELL_CARDIAC_STRESS))) {
          SV(SPELL_CARDIAC_STRESS);
          dam /= 2;
        }
        if (caster->reconcileDamage(caster, dam/3, SPELL_CARDIAC_STRESS) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        act("Oops! You nearly exploded your own heart!", 
            FALSE, caster, NULL, victim, TO_CHAR);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int cardiacStress(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_CARDIAC_STRESS, victim))
    return FALSE;
  
  lag_t rounds = discArray[SPELL_CARDIAC_STRESS]->lag;
  taskDiffT diff = discArray[SPELL_CARDIAC_STRESS]->task;
  
  start_cast(caster, victim, NULL, caster->roomp, SPELL_CARDIAC_STRESS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  
  return TRUE;
}

int castCardiacStress(TBeing *caster, TBeing *victim)
{
  int rc = 0;
  
  int level = caster->getSkillLevel(SPELL_CARDIAC_STRESS);
  int bKnown = caster->getSkillValue(SPELL_CARDIAC_STRESS);
  
  int ret=cardiacStress(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_CARDIAC_STRESS));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int cardiacStress(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
    tReturn;
  
  tReturn = cardiacStress(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);
  
  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);
  
  return tRc;
}

int bloodBoil(TBeing *caster, TBeing *victim, TMagicItem *tObj)
{
  int tLevel = tObj->getMagicLevel(),
      tKnown = tObj->getMagicLearnedness(),
      tReturn = 0;

  tReturn = bloodBoil(caster, victim, tLevel, tKnown, 0);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}

int bloodBoil(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't boil $N's blood -- $E's a god! ", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  level = min(level, 45);

  int dam = caster->getSkillDam(victim, SPELL_BLOOD_BOIL, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_BLOOD_BOIL)) {
    caster->reconcileHurt(victim, discArray[SPELL_BLOOD_BOIL]->alignMod);

    switch (critSuccess(caster, SPELL_BLOOD_BOIL)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLOOD_BOIL);
        dam <<= 1;
        act("<R>$n directs <W>**INTENSE HEAT**<R> from $s hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
        act("<R>You direct <W>**INTENSE HEAT**<R> from your hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
        act("<R>$n directs <W>**INTENSE HEAT**<R> from $s hands, boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_BLOOD_BOIL))) {
          SV(SPELL_BLOOD_BOIL);
          dam /= 2;
          act("<r>$n directs heat from $s hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("<r>You direct heat from your hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("<r>$n directs heat from $s hands, boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        } else {
          act("<R>$n directs heat from $s hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("<R>You direct heat from your hands, boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("<R>$n directs heat from $s hands, boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        }
        break;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_BLOOD_BOIL) == -1)
      return SPELL_FAIL + VICTIM_DEAD;
    return SPELL_FAIL;
  } else {
    switch(critFail(caster, SPELL_BLOOD_BOIL)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_BLOOD_BOIL);
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
        act("<R>$n screwed up $s ritual and burned $mself!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
        act("<R>You direct <W>**INTENSE HEAT**<R> heat from your hands, boiling <W>YOUR OWN<R> blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
        act("<R>$n has just tried to harm you!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, SPELL_BLOOD_BOIL) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int bloodBoil(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassShamanChecks(caster, SPELL_BLOOD_BOIL, victim))
       return FALSE;

  lag_t rounds = discArray[SPELL_BLOOD_BOIL]->lag;
  diff = discArray[SPELL_BLOOD_BOIL]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_BLOOD_BOIL, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

     return TRUE;
}

int castBloodBoil(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_BLOOD_BOIL);
  int bKnown = caster->getSkillValue(SPELL_BLOOD_BOIL);

  ret=bloodBoil(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_BLOOD_BOIL));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}
