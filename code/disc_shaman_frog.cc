/*******************************************************************
 *                                                                 *
 *                                                                 *
 *   disc_shaman_frog.cc               Shaman Discipline Disc      *
 *                                                                 *
 *   SneezyMUD Development - All Rights Reserved                   *
 *                                                                 *
 *******************************************************************/

#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_frog.h"


// STORMY SKIES
 
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
      vlogf(LOG_JESUS, "Stormy Skies Damage: %d", dam);
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
      vlogf(LOG_JESUS, "Stormy Skies Damage: %d", dam);
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

int stormySkies(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

   if (!bPassShamanChecks(caster, SPELL_STORMY_SKIES, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_STORMY_SKIES]->lag;
    diff = discArray[SPELL_STORMY_SKIES]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_STORMY_SKIES, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

      return TRUE;
}

int castStormySkies(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_STORMY_SKIES);
  int bKnown = caster->getSkillValue(SPELL_STORMY_SKIES);

  ret=stormySkies(caster,victim,level,bKnown);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int stormySkies(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=stormySkies(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// END STORMY SKIES
// AQUATIC BLAST

int aquaticBlast(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  int rc;
  TThing *t;

  level = min(level, 10);

  int dam = caster->getSkillDam(victim, SPELL_AQUATIC_BLAST, level, adv_learn);

  if (bSuccess(caster, bKnown, SPELL_AQUATIC_BLAST)) {
    caster->reconcileHurt(victim,discArray[SPELL_AQUATIC_BLAST]->alignMod);

    if ((critSuccess(caster, SPELL_AQUATIC_BLAST) ||
        critSuccess(caster, SPELL_AQUATIC_BLAST) ||
        critSuccess(caster,SPELL_AQUATIC_BLAST)) &&
        !caster->isNotPowerful(victim, level, SPELL_AQUATIC_BLAST, SILENT_YES)) {
      CS(SPELL_AQUATIC_BLAST);
      dam *= 2;
      act("A HUGE BLAST of water smacks into $N knocking $M over!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a HUGE BLAST of water at you, knocking you down!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a HUGE BLAST of water in $N's direction, knocking $M over!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
      victim->dropPool(50, LIQ_WATER);

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
    } else if (victim->isLucky(caster->spellLuckModifier(SPELL_AQUATIC_BLAST))) {

      act("A stream of water smacks into $N. Must have been a dud.",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a stream of water in your direction. Must have been a dud.",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a stream of water in $N's direction. Must have been a dud.",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
      victim->dropPool(10, LIQ_WATER);

      SV(SPELL_AQUATIC_BLAST);
      dam /= 2;
    } else {
      act("A forceful blast of water smacks into $N!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a forceful blast of water in your direction!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a forceful blast of water in $N's direction!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
      victim->dropPool(25, LIQ_WATER);
    }
    if (caster->reconcileDamage(victim, dam, SPELL_AQUATIC_BLAST) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
    vlogf(LOG_JESUS, "Aquatic Blast Damage: %d", dam);
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("$n just tried to attack you.", FALSE, caster, 0, victim, TO_VICT, ANSI_BLUE);
    if (critFail(caster, SPELL_AQUATIC_BLAST) == CRIT_F_HITSELF) {
      CF(SPELL_AQUATIC_BLAST);
      act("You summon the powers of the frog, but it leaves you all wet!",
           FALSE, caster, NULL, 0, TO_CHAR, ANSI_BLUE);
      act("$n does something stupid makes $mself all wet!",
           FALSE, caster, NULL, 0, TO_ROOM, ANSI_BLUE);
      caster->dropPool(10, LIQ_WATER);
      if (caster->reconcileDamage(caster, dam, SPELL_AQUATIC_BLAST) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int aquaticBlast(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

   if (!bPassShamanChecks(caster, SPELL_AQUATIC_BLAST, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_AQUATIC_BLAST]->lag;
    diff = discArray[SPELL_AQUATIC_BLAST]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_AQUATIC_BLAST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

      return TRUE;
}

int castAquaticBlast(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_AQUATIC_BLAST);
  int bKnown = caster->getSkillValue(SPELL_AQUATIC_BLAST);

  ret=aquaticBlast(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_AQUATIC_BLAST));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int aquaticBlast(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=aquaticBlast(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// END AQUATIC BLAST
// SHAPESHIFT

static struct PolyType ShapeShiftList[] =
{
  {"chicken"   , 30,   1, 1213, DISC_SHAMAN_FROG, RACE_NORACE},
  {"horsefly"     , 31,   1, 15420, DISC_SHAMAN_FROG, RACE_NORACE},
  {"slug"     , 35,  10,  5126, DISC_SHAMAN_FROG, RACE_NORACE},
  {"snake"    , 37,  20,  3412, DISC_SHAMAN_FROG, RACE_NORACE},
  {"dolphin"  , 40,  45, 12432, DISC_SHAMAN_FROG, RACE_NORACE},
  {"bear"     , 42,  75,  3403, DISC_SHAMAN_FROG, RACE_NORACE},
  {"crow"     , 44,  70, 14350, DISC_SHAMAN_FROG, RACE_NORACE},
  {"hawk"     , 48, 100, 14440, DISC_SHAMAN_FROG, RACE_NORACE},
  {"spider"   , 49,  80,  7717, DISC_SHAMAN_FROG, RACE_NORACE},
  {"\n"        , -1,  -1,    -1, DISC_SHAMAN_FROG, RACE_NORACE}
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

    mob->setLifeforce(min((mob->getLifeforce() - 15), 85));
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

  if (!bPassShamanChecks(caster, SPELL_SHAPESHIFT, caster))
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

// END SHAPESHIFT

int deathWave(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  char buf[256];
  string bBuf;

  if (caster->isNotPowerful(victim, level, SPELL_DEATHWAVE, SILENT_NO))
    return SPELL_FAIL;

  level = min(level, 50);

  int dam = caster->getSkillDam(victim, SPELL_DEATHWAVE, level, adv_learn);
  int beams = (dam / 3) + ::number(0, (caster->GetMaxLevel() / 10));
  beams = max(beams, 1);

  caster->reconcileHurt(victim, discArray[SPELL_DEATHWAVE]->alignMod);

  if (bSuccess(caster, bKnown,SPELL_DEATHWAVE)) {
    switch (critSuccess(caster, SPELL_DEATHWAVE)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_DEATHWAVE);
        dam *= 2;
        beams *= 2;
        sprintf(buf, "%d", beams);
        bBuf = buf;
        bBuf += " intense black energy beam";
        if (beams != 1)
          bBuf += "s expel";
        else
          bBuf += " expels";

        sprintf(buf, "%s from $n's hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's hands and enter your body distorting your soul!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_DEATHWAVE);
        dam *= 3;
        beams *=3;

        sprintf(buf, "%d", beams);
        bBuf = buf;
        bBuf += " DEADLY black energy beam";
        if (beams != 1)
          bBuf += "s expel";
        else
          bBuf += " expels";

        sprintf(buf, "%s from $n's hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's hands and enter your body distorting your soul!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        sprintf(buf, "%d", beams);
        bBuf = buf;
        bBuf += " black energy beam";
        if (beams != 1)
          bBuf += "s expel";
        else
          bBuf += " expels";

        sprintf(buf, "%s from $n's hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your hands and enter $N's body!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's hands and enter your body distorting your soul!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        if (victim->isLucky(caster->spellLuckModifier(SPELL_DEATHWAVE))) {
          SV(SPELL_DEATHWAVE);
          dam /= 2;
        }
    }

    if (caster->reconcileDamage(victim, dam, SPELL_DEATHWAVE) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;

    return SPELL_SUCCESS;
    vlogf(LOG_JESUS, "Death Wave Damage: %d", dam);
  } else {
    switch (critFail(caster, SPELL_DEATHWAVE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_DEATHWAVE);
        sprintf(buf, "%d", beams);
        bBuf = buf;
        bBuf += " black energy beam";
        if (beams != 1)
          bBuf += "s expel";
        else
          bBuf += " expels";

        sprintf(buf, "%s from $n's hands and blow up in $n's face!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your hands and blow up in your face!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's hands and blow up in $n's face!", bBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, SPELL_DEATHWAVE) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;

        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    } 
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int deathWave(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int rc = 0;
  int ret = 0;

  ret = deathWave(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD)) 
    ADD_DELETE(rc, DELETE_VICT);
  
  if (IS_SET(ret, CASTER_DEAD)) 
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int deathWave(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DEATHWAVE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DEATHWAVE]->lag;
  diff = discArray[SPELL_DEATHWAVE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DEATHWAVE, diff, 1, "", rounds, 
caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castDeathWave(TBeing *caster, TBeing *victim)
{
  int level;
  int rc = 0;
  int ret = 0;

  level = caster->getSkillLevel(SPELL_DEATHWAVE);
  int bKnown = caster->getSkillValue(SPELL_DEATHWAVE);

  ret=deathWave(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_DEATHWAVE));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}
