//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD: disc_armadillo.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_armadillo.h"


static struct PolyType ShapeShiftList[] =
{
  {"gopher"   , 30,   1, 25401, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"deer"     , 31,   1, 14105, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"wolf"     , 35,  10,  3400, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"snake"    , 37,  20,  3412, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"moose"    , 39,  30, 10200, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"dolphin"  , 40,  45, 12432, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"bear"     , 42,  75,  3403, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"crow"     , 44,  70, 14350, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"shark"    , 40,  60, 12413, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"hawk"     , 48, 100, 14440, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"saberfish", 49,  80,  5503, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"spider"   , 49,  80,  7717, DISC_SHAMAN_ARMADILLO, RACE_NORACE},
  {"\n"        , -1,  -1,    -1, DISC_SHAMAN_ARMADILLO, RACE_NORACE}
};

int shapeShift(TBeing *caster, int level, byte bKnown)
{
  int i, ret = 0;
  bool nameFound = FALSE;
  bool found = FALSE;
  TBeing *mob;
  const char * buffer;
  affectedData aff;
  affectedData aff2;

  buffer = caster->spelltask->orig_arg;
 
  discNumT das = getDisciplineNumber(SPELL_SHAPESHIFT, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, "Bad disc for SPELL_SHAPESHIFT");
    return SPELL_FAIL;
  }
  for (i = 0; *ShapeShiftList[i].name != '\n'; i++) {
    if (is_abbrev(buffer, ShapeShiftList[i].name)) {
      nameFound = TRUE;
    } else {
      continue;
    }
    if ((caster->getDiscipline(das)->getLearnedness() >= ShapeShiftList[i].learning) && (level > 25))
      break;
  }

  if (*ShapeShiftList[i].name == '\n') {
    if (nameFound) {
      caster->sendTo("You are not powerful enough yet to change into such a creature.\n\r");
    } else {
      caster->sendTo("Couldn't find any of those.\n\r");
    }
    return SPELL_FAIL;
  }
  if (!(mob = read_mobile(ShapeShiftList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't summon an image of that creature.\n\r");
    return SPELL_FAIL;
  }
  thing_to_room(mob,ROOM_VOID);   // just so if extracted it isn't in NOWHERE 

  // Check to make sure that there is no snooping going on. 
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->nothingHappens();
    vlogf(LOG_BUG,"PC tried to shapeshift while being snooped");
    delete mob;
    mob = NULL;
    return SPELL_FAIL;
  }
  if (caster->desc->original) {
    // implies they are switched, while already switched (as x switch)
    caster->sendTo("You already seem to be switched.\n\r");
    delete mob;
    mob = NULL;
    return SPELL_FAIL;
  }
  if (caster->desc->snoop.snoop_by)
    caster->desc->snoop.snoop_by->doSnoop(caster->desc->snoop.snoop_by->name);

  // first add the attempt -- used to regulate attempts
  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = (1 + (level/15)) * UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = SPELL_SHAPESHIFT;
  caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

  if (bSuccess(caster, bKnown, SPELL_SHAPESHIFT)) {
    switch (critSuccess(caster, SPELL_SHAPESHIFT)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_SHAPESHIFT);
        ret = SPELL_CRIT_SUCCESS;
      case CRIT_S_NONE:
        break;
   }

    --(*mob);
    *caster->roomp += *mob;
    SwitchStuff(caster, mob);

    act("$n's flesh melts and flows into the shape of $N.", TRUE, caster, NULL, mob, TO_NOTVICT);
    for (i=MIN_WEAR;i < MAX_WEAR;i++) {
      if (caster->equipment[i]) {
        found = TRUE;
        break;
      }
    }
    if (found) {
      act("Your equipment falls from your body as your flesh turns liquid.",
               TRUE, caster, NULL, mob, TO_CHAR);
      act("Slowly you take on the shape of $N.", 
               TRUE, caster, NULL, mob, TO_CHAR);
    } else {
      act("Your flesh turns liquid.", TRUE, caster, NULL, mob, TO_CHAR);
      act("Slowly your flesh melts and you take on the shape of $N.", TRUE, caster, NULL, mob, TO_CHAR);
    }
  
    --(*caster);
    thing_to_room(caster, ROOM_POLY_STORAGE);

    // stop following whoever you are following.. 
    if (caster->master)
      caster->stopFollower(TRUE);

    // switch caster into mobile 
    caster->desc->character = mob;
    caster->desc->original = dynamic_cast<TPerson *>(caster);

#if 0
    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.location = APPLY_NONE;
    aff2.duration = duration + ((2 + (level/5)) * UPDATES_PER_MUDHOUR);
    aff2.bitvector = 0;
    aff2.modifier = SPELL_SHAPESHIFT;
    mob->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
#endif

    mob->desc = caster->desc;
    caster->desc = NULL;
    caster->polyed = POLY_TYPE_POLYMORPH;

    SET_BIT(mob->specials.act, ACT_POLYSELF);
    SET_BIT(mob->specials.act, ACT_NICE_THIEF);
    SET_BIT(mob->specials.act, ACT_SENTINEL);
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
    REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
    REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

    mob->setMana(min((mob->getMana() - 15), 85));
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int shapeShift(TBeing *caster, const char * buffer)
{
  taskDiffT diff;
  int level;
  int i;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SPELL_SHAPESHIFT))
{
    act("You are not prepared to try to shapeshift yourself again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_SHAPESHIFT);
  for (i = 0; *ShapeShiftList[i].name != '\n'; i++) {
    if (level < ShapeShiftList[i].level)
      continue;
    if (is_abbrev(buffer, ShapeShiftList[i].name))
      break;
  }

  if (*ShapeShiftList[i].name == '\n') {
    caster->sendTo("Couldn't find any of those.\n\r");
    return SPELL_FAIL;
  }

  // Check to make sure that there is no snooping going on.
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->nothingHappens();
    vlogf(LOG_BUG,"PC tried to shapeshift while being snooped");
    return SPELL_FAIL;
  }

  if (caster->desc->original) {
    // implies they are switched, while already switched (as x switch)
    caster->sendTo("You already seem to be switched.\n\r");
    return SPELL_FAIL;
  }
  if (caster->desc->snoop.snoop_by)
    caster->desc->snoop.snoop_by->doSnoop(caster->desc->snoop.snoop_by->name);

  if (!bPassMageChecks(caster, SPELL_SHAPESHIFT, caster))
    return FALSE;

  lag_t rounds = discArray[SPELL_SHAPESHIFT]->lag;
  diff = discArray[SPELL_SHAPESHIFT]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_SHAPESHIFT, diff, 1,
buffer, rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castShapeShift(TBeing *caster)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_SHAPESHIFT);
  int bKnown = caster->getSkillValue(SPELL_SHAPESHIFT);

  if ((ret=shapeShift(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else 
    caster->nothingHappens();
  return TRUE;
}

int sticksToSnakes(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  TMonster *snake;
  affectedData aff; 
  int rc, mobile;
  followData *k, *n;

  if (level < 26) {
     mobile = MOB_SNAKES25;
  } else if (level < 31) {
     mobile = MOB_SNAKES30;
  } else if (level < 36) {
     mobile = MOB_SNAKES35;
  } else if (level < 41) {
     mobile = MOB_SNAKES40;
  } else {
     mobile = MOB_SNAKES50;
  }


  if(!(snake = read_mobile(mobile, VIRTUAL))) {
    vlogf(LOG_BUG, "Spell STICKS_TO_SNAKES unable to load mob...");
    caster->sendTo("Unable to create the snake, please report this.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if ((caster->followers) && (caster->GetMaxLevel() < GOD_LEVEL1))  {
      for (k = caster->followers; k; k = n) {
        n = k->next;
        if (!strcmp(k->follower->getName(), snake->getName())) {
          act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
          caster->sendTo("You would have to be immortal to summon another snake!\n\r");
          delete snake;
          snake=NULL;
          return SPELL_FAIL;
        }
      }
   }

  float lvl = max(1.0, (float) level/2.0);
  snake->setDamPrecision(20);

  snake->setExp(0);
  snake->setMoney(0);
  snake->setHitroll(0);

  *caster->roomp += *snake;
  act("A strange yellow mist appears and suddenly changes into $n!", TRUE, snake, NULL, caster, TO_ROOM);
  act("A strange yellow mist appears and suddenly changes into $n!", TRUE, snake, NULL, caster, TO_CHAR);

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_STICKS_TO_SNAKES)) {
    switch(critSuccess(caster, SPELL_STICKS_TO_SNAKES)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_STICKS_TO_SNAKES);
        act("The snake appears to be particularly strong and particularly angry!", TRUE, caster, 0, snake, TO_ROOM);
        act("The snake appears to be particularly strong and particularly angry!", TRUE, caster, 0, snake, TO_CHAR);
        lvl *= 2;
        break;
      default:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_STICKS_TO_SNAKES))) {
          SV(SPELL_STICKS_TO_SNAKES);
          lvl /=2;
        }
        break;
    }

    if (!caster->fight()) {
      if (caster->reconcileDamage(victim, 0, SPELL_STICKS_TO_SNAKES) == -1)
        return SPELL_SUCCESS + VICTIM_DEAD;
    }

    snake->setLevel(WARRIOR_LEVEL_IND, lvl);
    snake->setHPLevel(lvl);
    snake->setHPFromHPLevel();
    snake->setACLevel(lvl);
    snake->setACFromACLevel();
    snake->setDamLevel(lvl);

    if (!IS_SET(snake->specials.act, ACT_SENTINEL))
      SET_BIT(snake->specials.act, ACT_SENTINEL);

    if (!snake->master)
      caster->addFollower(snake);

    act("After following $n, the coiled snake bares its fangs and springs at $N!", TRUE, caster, 0, victim, TO_NOTVICT);
    act("After following you, the coiled snake bares its fangs and springs at $N!", TRUE, caster, 0, victim, TO_CHAR);
    act("After following $n, the snake bares its fangs and springs to attack you!", TRUE, caster, 0, victim, TO_VICT);


    aff.type = SPELL_STICKS_TO_SNAKES;
    aff.level = level;
    aff.duration = 32000;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = 0;
    snake->affectTo(&aff);

    if (snake->reconcileDamage(victim, 0, SPELL_STICKS_TO_SNAKES) == -1)
       return SPELL_SUCCESS + VICTIM_DEAD;

    return SPELL_SUCCESS;

  } else {
    snake->setLevel(WARRIOR_LEVEL_IND, lvl);
    snake->setHPLevel(lvl);
    snake->setHPFromHPLevel();
    snake->setACLevel(lvl);
    snake->setACFromACLevel();
    snake->setDamLevel(lvl);

    switch (critFail(caster, SPELL_STICKS_TO_SNAKES)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_STICKS_TO_SNAKES);
        act("$n loses control of the magic $e has unleashed!", TRUE, caster, 0, snake, TO_ROOM);
        act("You lose control of the magic you have unleashed!", TRUE, caster, 0, snake, TO_CHAR);

        rc = snake->hit(caster);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        }
        if (rc == DELETE_THIS) {
          delete snake;
          snake = NULL;
        }
        return SPELL_CRIT_FAIL;
      default:
        break;
    }
    caster->sendTo("You don't seem to have control of the snake.\n\r");
    act("The snake seems to have no interest in $n!", TRUE, caster, 0, snake, TO_ROOM);
    caster->sendTo("The snake reverts back into sticks.\n\r");
    act("$N reverts back into the sticks from which it came.", TRUE, caster, 0, snake, TO_ROOM);
    *snake->roomp += *read_object(OBJ_INERT_STICK, VIRTUAL);
    delete snake;
    snake = NULL;
    return SPELL_FAIL;
  }
}


int sticksToSnakes(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc=0;

  ret=sticksToSnakes(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
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

int sticksToSnakes(TBeing * caster, TBeing * victim)
{
  int level,ret;
  int rc = 0;

  if (!bPassMageChecks(caster, SPELL_STICKS_TO_SNAKES, victim)) 
    return FALSE;

  level = caster->getSkillLevel(SPELL_STICKS_TO_SNAKES);
  int bKnown = caster->getSkillValue(SPELL_STICKS_TO_SNAKES);

  ret=sticksToSnakes(caster, victim, level, bKnown);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;

}

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

 
int stormySkies(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  int rc;

  if (caster->isNotPowerful(victim, level, SPELL_STORMY_SKIES, SILENT_NO)) {
    return SPELL_FAIL;
  }

  if (!((victim->roomp->getWeather() == WEATHER_RAINY) || 
     (victim->roomp->getWeather() == WEATHER_LIGHTNING) ||
     (victim->roomp->getWeather() == WEATHER_SNOWY))) {
    caster->sendTo("You fail to call upon the weather to aid you!\n\r");
    if (!victim->outside())
      caster->sendTo("You have to be outside to cast this spell!\n\r");
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_STORMY_SKIES]->alignMod);
  
  int dam = caster->getSkillDam(victim, SPELL_STORMY_SKIES, caster->getSkillLevel(SPELL_STORMY_SKIES), caster->getAdvLearning(SPELL_STORMY_SKIES));
 
  if ((victim->roomp->getWeather() == WEATHER_RAINY) ||
     (victim->roomp->getWeather() == WEATHER_LIGHTNING)) {
    if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_STORMY_SKIES)) {
      if (victim->isLucky(caster->spellLuckModifier(SPELL_STORMY_SKIES)))
        dam /= 2; // half damage
 
      if (critSuccess(caster, SPELL_STORMY_SKIES)) {
        CS(SPELL_STORMY_SKIES);
        dam *= 2;
      }
      act("$n summons a lightning bolt from the stormy skies and causes it to hit $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("You summon a lightning bolt from the stormy skies and cause it to hit $N!", FALSE, caster, NULL, victim, TO_CHAR);
      act("$n summons a lightning bolt from the stormy skies and causes it to hit you!", FALSE, caster, NULL, victim, TO_VICT);
      if (caster->reconcileDamage(victim, dam, SPELL_STORMY_SKIES) == -1)
        return SPELL_SUCCESS + VICTIM_DEAD;
      rc = victim->lightningEngulfed();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return SPELL_SUCCESS + VICTIM_DEAD;
      return SPELL_SUCCESS;
    } else {
      switch (critFail(caster, SPELL_STORMY_SKIES)) {
        case CRIT_F_HITSELF:
          CF(SPELL_STORMY_SKIES);
          act("$n summons lightning from the stormy skies, bringing down a bolt upon $mself!", FALSE, caster, NULL, NULL, TO_ROOM);
          act("You summon lightning from the stormy skies, bringing down a bolt upon yourself!", FALSE, caster, NULL, NULL, TO_CHAR);
          act("That lightning bolt was intended for you!", FALSE, caster, NULL, victim, TO_VICT);
          if (caster->reconcileDamage(caster, dam,SPELL_STORMY_SKIES) == -1)
            return SPELL_CRIT_FAIL + CASTER_DEAD;
          return SPELL_CRIT_FAIL;
          break;
        case CRIT_F_HITOTHER:
        case CRIT_F_NONE:
          break;
      }
      caster->sendTo("Nothing seems to happen.\n\r");
      return SPELL_FAIL;
    }
  } else if (victim->roomp->getWeather() == WEATHER_SNOWY) { 
    if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_STORMY_SKIES)) {
      if (victim->isLucky(caster->spellLuckModifier(SPELL_STORMY_SKIES)))
        dam /= 2;         // half damage
 
      if (critSuccess(caster, SPELL_STORMY_SKIES)) {
        CS(SPELL_STORMY_SKIES);
        dam *= 2;
      }
      act("$n summons hail from the snowy sky and guides it down upon $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("You summon hail from the snowy sky and guide it down upon $N!", FALSE, caster, NULL, victim, TO_CHAR);
      act("$n summons hail from the snowy sky and guides it down upon you!", FALSE, caster, NULL, victim, TO_VICT);
      if (caster->reconcileDamage(victim, dam, SPELL_STORMY_SKIES) == -1)
        return SPELL_SUCCESS + VICTIM_DEAD;
      return SPELL_SUCCESS;
    } else {
      switch (critFail(caster, SPELL_STORMY_SKIES)) {
        case CRIT_F_HITSELF:
          CF(SPELL_STORMY_SKIES);
          act("$n summons hail from the snowy sky and guids it down upon $mself!", FALSE, caster, NULL, NULL, TO_NOTVICT);
          act("You summon hail from the snowy sky and guid it down upon yourself!", FALSE, caster, NULL, NULL, TO_CHAR);
          act("That hail storm was intended for you!", FALSE, caster, NULL, victim, TO_VICT);
          if (caster->reconcileDamage(caster, dam,SPELL_STORMY_SKIES) == -1)
            return SPELL_CRIT_FAIL + CASTER_DEAD;
          return SPELL_CRIT_FAIL;
          break;
        case CRIT_F_HITOTHER:
        case CRIT_F_NONE:
          break;
      }
      caster->sendTo("Nothing seems to happen.\n\r");
      return SPELL_FAIL;
    }
  } else {
    caster->sendTo("The weather here isn't quite right.\n\r");
    return SPELL_FAIL;
  }
}

int stormySkies(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret;
  int rc = 0;

  ret=stormySkies(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int stormySkies(TBeing * caster, TBeing * victim)
{
  int ret,level;
 
  if (!bPassMageChecks(caster, SPELL_STORMY_SKIES, victim))
    return FALSE;
 
  level = caster->getSkillLevel(SPELL_STORMY_SKIES);
  int bKnown = caster->getSkillValue(SPELL_STORMY_SKIES);
 
  if ((ret=stormySkies(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  return FALSE;
}
 
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

    for (t = caster->roomp->stuff; t; t = t2) {
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
 
