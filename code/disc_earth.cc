//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_earth.h"
#include "obj_magic_item.h"

int slingShot(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{

  level = min(level, 10);

  int dam = caster->getSkillDam(victim, SPELL_SLING_SHOT, level, adv_learn);
  caster->reconcileHurt(victim,discArray[SPELL_SLING_SHOT]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SLING_SHOT)) {
    if (victim->isLucky(caster->spellLuckModifier(SPELL_SLING_SHOT)) &&
        victim->awake()) {
      SV(SPELL_SLING_SHOT);
      dam /= 2;
      act("$N is able to partially deflect your rock.",
            FALSE, caster, NULL, victim, TO_CHAR);
      act("You are able to partially deflect the rock hurled by $n.",
            FALSE, caster, NULL, victim, TO_VICT);
      act("$N partially deflects a rock hurled by $n.",
            FALSE, caster, NULL, victim, TO_NOTVICT);      
    } else {
      switch (critSuccess(caster, SPELL_SLING_SHOT)) {
        case CRIT_S_KILL:
        case CRIT_S_TRIPLE:
        case CRIT_S_DOUBLE:
          CS(SPELL_SLING_SHOT);
          dam *= 2;
          act("You summon forth a huge rock that beans $N right between the eyes!",
                FALSE, caster, NULL, victim, TO_CHAR, ANSI_GREEN);
          act("$n summons forth a huge rock that smacks you right between your eyes!",
                FALSE, caster, NULL, victim, TO_VICT, ANSI_GREEN);
          act("$n summons forth a huge rock that beans $N right between the eyes!!",
                FALSE, caster, NULL, victim, TO_NOTVICT);      
          break;
        default:
          act("You cause a rock to leap out and hit $N in the noggin!",
                FALSE, caster, NULL, victim, TO_CHAR);
          act("$n causes a rock to leap out and hit you in the noggin!",
                FALSE, caster, NULL, victim, TO_VICT);
          act("$n causes a rock to leap out and hit $N in the noggin!",
                FALSE, caster, NULL, victim, TO_NOTVICT);
      }
    }
    victim->roomp->playsound(SOUND_SPELL_SLING_SHOT, SOUND_TYPE_MAGIC);
    if (caster->reconcileDamage(victim, dam, SPELL_SLING_SHOT) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("$n just tried to attack you.", FALSE, caster, 0, victim, TO_VICT);
    if (critFail(caster, SPELL_SLING_SHOT) == CRIT_F_HITSELF) {
      CF(SPELL_SLING_SHOT);
      act("You cause a rock to leap out and hit yourself in the noggin!",
           FALSE, caster, NULL, 0, TO_CHAR, ANSI_GREEN);
      act("$n causes a rock to leap out and hit $mself in the noggin!",
           FALSE, caster, NULL, 0, TO_ROOM);
      if (caster->reconcileDamage(caster, dam, SPELL_SLING_SHOT) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int slingShot(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_SLING_SHOT, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SLING_SHOT]->lag;
  taskDiffT diff = discArray[SPELL_SLING_SHOT]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SLING_SHOT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castSlingShot(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_SLING_SHOT);

  ret=slingShot(caster,victim,level,caster->getSkillValue(SPELL_SLING_SHOT), caster->getAdvLearning(SPELL_SLING_SHOT));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int graniteFists(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  level = min(level, 25);

  int dam = caster->getSkillDam(victim, SPELL_GRANITE_FISTS, level, adv_learn);

  caster->reconcileHurt(victim,discArray[SPELL_GRANITE_FISTS]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_GRANITE_FISTS)) {
    switch (critSuccess(caster, SPELL_GRANITE_FISTS)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        dam *= 2;
        act("Your fists turn to granite as you NAIL $N!  CRUNCH!",
               FALSE, caster, NULL, victim, TO_CHAR);
        act("$n's fists turn to granite as $e NAILS $N!  CRUNCH!",
               FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$n's fists turn to granite as $e NAILS you!  CRUNCH!",
               FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        act("Your fists turn to granite as you punch $N!  Whap!",
               FALSE, caster, NULL, victim, TO_CHAR);
        act("$n's fists turn to granite as $e pummels $N!  Kapow!",
               FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$n's fists turn to granite as $e pummels you!  Ooof!",
               FALSE, caster, NULL, victim, TO_VICT);
        if (victim->isLucky(caster->spellLuckModifier(SPELL_GRANITE_FISTS))) {
          SV(SPELL_GRANITE_FISTS);
          dam /= 2;
        }
        break;
    }

    victim->roomp->playsound(SOUND_SPELL_GRANITE_FIST, SOUND_TYPE_MAGIC);
    if (caster->reconcileDamage(victim, dam, SPELL_GRANITE_FISTS) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("$n just tried to attack you!", FALSE, caster, NULL, victim, TO_VICT);
    if (critFail(caster, SPELL_GRANITE_FISTS) == CRIT_F_HITSELF) {
      CF(SPELL_GRANITE_FISTS);
      act("Whap! Ouch!  Doh! Your fists turn to granite as you punch yourself!",
          FALSE, caster, NULL, victim, TO_CHAR);
      act("Doh! $n's fists turn to granite as $e pummels $mself! Idiot!",
          FALSE, caster, NULL, victim, TO_ROOM);
      if (caster->reconcileDamage(caster, dam, SPELL_GRANITE_FISTS) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}


int graniteFists(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

     if (!bPassMageChecks(caster, SPELL_GRANITE_FISTS, victim))
        return FALSE; 

     lag_t rounds = discArray[SPELL_GRANITE_FISTS]->lag;
     diff = discArray[SPELL_GRANITE_FISTS]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_GRANITE_FISTS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

       return TRUE;
}

int castGraniteFists(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_GRANITE_FISTS);

  ret=graniteFists(caster,victim,level,caster->getSkillValue(SPELL_GRANITE_FISTS), caster->getAdvLearning(SPELL_GRANITE_FISTS));
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

int pebbleSpray(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  TThing *t, *t2;
  TBeing *vict = NULL;

  level = min(level, 15);

  int dam = caster->getSkillDam(NULL, SPELL_PEBBLE_SPRAY, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_PEBBLE_SPRAY)) {
    act("You whip up a spray of pebbles.", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n whips up a spray of pebbles.", FALSE, caster, NULL, NULL, TO_ROOM);
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;

      if (!caster->inGroup(*vict) && !vict->isImmortal()) {
        caster->reconcileHurt(vict, discArray[SPELL_PEBBLE_SPRAY]->alignMod);
        act("$n is pelted with pebbles!", FALSE, vict, NULL, NULL, TO_ROOM);
        act("You are pelted with pebbles!", FALSE, vict, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(vict, dam, SPELL_PEBBLE_SPRAY) == -1) {
          delete vict;
          vict = NULL;
        }
      } else {
        //act("$n dodges the pebbles!", TRUE, vict, NULL, 0 , TO_ROOM);
        act("You dodge the pebbles!", TRUE, vict, NULL, NULL, TO_CHAR);
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_PEBBLE_SPRAY)) {
      CF(SPELL_PEBBLE_SPRAY);
      act("Ohno!  You pelt yourself with the pebbles!", 
            FALSE, caster, NULL, NULL, TO_CHAR);
      act("$n pelts $mself with the pebbles!  Doh!", 
            FALSE, caster, NULL, NULL, TO_ROOM);
      if (caster->reconcileDamage(caster, dam, SPELL_PEBBLE_SPRAY) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int pebbleSpray(TBeing * caster)
{
  if (!bPassMageChecks(caster, SPELL_PEBBLE_SPRAY, NULL))
    return FALSE; 

  lag_t rounds = discArray[SPELL_PEBBLE_SPRAY]->lag;
  taskDiffT diff = discArray[SPELL_PEBBLE_SPRAY]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_PEBBLE_SPRAY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castPebbleSpray(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_PEBBLE_SPRAY);

  ret=pebbleSpray(caster,level,caster->getSkillValue(SPELL_PEBBLE_SPRAY), caster->getAdvLearning(SPELL_PEBBLE_SPRAY));
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

int sandBlast(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  TThing *t, *t2;
  TBeing *vict = NULL;

  level = min(level, 33);

  int dam = caster->getSkillDam(NULL, SPELL_SAND_BLAST, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_SAND_BLAST)) {
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;

      if (!caster->inGroup(*vict) && !vict->isImmortal()) {
        caster->reconcileHurt(vict, discArray[SPELL_SAND_BLAST]->alignMod);
        act("$n is BLASTED with sand!", FALSE, vict, NULL, NULL, TO_ROOM);
        act("You are BLASTED with sand!", FALSE, vict, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(vict, dam, SPELL_SAND_BLAST) == -1) {
          delete vict;
          vict = NULL;
        }
      } else {
        //act("$n dodges the blast of sand!", TRUE, vict, NULL, 0, TO_ROOM);
        act("You dodge the blast of sand!", TRUE, vict, NULL, NULL, TO_CHAR);
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_SAND_BLAST)) {
      CF(SPELL_SAND_BLAST);
      caster->sendTo("Uh oh!\n\r");
      act("Uh oh!  Something didn't go quite right.", 
             FALSE, caster, 0, 0, TO_ROOM);
      for (t = caster->roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        vict = dynamic_cast<TBeing *>(t);
        if (!vict)
          continue;

        if (caster->inGroup(*vict) && caster != vict) {
          caster->reconcileHurt(vict, discArray[SPELL_SAND_BLAST]->alignMod);
          act("$n is BLASTED with sand!", FALSE, vict, NULL, NULL, TO_ROOM);
          act("You are BLASTED with sand!", FALSE, vict, NULL, NULL, TO_CHAR);
          if (caster->reconcileDamage(vict, dam, SPELL_SAND_BLAST) == -1) {
            delete vict;
            vict = NULL;
          }
        } else {
          //act("$n dodges the blast of sand!", FALSE, vict, NULL, NULL, TO_ROOM);
          act("You dodge the blast of sand!", FALSE, vict, NULL, NULL, TO_CHAR);
        }
      }
      if (caster->reconcileDamage(caster, dam, SPELL_SAND_BLAST) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    } else 
      caster->nothingHappens();
    
    return SPELL_FAIL;
  }
}

int sandBlast(TBeing * caster)
{
  if (!bPassMageChecks(caster, SPELL_SAND_BLAST, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_SAND_BLAST]->lag;
  taskDiffT diff = discArray[SPELL_SAND_BLAST]->task;
 
  start_cast(caster, NULL, NULL, caster->roomp, SPELL_SAND_BLAST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castSandBlast(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_SAND_BLAST);

  act("You cause a blast of sand to kick up and whip through the area.", 
             FALSE, caster, NULL, NULL, TO_CHAR);
  act("$n causes a blast of sand to kick up and whip through the area.", 
             FALSE, caster, NULL, NULL, TO_ROOM);

  ret=sandBlast(caster,level,caster->getSkillValue(SPELL_SAND_BLAST), caster->getAdvLearning(SPELL_SAND_BLAST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret == SPELL_CRIT_FAIL){
    }
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int lavaStream(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  TThing * t, *t2;
  TBeing *vict = NULL;

  level = min(level, 60);

  int dam = caster->getSkillDam(NULL, SPELL_LAVA_STREAM, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_LAVA_STREAM)) {
    act("You call forth molten lava from the earth!", 
          FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n calls forth molten lava from the earth!", 
          FALSE, caster, NULL, NULL, TO_ROOM);
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;

      if (!caster->inGroup(*vict) && !vict->isImmortal()) {
        caster->reconcileHurt(vict, discArray[SPELL_LAVA_STREAM]->alignMod);
        act("$n screams in agony as $e is burned by the lava!", 
              FALSE, vict, NULL, NULL, TO_ROOM);
        act("You scream in agony as you are burned by the lava!", 
              FALSE, vict, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(vict, dam, SPELL_LAVA_STREAM) == -1) {
          delete vict;
          vict = NULL;
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_LAVA_STREAM)) {
      CF(SPELL_LAVA_STREAM);
      dam /= 2;
      act("Umm...something didn't seem to go just right.", 
           FALSE, caster, NULL, NULL, TO_CHAR);
      for (t = caster->roomp->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        vict = dynamic_cast<TBeing *>(t);
        if (!vict)
          continue;

        if (caster->inGroup(*vict) && caster != vict && !vict->isImmortal()) {
          caster->reconcileHurt(vict, discArray[SPELL_LAVA_STREAM]->alignMod);
          act("$n screams in agony as $e is burned by the lava!", 
              FALSE, vict, NULL, NULL, TO_ROOM);
          act("You scream in agony as you are burned by the lava!", 
              FALSE, vict, NULL, NULL, TO_CHAR);
          if (caster->reconcileDamage(vict, dam, SPELL_LAVA_STREAM) == -1) {
            delete vict;
            vict = NULL;
          }
        }
      }
      if (caster->reconcileDamage(caster, dam, SPELL_LAVA_STREAM) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int lavaStream(TBeing * caster)
{
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("There are no underwater volcanos nearby!\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_LAVA_STREAM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_LAVA_STREAM]->lag;
  taskDiffT diff = discArray[SPELL_LAVA_STREAM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_LAVA_STREAM, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castLavaStream(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_LAVA_STREAM);
  
  ret=lavaStream(caster,level,caster->getSkillValue(SPELL_LAVA_STREAM), caster->getAdvLearning(SPELL_LAVA_STREAM));
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

int meteorSwarm(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  char buf[81];

  if (!caster->outside()) {
    caster->sendTo("You can only do this outside!\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 45);

  int damage = caster->getSkillDam(victim, SPELL_METEOR_SWARM, level, adv_learn);

  // just used for effect, has no impact on damage
  int num_meteors = level; 

  if (caster->bSuccess(bKnown, SPELL_METEOR_SWARM)) {
    switch (critSuccess(caster, SPELL_METEOR_SWARM)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_METEOR_SWARM);
        damage <<= 1;
        num_meteors <<= 1;
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_METEOR_SWARM);
        damage *= 3;
        num_meteors *= 3;
        break;
      default:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_METEOR_SWARM))) {
          SV(SPELL_METEOR_SWARM);
          damage /= 2;
          num_meteors /= 2;
        }
        break;
    }

    victim->roomp->playsound(SOUND_SPELL_METEOR_SWARM, SOUND_TYPE_MAGIC);

    sprintf(buf, "At $n's call, %d meteors blast $N to smithereens!", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You call forth %d meteors and blast $N to smithereens!", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    sprintf(buf, "$n calls forth %d meteors that blast you to smithereens!", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_VICT);
    caster->reconcileHurt(victim,discArray[SPELL_METEOR_SWARM]->alignMod);
    if (caster->reconcileDamage(victim, damage, SPELL_METEOR_SWARM) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_METEOR_SWARM)) {
      case CRIT_F_HITSELF:
        CF(SPELL_METEOR_SWARM);
        sprintf(buf, "At $n's call, %d meteors blast $n to smithereens!", num_meteors);
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "You call forth %d meteors and blast yourself to smithereens!", num_meteors);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "$n calls forth %d meteors that blast $E to smithereens!", num_meteors);
        act(buf, FALSE, caster, NULL, victim, TO_VICT);

        damage /= 3;

        if (caster->reconcileDamage(caster, damage, SPELL_METEOR_SWARM) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_HITOTHER:
      case CRIT_F_NONE:
        break;
    }
    sprintf(buf, "At $n's call, %d meteors burn up in space.", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
    sprintf(buf, "You call forth %d meteors and they burn up in space.", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    sprintf(buf, "$n calls forth %d meteors that burn up in space.", num_meteors);
    act(buf, FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }
}

int meteorSwarm(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret;
  int rc = 0;

  ret = meteorSwarm(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int meteorSwarm(TBeing *caster, TBeing * victim)
{
  taskDiffT diff;

  if (!caster->outside()) {
    caster->sendTo("You can only cast this outside!\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (!bPassMageChecks(caster, SPELL_METEOR_SWARM, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_METEOR_SWARM]->lag;
  diff = discArray[SPELL_METEOR_SWARM]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_METEOR_SWARM, diff,
1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return FALSE;
}

int castMeteorSwarm(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_METEOR_SWARM);

  ret=meteorSwarm(caster,victim,level,caster->getSkillValue(SPELL_METEOR_SWARM), caster->getAdvLearning(SPELL_METEOR_SWARM));
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

int stoneSkin(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff1, aff2;

  if (victim->affectedBySpell(SPELL_FLAMING_FLESH)) {
    act("$N's skin is already defended by elementals of fire.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }
  if (caster->bSuccess(bKnown, SPELL_STONE_SKIN)) {
    caster->reconcileHelp(caster,discArray[SPELL_STONE_SKIN]->alignMod);

    // ARMOR APPLY
    aff1.type = SPELL_STONE_SKIN;
    aff1.level = level;
    aff1.duration = aff1.level * UPDATES_PER_MUDHOUR;
    aff1.location = APPLY_ARMOR;
    aff1.modifier = -75;

    // PIERCE IMMUNITY
    aff2.type = SPELL_STONE_SKIN;
    aff2.level = level;
    aff2.duration = aff2.level * UPDATES_PER_MUDHOUR;
    aff2.location = APPLY_IMMUNITY;
    aff2.modifier = IMMUNE_PIERCE;
    aff2.modifier2 = 15;

    switch (critSuccess(caster, SPELL_STONE_SKIN)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_STONE_SKIN);
        aff1.duration = aff1.duration * 3 / 2;
        aff1.modifier = aff1.modifier * 3 / 2;
        aff2.duration = aff2.duration * 3 / 2;
        break;
      default:
        break;
    } 
    if (caster != victim)
      aff1.modifier /= 5;

    victim->affectJoin(caster, &aff1, AVG_DUR_NO, AVG_EFF_YES);
    victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES);

    act("$n's skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_ROOM);
    act("Your skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    act("$n's skin turns to a thin shale then fades back.", FALSE, victim, NULL, NULL, TO_ROOM);
    act("Your skin turns to a thin shale then fades back.", FALSE, victim, NULL, NULL, TO_CHAR);
    return SPELL_FAIL;
  }
}

void stoneSkin(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  act("$p calls upon the powers of the elementals of earth.", FALSE, caster, obj, 0, TO_ROOM);
  act("$p calls upon the powers of the elementals of earth.", FALSE, caster, obj, 0, TO_CHAR);
  stoneSkin(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int stoneSkin(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

  if (victim->affectedBySpell(SPELL_FLAMING_FLESH)) {
    act("$N's skin is already defended by elementals of fire.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }
  if (!bPassMageChecks(caster, SPELL_STONE_SKIN, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_STONE_SKIN]->lag;
  diff = discArray[SPELL_STONE_SKIN]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_STONE_SKIN, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return FALSE;
}

int castStoneSkin(TBeing * caster, TBeing * victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_STONE_SKIN);

  act("$n calls upon the powers of the elementals of earth.", FALSE, caster, 0, 0, TO_ROOM);
  act("You call upon the powers of the elementals of earth.", FALSE, caster, 0, 0, TO_CHAR);

  ret=stoneSkin(caster,victim,level,caster->getSkillValue(SPELL_STONE_SKIN));

  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int trailSeek(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;

  if (caster->bSuccess(bKnown, SPELL_TRAIL_SEEK)) {
    caster->reconcileHelp(victim,discArray[SPELL_TRAIL_SEEK]->alignMod);

    aff.type = SPELL_TRAIL_SEEK;
    aff.level = level;
    aff.duration = aff.level * UPDATES_PER_MUDHOUR / 3;   /* short lived */
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;

    switch (critSuccess(caster, SPELL_TRAIL_SEEK)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_TRAIL_SEEK);
        aff.duration = aff.duration * 3 / 2;
        break;
      default:
        break;
    } 
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    if (caster != victim) {
      act("You have successfully granted $N the senses of a bloodhound!", 
          FALSE, caster, NULL, victim, TO_CHAR);
      act("You have been successfully granted the senses of a bloodhound!", 
          FALSE, caster, NULL, victim, TO_VICT);
      act("You become much more attuned to your senses!", 
          FALSE, caster, NULL, victim, TO_VICT);
      act("You whiff the aromas of many whom have passed through here.", 
          FALSE, caster, NULL, victim, TO_VICT);
      act("$N's eyes glow with a faint blue light for a moment.", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
    } else {
      act("You have successfully given yourself the senses of a bloodhound!", 
          FALSE, caster, NULL, 0, TO_CHAR);
      act("You become much more attuned to your senses!", 
          FALSE, caster, NULL, 0, TO_CHAR);
      act("You whiff the aromas of many whom have passed through here.", 
          FALSE, caster, NULL, 0, TO_CHAR);
      act("$n's eyes glow with a faint blue light for a moment.", 
          FALSE, caster, NULL, 0, TO_ROOM);
    }
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void trailSeek(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int rc;

  rc = trailSeek(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (rc == SPELL_SUCCESS) {
  } else {
  }
}

int trailSeek(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_TRAIL_SEEK, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_TRAIL_SEEK]->lag;
  diff = discArray[SPELL_TRAIL_SEEK]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_TRAIL_SEEK, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;
}

int castTrailSeek(TBeing * caster, TBeing * victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_TRAIL_SEEK);

  ret=trailSeek(caster,victim,level,caster->getSkillValue(SPELL_TRAIL_SEEK));
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int conjureElemEarth(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(EARTH_ELEMENTAL, VIRTUAL))) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CONJURE_EARTH, 0);

  if (caster->bSuccess(bKnown, SPELL_CONJURE_EARTH)) {
    act("You summon the powers of the natural earthly forces!", 
             TRUE, caster, NULL, NULL, TO_CHAR);
    act("$n summons the powers of the natural earthly forces!", 
             TRUE, caster, NULL, NULL, TO_ROOM);
    /* charm them for a while */
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CONJURE_EARTH;
    aff.level = level;
    aff.duration  = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this skill
    // this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    /* Add hp for higher levels - Russ */
    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());
    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CONJURE_EARTH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CONJURE_EARTH);
        act("$N flexes $S overly strong muscles.",
                  TRUE, caster, 0, victim, TO_ROOM);
        caster->sendTo("You have conjured an unusually strong elemental!\n\r");
        victim->setMaxHit((int) (victim->hitLimit() * 1.5));
        victim->setHit((int) (victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!",
             TRUE, caster, NULL, victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!",
             TRUE, caster, NULL, victim, TO_ROOM);
      act("You've created a monster; $N hates you!",
             FALSE, caster, NULL, victim, TO_CHAR);
      victim->affectFrom(SPELL_CONJURE_EARTH);
      victim->affectFrom(AFFECT_THRALL);
      return SPELL_FAIL;
    }
    caster->addFollower(victim);
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("Hmmm...that didn't feel quite right.", 
           FALSE, caster, NULL, NULL, TO_CHAR);
    act("When your eyes recover, you see $N standing before you!",
           TRUE, caster, NULL, victim, TO_NOTVICT);
    act("You've created a monster; $N hates you!", 
           FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int conjureElemEarth(TBeing * caster)
{
  if (real_mobile(EARTH_ELEMENTAL) < 0) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_CONJURE_EARTH, NULL))
    return FALSE;

  if (caster->roomp->notRangerLandSector() &&
      !caster->roomp->isCitySector() && !caster->roomp->isRoadSector()) {
    caster->sendTo("There doesn't seem to be enough earth here to conjure an earth elemental.\n\r");
    return FALSE;
  }

  lag_t rounds = discArray[SPELL_CONJURE_EARTH]->lag;
  taskDiffT diff = discArray[SPELL_CONJURE_EARTH]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CONJURE_EARTH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;
}

int castConjureElemEarth(TBeing * caster)
{
int ret,level; 

  level = caster->getSkillLevel(SPELL_CONJURE_EARTH);

  if ((ret=conjureElemEarth(caster,level,caster->getSkillValue(SPELL_CONJURE_EARTH))) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int protectionFromEarth(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;
 
  aff.type = SPELL_PROTECTION_FROM_EARTH;
  aff.level = level;
  aff.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_EARTH;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;
 
  if (caster->bSuccess(bKnown,SPELL_PROTECTION_FROM_EARTH)) {
    act("$n glows with a faint brown aura for a brief moment.", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a faint brown aura for a brief moment.", FALSE, victim, NULL, NULL, TO_CHAR);
    switch (critSuccess(caster, SPELL_PROTECTION_FROM_EARTH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_PROTECTION_FROM_EARTH);
        aff.duration = (10 + (level / 2)) * UPDATES_PER_MUDHOUR;
        aff.modifier2 = (level * 2);
        break;
      case CRIT_S_NONE:
        break;
    }
 
    if (caster != victim) {
      aff.modifier2 /= 2;
    }
 
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    caster->reconcileHelp(victim, discArray[SPELL_PROTECTION_FROM_EARTH]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void protectionFromEarth(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  protectionFromEarth(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int protectionFromEarth(TBeing * caster, TBeing *v)
{
  if (!bPassMageChecks(caster, SPELL_PROTECTION_FROM_EARTH, v))
    return FALSE;

  lag_t rounds = discArray[SPELL_PROTECTION_FROM_EARTH]->lag;
  taskDiffT diff = discArray[SPELL_PROTECTION_FROM_EARTH]->task;

  start_cast(caster, v, NULL, caster->roomp, SPELL_PROTECTION_FROM_EARTH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;
}

int castProtectionFromEarth(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_PROTECTION_FROM_EARTH);
 
  int ret=protectionFromEarth(caster,victim,level,caster->getSkillValue(SPELL_PROTECTION_FROM_EARTH));
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}
