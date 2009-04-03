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
#include "obj_magic_item.h"

int vampireTransform(TBeing *ch)
{
  TMonster *mob;
  PolyType bat = {"bat", 50, 1, 13749, DISC_NONE, RACE_NORACE };

  if (!ch->isPc() || IS_SET(ch->specials.act, ACT_POLYSELF) ||
      ch->polyed != POLY_TYPE_NONE){
    act("You are already transformed into another shape.",
	TRUE, ch, NULL, NULL, TO_CHAR);
    return FALSE;
  }
  
  if (!(mob = read_mobile(13749, VIRTUAL))) {
    ch->sendTo("It didn't seem to work.\n\r");
    return FALSE;
  }
  thing_to_room(mob,ROOM_VOID);
  mob->swapToStrung();
  

  act("You use your dark powers to transform into $N.", 
       TRUE, ch, NULL, mob, TO_CHAR);
  act("$n transforms into $N.",
       TRUE, ch, NULL, mob, TO_ROOM);

  SwitchStuff(ch, mob);
  setCombatStats(ch, mob, bat, SPELL_POLYMORPH);
  
  --(*mob);
  *ch->roomp += *mob;
  --(*ch);
  thing_to_room(ch, ROOM_POLY_STORAGE);
  
  // stop following whoever you are following.
  if (ch->master)
    ch->stopFollower(TRUE);
  
  // switch ch into mobile 
  ch->desc->character = mob;
  ch->desc->original = dynamic_cast<TPerson *>(ch);

  mob->desc = ch->desc;
  ch->desc = NULL;
  ch->polyed = POLY_TYPE_POLYMORPH;

  SET_BIT(mob->specials.act, ACT_DISGUISED);
  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
  REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
  REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

  appendPlayerName(ch, mob);
  
  mob->setHeight(ch->getHeight()/5);
  mob->setWeight(ch->getWeight()/10);

  return TRUE;
}



struct TransformLimbType TransformLimbList[LAST_TRANSFORM_LIMB] =
{
  {"hands", 6, 20, "bear claws", WEAR_HAND_R, AFFECT_TRANSFORMED_HANDS, DISC_RANGER},
  {"arms", 30, 75,"falcon wings", WEAR_ARM_R, AFFECT_TRANSFORMED_ARMS,
DISC_ANIMAL},
  {"legs", 20, 15, "a dolphin's tail", WEAR_LEG_R, AFFECT_TRANSFORMED_LEGS, DISC_ANIMAL},
  {"neck", 15, 40, "some fish gills", WEAR_NECK, AFFECT_TRANSFORMED_NECK, DISC_ANIMAL},
  {"head", 12, 60, "an eagle's head", WEAR_HEAD, AFFECT_TRANSFORMED_HEAD, DISC_RANGER},
  {"all", 1, 1, "all your limbs", MAX_WEAR, TYPE_UNDEFINED, DISC_RANGER}
};

int transformLimb(TBeing * caster, const char * buffer, int level, byte bKnown)
{
  int ret;
  bool multi = TRUE;
  wearSlotT limb = WEAR_NOWHERE;
  affectedData aff;
  affectedData aff2;
  bool two_affects = FALSE;
  int i;
  char newl[20];
  char old[20];
  char buf[256];

  if (caster->affectedBySpell(SPELL_POLYMORPH)) {
    caster->sendTo("You can't transform while polymorphed.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  // failure = scrapped item.  no item damage allowed in arena
  // this is to block problem in doTransformDrop()
  if (caster->roomp && caster->roomp->isRoomFlag(ROOM_ARENA)) {
    act("A magic power prevents anything from happening here.",
         FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.",
         TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FALSE;
  }

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > level) {
        return SPELL_SAVE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > caster->getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        return SPELL_SAVE;
      }
      break;
    }
  }

  if (i >= LAST_TRANSFORM_LIMB) {
    caster->sendTo("Couldn't find any such limb to transform.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  
  if (limb == MAX_WEAR) {
    caster->sendTo("You can't transform all of your limbs.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (!caster->isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  aff.type = SKILL_TRANSFORM_LIMB;
  aff.location = APPLY_NONE;
  aff.duration = (combatRound((level / 5) + 2));
  aff.bitvector = 0;
  aff.modifier = 0; 

  aff2.type = SKILL_TRANSFORM_LIMB;
  aff2.location = APPLY_NONE;
  aff2.duration = combatRound(((level / 5) + 2));
  aff2.bitvector = 0;
  aff2.modifier = 0;

  switch (limb) {
    case WEAR_NECK:
      aff.type = AFFECT_TRANSFORMED_NECK;
      aff.bitvector = AFF_WATERBREATH | AFF_SILENT;
      multi = FALSE;
      break;
    case WEAR_HEAD:
      aff.type = AFFECT_TRANSFORMED_HEAD;
      aff.bitvector = AFF_SILENT | AFF_CLARITY;
      multi = FALSE;
      break;
    case WEAR_HAND_R:
      aff.type = AFFECT_TRANSFORMED_HANDS;
      aff.location = APPLY_DAMROLL;

      // this prevents weapon use, so we want the effect to sort of make
      // up for that fact.  But since NPCs don't use weapons, and their
      // barehand dam is naturally high, lets not goose it for them too much
      if (caster->isPc())
        aff.modifier = 2 + (level / 4);
      else
        aff.modifier = 1 + (level/17);

      aff2.type = AFFECT_TRANSFORMED_HANDS;
      aff2.location = APPLY_SPELL;
      aff2.modifier = SKILL_CLIMB;
      aff2.modifier2 = 50;
      two_affects = TRUE;
      break;
    case WEAR_ARM_R:
      aff.type = AFFECT_TRANSFORMED_ARMS;
      aff.bitvector = AFF_FLYING;
      break;
    case WEAR_LEG_R:
      aff.type = AFFECT_TRANSFORMED_LEGS;
      aff.location = APPLY_SPELL;
      aff.modifier = SKILL_SWIM;
      aff.modifier2 = 50;
      break;
    default:
      vlogf(LOG_BUG, "Bad limb case in TRANSFORM_LIMB");
      caster->sendTo("Bug in your limbs, tell a god and put in bug file.\n\r");
      return FALSE;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SKILL_TRANSFORM_LIMB)) {
    switch (critSuccess(caster, SKILL_TRANSFORM_LIMB)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SKILL_TRANSFORM_LIMB);
        aff.duration *= 2;
        aff2.duration *= 2;
        ret = SPELL_CRIT_SUCCESS;
        sprintf(newl, "%s", TransformLimbList[i].newName);
        sprintf(old, "%s", TransformLimbList[i].name);
        if (multi)
          sprintf(buf, "Your %s glow as they transform into %s!", old, newl);
        else
          sprintf(buf, "Your %s glows as it transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_CHAR);
        if (multi)
          sprintf(buf, "$n's %s liquify and then transform into %s!", old, newl);
        else
          sprintf(buf, "$n's %s liquifies and then transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_ROOM);
        break;
      default:
        sprintf(newl, "%s", TransformLimbList[i].newName);
        sprintf(old, "%s", TransformLimbList[i].name);
        if (multi)
          sprintf(buf, "Your %s tingle as they transform into %s!", old, newl); 
        else 
          sprintf(buf, "Your %s tingles as it transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_CHAR);
        if (multi)
          sprintf(buf, "$n's %s liquify and then transform into %s!", old, newl);
        else
          sprintf(buf, "$n's %s liquifies and then transforms into %s!", old, newl);
        act(buf, FALSE, caster, 0, 0, TO_ROOM);
        ret = SPELL_SUCCESS;
        break;
    }
    caster->affectTo(&aff);
    if (two_affects)
      caster->affectTo(&aff2);

    caster->makeLimbTransformed(caster, limb, TRUE);
    return ret;
  } else {  
    switch (critFail(caster, SKILL_TRANSFORM_LIMB)) {  
      case CRIT_F_HITOTHER:
        CF(SKILL_TRANSFORM_LIMB);
        caster->rawBleed(limb, (level * 3) + 100, SILENT_NO, CHECK_IMMUNITY_YES);
        return SPELL_CRIT_FAIL;
      default:
        return SPELL_FAIL;
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    }
  }
}


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
    if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_STORMY_SKIES)) {
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
    if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_STORMY_SKIES)) {
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

  level = min(level, 50);

  int dam = caster->getSkillDam(victim, SPELL_AQUATIC_BLAST, level, adv_learn);

  if (victim->getImmunity(IMMUNE_WATER) >= 100) {
    act("$N is immune to water damage!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_AQUATIC_BLAST)) {
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

static const int LAST_SHAPED_MOB = 10;
struct PolyType ShapeShiftList[LAST_SHAPED_MOB] =
//   name,    level, learning, vnum, discipline, race
{
  {"chicken"  , 25,   1, 1213, DISC_SHAMAN_FROG, RACE_NORACE},  // L 19
  {"horsefly" , 25,  50, 15420, DISC_SHAMAN_FROG, RACE_NORACE}, // L 6
  {"slug"     , 30,  20,  5126, DISC_SHAMAN_FROG, RACE_NORACE}, // L 7
  {"snake"    , 35,  20,  3412, DISC_SHAMAN_FROG, RACE_NORACE}, // L 40
  {"dolphin"  , 40,  70, 12432, DISC_SHAMAN_FROG, RACE_NORACE}, // L 14
  {"bear"     , 45,  75,  3403, DISC_SHAMAN_FROG, RACE_NORACE}, // L 43
  {"crow"     , 40,  50, 14350, DISC_SHAMAN_FROG, RACE_NORACE}, // L 7
  {"hawk"     , 45,  80, 14440, DISC_SHAMAN_FROG, RACE_NORACE}, // L 27
  {"spider"   , 49, 100,  7717, DISC_SHAMAN_FROG, RACE_NORACE}, // L 55
  {"\n"       , -1,  -1,    -1, DISC_SHAMAN_FROG, RACE_NORACE}
};

int shapeShift(TBeing *caster, int level, byte bKnown)
{
  int i, ret = 0;
//  bool nameFound = FALSE;
  bool found = FALSE;
  TMonster *mob;
  const char * buffer;
  affectedData aff;
  affectedData aff2;

  buffer = caster->spelltask->orig_arg;
 
  discNumT das = getDisciplineNumber(SPELL_SHAPESHIFT, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, "Bad disc for SPELL_SHAPESHIFT");
    return SPELL_FAIL;
  }

  for (i = 0; (i < LAST_SHAPED_MOB); i++) {
    if ((signed) ShapeShiftList[i].tRace != RACE_NORACE &&
	!caster->isImmortal() &&
	caster->getRace() != (signed) ShapeShiftList[i].tRace)
      continue; // ? never true, because race is RACE_NORACE in every case

    if (ShapeShiftList[i].level > caster->GetMaxLevel())
      continue;

    if (ShapeShiftList[i].learning > caster->getSkillValue(SPELL_SHAPESHIFT))
      continue;

    if (!isname(buffer, ShapeShiftList[i].name))
      continue;               

    break;
  }

  if (i >= LAST_SHAPED_MOB) { // i > LAST_SHAPED_MOB cannot happen from above
    caster->sendTo("You haven't a clue where to start on that one.\n\r");
    return SPELL_FAIL;
  } 

 if (!(mob = read_mobile(ShapeShiftList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't summon an image of that creature.\n\r");
    return SPELL_FAIL;
  }
  thing_to_room(mob,ROOM_VOID);   // just so if extracted it isn't in NOWHERE 
  mob->swapToStrung();

 int duration;
  
  if (caster->bSuccess(bKnown, SPELL_SHAPESHIFT)) {
    switch (critSuccess(caster, SPELL_SHAPESHIFT)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        duration = (2 + level / 5) * UPDATES_PER_MUDHOUR;
        CS(SPELL_SHAPESHIFT);
        ret = SPELL_CRIT_SUCCESS;
      case CRIT_S_NONE:
      default:
        duration = (1 + level / 10) * UPDATES_PER_MUDHOUR;
        break;
   }

  // first add the attempt -- used to regulate attempts
  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = duration + UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = SPELL_SHAPESHIFT;
  caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    --(*mob);
    *caster->roomp += *mob;
    SwitchStuff(caster, mob);
    setCombatStats(caster, mob, ShapeShiftList[i], SPELL_SHAPESHIFT);

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

#if 1
    // used to wear off
    aff2.type = SPELL_SHAPESHIFT;
    aff2.location = APPLY_NONE;
    aff2.duration = duration;
    aff2.bitvector = 0;
    aff2.modifier = 0;
    mob->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);
#endif

    mob->desc = caster->desc;
    caster->desc = NULL;
    caster->polyed = POLY_TYPE_SHAPESHIFT;
    
    
    // transfer spells - some bugs
//    caster->spellAffectJoin(mob, caster->affected, AVG_DUR_NO, AVG_EFF_YES);

    SET_BIT(mob->specials.act, ACT_POLYSELF);
    SET_BIT(mob->specials.act, ACT_NICE_THIEF);
    SET_BIT(mob->specials.act, ACT_SENTINEL);
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
    REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
    REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

    // fix name so poly'd shaman can be found
    appendPlayerName(caster, mob);
        
//    mob->setLifeforce(min((mob->getLifeforce() - 15), 85));
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int shapeShift(TBeing *caster, const char * buffer)
{
  taskDiffT diff;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SPELL_SHAPESHIFT))
{
    act("You are not prepared to try to shapeshift yourself again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  if (caster->roomp && caster->roomp->isIndoorSector() &&
      !(caster->roomp->isWaterSector() || caster->roomp->isUnderwaterSector()
          || caster->isImmortal()) ) {
    act("Only the outdoors affords the connection to nature required to shift your shape.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }

 // Check to make sure that there is no snooping going on.
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->nothingHappens();
    vlogf(LOG_BUG,"Immort tried to shapeshift while snooping.");
    return SPELL_FAIL;
  }

  if (caster->desc->original) {
    // implies they are switched, while already switched (as x switch)
    caster->sendTo("You already seem to be switched.\n\r");
    return SPELL_FAIL;
  }
  if (caster->desc->snoop.snoop_by)
    caster->desc->snoop.snoop_by->doSnoop(caster->desc->snoop.snoop_by->getName());

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

int TBeing::doTransform(const char *argument) 
{
  int i = 0, bKnown = 0;
  wearSlotT limb = WEAR_NOWHERE;
  int level, ret = 0;
  int rc = 0;
  char buffer[256];

//  sendTo("This is disabled due to a bug right now.\n\r");
//  return FALSE;

  if (!doesKnowSkill(SKILL_TRANSFORM_LIMB)) {
    if(isVampire())
      return vampireTransform(this);

    sendTo("You know nothing about transforming your limbs.\n\r");
    return FALSE;
  }

  strcpy(buffer, argument);

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > getSkillLevel(SKILL_TRANSFORM_LIMB)) {
        sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      break;
    }
  }
  if (i >= LAST_TRANSFORM_LIMB) {
    if(isVampire())
      return vampireTransform(this);

    sendTo("Couldn't find any such limb to transform.\n\r");
    return FALSE;
  }

  if (limb == MAX_WEAR) {
    sendTo("You can't transform all of your limbs.\n\r");
    return FALSE;
  }

  if (!isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, this, NULL, NULL, TO_CHAR);
    return FALSE;
  }

  level = getSkillLevel(SKILL_TRANSFORM_LIMB);
  bKnown = getSkillValue(SKILL_TRANSFORM_LIMB);

  ret=transformLimb(this, buffer, level, bKnown);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Something went wrong with the magic.",
          FALSE, this, 0, NULL, TO_CHAR);
      act("You feel your own limb open and your blood start to drain!",
          FALSE, this, 0, NULL, TO_CHAR);
      act("Something went wrong with $n's magic.",
          FALSE, this, 0, NULL, TO_ROOM);
      act("$n seems to have caused $s limbs to start bleeding!",
          FALSE, this, 0, NULL, TO_ROOM);
  } else if (ret==SPELL_SAVE) {
      act("You are not powerful enough to transform that limb.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
      act("You fail to transform your limbs.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  } else {
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", 
          FALSE, this, NULL, NULL, TO_ROOM);
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;

}

int transformLimb(TBeing * caster, const char * buffer)
{
  taskDiffT diff;
  int i;
  wearSlotT limb = WEAR_NOWHERE;

  for(i = 0; i < LAST_TRANSFORM_LIMB; i++) {
    if (is_abbrev(buffer,TransformLimbList[i].name)) {
      limb = TransformLimbList[i].limb;
      if (TransformLimbList[i].level > caster->getSkillLevel(SKILL_TRANSFORM_LIMB)) {
        caster->sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      // NOTE: this is DISC learning, not skill (intentional)
      if (TransformLimbList[i].learning > caster->getDiscipline((TransformLimbList[i].discipline))->getLearnedness()) {
        caster->sendTo("You can not transform that limb yet.");
        return FALSE;
      }
      break;
    }
  }
  if (i >= LAST_TRANSFORM_LIMB) {
    caster->sendTo("Couldn't find any such limb to transform.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (limb == MAX_WEAR) {
    caster->sendTo("You can't transform all of your limbs.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

  if (!caster->isTransformableLimb(limb, TRUE)) {
    act("Something prevents your limb from being transformed.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }

    if (!bPassMageChecks(caster, SKILL_TRANSFORM_LIMB, caster))
      return FALSE;

    lag_t rounds = discArray[SKILL_TRANSFORM_LIMB]->lag;
    diff = discArray[SKILL_TRANSFORM_LIMB]->task;

    
    start_cast(caster, NULL, NULL, caster->roomp, SKILL_TRANSFORM_LIMB, diff, 1, buffer, rounds, caster->in_room, 0, 0,TRUE, 0);
    return FALSE;
}

int castTransformLimb(TBeing * caster)
{
  int level, ret;
  int rc = 0;

  level = caster->getSkillLevel(SKILL_TRANSFORM_LIMB);
  int bKnown = caster->getSkillValue(SKILL_TRANSFORM_LIMB);

  ret=transformLimb(caster, caster->spelltask->orig_arg, level, bKnown);
  if (ret==SPELL_SUCCESS) {
  } else if (ret==SPELL_CRIT_SUCCESS) {
  } else if (ret==SPELL_CRIT_FAIL) {
      act("Something went wrong with your spell.",
          FALSE, caster, 0, NULL, TO_CHAR);
      act("You feel your own limb open and your blood start to drain!",
          FALSE, caster, 0, NULL, TO_CHAR);
      act("Something went wrong with $n's spell.",
          FALSE, caster, 0, NULL, TO_ROOM);
      act("$n seems to have caused $s limbs to start bleeding!",
          FALSE, caster, 0, NULL, TO_ROOM);
  } else if (ret==SPELL_SAVE) {
      act("You are not powerful enough to transform that limb.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  } else if (ret==SPELL_FAIL) {
      act("You fail to transform your limbs.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  } else {
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
  }

  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}



// END SHAPESHIFT

int deathWave(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  char buf[256];
  sstring bBuf;

  level = min(level, 75);

  int dam = caster->getSkillDam(victim, SPELL_DEATHWAVE, level, adv_learn);
  int beams = (dam / 3) + ::number(0, (caster->GetMaxLevel() / 10));
  beams = max(beams, 1);

  if (victim->getImmunity(IMMUNE_ENERGY) >= 100) {
    act("$N is immune to energy and thaumaturgy!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_DEATHWAVE]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_DEATHWAVE)) {
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
