/*******************************************************************
 *                                                                 *
 *                                                                 *
 *   disc_shaman_armadillo.cc          Shaman Discipline Disc      *
 *                                                                 *
 *   SneezyMUD Development - All Rights Reserved                   *
 *                                                                 *
 *******************************************************************/
 
#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_armadillo.h"
#include "obj_magic_item.h"


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
  int bKnown= getSkillValue(SPELL_EARTHMAW);

  int dam = getSkillDam(victim, SPELL_EARTHMAW, lev, getAdvLearning(SPELL_EARTHMAW));


  if (!useComponent(findComponent(SPELL_EARTHMAW), this, CHECK_ONLY_NO))
    return FALSE;

  addToWait((int)combatRound(discArray[SPELL_EARTHMAW]->lag));
  reconcileHurt(victim,discArray[SPELL_EARTHMAW]->alignMod);
  
  if (bSuccess(bKnown, SPELL_EARTHMAW)) {
    
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


// THORNFLESH

int thornflesh(TBeing *caster)
{
  if (caster->affectedBySpell(SPELL_THORNFLESH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  if (!bPassShamanChecks(caster, SPELL_THORNFLESH, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_THORNFLESH]->lag;
  taskDiffT diff = discArray[SPELL_THORNFLESH]->task;

  start_cast(caster, caster, NULL, caster->roomp, SPELL_THORNFLESH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castThornflesh(TBeing *caster)
{
  int ret,level;

  if (caster && caster->affectedBySpell(SPELL_THORNFLESH)) {
    act("You flesh is armored well enough.", FALSE, caster, NULL, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_THORNFLESH);
  int bKnown = caster->getSkillValue(SPELL_THORNFLESH);

  ret=thornflesh(caster,level,bKnown);

  return ret;
}

int thornflesh(TBeing *caster, int level, byte bKnown)
{
  affectedData aff;

  if (caster->affectedBySpell(SPELL_THORNFLESH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  aff.type = SPELL_THORNFLESH;
  aff.duration = max(min(level/10, 5), 1) * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = level;

  if (caster->bSuccess(bKnown, SPELL_THORNFLESH)) {
    switch (critSuccess(caster, SPELL_THORNFLESH)) {
      case CRIT_S_KILL:
        CS(SPELL_THORNFLESH);
        aff.duration *= 2;
        act("LARGE thorns emerge from your body!", FALSE, caster, 0, 0, TO_CHAR, ANSI_ORANGE);
        act("LARGE thorns emerge from $n's body!", FALSE, caster, 0, 0, TO_ROOM, ANSI_ORANGE);
        break;
      default:
        act("Thorns emerge from your body!", FALSE, caster, 0, 0, TO_CHAR, ANSI_ORANGE);
        act("Thorns emerge from $n's body!", FALSE, caster, 0, 0, TO_ROOM, ANSI_ORANGE);
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

static bool canBeLunged(TBeing *caster, TBeing *victim)
{
  if (victim->isAffected(AFF_WATERBREATH) &&
      !(victim->affectedBySpell(SPELL_GILLS_OF_FLESH) ||
      !(victim->affectedBySpell(SPELL_AQUALUNG) ||
       victim->affectedBySpell(SPELL_BREATH_OF_SARAHAGE)))) {
    if (caster != victim)
      act("$N already has the ability to breathe underwater.", FALSE, caster, NULL, victim, TO_CHAR);
    else
      act("You already have the ability to breathe underwater.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return true;
  }
  return false;
}

int aqualung(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;

  if (canBeLunged(caster, victim))
    return FALSE;

  if (caster->bSuccess(bKnown, SPELL_AQUALUNG)) {

    caster->reconcileHelp(victim,discArray[SPELL_AQUALUNG]->alignMod);
    aff.type = SPELL_AQUALUNG;
    aff.level = level;
    aff.duration = 8 * UPDATES_PER_MUDHOUR;
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

    act("A transparent globe surrounds your head!", TRUE, victim, NULL, NULL, TO_CHAR, ANSI_BLUE);
    act("A transparent globe surrounds $N's head!", TRUE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
    if (victim != caster)
      act("You bestow upon $N the ability to breathe water!", TRUE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void aqualung(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  aqualung(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int aqualung(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

  if (canBeLunged(caster, victim))
    return FALSE;

  if (!bPassShamanChecks(caster, SPELL_AQUALUNG, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_AQUALUNG]->lag;
  diff = discArray[SPELL_AQUALUNG]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_AQUALUNG, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castAqualung(TBeing * caster, TBeing * victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_AQUALUNG);
  int bKnown= caster->getSkillValue(SPELL_AQUALUNG);

  if ((ret=aqualung(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

// END AQUALUNG
// SHADOW WALK
int shadowWalk(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

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
    aff.duration = 18 * UPDATES_PER_MUDHOUR;
    aff.modifier = -40;
    aff.location = APPLY_ARMOR;
    aff.bitvector = AFF_SHADOW_WALK;

    switch (critSuccess(caster, SPELL_SHADOW_WALK)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SHADOW_WALK);
        aff.duration = 28 * UPDATES_PER_MUDHOUR;
        aff.modifier = -60;
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

void shadowWalk(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret=shadowWalk(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

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

int shadowWalk(TBeing *caster, TBeing *victim)
{
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

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SHADOW_WALK, diff, 1, "", 
rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castShadowWalk(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_SHADOW_WALK);
  int bKnown = caster->getSkillValue(SPELL_SHADOW_WALK);

  int ret=shadowWalk(caster,victim,level,bKnown);
  if (ret== SPELL_SUCCESS) {
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

int celerite(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;
  caster->reconcileHelp(victim, discArray[SPELL_CELERITE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CELERITE)) {
    aff.type = SPELL_CELERITE;
    aff.level = level;
    aff.duration = (aff.level / 3) * UPDATES_PER_MUDHOUR;
    aff.modifier = -100;
    aff.location = APPLY_ARMOR;
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

    act("$N moves more easily.",
        FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_YELLOW_BOLD);
    act("The power of the loa takes dominion inside of you!", 
        FALSE, victim, NULL, NULL, TO_CHAR, ANSI_YELLOW_BOLD);
    return SPELL_SUCCESS;
  } else {
    act("The loa ignore your unfaithful request!",
        FALSE, caster, NULL, victim, TO_CHAR, ANSI_YELLOW);
    caster->addToLifeforce(-10);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
}

int celerite(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassShamanChecks(caster, SPELL_CELERITE, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_CELERITE]->lag;
     diff = discArray[SPELL_CELERITE]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_CELERITE, diff, 1, "", 
rounds, caster->in_room, 0, 0,TRUE, 0);
       return TRUE;
}

void celerite(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  celerite(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int castCelerite(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_CELERITE);
  int bKnown = caster->getSkillValue(SPELL_CELERITE);

  if ((ret=celerite(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

