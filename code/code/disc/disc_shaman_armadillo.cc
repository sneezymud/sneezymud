#include "extern.h"
#include "handler.h"
#include "room.h"
#include "being.h"
#include "disease.h"
#include "monster.h"
#include "person.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_armadillo.h"
#include "obj_magic_item.h"

int TBeing::doEarthmaw(const char* argument) {
  if (!doesKnowSkill(SPELL_EARTHMAW)) {
    sendTo("You do no know the secrets of the earthmaw spell.\n\r");
    return FALSE;
  }

  if (this->roomp->notRangerLandSector()) {
    sendTo(
      "You must be in a wilderness landscape for the earthmaw spell to be "
      "effective!\n\r");
    return FALSE;
  }
  char tTarget[256];
  TObj* tObj = NULL;
  TBeing* victim = NULL;
  TBeing* horsie = NULL;

  if (checkBusy())
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
  int bKnown = getSkillValue(SPELL_EARTHMAW);

  int dam =
    getSkillDam(victim, SPELL_EARTHMAW, lev, getAdvLearning(SPELL_EARTHMAW));

  if (!useComponent(findComponent(SPELL_EARTHMAW), this, CHECK_ONLY_NO))
    return FALSE;

  addToWait((int)combatRound(discArray[SPELL_EARTHMAW]->lag));
  reconcileHurt(victim, discArray[SPELL_EARTHMAW]->alignMod);

  if (bSuccess(bKnown, SPELL_EARTHMAW)) {
    if (critSuccess(this, SPELL_EARTHMAW) == CRIT_S_DOUBLE) {
      CS(SPELL_EARTHMAW);
      dam *= 2;
      act("<Y>An incredibly large fissure opens up in the ground below you!<1>",
        FALSE, this, NULL, victim, TO_VICT);
      act(
        "<Y>An incredibly large fissure opens up in the ground below "
        "$N<Y><o>!<1>",
        FALSE, this, NULL, victim, TO_NOTVICT);
      act(
        "<Y>An incredibly large fissure opens up in the ground below "
        "$N<Y><o>!<1>",
        FALSE, this, NULL, victim, TO_CHAR);
    } else {
      act("<o>A large fissure opens up in the ground below you!<1>", FALSE,
        this, NULL, victim, TO_VICT);
      act("<o>A large fissure opens up in the ground below $N<1><o>!<1>", FALSE,
        this, NULL, victim, TO_NOTVICT);
      act("<o>A large fissure opens up in the ground below $N<1><o>!<1>", FALSE,
        this, NULL, victim, TO_CHAR);
    }

    if ((horsie = dynamic_cast<TBeing*>(victim->riding))) {
      act("$N collapses beneath $n as the $g gives way!", TRUE, victim, 0,
        horsie, TO_ROOM);
      act("$N collapses beneath you as the $g gives way!", TRUE, victim, 0,
        horsie, TO_CHAR);
      victim->fallOffMount(victim->riding, POSITION_SITTING);

      act("<o>$N<1><o> tumbles into the fissure!<1>", FALSE, this, NULL, horsie,
        TO_CHAR);
      act("<o>$N<1><o> tumbles into the fissure!<1>", FALSE, this, NULL, horsie,
        TO_NOTVICT);
      act("<o>You tumble into the fissure!<1>", FALSE, this, NULL, horsie,
        TO_VICT);
    }

    act(
      "<o>$N<1><o> tumbles into the fissure, which collapses on top of $m!<1>",
      FALSE, this, NULL, victim, TO_CHAR);
    act(
      "<o>$N<1><o> tumbles into the fissure, which collapses on top of $m!<1>",
      FALSE, this, NULL, victim, TO_NOTVICT);
    act("<o>You tumble into the fissure, which collapses on top of you!<1>",
      FALSE, this, NULL, victim, TO_VICT);

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

// THORNFLESH

int thornflesh(TBeing* caster) {
  if (caster->affectedBySpell(SPELL_THORNFLESH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  if (!bPassShamanChecks(caster, SPELL_THORNFLESH, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_THORNFLESH]->lag;
  taskDiffT diff = discArray[SPELL_THORNFLESH]->task;

  start_cast(caster, caster, NULL, caster->roomp, SPELL_THORNFLESH, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
}

int castThornflesh(TBeing* caster) {
  int ret, level;

  if (caster && caster->affectedBySpell(SPELL_THORNFLESH)) {
    act("You flesh is armored well enough.", FALSE, caster, NULL, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_THORNFLESH);
  int bKnown = caster->getSkillValue(SPELL_THORNFLESH);

  ret = thornflesh(caster, level, bKnown);

  return ret;
}

int thornflesh(TBeing* caster, int level, short bKnown) {
  affectedData aff;

  if (caster->affectedBySpell(SPELL_THORNFLESH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  aff.type = SPELL_THORNFLESH;
  aff.duration = caster->durationModify(SPELL_THORNFLESH,
    max(min(level / 10, 5), 1) * Pulse::UPDATES_PER_MUDHOUR);
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = level;

  if (caster->bSuccess(bKnown, SPELL_THORNFLESH)) {
    switch (critSuccess(caster, SPELL_THORNFLESH)) {
      case CRIT_S_KILL:
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
        CS(SPELL_THORNFLESH);
        aff.duration *= 2;
        act("LARGE thorns emerge from your body!", FALSE, caster, 0, 0, TO_CHAR,
          ANSI_ORANGE);
        act("LARGE thorns emerge from $n's body!", FALSE, caster, 0, 0, TO_ROOM,
          ANSI_ORANGE);
        break;
      default:
        act("Thorns emerge from your body!", FALSE, caster, 0, 0, TO_CHAR,
          ANSI_ORANGE);
        act("Thorns emerge from $n's body!", FALSE, caster, 0, 0, TO_ROOM,
          ANSI_ORANGE);
        break;
    }
    caster->affectTo(&aff, -1);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

// END THORNFLESH
// AQUALUNG

static bool canBeLunged(TBeing* caster, TBeing* victim) {
  if (victim->isAffected(AFF_WATERBREATH) &&
      !(victim->affectedBySpell(SPELL_GILLS_OF_FLESH) ||
        !(victim->affectedBySpell(SPELL_AQUALUNG) ||
          victim->affectedBySpell(SPELL_BREATH_OF_SARAHAGE)))) {
    if (caster != victim)
      act("$N already has the ability to breathe underwater.", FALSE, caster,
        NULL, victim, TO_CHAR);
    else
      act("You already have the ability to breathe underwater.", FALSE, caster,
        NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return true;
  }
  return false;
}

int aqualung(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  if (canBeLunged(caster, victim))
    return FALSE;

  if (caster->bSuccess(bKnown, SPELL_AQUALUNG)) {
    caster->reconcileHelp(victim, discArray[SPELL_AQUALUNG]->alignMod);
    aff.type = SPELL_AQUALUNG;
    aff.level = level;
    aff.duration =
      caster->durationModify(SPELL_AQUALUNG, 8 * Pulse::UPDATES_PER_MUDHOUR);
    aff.modifier = 0;
    aff.renew = aff.duration;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_WATERBREATH;

    switch (critSuccess(caster, SPELL_AQUALUNG)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_AQUALUNG);
        aff.duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_NO)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("A transparent globe surrounds your head!", TRUE, victim, NULL, NULL,
      TO_CHAR, ANSI_BLUE);
    act("A transparent globe surrounds $N's head!", TRUE, caster, NULL, victim,
      TO_NOTVICT, ANSI_BLUE);
    if (victim != caster)
      act("You bestow upon $N the ability to breathe water!", TRUE, caster,
        NULL, victim, TO_CHAR, ANSI_BLUE);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void aqualung(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  aqualung(caster, victim, obj->getMagicLevel(), obj->getMagicLearnedness());
}

int aqualung(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (canBeLunged(caster, victim))
    return FALSE;

  if (!bPassShamanChecks(caster, SPELL_AQUALUNG, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_AQUALUNG]->lag;
  diff = discArray[SPELL_AQUALUNG]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_AQUALUNG, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castAqualung(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_AQUALUNG);
  int bKnown = caster->getSkillValue(SPELL_AQUALUNG);

  if ((ret = aqualung(caster, victim, level, bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

// END AQUALUNG
// SHADOW WALK
int shadowWalk(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff, aff2;

#if 0
  if (victim->affectedBySpell(SPELL_SHADOW_WALK)) {
    char buf[256];
    act("You already walk among the shadows.", FALSE, caster, NULL, victim, 
TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
#endif

  caster->reconcileHelp(victim, discArray[SPELL_SHADOW_WALK]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SHADOW_WALK)) {
    aff.type = SPELL_SHADOW_WALK;
    aff.level = level;
    aff.duration = caster->durationModify(SPELL_SHADOW_WALK,
      18 * Pulse::UPDATES_PER_MUDHOUR);
    aff.modifier = -40;
    aff.location = APPLY_ARMOR;
    aff.bitvector = AFF_SHADOW_WALK;

    aff2.type = SPELL_SHADOW_WALK;
    aff2.level = level;
    aff2.duration = aff.duration;
    aff2.location = APPLY_IMMUNITY;
    aff2.modifier = IMMUNE_NONMAGIC;
    aff2.modifier2 = 10;
    aff2.bitvector = AFF_SHADOW_WALK;

    switch (critSuccess(caster, SPELL_SHADOW_WALK)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SHADOW_WALK);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void shadowWalk(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  int ret = shadowWalk(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());

  if (ret == SPELL_SUCCESS) {
#if 0
    act("$n becomes darker and walks a little swifter!", FALSE, victim, NULL, NULL,
                 TO_ROOM, ANSI_GREEN);
    act("You now walk among the shadows!", FALSE, victim, NULL, NULL, TO_CHAR,
                 ANSI_GREEN);
#endif
  } else {
    caster->nothingHappens();
  }
}

int shadowWalk(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_SHADOW_WALK, victim))
    return FALSE;

#if 0
  if (victim->affectedBySpell(SPELL_SHADOW_WALK)) {
    char buf[256];
    act("You already are walking among the shadows.", FALSE, caster, NULL, victim, 
TO_CHAR);
    return FALSE;
  }
#endif

  lag_t rounds = discArray[SPELL_SHADOW_WALK]->lag;
  diff = discArray[SPELL_SHADOW_WALK]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SHADOW_WALK, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castShadowWalk(TBeing* caster, TBeing* victim) {
  int level = caster->getSkillLevel(SPELL_SHADOW_WALK);
  int bKnown = caster->getSkillValue(SPELL_SHADOW_WALK);

  int ret = shadowWalk(caster, victim, level, bKnown);
  if (ret == SPELL_SUCCESS) {
#if 0
    act("$n becomes dark and walks a little swifter!", FALSE, victim, NULL, NULL, 
TO_ROOM, ANSI_GREEN);
    act("You now walk among the shadows!", FALSE, victim, NULL, NULL, TO_CHAR, 
ANSI_GREEN);
#endif
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

// END SHADOW WALK

int celerite(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;
  caster->reconcileHelp(victim, discArray[SPELL_CELERITE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CELERITE)) {
    aff.type = SPELL_CELERITE;
    aff.level = level;
    aff.duration = caster->durationModify(SPELL_CELERITE,
      (aff.level / 3) * Pulse::UPDATES_PER_MUDHOUR);
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    switch (critSuccess(caster, SPELL_CELERITE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CELERITE);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("$N moves more easily.", FALSE, caster, NULL, victim, TO_NOTVICT,
      ANSI_YELLOW_BOLD);
    act("The power of the loa takes dominion inside of you!", FALSE, victim,
      NULL, NULL, TO_CHAR, ANSI_YELLOW_BOLD);
    return SPELL_SUCCESS;
  } else {
    act("The loa ignore your unfaithful request!", FALSE, caster, NULL, victim,
      TO_CHAR, ANSI_YELLOW);
    caster->addToLifeforce(-10);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
}

int celerite(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_CELERITE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CELERITE]->lag;
  diff = discArray[SPELL_CELERITE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CELERITE, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

void celerite(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  celerite(caster, victim, obj->getMagicLevel(), obj->getMagicLearnedness());
}

int castCelerite(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_CELERITE);
  int bKnown = caster->getSkillValue(SPELL_CELERITE);

  if ((ret = celerite(caster, victim, level, bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}


void shieldOfMists(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  shieldOfMists(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());
}

int shieldOfMists(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_SHIELD_OF_MISTS, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SHIELD_OF_MISTS]->lag;
  diff = discArray[SPELL_SHIELD_OF_MISTS]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SHIELD_OF_MISTS, diff,
    1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castShieldOfMists(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_SHIELD_OF_MISTS);
  int bKnown = caster->getSkillValue(SPELL_SHIELD_OF_MISTS);

  if ((ret = shieldOfMists(caster, victim, level, bKnown)) == SPELL_SUCCESS) {}
  return TRUE;
}

int cheval(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_CHEVAL]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CHEVAL)) {
    aff.type = SPELL_CHEVAL;
    aff.level = level;
    aff.duration = caster->durationModify(SPELL_CHEVAL,
      (aff.level / 3) * Pulse::UPDATES_PER_MUDHOUR);
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    switch (critSuccess(caster, SPELL_CHEVAL)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CHEVAL);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("$N seems as if $E is possessed!", FALSE, caster, NULL, victim,
      TO_NOTVICT, ANSI_GREEN);
    act("The power of the loa takes dominion inside of you!", FALSE, victim,
      NULL, NULL, TO_CHAR, ANSI_GREEN);
    return SPELL_SUCCESS;
  } else {
    act("The loa ignore your unfaithful request!", FALSE, caster, NULL, victim,
      TO_CHAR, ANSI_GREEN);
    caster->addToLifeforce(-10);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
}

int cheval(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_CHEVAL, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CHEVAL]->lag;
  diff = discArray[SPELL_CHEVAL]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CHEVAL, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

void cheval(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  cheval(caster, victim, obj->getMagicLevel(), obj->getMagicLearnedness());
}

int castCheval(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_CHEVAL);
  int bKnown = caster->getSkillValue(SPELL_CHEVAL);

  if ((ret = cheval(caster, victim, level, bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}


int senseLifeShaman(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_SENSE_LIFE_SHAMAN]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SENSE_LIFE_SHAMAN)) {
    aff.type = SPELL_SENSE_LIFE_SHAMAN;
    aff.duration = caster->durationModify(SPELL_SENSE_LIFE_SHAMAN,
      level * 2 / 3 * Pulse::UPDATES_PER_MUDHOUR);
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_SENSE_LIFE;

    switch (critSuccess(caster, SPELL_SENSE_LIFE_SHAMAN)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SENSE_LIFE_SHAMAN);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void senseLifeShaman(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  int ret;

  ret = senseLifeShaman(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());
  if (ret == SPELL_SUCCESS) {
    victim->sendTo("You feel more aware of the world about you.\n\r");
    act("$n's eyes flicker a faint pale blue.", FALSE, victim, NULL, NULL,
      TO_ROOM, ANSI_CYAN);
  } else {
    caster->nothingHappens();
  }
}

int senseLifeShaman(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_SENSE_LIFE_SHAMAN, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SENSE_LIFE_SHAMAN]->lag;
  diff = discArray[SPELL_SENSE_LIFE_SHAMAN]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SENSE_LIFE_SHAMAN, diff,
    1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castSenseLifeShaman(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_SENSE_LIFE_SHAMAN);
  int bKnown = caster->getSkillValue(SPELL_SENSE_LIFE_SHAMAN);

  ret = senseLifeShaman(caster, victim, level, bKnown);
  if (ret == SPELL_SUCCESS) {
    victim->sendTo("You feel more aware of the world about you.\n\r");
    act("$n's eyes flicker a faint pale blue.", FALSE, victim, NULL, NULL,
      TO_ROOM, ANSI_CYAN);
  } else
    caster->nothingHappens();

  return TRUE;
}

int detectShadow(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_DETECT_SHADOW]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_DETECT_SHADOW)) {
    aff.type = SPELL_DETECT_SHADOW;
    aff.duration = caster->durationModify(SPELL_DETECT_SHADOW,
      level * 3 / 2 * Pulse::UPDATES_PER_MUDHOUR);
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_DETECT_INVISIBLE;

    switch (critSuccess(caster, SPELL_DETECT_SHADOW)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_DETECT_SHADOW);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("$n's eyes briefly glow red.", FALSE, victim, 0, 0, TO_ROOM,
      ANSI_RED_BOLD);
    act("Your eyes sting.", FALSE, victim, 0, 0, TO_CHAR, ANSI_RED_BOLD);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void detectShadow(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  detectShadow(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());
}

int detectShadow(TBeing* caster, TBeing* victim) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DETECT_SHADOW, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DETECT_SHADOW]->lag;
  diff = discArray[SPELL_DETECT_SHADOW]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DETECT_SHADOW, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castDetectShadow(TBeing* caster, TBeing* victim) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_DETECT_SHADOW);
  int bKnown = caster->getSkillValue(SPELL_DETECT_SHADOW);

  ret = detectShadow(caster, victim, level, bKnown);

  if (ret == SPELL_SUCCESS) {}
  return TRUE;
}


int djallasProtection(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff, aff2, aff3, aff4;

  aff.type = SPELL_DJALLA;
  aff.level = level;
  aff.duration = caster->durationModify(SPELL_DJALLA,
    (3 + (level / 2)) * Pulse::UPDATES_PER_MUDHOUR);
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_SUMMON;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;

  aff2.type = SPELL_DJALLA;
  aff2.level = level;
  aff2.duration = aff.duration;
  aff2.location = APPLY_IMMUNITY;
  aff2.modifier = IMMUNE_POISON;
  aff2.modifier2 = ((level * 2) / 3);
  aff2.bitvector = 0;

  aff3.type = SPELL_DJALLA;
  aff3.level = level;
  aff3.duration = aff.duration;
  aff3.location = APPLY_IMMUNITY;
  aff3.modifier = IMMUNE_DRAIN;
  aff3.modifier2 = ((level * 2) / 3);
  aff3.bitvector = 0;

  aff4.type = SPELL_DJALLA;
  aff4.level = level;
  aff4.duration = aff.duration;
  aff4.location = APPLY_IMMUNITY;
  aff4.modifier = IMMUNE_ENERGY;
  aff4.modifier2 = ((level * 2) / 3);
  aff4.bitvector = 0;

  if (caster->bSuccess(bKnown, SPELL_DJALLA)) {
    switch (critSuccess(caster, SPELL_DJALLA)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_DJALLA);
        aff.duration *= 2;
        aff.modifier2 = (level * 2);
        aff2.duration *= 2;
        aff2.modifier2 = (level * 2);
        aff3.duration *= 2;
        aff3.modifier2 = (level * 2);
        aff4.duration *= 2;
        aff4.modifier2 = (level * 2);
        act("$n becomes one with the spirits.", FALSE, victim, NULL, NULL,
          TO_ROOM, ANSI_GREEN);
        act("You have been greatly blessed with the protection of Djalla!",
          FALSE, victim, NULL, NULL, TO_CHAR, ANSI_GREEN);
        break;
      case CRIT_S_NONE:
        act("$n becomes one with the spirits.", FALSE, victim, NULL, NULL,
          TO_ROOM, ANSI_GREEN);
        act("You have been granted the protection of Djalla!", FALSE, victim,
          NULL, NULL, TO_CHAR, ANSI_GREEN);
        break;
    }

    if (caster != victim) {
      aff.modifier2 /= 2;
      aff2.modifier2 /= 2;
      aff3.modifier2 /= 2;
      aff4.modifier2 /= 2;
    }

    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff3, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff4, AVG_DUR_NO, AVG_EFF_YES);
    caster->reconcileHelp(victim, discArray[SPELL_DJALLA]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
void djallasProtection(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  djallasProtection(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());
}

int djallasProtection(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_DJALLA, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DJALLA]->lag;
  taskDiffT diff = discArray[SPELL_DJALLA]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DJALLA, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castDjallasProtection(TBeing* caster, TBeing* victim) {
  int level = caster->getSkillLevel(SPELL_DJALLA);
  int bKnown = caster->getSkillValue(SPELL_DJALLA);

  int ret = djallasProtection(caster, victim, level, bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int legbasGuidance(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff, aff2, aff3, aff4;

  aff.type = SPELL_LEGBA;
  aff.level = level;
  aff.duration = caster->durationModify(SPELL_LEGBA,
    (3 + (level / 2)) * Pulse::UPDATES_PER_MUDHOUR);
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_BLEED;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;

  aff2.type = SPELL_LEGBA;
  aff2.level = level;
  aff2.duration = aff.duration;
  aff2.location = APPLY_IMMUNITY;
  aff2.modifier = IMMUNE_EARTH;
  aff2.modifier2 = ((level * 2) / 3);
  aff2.bitvector = 0;

  aff3.type = SPELL_LEGBA;
  aff3.level = level;
  aff3.duration = aff.duration;
  aff3.location = APPLY_IMMUNITY;
  aff3.modifier = IMMUNE_CHARM;
  aff3.modifier2 = ((level * 2) / 3);
  aff3.bitvector = 0;

  aff4.type = SPELL_LEGBA;
  aff4.level = level;
  aff4.duration = aff.duration;
  aff4.location = APPLY_IMMUNITY;
  aff4.modifier = IMMUNE_SLEEP;
  aff4.modifier2 = ((level * 2) / 3);
  aff4.bitvector = 0;

  if (caster->bSuccess(bKnown, SPELL_LEGBA)) {
    switch (critSuccess(caster, SPELL_LEGBA)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_LEGBA);
        aff.duration *= 2;
        aff.modifier2 = (level * 2);
        aff2.duration *= 2;
        aff2.modifier2 = (level * 2);
        aff3.duration *= 2;
        aff3.modifier2 = (level * 2);
        aff4.duration *= 2;
        aff4.modifier2 = (level * 2);
        act("$n becomes one with the spirits.", FALSE, victim, NULL, NULL,
          TO_ROOM, ANSI_GREEN);
        act("You have been greatly blessed with the protection of Legba!",
          FALSE, victim, NULL, NULL, TO_CHAR, ANSI_GREEN);
        break;
      case CRIT_S_NONE:
        act("$n becomes one with the spirits.", FALSE, victim, NULL, NULL,
          TO_ROOM, ANSI_GREEN);
        act("You have been granted the protection of Legba!", FALSE, victim,
          NULL, NULL, TO_CHAR, ANSI_GREEN);
        break;
    }

    if (caster != victim) {
      aff.modifier2 /= 2;
      aff2.modifier2 /= 2;
      aff3.modifier2 /= 2;
      aff4.modifier2 /= 2;
    }

    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff3, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff4, AVG_DUR_NO, AVG_EFF_YES);
    caster->reconcileHelp(victim, discArray[SPELL_LEGBA]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
void legbasGuidance(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  legbasGuidance(caster, victim, obj->getMagicLevel(),
    obj->getMagicLearnedness());
}

int legbasGuidance(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_LEGBA, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_LEGBA]->lag;
  taskDiffT diff = discArray[SPELL_LEGBA]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LEGBA, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castLegbasGuidance(TBeing* caster, TBeing* victim) {
  int level = caster->getSkillLevel(SPELL_LEGBA);
  int bKnown = caster->getSkillValue(SPELL_LEGBA);

  int ret = legbasGuidance(caster, victim, level, bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int chaseSpirits(TBeing* caster, TObj* obj, int, short bKnown) {
  int i;

  if (caster->bSuccess(bKnown, SPELL_CHASE_SPIRIT)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].location != APPLY_LIGHT) &&
          (obj->affected[i].location != APPLY_NOISE) &&
          (obj->affected[i].location != APPLY_HIT) &&
          (obj->affected[i].location != APPLY_CHAR_WEIGHT) &&
          (obj->affected[i].location != APPLY_CHAR_HEIGHT) &&
          (obj->affected[i].location != APPLY_MOVE) &&
          (obj->affected[i].location != APPLY_ARMOR)) {
        obj->affected[i].location = APPLY_NONE;
        obj->affected[i].modifier = 0;
        obj->affected[i].modifier2 = 0;
        obj->affected[i].bitvector = 0;
      }
    }
    obj->remObjStat(ITEM_MAGIC);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void chaseSpirits(TBeing* caster, TObj* tar_obj, TMagicItem* obj) {
  int ret = 0;
  int level = obj->getMagicLevel();

  ret = chaseSpirits(caster, tar_obj, level, obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p chases the spirits from $n...", FALSE, tar_obj, obj, NULL, TO_ROOM);
  }
}

int chaseSpirits(TBeing* caster, TObj* obj) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_CHASE_SPIRIT, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_CHASE_SPIRIT]->lag;
  diff = discArray[SPELL_CHASE_SPIRIT]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_CHASE_SPIRIT, diff, 2, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castChaseSpirits(TBeing* caster, TObj* obj) {
  int ret = 0, level;

  level = caster->getSkillLevel(SPELL_CHASE_SPIRIT);

  ret =
    chaseSpirits(caster, obj, level, caster->getSkillValue(SPELL_CHASE_SPIRIT));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("You chase away the evil spirits within $N...", FALSE, caster, NULL,
      obj, TO_CHAR);
    act("$n chases away the evil spirits within $N...", FALSE, caster, NULL,
      obj, TO_ROOM);
  }
  return ret;
}

int chaseSpirits(TBeing* caster, TBeing* victim, int, short bKnown) {
  caster->reconcileHurt(victim, discArray[SPELL_CHASE_SPIRIT]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CHASE_SPIRIT)) {
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int chaseSpirits(TBeing* caster, TBeing* victim, TMagicItem* obj) {
  mud_assert(caster != NULL, "chaseSpirits(): no caster");
  mud_assert(victim != NULL, "chaseSpirits(): no victim");

  int level = obj->getMagicLevel();

  int ret = chaseSpirits(caster, victim, level, obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p chases away the evil spirits within you...", FALSE, victim, obj,
      NULL, TO_CHAR);
    act("$p chases away the evil spirits within $n...", FALSE, victim, obj,
      NULL, TO_ROOM);
    int rc = genericChaseSpirits(caster, victim, level, caster->isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return 0;
}

int chaseSpirits(TBeing* caster, TBeing* victim) {
  if (!bPassShamanChecks(caster, SPELL_CHASE_SPIRIT, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CHASE_SPIRIT]->lag;
  taskDiffT diff = discArray[SPELL_CHASE_SPIRIT]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CHASE_SPIRIT, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castChaseSpirits(TBeing* caster, TBeing* victim) {
  mud_assert(caster != NULL, "castChaseSpirits(): no caster");
  mud_assert(victim != NULL, "castChaseSpirits(): no victim");

  int level = caster->getSkillLevel(SPELL_CHASE_SPIRIT);
  if (caster->isNotPowerful(victim, level, SPELL_CHASE_SPIRIT, SILENT_NO)) {
    return 0;
  }

  int ret = chaseSpirits(caster, victim, level,
    caster->getSkillValue(SPELL_CHASE_SPIRIT));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster != victim) {
      act("You chase away the evil spirits within $N...", FALSE, caster, NULL,
        victim, TO_CHAR);
      act("$n chases away the evil spirits within you...", FALSE, caster, NULL,
        victim, TO_VICT);
      act("$n chases away the evil spirits within $N...", FALSE, caster, NULL,
        victim, TO_NOTVICT);
    } else {
      act("The loa chases away the evil within you...", FALSE, caster, NULL, 0,
        TO_CHAR);
      act("$n's face glows with much relief.", FALSE, caster, NULL, 0, TO_ROOM);
    }
    int rc = genericChaseSpirits(caster, victim, level, caster->isImmortal());
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  return TRUE;
}

// returns DELETE_VICT (vict)
int genericChaseSpirits(TBeing* caster, TBeing* victim, int,
  immortalTypeT immortal, safeTypeT safe) {
  mud_assert(victim != NULL, "genericChaseSpirits(): no victim");

  TMonster* tvm = dynamic_cast<TMonster*>(victim);
  spellNumT spell;
  int rc;

  struct chaseStruct {
      spellNumT spell;
      bool aggressive_act;
      bool needs_saving_throw;
      bool death_time_only;  // this is primarily for prayers
  };

  chaseStruct chaseArray[] = {
    // air disc
    {SPELL_FEATHERY_DESCENT, false, true, false},
    {SPELL_FLY, true, true, false},
    {SPELL_ANTIGRAVITY, true, true, false},
    {SPELL_LEVITATE, true, true, false},
    {SPELL_FALCON_WINGS, true, true, false},
    {SPELL_PROTECTION_FROM_AIR, true, true, false},
    // alchemy
    {SPELL_DETECT_MAGIC, false, true, false},
    // earth
    {SPELL_STONE_SKIN, false, true, false},
    {SPELL_TRAIL_SEEK, false, true, false},
    {SPELL_PROTECTION_FROM_EARTH, true, true, false},
    // fire
    {SPELL_FAERIE_FIRE, false, false, false},
    {SPELL_FLAMING_FLESH, false, true, false},
    {SPELL_INFRAVISION, false, true, false},
    {SPELL_PROTECTION_FROM_FIRE, true, true, false},
    // sorcery
    {SPELL_SORCERERS_GLOBE, true, true, false},
    {SPELL_BIND, false, false, false},
    {SPELL_PROTECTION_FROM_ENERGY, true, true, false},
    // spirit
    {SPELL_SILENCE, false, true, false},
    {SPELL_ENSORCER, false, false, false},
    {SPELL_INVISIBILITY, false, true, false},
    {SPELL_STEALTH, false, true, false},
    {SPELL_ACCELERATE, true, true, false},
    {SPELL_HASTE, true, true, false},
    {SPELL_CALM, false, true, false},
    {SPELL_SENSE_LIFE, false, true, false},
    {SPELL_DETECT_INVISIBLE, false, true, false},
    {SPELL_TRUE_SIGHT, false, true, false},
    {SPELL_FEAR, false, false, false},
    {SPELL_SLUMBER, false, false, false},
    // water
    {SPELL_ICY_GRIP, false, false, false},
    {SPELL_GILLS_OF_FLESH, true, true, false},
    {SPELL_PROTECTION_FROM_WATER, true, true, false},
    {SPELL_PLASMA_MIRROR, true, true, false},
    {SPELL_GARMULS_TAIL, false, true, false},

    // cleric prayers - these should be death-time only stuff
    {SPELL_SANCTUARY, true, true, true},
    {SPELL_ARMOR, true, true, true},
    {SPELL_ARMOR_DEIKHAN, true, true, true},
    {SPELL_BLESS, true, true, true},
    {SPELL_BLESS_DEIKHAN, true, true, true},
    {SPELL_BLINDNESS, false, false, true},
    {SPELL_PARALYZE, false, false, true},
    {SPELL_POISON, false, false, true},
    {SPELL_POISON_DEIKHAN, false, false, true},
    {SPELL_CURSE, false, false, true},
    {SPELL_CURSE_DEIKHAN, false, false, true},
    // shaman stuff
    {SPELL_STUPIDITY, true, true, false},
    {SPELL_CELERITE, true, true, false},
    {SPELL_LEGBA, true, true, false},
    {SPELL_DJALLA, true, true, false},
    {SPELL_SENSE_LIFE_SHAMAN, true, true, false},
    {SPELL_DETECT_SHADOW, true, true, false},
    {SPELL_INTIMIDATE, true, true, false},
    {SPELL_CHEVAL, true, true, false},
    {SPELL_HYPNOSIS, true, true, false},
    {SPELL_CLARITY, true, true, false},
    {SPELL_AQUALUNG, true, true, false},
    {SPELL_THORNFLESH, true, true, false},
    {SPELL_SHIELD_OF_MISTS, true, true, false},
    {SPELL_CONTROL_UNDEAD, true, true, false},
    {SPELL_RESURRECTION, true, true, false},
    {SPELL_DANCING_BONES, true, true, false},
    {SPELL_VOODOO, true, true, false},

#if 0
    // these effects are on mobs
    // death-time-only is silly to check for
    { SPELL_STICKS_TO_SNAKES, false, false, true },
    { SPELL_LIVING_VINES, false, false, true },
    { SPELL_PLAGUE_LOCUSTS, false, false, true },
#endif

#if 0
    // not yet implemented
    { SPELL_DETECT_POISON, false, true, false },
    { SPELL_DETECT_POISON_DEIKHAN, false, true, false },
#endif

#if 0
    // skills that should be usable again if they die
    // these use to have a check for ARENA-death
    // not sure how to do this in new setup, so commented out for time being
    { SKILL_TRANSFIX, false, false, true },
    { SKILL_CHI, false, false, true },
    { SKILL_DOORBASH, false, false, true },
    { SKILL_TRANSFORM_LIMB, false, false, true },
    { SKILL_BARKSKIN, false, false, true },
    { SKILL_TRACK, false, false, true },
    { SKILL_CONCEALMENT, false, false, true },
    { SKILL_FORAGE, false, false, true },
    { SKILL_SEEKWATER, false, false, true },
    { SKILL_ENCAMP, false, false, true },
    { SKILL_DIVINATION, false, false, true },
    { SKILL_SPY, false, false, true },
    { SKILL_DISGUISE, false, false, true },
    { SKILL_BERSERK, false, false, true },
    { SKILL_DEATHSTROKE, false, false, true },
    { SKILL_DOORBASH, false, false, true },
    { SKILL_QUIV_PALM, false, false, true },
#endif

    {TYPE_UNDEFINED, false, false, false}  // this is final terminator
    // spell, aggressive, saving throw, death_time_only
  };

  int iter;
  for (iter = 0; chaseArray[iter].spell != TYPE_UNDEFINED; iter++) {
    spell = chaseArray[iter].spell;

    // check if they have the spell
    // should decay if !caster (death-time) or if set to decay all the time
    if ((!caster || !chaseArray[iter].death_time_only) &&
        victim->affectedBySpell(spell)) {
      // immortals should always succeed
      // make a save otherwise
      // there is assumption that !caster (death-time) will have immortal=true
      if (immortal || !chaseArray[iter].needs_saving_throw ||
          !victim->isLucky(caster->spellLuckModifier(SPELL_CHASE_SPIRIT))) {
        rc = victim->spellWearOff(spell, safe);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_VICT;
        victim->affectFrom(spell);
      }
      // aggressive Act
      if (caster && !victim->fight() && tvm) {
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
      }
    }
  }

  if (caster && victim->isAffected(AFF_SANCTUARY)) {
    if (immortal ||
        !victim->isLucky(caster->spellLuckModifier(SPELL_CHASE_SPIRIT))) {
      REMOVE_BIT(victim->specials.affectedBy, AFF_SANCTUARY);
      victim->sendTo(
        "You feel more vulnerable as your white aura slowly fades.\n\r");
      act("The white glow around $n's body fades.", FALSE, victim, NULL, NULL,
        TO_ROOM);
    }
    // aggressive Act
    if (caster && !victim->fight() && tvm) {
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
    }
  }
  return FALSE;
}
