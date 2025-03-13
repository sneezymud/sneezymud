#include <stdio.h>

#include "room.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_skunk.h"
#include "obj_magic_item.h"
#include "person.h"
#include "monster.h"

int deathMist(TBeing* caster, int level, short bKnown) {
  TBeing* tmp_victim;
  TThing* t;
  affectedData aff, aff2;
  int found = FALSE;

  if (caster->bSuccess(bKnown, SPELL_DEATH_MIST)) {
    act("<g>A misty cloud escapes your open mouth.<1>", FALSE, caster, NULL,
      NULL, TO_CHAR);
    act("$n opens $s mouth and a chilling green mist pours out.", TRUE, caster,
      0, 0, TO_ROOM, ANSI_GREEN);

    aff.type = SPELL_DEATH_MIST;
    aff.level = 30;
    aff.duration = caster->durationModify(SPELL_DEATH_MIST,
      (25) * Pulse::UPDATES_PER_MUDHOUR);
    aff.modifier = -10;
    aff.location = APPLY_STR;
    aff.bitvector = AFF_SYPHILIS;

    aff2.type = AFFECT_DISEASE;
    aff2.level = 30;
    aff2.duration = caster->durationModify(SPELL_DEATH_MIST,
      (25) * Pulse::UPDATES_PER_MUDHOUR);
    aff2.modifier = DISEASE_SYPHILIS;
    aff2.location = APPLY_NONE;
    aff2.bitvector = AFF_SYPHILIS;

    for (StuffIter it = caster->roomp->stuff.begin();
         it != caster->roomp->stuff.end();) {
      t = *(it++);
      tmp_victim = dynamic_cast<TBeing*>(t);
      if (!tmp_victim)
        continue;
      if (caster != tmp_victim && !tmp_victim->isImmortal() &&
          !tmp_victim->isImmune(IMMUNE_DISEASE, WEAR_WAIST) &&
          !tmp_victim->isAffected(AFF_SYPHILIS)) {
        if (caster->inGroup(*tmp_victim)) {
          if (!caster->bSuccess(bKnown, SPELL_DEATH_MIST)) {
            caster->reconcileHelp(tmp_victim,
              discArray[SPELL_DEATH_MIST]->alignMod);
            tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
            tmp_victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
            found = TRUE;
            act("You feel a stinging in your waist.", FALSE, caster, NULL,
              tmp_victim, TO_VICT);
            act("$n starts to look a little uncomfortable.", TRUE, tmp_victim,
              NULL, NULL, TO_ROOM);
          }
        } else {
          caster->reconcileHelp(tmp_victim,
            discArray[SPELL_DEATH_MIST]->alignMod);
          tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
          tmp_victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
          found = TRUE;
          act("You feel a stinging in your waist.", FALSE, caster, NULL,
            tmp_victim, TO_VICT);
          act("$n starts to look a little uncomfortable.", TRUE, tmp_victim,
            NULL, NULL, TO_ROOM);
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
      act("You feel a stinging in your waist.", FALSE, caster, NULL, NULL,
        TO_CHAR);
      act("$n starts to look a little uncomfortable.", TRUE, caster, NULL, NULL,
        TO_ROOM);
    }
    if (!found) {
      caster->sendTo(
        "Nothing in the vicinity appears to have been further discomforted by "
        "your ritual.\n\r");
      //  return SPELL_FAIL;
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int deathMist(TBeing* caster) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DEATH_MIST, NULL))
    return TRUE;

  lag_t rounds = discArray[SPELL_DEATH_MIST]->lag;
  diff = discArray[SPELL_DEATH_MIST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_DEATH_MIST, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castDeathMist(TBeing* caster) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_DEATH_MIST);
  int bKnown = caster->getSkillValue(SPELL_DEATH_MIST);

  if ((ret = deathMist(caster, level, bKnown)) == SPELL_SUCCESS) {
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}
// END DEATH MIST

int cleanse(TBeing* caster, TBeing* victim, int level, short learn,
  spellNumT spell) {
  char buf[256];
  affectedData aff;

  if (spell == SKILL_WOHLIN ||
      caster->bSuccess(learn, caster->getPerc(), spell)) {
    if (victim->isAffected(AFF_SYPHILIS) ||
        victim->affectedBySpell(SPELL_DEATH_MIST) ||
        victim->hasDisease(DISEASE_SYPHILIS)) {
      sprintf(buf, "You succeed in curing the syphilis in %s body.",
        (caster == victim) ? "your" : "$N's");
      act(buf, FALSE, caster, NULL, victim, TO_CHAR);
      if (caster != victim)
        act(
          "With a sigh of relief you feel that your syphilis has been removed.",
          FALSE, caster, NULL, victim, TO_VICT);
      act("The syphilis in $N's body has been removed!", FALSE, caster, NULL,
        victim, TO_NOTVICT);
      act("The syphilis in your body has been removed! What a relief!", FALSE,
        victim, NULL, NULL, TO_CHAR);
      victim->affectFrom(SPELL_DEATH_MIST);
      victim->diseaseFrom(DISEASE_SYPHILIS);
      caster->reconcileHelp(victim, discArray[spell]->alignMod);
    } else {
      if (spell == SKILL_WOHLIN)
        return FALSE;
      act("Can't do this...noone here has syphilis.", FALSE, caster, NULL, NULL,
        TO_CHAR);
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
        aff.duration = (aff.level << 1) * Pulse::UPDATES_PER_MUDHOUR;
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

int cleanse(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_CLEANSE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CLEANSE]->lag;
  taskDiffT diff = discArray[SPELL_CLEANSE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CLEANSE, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castCleanse(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_CLEANSE);
  int bKnown = caster->getSkillValue(SPELL_CLEANSE);

  if ((ret = cleanse(caster, victim, level, bKnown, SPELL_CLEANSE)) ==
      SPELL_SUCCESS) {
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

int cleanse(TBeing* caster, TBeing* victim, TMagicItem* tMagItem) {
  int tRc = FALSE;
  int tReturn;

  //  tReturn = cleanse(caster, victim, tMagItem->getMagicLevel(),
  //  tMagItem->getMagicLearnedness(), 0);
  tReturn = cleanse(caster, victim, tMagItem->getMagicLevel(),
    tMagItem->getMagicLearnedness(), SPELL_CLEANSE);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

// LICH TOUCH

int lichTouch(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!", FALSE, caster, NULL, victim,
      TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_LICH_TOUCH, level, adv_learn);

  int hpGain = ::number(70, 105) * (caster->getSkillValue(SPELL_LICH_TOUCH) / 100);
  int lfGain = ::number(140, 200) * (caster->getSkillValue(SPELL_LICH_TOUCH) / 100);
  int vit = ::number(50, 100) * (caster->getSkillValue(SPELL_LICH_TOUCH) / 100);

  caster->reconcileHurt(victim, discArray[SPELL_LICH_TOUCH]->alignMod);
  bool save = victim->isLucky(caster->spellLuckModifier(SPELL_LICH_TOUCH));

  if (victim->getImmunity(IMMUNE_DRAIN) >= 100) {
    act("$N is immune to draining!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim,
      TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL,
      victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_LICH_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL,
      victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL,
      victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster,
      NULL, victim, TO_VICT);
    TPerson* pers;
    switch (critSuccess(caster, SPELL_LICH_TOUCH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
        CS(SPELL_LICH_TOUCH);
        dam *= 2;
        vit *= 2;
        lfGain *= 2;
        break;
      case CRIT_S_KILL:
        CS(SPELL_LICH_TOUCH);
        pers = dynamic_cast<TPerson*>(victim);
        if (pers && !save) {
          pers->dropLevel(pers->bestClass());
          pers->setTitle(false);
          vlogf(LOG_MISC,
            format("%s just lost a level from a critical lich touch!") %
              pers->getName());
        }
        break;
      case CRIT_S_NONE:
        break;
    }
    if (save) {
      SV(SPELL_LICH_TOUCH);
      dam /= 2;
      vit /= 2;
      lfGain /= 2;
    }
    caster->addToHit(hpGain);
    caster->addToLifeforce(lfGain);
    caster->updatePos();
    if (!victim->isImmortal())
      victim->addToMove(-vit);
    if (caster->reconcileDamage(victim, dam, SPELL_LICH_TOUCH) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  }

  switch (critFail(caster, SPELL_LICH_TOUCH)) {
    case CRIT_F_HITSELF:
    case CRIT_F_HITOTHER:
      CF(SPELL_LICH_TOUCH);
      act("$n's body glows a dark, evil-looking red!", FALSE, caster, NULL,
        NULL, TO_ROOM);
      act(
        "You sang the invokation incorrectly! The ancestors are EXTREMELY "
        "pissed!",
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

int lichTouch(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_LICH_TOUCH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_LICH_TOUCH]->lag;
  taskDiffT diff = discArray[SPELL_LICH_TOUCH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LICH_TOUCH, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castLichTouch(TBeing* caster, TBeing* victim) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_LICH_TOUCH);
  int bKnown = caster->getSkillValue(SPELL_LICH_TOUCH);

  ret = lichTouch(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_LICH_TOUCH));
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int lichTouch(TBeing* tMaster, TBeing* tSucker, TMagicItem* tMagItem) {
  int tRc = FALSE, tReturn;

  tReturn = lichTouch(tMaster, tSucker, tMagItem->getMagicLevel(),
    tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}
// END LICH TOUCH

int cardiacStress(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("Gods dont have heart attacks, they don't have hearts!", FALSE, caster,
      NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_CARDIAC_STRESS, level, adv_learn);

  caster->reconcileHurt(victim, discArray[SPELL_CARDIAC_STRESS]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CARDIAC_STRESS)) {
    act("$N clutches $S chest and keels over in EXTREME pain!", FALSE, caster,
      NULL, victim, TO_CHAR, ANSI_RED_BOLD);
    act("The stress on your heart is INTENSE!! You fall down from the pain!",
      FALSE, caster, NULL, victim, TO_VICT, ANSI_RED_BOLD);
    act(
      "$n spits on $N who clutches at $S chest and keels over in EXTREME pain!",
      FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED_BOLD);
    switch (critSuccess(caster, SPELL_CARDIAC_STRESS)) {
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
        act(
          "You screwed up the ritual and the loa make you pay for your "
          "mistake!",
          FALSE, caster, NULL, 0, TO_CHAR, ANSI_RED_BOLD);
        act("$n clutches $s chest in EXTREME agony!", FALSE, caster, NULL, 0,
          TO_ROOM, ANSI_RED_BOLD);
        if (caster->isLucky(caster->spellLuckModifier(SPELL_CARDIAC_STRESS))) {
          SV(SPELL_CARDIAC_STRESS);
          dam /= 2;
        }
        if (caster->reconcileDamage(caster, dam / 3, SPELL_CARDIAC_STRESS) ==
            -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        act("Oops! You nearly exploded your own heart!", FALSE, caster, NULL,
          victim, TO_CHAR);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int cardiacStress(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_CARDIAC_STRESS, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CARDIAC_STRESS]->lag;
  taskDiffT diff = discArray[SPELL_CARDIAC_STRESS]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CARDIAC_STRESS, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castCardiacStress(TBeing* caster, TBeing* victim) {
  int rc = 0;

  int level = caster->getSkillLevel(SPELL_CARDIAC_STRESS);
  int bKnown = caster->getSkillValue(SPELL_CARDIAC_STRESS);

  int ret = cardiacStress(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_CARDIAC_STRESS));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int cardiacStress(TBeing* tMaster, TBeing* tSucker, TMagicItem* tMagItem) {
  int tRc = FALSE, tReturn;

  tReturn = cardiacStress(tMaster, tSucker, tMagItem->getMagicLevel(),
    tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

int bloodBoil(TBeing* caster, TBeing* victim, TMagicItem* tObj) {
  int tLevel = tObj->getMagicLevel(), tKnown = tObj->getMagicLearnedness(),
      tReturn = 0;

  tReturn = bloodBoil(caster, victim, tLevel, tKnown, 0);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}

int bloodBoil(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("You can't boil $N's blood -- $E's a god! ", FALSE, caster, NULL,
      victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_BLOOD_BOIL, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_BLOOD_BOIL)) {
    caster->reconcileHurt(victim, discArray[SPELL_BLOOD_BOIL]->alignMod);

    switch (critSuccess(caster, SPELL_BLOOD_BOIL)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLOOD_BOIL);
        dam <<= 1;
        act(
          "<R>$n directs <W>**INTENSE HEAT**<R> from $s hands, boiling $N's "
          "blood!<z>",
          FALSE, caster, NULL, victim, TO_NOTVICT);
        act(
          "<R>You direct <W>**INTENSE HEAT**<R> from your hands, boiling $N's "
          "blood!<z>",
          FALSE, caster, NULL, victim, TO_CHAR);
        act(
          "<R>$n directs <W>**INTENSE HEAT**<R> from $s hands, boiling your "
          "blood!<z>",
          FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_BLOOD_BOIL))) {
          SV(SPELL_BLOOD_BOIL);
          dam /= 2;
          act("<r>$n directs heat from $s hands, boiling $N's blood!<z>", FALSE,
            caster, NULL, victim, TO_NOTVICT);
          act("<r>You direct heat from your hands, boiling $N's blood!<z>",
            FALSE, caster, NULL, victim, TO_CHAR);
          act("<r>$n directs heat from $s hands, boiling your blood!<z>", FALSE,
            caster, NULL, victim, TO_VICT);
        } else {
          act("<R>$n directs heat from $s hands, boiling $N's blood!<z>", FALSE,
            caster, NULL, victim, TO_NOTVICT);
          act("<R>You direct heat from your hands, boiling $N's blood!<z>",
            FALSE, caster, NULL, victim, TO_CHAR);
          act("<R>$n directs heat from $s hands, boiling your blood!<z>", FALSE,
            caster, NULL, victim, TO_VICT);
        }
        break;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_BLOOD_BOIL) == -1)
      return SPELL_FAIL + VICTIM_DEAD;
    return SPELL_FAIL;
  } else {
    switch (critFail(caster, SPELL_BLOOD_BOIL)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_BLOOD_BOIL);
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
        act("<R>$n screwed up $s ritual and burned $mself!<z>", FALSE, caster,
          NULL, victim, TO_NOTVICT);
        act(
          "<R>You direct <W>**INTENSE HEAT**<R> heat from your hands, boiling "
          "<W>YOUR OWN<R> blood!<z>",
          FALSE, caster, NULL, victim, TO_CHAR);
        act("<R>$n has just tried to harm you!<z>", FALSE, caster, NULL, victim,
          TO_VICT);
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

int bloodBoil(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_BLOOD_BOIL, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_BLOOD_BOIL]->lag;
  diff = discArray[SPELL_BLOOD_BOIL]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_BLOOD_BOIL, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castBloodBoil(TBeing* caster, TBeing* victim) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_BLOOD_BOIL);
  int bKnown = caster->getSkillValue(SPELL_BLOOD_BOIL);

  ret = bloodBoil(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_BLOOD_BOIL));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}


int vampiricTouch(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!", FALSE, caster, NULL, victim,
      TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (1 > victim->getHit()) {
    act("That creature has nothing left for you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  int damage = caster->getSkillDam(victim, SPELL_VAMPIRIC_TOUCH, level, adv_learn);
  int hpGain = ::number(30, 75) * (caster->getSkillValue(SPELL_VAMPIRIC_TOUCH) / 100);
  int lfGain = ::number(60, 150) * (caster->getSkillValue(SPELL_VAMPIRIC_TOUCH) / 100);
  bool save = victim->isLucky(caster->spellLuckModifier(SPELL_VAMPIRIC_TOUCH));

  if (caster->bSuccess(bKnown, SPELL_VAMPIRIC_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL,
      victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL,
      victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster,
      NULL, victim, TO_VICT);
    
    switch (critSuccess(caster, SPELL_VAMPIRIC_TOUCH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_VAMPIRIC_TOUCH);
        damage *= 2;
        lfGain *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (save) {
      SV(SPELL_VAMPIRIC_TOUCH);
      damage /= 2;
      lfGain /= 2;
    }
    caster->reconcileHurt(victim, discArray[SPELL_VAMPIRIC_TOUCH]->alignMod);
    caster->addToHit(hpGain);
    caster->addToLifeforce(lfGain);
    caster->updatePos();
    if (caster->reconcileDamage(victim, damage, SPELL_VAMPIRIC_TOUCH) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  }
  // Failure
  switch (critFail(caster, SPELL_VAMPIRIC_TOUCH)) {
    case CRIT_F_HITSELF:
    case CRIT_F_HITOTHER:
      CF(SPELL_VAMPIRIC_TOUCH);
      act("$n's body glows a dark, evil-looking red!", FALSE, caster, NULL,
        NULL, TO_ROOM);
      act(
        "You sang the invokation incorrectly! The ancestors are EXTREMELY "
        "pissed!",
        FALSE, caster, NULL, NULL, TO_CHAR);
      victim->addToHit(damage);
      if (caster->reconcileDamage(caster, damage, SPELL_VAMPIRIC_TOUCH) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    case CRIT_F_NONE:
      break;
  }
  caster->nothingHappens();
  return SPELL_FAIL;
}

int vampiricTouch(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_VAMPIRIC_TOUCH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_VAMPIRIC_TOUCH]->lag;
  taskDiffT diff = discArray[SPELL_VAMPIRIC_TOUCH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_VAMPIRIC_TOUCH, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castVampiricTouch(TBeing* caster, TBeing* victim) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_VAMPIRIC_TOUCH);
  int bKnown = caster->getSkillValue(SPELL_VAMPIRIC_TOUCH);

  ret = vampiricTouch(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_VAMPIRIC_TOUCH));
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int vampiricTouch(TBeing* tMaster, TBeing* tSucker, TMagicItem* tMagItem) {
  int tRc = FALSE, tReturn;

  tReturn = vampiricTouch(tMaster, tSucker, tMagItem->getMagicLevel(),
    tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

int lifeLeech(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!", FALSE, caster, NULL, victim,
      TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (1 > victim->getHit()) {
    act("That creature has nothing left for you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  int damage = caster->getSkillDam(victim, SPELL_LIFE_LEECH, level, adv_learn);
  int hpGain = ::number(10, 35) * (caster->getSkillValue(SPELL_LIFE_LEECH) / 100);
  int lfGain = ::number(20, 70) * (caster->getSkillValue(SPELL_LIFE_LEECH) / 100);

  if (victim->getImmunity(IMMUNE_DRAIN) >= 100) {
    act("$N is immune to draining!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim,
      TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL,
      victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_LIFE_LEECH)) {
    act("$N buckles in pain as life is drawn from $S body!", FALSE, caster,
      NULL, victim, TO_NOTVICT);
    act("$N buckles in pain as life is drawn from $S body!", FALSE, caster,
      NULL, victim, TO_CHAR);
    act("You buckle in pain as life is drawn from your body!", FALSE, caster,
      NULL, victim, TO_VICT);
    switch (critSuccess(caster, SPELL_LIFE_LEECH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_LIFE_LEECH);
        damage *= 2;
        lfGain *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    caster->reconcileHurt(victim, discArray[SPELL_LIFE_LEECH]->alignMod);
    caster->addToHit(hpGain);
    caster->addToLifeforce(lfGain);
    caster->updatePos();
    if (caster->reconcileDamage(victim, damage, SPELL_LIFE_LEECH) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  }

  switch (critFail(caster, SPELL_LIFE_LEECH)) {
    case CRIT_F_HITSELF:
    case CRIT_F_HITOTHER:
      CF(SPELL_LIFE_LEECH);
      act("<r>$n's body glows a dark, evil-looking red!<z>", FALSE, caster,
        NULL, NULL, TO_ROOM);
      act(
        "<r>You sang the invokation incorrectly! The ancestors are<z> "
        "<R>EXTREMELY<z> <r>pissed!<z>",
        FALSE, caster, NULL, NULL, TO_CHAR);
      if (caster->reconcileDamage(caster, damage, SPELL_LIFE_LEECH) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      victim->addToHit(damage);
      return SPELL_CRIT_FAIL;
    case CRIT_F_NONE:
      break;
  }
  caster->nothingHappens();
  return SPELL_FAIL;
}

int lifeLeech(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_LIFE_LEECH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_LIFE_LEECH]->lag;
  taskDiffT diff = discArray[SPELL_LIFE_LEECH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LIFE_LEECH, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castLifeLeech(TBeing* caster, TBeing* victim) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_LIFE_LEECH);
  int bKnown = caster->getSkillValue(SPELL_LIFE_LEECH);

  ret = lifeLeech(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_LIFE_LEECH));
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int lifeLeech(TBeing* tMaster, TBeing* tSucker, TMagicItem* tMagItem) {
  int tRc = FALSE, tReturn;

  tReturn = lifeLeech(tMaster, tSucker, tMagItem->getMagicLevel(),
    tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}


int intimidate(TBeing* caster, TBeing* victim, int level, short bKnown) {
  int rc;

  if (caster->isNotPowerful(victim, level, SPELL_INTIMIDATE, SILENT_NO)) {
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_INTIMIDATE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_INTIMIDATE)) {
    if (victim->isLucky(caster->spellLuckModifier(SPELL_INTIMIDATE)) ||
        victim->isImmune(IMMUNE_FEAR, WEAR_BODY)) {
      SV(SPELL_INTIMIDATE);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("You feel intimidated briefly.", FALSE, caster, NULL, victim, TO_VICT,
        ANSI_YELLOW_BOLD);
    } else {
      act("$N is totally intimidated by $n!", FALSE, caster, NULL, victim,
        TO_NOTVICT, ANSI_YELLOW_BOLD);
      act("$N is totally intimidated by you.", FALSE, caster, NULL, victim,
        TO_CHAR, ANSI_YELLOW_BOLD);
      act("$n intimidates you! RUN!!!", FALSE, caster, NULL, victim, TO_VICT,
        ANSI_YELLOW_BOLD);

      rc = victim->doFlee("");
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return SPELL_SUCCESS + VICTIM_DEAD;
      }

      // this will keep them running away
      if (!victim->isPc() && level >= (int)(victim->GetMaxLevel() * 1.1)) {
        dynamic_cast<TMonster*>(victim)->addFeared(caster);
      }

      // but since once at full HP (and fearing) they will grow a hatred
      // we need a way to make the fear last a little while....
      affectedData aff;
      aff.type = SPELL_INTIMIDATE;
      aff.duration = caster->durationModify(SPELL_INTIMIDATE,
        level * Pulse::UPDATES_PER_MUDHOUR / 2);
      aff.renew = aff.duration;  // renewable immediately

      // we've made raw immunity check, but allow it to reduce effects too
      aff.duration *= (100 - victim->getImmunity(IMMUNE_FEAR));
      aff.duration /= 100;

      victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_INTIMIDATE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_INTIMIDATE);
        act("$n attempts to flee...from $mself?!? Strange...", FALSE, caster,
          NULL, NULL, TO_ROOM, ANSI_YELLOW_BOLD);
        act(
          "Damn! The loa have intimidated you by yourself. Run for your life!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_YELLOW_BOLD);
        act("$n was trying to invoke ritual on you!", FALSE, caster, NULL,
          victim, TO_VICT, ANSI_YELLOW_BOLD);
        rc = caster->doFlee("");
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        }
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int intimidate(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_INTIMIDATE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_INTIMIDATE]->lag;
  taskDiffT diff = discArray[SPELL_INTIMIDATE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_INTIMIDATE, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castIntimidate(TBeing* caster, TBeing* victim) {
  int rc = 0;

  int level = caster->getSkillLevel(SPELL_INTIMIDATE);
  int bKnown = caster->getSkillValue(SPELL_INTIMIDATE);

  int ret = intimidate(caster, victim, level, bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  } else {
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int flatulence(TBeing* caster, int level, short bKnown, int adv_learn) {
  TThing* t;
  TBeing* vict = NULL;

  int dam = caster->getSkillDam(NULL, SPELL_FLATULENCE, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_FLATULENCE)) {
    act("<o>You turn around quickly and pass gas!<1>", FALSE, caster, NULL,
      NULL, TO_CHAR);
    act("<o>$n turns around quickly and passes gas!<1>", FALSE, caster, NULL,
      NULL, TO_ROOM);
    for (StuffIter it = caster->roomp->stuff.begin();
         it != caster->roomp->stuff.end();) {
      t = *(it++);
      vict = dynamic_cast<TBeing*>(t);
      if (!vict)
        continue;

      if (!caster->inGroup(*vict) && !(vict->isImmortal()) &&
          !(vict->getImmunity(IMMUNE_SUFFOCATION) >= 100)) {
        caster->reconcileHurt(vict, discArray[SPELL_FLATULENCE]->alignMod);
        act("$n is choked by the natural gasses!", FALSE, vict, NULL, NULL,
          TO_ROOM);
        act("You are choked by the natural gasses!", FALSE, vict, NULL, NULL,
          TO_CHAR);
        if (caster->reconcileDamage(vict, dam, SPELL_FLATULENCE) == -1) {
          delete vict;
          vict = NULL;
        }
      } else {
        act(
          "$n takes a deep breath and holds it until the noxious fumes "
          "disperse.",
          TRUE, vict, NULL, 0, TO_ROOM);
        act(
          "You take a deep breath and hold it until the noxious fumes "
          "disperse.",
          TRUE, vict, NULL, NULL, TO_CHAR);
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_FLATULENCE)) {
      CF(SPELL_FLATULENCE);
      act("Oh no!!! That one stuck with you!", FALSE, caster, NULL, NULL,
        TO_CHAR);
      act("$n chokes on $s own fumes!!", FALSE, caster, NULL, NULL, TO_ROOM);
      if (caster->reconcileDamage(caster, dam, SPELL_FLATULENCE) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int flatulence(TBeing* caster) {
  if (!bPassShamanChecks(caster, SPELL_FLATULENCE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_FLATULENCE]->lag;
  taskDiffT diff = discArray[SPELL_FLATULENCE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_FLATULENCE, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castFlatulence(TBeing* caster) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_FLATULENCE);

  ret = flatulence(caster, level, caster->getSkillValue(SPELL_FLATULENCE),
    caster->getAdvLearning(SPELL_FLATULENCE));
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

int soulTwist(TBeing* caster, TBeing* victim, int level, short bKnown,
  int adv_learn) {
  if (victim->isImmortal()) {
    act("You can't twist a gods soul!", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (victim->getImmunity(IMMUNE_DRAIN) >= 100) {
    act("$N is immune to draining!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim,
      TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL,
      victim, TO_VICT);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_SOUL_TWIST, level, adv_learn);
  caster->reconcileHurt(victim, discArray[SPELL_SOUL_TWIST]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SOUL_TWIST)) {
    switch (critSuccess(caster, SPELL_SOUL_TWIST)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        act("$N screams in EXTREME pain!.", FALSE, caster, 0, victim, TO_CHAR);
        act("You scream in EXTREME pain!", FALSE, caster, 0, victim, TO_VICT);
        act("$N screams in EXTREME pain!", FALSE, caster, 0, victim,
          TO_NOTVICT);
        CS(SPELL_SOUL_TWIST);
        dam <<= 1;
        break;
      case CRIT_S_NONE:
        act("$N screams in pain!.", FALSE, caster, 0, victim, TO_CHAR);
        act("You scream in pain!", FALSE, caster, 0, victim, TO_VICT);
        act("$N screams in pain!", FALSE, caster, 0, victim, TO_NOTVICT);
        if (victim->isLucky(caster->spellLuckModifier(SPELL_SOUL_TWIST))) {
          SV(SPELL_SOUL_TWIST);
          dam /= 2;
        }
    }

    if (caster->reconcileDamage(victim, dam, SPELL_SOUL_TWIST) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_SOUL_TWIST)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_SOUL_TWIST);
        act("You scream in EXTREME pain!.", FALSE, caster, 0, victim, TO_CHAR);
        act("$n screams in EXTREME pain!", FALSE, caster, 0, victim, TO_ROOM);
        if (caster->reconcileDamage(caster, dam, SPELL_SOUL_TWIST) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int soulTwist(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  int ret = 0;
  int rc = 0;

  ret = soulTwist(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int soulTwist(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_SOUL_TWIST, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SOUL_TWIST]->lag;
  diff = discArray[SPELL_SOUL_TWIST]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SOUL_TWIST, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castSoulTwist(TBeing* caster, TBeing* victim) {
  int ret, level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_SOUL_TWIST);
  int bKnown = caster->getSkillValue(SPELL_SOUL_TWIST);

  ret = soulTwist(caster, victim, level, bKnown,
    caster->getAdvLearning(SPELL_SOUL_TWIST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret == SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}
