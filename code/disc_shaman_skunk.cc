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


int deathMist(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  TThing * t, *t2;
  TBeing *vict = NULL;
  TBeing *vict2 = NULL;
  affectedData aff, aff2;
  level = min(level, 50);

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

  caster->reconcileHurt(vict, discArray[SPELL_DEATH_MIST]->alignMod);

  if (bSuccess(caster, bKnown, SPELL_DEATH_MIST)) {
    act("You call forth the wrath of the ancestors!", 
          FALSE, caster, NULL, NULL, TO_CHAR);
    act("A green mist comes forth from $n's mouth!", 
          FALSE, caster, NULL, NULL, TO_ROOM);
    for (t = caster->roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      vict = dynamic_cast<TBeing *>(t);
      vict2 = dynamic_cast<TBeing *>(t2);
      if (!vict)
        continue;
      if (vict->isImmune(IMMUNE_POISON, level)) {
	act("The invokation seems to have no affect on $N!",
	    FALSE, caster, NULL, vict, TO_CHAR);
	act("$n just tried to poison you. Luckily you are immune.",
	    FALSE, caster, NULL, vict, TO_VICT);
	caster->nothingHappens();
	return SPELL_FAIL;
      }
      if (vict->affectedBySpell(SPELL_POISON) || vict->affectedBySpell(SPELL_POISON_DEIKHAN)) {
	act("Your poison mist has no affect on $N.",
	    FALSE, caster, NULL, vict, TO_CHAR);
	act("$n just tried to poison you again!",
	    FALSE, caster, NULL, vict, TO_VICT);
	caster->nothingHappens();
	return SPELL_FAIL;
      }
      if (caster->isNotPowerful(vict, level, SPELL_DEATH_MIST, SILENT_YES)) {
	act("Your prayer seems to have little or no affect on $N!",
	    FALSE, caster, NULL, vict, TO_CHAR);
	act("$n just tried to poison you. Lucky $e was so weak.",
	    FALSE, caster, NULL, vict, TO_VICT);
	caster->nothingHappens();
	return SPELL_FAIL;
      }
      if (!vict->isImmortal()) {
	if (vict->isLucky(caster->spellLuckModifier(SPELL_DEATH_MIST))) {
	  SV(SPELL_DEATH_MIST);
	  aff.duration /= 2;
	}
	switch (critSuccess(caster, SPELL_DEATH_MIST)) {
	  case CRIT_S_DOUBLE:
	  case CRIT_S_TRIPLE:
	  case CRIT_S_KILL:
	    CS(SPELL_DEATH_MIST);
	    aff.duration *= 2;
	    break;
	  case CRIT_S_NONE:
	    break;
	}
        act("$n chokes and coughs on the noxious green fumes!", 
              FALSE, vict, NULL, NULL, TO_ROOM);
        act("You choke and cough on the noxious green fumes!", 
              FALSE, vict, NULL, NULL, TO_CHAR);
	vict->affectTo(&aff);
	vict->affectTo(&aff2);
	disease_start(vict, &aff2);
	if (::number(0,3) >= 1) {
	  vict2->affectTo(&aff);
	  vict2->affectTo(&aff2);
	  disease_start(vict2, &aff2);
	}
	if (::number(0,10) > 8) {
	  caster->affectTo(&aff);
	  caster->affectTo(&aff2);
	  disease_start(caster, &aff2);
	  act("You did not let all the mist out and have poisoned yourself as well!",
              FALSE, vict, NULL, NULL, TO_CHAR);
	}
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_DEATH_MIST)) {
      CF(SPELL_DEATH_MIST);
      act("You scream in agony! You have used the power of your ancestors incorrectly!", 
           FALSE, caster, NULL, NULL, TO_CHAR);
      caster->setLifeforce(0);
      caster->setHit(-30);
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int deathMist(TBeing * caster)
{
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("This won't have much affect down here.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_DEATH_MIST, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_DEATH_MIST]->lag;
  taskDiffT diff = discArray[SPELL_DEATH_MIST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_DEATH_MIST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castDeathMist(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_DEATH_MIST);
  
  ret=deathMist(caster,level,caster->getSkillValue(SPELL_DEATH_MIST), caster->getAdvLearning(SPELL_DEATH_MIST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
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
  int lfmod = dice(number(1,level),4);

  if (bSuccess(caster, bKnown,SPELL_LICH_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster, NULL, victim, TO_VICT);
    caster->addToLifeforce(lfmod);
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
        // I am basing this on energy drain
        // Russ had this nasty sucker below for supah crit...i left it
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
