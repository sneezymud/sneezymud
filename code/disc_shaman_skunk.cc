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




















