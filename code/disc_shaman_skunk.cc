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

int deathMist(TBeing *caster, int level, byte bKnown)
{
  TBeing *tmp_victim;
  TThing *t, *t2;
  affectedData aff, aff2;
  int found = FALSE;
  if (bSuccess(caster, bKnown, SPELL_DEATH_MIST)) {
    caster->sendTo("A misty cloud escapes your open mouth.\n\r",ANSI_GREEN);
    act("$n opens $s mouth and a chilling green mist pours out.",
        TRUE,caster,0,0,TO_ROOM,ANSI_GREEN);

    aff.type = SPELL_POISON;
    aff.level = 30;
    aff.duration = (25) * UPDATES_PER_MUDHOUR;
    aff.modifier = -20;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_POISON;
    aff.duration = (25) * UPDATES_PER_MUDHOUR;
    aff2.type = AFFECT_DISEASE;
    aff2.level = 0;
    aff2.modifier = DISEASE_POISON;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_POISON;

    for (t = caster->roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if (caster != tmp_victim && !tmp_victim->isImmortal()) {
	//        if (caster->inGroup(*tmp_victim)) {
          if (!tmp_victim->isAffected(AFF_POISON)) {
            caster->reconcileHelp(tmp_victim,discArray[SPELL_DEATH_MIST]->alignMod);
            tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
            found = TRUE;
          }
	  // }
      } 
    }
    if (!caster->isAffected(AFF_POISON)) {
      caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
      found = TRUE;
    }
    if (!found) {
      caster->sendTo("Everything here seems to be poisonous.\n\r");
      return SPELL_FAIL;
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
// END DEADTH MIST
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
  int lfmod = ::number(20,((level*5)/2));

  if (bSuccess(caster, bKnown,SPELL_LICH_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster, NULL, victim, TO_VICT);
    caster->addToLifeforce(lfmod);
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
          vlogf(LOG_MISC, "%s just lost a level from a critical lich touch!", pers->getName());
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
    vlogf(LOG_JESUS, "Lich Touch damage: %d LFmod: %d caster: %s victim: %s", dam, lfmod, caster->getName(), victim->getName());
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

int cardiacStress(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  int rc;
  TThing *t;

  level = min(level, 70);

  int dam = caster->getSkillDam(victim, SPELL_CARDIAC_STRESS, level, adv_learn);

  if (bSuccess(caster, bKnown, SPELL_CARDIAC_STRESS)) {
    caster->reconcileHurt(victim,discArray[SPELL_CARDIAC_STRESS]->alignMod);
    if ((critSuccess(caster, SPELL_CARDIAC_STRESS) ||
        !caster->isNotPowerful(victim, level, SPELL_CARDIAC_STRESS, SILENT_YES))) {
      CS(SPELL_CARDIAC_STRESS);
      dam *= 2;
      act("$N clutches $S chest and keels over in EXTREME pain!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED_BOLD);
      act("The stress on your heart is INTENSE!! You fall down from the pain!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_RED_BOLD);
      act("$n spits on $N as $E clutches at $S chest keels over in EXTREME pain!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED_BOLD);
      if (victim->riding)
        victim->dismount(POSITION_STANDING);
      while ((t = victim->rider)) {
        rc = t->fallOffMount(victim, POSITION_STANDING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
      }
      victim->setPosition(POSITION_SITTING);
      victim->addToWait(combatRound(1));
    } else if (victim->isLucky(caster->spellLuckModifier(SPELL_CARDIAC_STRESS))) {

      act("$N holds $S chest in discomfort.",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
      act("Your chest throbs in pain. Wonder what's wrong...",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);
      act("$N holds $S chest in discomfort.",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);

      SV(SPELL_CARDIAC_STRESS);
      dam /= 2;
    } else {
      act("$N clutches $S chest in pain!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
      act("You clutch your chest in pain!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);
      act("$N clutches $S chest in pain!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
    }
    vlogf(LOG_JESUS, "Coronary damgae: %d caster: %s victim: %s", dam, caster->getName(), victim->getName());
    if (caster->reconcileDamage(victim, dam, SPELL_CARDIAC_STRESS) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("$n just tried to attack you.", FALSE, caster, 0, victim, TO_VICT, ANSI_RED);
    if (critFail(caster, SPELL_CARDIAC_STRESS) == CRIT_F_HITSELF) {
      CF(SPELL_CARDIAC_STRESS);
      act("You screwed up the ritual and the loa make you pay for your mistake!",
           FALSE, caster, NULL, 0, TO_CHAR, ANSI_RED_BOLD);
      act("$n clutches $s chest in EXTREME agony!",
           FALSE, caster, NULL, 0, TO_ROOM, ANSI_RED_BOLD);
      if (caster->reconcileDamage(caster, dam, SPELL_CARDIAC_STRESS) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int cardiacStress(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

   if (!bPassShamanChecks(caster, SPELL_CARDIAC_STRESS, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_CARDIAC_STRESS]->lag;
    diff = discArray[SPELL_CARDIAC_STRESS]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_CARDIAC_STRESS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

      return TRUE;
}

int castCardiacStress(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_CARDIAC_STRESS);
  int bKnown = caster->getSkillValue(SPELL_CARDIAC_STRESS);

  ret=cardiacStress(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_CARDIAC_STRESS));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int cardiacStress(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=cardiacStress(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
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

  if (bSuccess(caster, bKnown, SPELL_BLOOD_BOIL)) {
    caster->reconcileHurt(victim, discArray[SPELL_BLOOD_BOIL]->alignMod);

    switch (critSuccess(caster, SPELL_BLOOD_BOIL)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLOOD_BOIL);
        dam <<= 1;
        act("<R>$n directs <W>**INTENSE HEAT**<R> from $s hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
        act("<R>You direct <W>**INTENSE HEAT**<R> from your hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
        act("<R>$n directs <W>**INTENSE HEAT**<R> from $s hands boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_BLOOD_BOIL))) {
          SV(SPELL_BLOOD_BOIL);
          dam /= 2;
          act("<r>$n directs heat from $s hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("<r>You direct heat from your hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("<r>$n directs heat from $s hands boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        } else {
          act("<R>$n directs heat from $s hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("<R>You direct heat from your hands boiling $N's blood!<z>", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("<R>$n directs heat from $s hands boiling your blood!<z>", 
                  FALSE, caster, NULL, victim, TO_VICT);
        }
        break;
    }
    vlogf(LOG_JESUS, "Boiling Blood damage: %d caster: %s victim: %s", dam, caster->getName(), victim->getName());
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
        act("<R>You direct <W>**INTENSE HEAT**<R> heat from your hands boiling <W>YOUR OWN<R> blood!<z>", 
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
