//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_water.h"
#include "obj_pool.h"
#include "obj_magic_item.h"
#include "combat.h"

int faerieFog(TBeing * caster, int, byte bKnown)
{
  TBeing *tmp_victim;
  TThing *t, *t2;

  if (caster->bSuccess(bKnown, SPELL_FAERIE_FOG)) {
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if ((caster != tmp_victim) && !tmp_victim->isImmortal()) {

        if (!caster->inGroup(*tmp_victim)) {
          if (tmp_victim->isAffected(AFF_INVISIBLE)) {
            if (tmp_victim->isLucky(caster->spellLuckModifier(SPELL_FAERIE_FOG))) {
              REMOVE_BIT(tmp_victim->specials.affectedBy, AFF_INVISIBLE);
              act("$n is briefly revealed, but disappears again.", TRUE, tmp_victim, NULL, NULL, TO_ROOM, ANSI_BLUE);
              act("You are briefly revealed, but disappear again.", TRUE, tmp_victim, NULL, NULL, TO_CHAR, ANSI_BLUE);
              SET_BIT(tmp_victim->specials.affectedBy, AFF_INVISIBLE);
            } else {
              if (tmp_victim->affectedBySpell(SPELL_INVISIBILITY))
                tmp_victim->affectFrom(SPELL_INVISIBILITY);
              else
                REMOVE_BIT(tmp_victim->specials.affectedBy, AFF_INVISIBLE);

              act("$n is revealed!", TRUE, tmp_victim, NULL, NULL, TO_ROOM, ANSI_BLUE);
              act("You are revealed!", TRUE, tmp_victim, NULL, NULL, TO_CHAR, ANSI_BLUE);
            }
          }
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int faerieFog(TBeing * caster)
{
  TThing *t, *t2;
  TBeing *victim;
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_FAERIE_FOG, NULL))
      return FALSE;

    lag_t rounds = discArray[SPELL_FAERIE_FOG]->lag;
    diff = discArray[SPELL_FAERIE_FOG]->task;

    start_cast(caster, NULL, NULL, caster->roomp, SPELL_FAERIE_FOG, diff, 1, "" , rounds, caster->in_room, 0, 0,TRUE, 0);

  for (t = caster->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    victim = dynamic_cast<TBeing *>(t);
    if (!victim)
      continue;
    if (!caster->inGroup(*victim) && !victim->isImmortal()) {
    }
  }
  return TRUE;
}

int castFaerieFog(TBeing * caster) 
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_FAERIE_FOG);
  int bKnown = caster->getSkillValue(SPELL_FAERIE_FOG);

  act("$n snaps $s fingers and a cloud of purple smoke billows forth!", TRUE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE);
  act("You snap your fingers and a cloud of purple smoke billows forth!", TRUE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE);

  if ((ret=faerieFog(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    act("Something went wrong -- your purple smoke quickly dissipates!", TRUE, caster, NULL, NULL, TO_CHAR, ANSI_PURPLE);
    act("The purple smoke quickly dissipates!", TRUE, caster, NULL, NULL, TO_ROOM, ANSI_PURPLE);
  }
  return TRUE;
}

int icyGrip(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  affectedData aff;

  if (victim->affectedBySpell(SPELL_ICY_GRIP)) {
    act("$N is already affected by icy grip.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_FAIL;
  }

  level = min(level, 25);

  int damage = caster->getSkillDam(victim, SPELL_ICY_GRIP, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_ICY_GRIP)) {
    aff.type = SPELL_ICY_GRIP;
    aff.duration = 12 * UPDATES_PER_MUDHOUR;
    aff.location = APPLY_STR;
    aff.modifier = -20;
    aff.bitvector = 0;
    aff.level = level;

    switch (critSuccess(caster, SPELL_ICY_GRIP)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ICY_GRIP);
        aff.duration *= 2;
      case CRIT_S_NONE:
        break;
    }

    if (victim->isLucky(caster->spellLuckModifier(SPELL_ICY_GRIP))) {
      SV(SPELL_ICY_GRIP);
      victim->sendTo("You were able to dodge one of the hands though!\n\r");
      aff.duration /= 2;
      aff.modifier = -10;
    }

    int immunity;

    if ((immunity = victim->getImmunity(getTypeImmunity(SPELL_ICY_GRIP)))) {
      aff.duration *= (100 - immunity);
      aff.duration /= 100;
    }

    victim->affectTo(&aff);
    act("You summon forth two ice-blue hands that rush toward $N and grab ahold of $M.", TRUE, caster, 0, victim, TO_CHAR, ANSI_BLUE);
    act("$n summons forth two ice-blue hands that rush quickly toward $N and grab ahold of $M.", TRUE, caster, 0, victim, TO_NOTVICT, ANSI_BLUE);
    act("$n summons forth two ice-blue hands that rush quickly toward you and grab ahold with a chilling grip.", TRUE, caster, 0, victim, TO_VICT, ANSI_BLUE);

    act("$N shakes violently as an icy grip drains the strength from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
    act("$N shakes violently as your icy grip drains the strength from $S body!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
    act("You shake violently as a deathly chill drains the strength from your body!", FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
    caster->reconcileHurt(victim, discArray[SPELL_ICY_GRIP]->alignMod);
    if (caster->reconcileDamage(victim, damage, SPELL_ICY_GRIP) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_ICY_GRIP)) {
      case CRIT_F_HITSELF:
        CF(SPELL_ICY_GRIP);
        act("$n's hands become frostbitten!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
        act("Your hands become frostbitten!  Man, that hurts!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
        act("For some reason, you feel a deep hostility towards $n.", FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
        if (caster->reconcileDamage(caster, damage,SPELL_ICY_GRIP) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        return SPELL_CRIT_FAIL;
        break;
      default:
        act("Your attempt to icily grip $N fails!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_CYAN);
        act("$n's attempt at icily gripping $N fails!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_CYAN);
        if (!victim->isPc()) {
          dynamic_cast<TMonster *>(victim)->addHated(caster);
        }
        break;
    }
    return SPELL_FAIL;
  }
}

int icyGrip(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=icyGrip(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int icyGrip(TBeing * caster, TBeing * victim)
{
  if (victim->affectedBySpell(SPELL_ICY_GRIP)) {
    act("$N is already affected by icy grip.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (!bPassMageChecks(caster, SPELL_ICY_GRIP, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_ICY_GRIP]->lag;
  taskDiffT diff = discArray[SPELL_ICY_GRIP]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_ICY_GRIP, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castIcyGrip(TBeing *caster, TBeing *victim)
{
  int ret = 0,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_ICY_GRIP);
  int bKnown = caster->getSkillValue(SPELL_ICY_GRIP);

  ret=icyGrip(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_ICY_GRIP));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    } else {
      act("For some reason, you feel a deep hostility towards $n.", FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      caster->nothingHappens();
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int wateryGrave(TBeing * caster, TBeing * victim, int level, byte bKnown, int)
{
  affectedData aff;

  if (victim->hasDisease(DISEASE_DROWNING)) {
    act("$N is already drowning.", TRUE,caster,0,victim,TO_CHAR, ANSI_BLUE);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (victim->isAffected(AFF_WATERBREATH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  level = min(level, 60);

  aff.type = AFFECT_DISEASE;
  aff.level = level/2;
  aff.duration = aff.level/2;
  aff.modifier = DISEASE_DROWNING;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  if (caster->bSuccess(bKnown, SPELL_WATERY_GRAVE)) {
    victim->affectTo(&aff);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    if (critFail(caster, SPELL_WATERY_GRAVE)) {
      CF(SPELL_WATERY_GRAVE);
      caster->affectTo(&aff);
      return SPELL_CRIT_FAIL;
    } 
    return SPELL_FAIL;
  }
}

int wateryGrave(TBeing * caster, TBeing * victim)
{
  if (victim->hasDisease(DISEASE_DROWNING)) {
    act("$N is already drowning.", TRUE,caster,0,victim,TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (victim->isAffected(AFF_WATERBREATH)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  if (!bPassMageChecks(caster, SPELL_WATERY_GRAVE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_WATERY_GRAVE]->lag;
  taskDiffT diff = discArray[SPELL_WATERY_GRAVE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_WATERY_GRAVE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castWateryGrave(TBeing * caster, TBeing * victim)
{
  int ret,level;

  caster->reconcileHurt(victim,discArray[SPELL_WATERY_GRAVE]->alignMod);

  level = caster->getSkillLevel(SPELL_WATERY_GRAVE);
  int bKnown = caster->getSkillLevel(SPELL_WATERY_GRAVE);

  ret=wateryGrave(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_WATERY_GRAVE));
  if (ret == SPELL_SUCCESS) {
    act("$n causes a globe of water to surround $N's head!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
    act("You cause a globe of water to surround $N's head!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
    act("$n causes a globe of water to surround your head!", FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
  } else {
    if (ret==SPELL_CRIT_FAIL) {
      act("$n causes a globe of water to surround $s own head!", FALSE, caster, NULL, victim, TO_ROOM, ANSI_BLUE_BOLD);
      act("You cause a globe of water to surround your own head!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE_BOLD);
    } else {
      caster->nothingHappens();
    }
  }
  return TRUE;
}

int arcticBlast(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  int rc = 0;
  TBeing *tmp_victim = NULL;
  TThing *t, *t2;

  level = min(level, 15);

  int damage = caster->getSkillDam(NULL, SPELL_ARCTIC_BLAST, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_ARCTIC_BLAST)) {
    caster->freezeRoom();

    switch (critSuccess(caster, SPELL_ARCTIC_BLAST)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ARCTIC_BLAST);
        damage *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if ((caster != tmp_victim) && !tmp_victim->isImmortal()) {

        if (!caster->inGroup(*tmp_victim)) {
          caster->reconcileHurt(tmp_victim, discArray[SPELL_ARCTIC_BLAST]->alignMod);

          act("$N can't escape the freezing cold -- $E's chilled to the bone!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
          act("$N can't escape the freezing cold -- $E's chilled to the bone!", FALSE, caster, NULL, tmp_victim, TO_CHAR);
          act("You can't escape the freezing cold -- you're chilled to the bone!", FALSE, caster, NULL, tmp_victim, TO_VICT);
          rc = tmp_victim->frostEngulfed();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }

          if (tmp_victim->isLucky(caster->spellLuckModifier(SPELL_ARCTIC_BLAST))) 
            damage /= 2;

          if (caster->reconcileDamage(tmp_victim, damage, SPELL_ARCTIC_BLAST) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
        }
      } else {
        act("You are able to avoid the cold!", 
                    TRUE, tmp_victim, NULL, NULL, TO_CHAR);
      }
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_ARCTIC_BLAST)) {
      case CRIT_F_HITSELF:
        CF(SPELL_ARCTIC_BLAST);
        act("The swirling ice and snow engulfs $n!",
            FALSE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE);
        act("Oops! The swirling ice and snow has engulfed you!",
            FALSE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE);
        rc = caster->frostEngulfed();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        if (caster->reconcileDamage(caster, damage, SPELL_ARCTIC_BLAST) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      default:
        act("The ice-storm melts quickly and simply gets everyone wet.",
               FALSE, caster, NULL, NULL, TO_CHAR);
        act("The ice-storm melts quickly and simply gets everyone wet.",
               FALSE, caster, NULL, NULL, TO_ROOM);
        break;
    }
    return SPELL_FAIL;
  }
}

int arcticBlast(TBeing * caster, TMagicItem * obj)
{
  int rc = 0;
  int ret = 0;

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You can't create an arctic blast underwater.\n\r");
    return FALSE;
  }

  act("A giant tornado of ice swirls from $p about you!", FALSE, caster, obj, NULL, TO_CHAR);
  act("$p unleashes a tornado of ice!", FALSE, caster, obj, NULL, TO_ROOM);

  ret = arcticBlast(caster,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);

  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int arcticBlast(TBeing * caster)
{
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You can't create an arctic blast underwater.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_ARCTIC_BLAST, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ARCTIC_BLAST]->lag;
  taskDiffT diff = discArray[SPELL_ARCTIC_BLAST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ARCTIC_BLAST, diff, 1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castArcticBlast(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_ARCTIC_BLAST);
  int bKnown = caster->getSkillValue(SPELL_ARCTIC_BLAST);

  act("A giant tornado of ice swirls about you!", FALSE, caster, NULL, NULL, TO_CHAR);
  act("$n unleashes a tornado of ice!", FALSE, caster, NULL, NULL, TO_ROOM);

  ret=arcticBlast(caster,level,bKnown, caster->getAdvLearning(SPELL_ARCTIC_BLAST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int iceStorm(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  int rc;
  int ret = 0;
  TBeing *tmp_victim = NULL;
  TThing *t, *t2;

  level = min(level, 33);

  int orig_damage = caster->getSkillDam(NULL, SPELL_ICE_STORM, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_ICE_STORM)) {
    switch (critSuccess(caster, SPELL_ICE_STORM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ICE_STORM);
        orig_damage *= 2;
        act("$n conjures a giant swirling storm of ice -- take cover!", 
            FALSE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE_BOLD);
        act("You conjure a giant swirling storm of ice!", 
            FALSE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE_BOLD);
        ret = SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        act("$n conjures a swirling storm of ice!", 
            FALSE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE_BOLD);
        act("You conjure a swirling storm of ice!", 
            FALSE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE_BOLD);
        ret = SPELL_SUCCESS;
        break;
    }
    caster->freezeRoom();
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if ((caster != tmp_victim) && !tmp_victim->isImmortal()) {

        if (!caster->inGroup(*tmp_victim)) {
          caster->reconcileHurt(tmp_victim, discArray[SPELL_ICE_STORM]->alignMod);
          int damage = orig_damage;

          if ((tmp_victim->isLucky(caster->spellLuckModifier(SPELL_ICE_STORM)))) {
            act("$N is able to dodge part of the ice storm!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT, ANSI_BLUE);
            act("$N is able to dodge part of the ice storm!", FALSE, caster, NULL, tmp_victim, TO_CHAR, ANSI_BLUE);
            act("You are able to dodge part of the ice storm!", FALSE, caster, NULL, tmp_victim, TO_VICT, ANSI_BLUE);
            damage /= 2;
          } else {
            act("$N is blasted by the ice!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT, ANSI_BLUE);
            act("$N is blasted by the ice!", FALSE, caster, NULL, tmp_victim, TO_CHAR, ANSI_BLUE);
            act("You are blasted by the ice!", FALSE, caster, NULL, tmp_victim, TO_VICT, ANSI_BLUE);
          }
          rc = tmp_victim->frostEngulfed();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
          if (caster->reconcileDamage(tmp_victim, damage, SPELL_ICE_STORM) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
        } else
          act("You are able to avoid the storm!", FALSE, caster, NULL, tmp_victim, TO_VICT, ANSI_BLUE);
      }
    }
    return ret;
  } else {
    switch (critFail(caster, SPELL_ICE_STORM)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_ICE_STORM);
        act("Something goes terribly, terribly wrong!!", 
            FALSE, caster, 0, 0, TO_CHAR, ANSI_BLUE);
        act("$n conjures an ice storm on top of $mself!", 
            FALSE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE_BOLD);
        act("You conjure an ice storm...right on your head!", 
            FALSE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE_BOLD);
        act("Your body feels frozen to the core!", 
            FALSE, caster, NULL, 0, TO_CHAR, ANSI_BLUE);
        act("$n causes $mself to be encased in a wall of cold air!", 
            FALSE, caster, NULL, 0, TO_ROOM, ANSI_BLUE);
        orig_damage = min(150, orig_damage);
        rc = caster->frostEngulfed();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        // backfire shouldn't do full damage
        if (caster->reconcileDamage(caster, orig_damage/2, SPELL_ICE_STORM) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
        break;
      default:
        caster->sendTo("You fail to summon an ice storm.\n\r");
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;
    }
  }
}

int iceStorm(TBeing * caster, TMagicItem * obj)
{
  int rc = 0;
  int ret = 0;

  ret = iceStorm(caster,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int iceStorm(TBeing * caster)
{
  if (!bPassMageChecks(caster, SPELL_ICE_STORM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ICE_STORM]->lag;
  taskDiffT diff = discArray[SPELL_ICE_STORM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ICE_STORM, diff, 1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castIceStorm(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_ICE_STORM);
  int bKnown = caster->getSkillValue(SPELL_ICE_STORM);

  ret=iceStorm(caster,level,bKnown, caster->getAdvLearning(SPELL_ICE_STORM));
  if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int tsunami(TBeing * caster, int level, byte bKnown, int adv_learn)
{
  TBeing *tmp_victim = NULL;
  TThing *t, *t2;

  level = min(level, 60);

  int orig_damage = caster->getSkillDam(NULL, SPELL_TSUNAMI, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_TSUNAMI)) {
    act("$n beckons forth a tidal wave!", 
         FALSE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE);
    act("You beckon forth a tidal wave!", 
         FALSE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE);
    caster->dropPool(100, LIQ_WATER);
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if ((caster != tmp_victim) && !tmp_victim->isImmortal()) {

        if (!caster->inGroup(*tmp_victim)) {
          caster->reconcileHurt(tmp_victim, discArray[SPELL_TSUNAMI]->alignMod);
          int damage = orig_damage;

          if ((tmp_victim->isLucky(caster->spellLuckModifier(SPELL_TSUNAMI)))) {
            act("$N learns how to surf, really quickly!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT, ANSI_BLUE);
            act("$N learns how to surf, really quickly!", FALSE, caster, NULL, tmp_victim, TO_CHAR, ANSI_BLUE);
            act("You learn how to surf, really quickly!", FALSE, caster, NULL, tmp_victim, TO_VICT, ANSI_BLUE);
            damage /= 2;
          } else {
            act("$N is leveled by the wave!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT, ANSI_BLUE);
            act("$N is leveled by the wave!", FALSE, caster, NULL, tmp_victim, TO_CHAR, ANSI_BLUE);
            act("You are leveled by the wave!", FALSE, caster, NULL, tmp_victim, TO_VICT, ANSI_BLUE);
            tmp_victim->setPosition(POSITION_SITTING);
          }
          if (caster->reconcileDamage(tmp_victim, damage, SPELL_TSUNAMI) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
          }
        } 
      }
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_TSUNAMI)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_TSUNAMI);
          act("Something goes terribly, terribly wrong!!", FALSE, caster, 0, 0, TO_CHAR, ANSI_WHITE);
          act("Your body is buffetted by the Tsunami you have summoned!", FALSE, caster, NULL, 0, TO_CHAR, ANSI_BLUE);
          act("$n is buffetted by a large wave summoned by $e own magic!", FALSE, caster, NULL, 0, TO_ROOM, ANSI_BLUE);
          orig_damage /= 3;

          if (caster->reconcileDamage(caster, orig_damage, SPELL_TSUNAMI) == -1)
            return SPELL_CRIT_FAIL + CASTER_DEAD;
          return SPELL_CRIT_FAIL;
          break;
      default:
        act("Your spell fails and you are unable to summon a Tsunami.", TRUE, caster, 0, 0, TO_CHAR);
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;
    }
  }
}

int tsunami(TBeing * caster)
{
  if (!bPassMageChecks(caster, SPELL_TSUNAMI, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_TSUNAMI]->lag;  
  taskDiffT diff = discArray[SPELL_TSUNAMI]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_TSUNAMI, diff, 1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castTsunami(TBeing * caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_TSUNAMI);;
  int bKnown = caster->getSkillLevel(SPELL_TSUNAMI);;

  ret=tsunami(caster,level,bKnown, caster->getAdvLearning(SPELL_TSUNAMI));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// *** dunno where this goes, this is an npc spell.
void spell_geyser(byte level, TBeing *ch, TBeing *, int)
{
  if (ch->in_room < 0)
    return;
  act("The Geyser erupts in a huge column of steam!", FALSE, ch, 0, 0, TO_ROOM);

#if 0
  int dam = dice(level, 3);

  for (tmp_victim = ch->roomp->people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if ((ch != tmp_victim) && ch->sameRoom(*tmp_victim)) {
      if ((tmp_victim->GetMaxLevel() < LOW_IMMORTAL) || (tmp_victim->isNpc())) {
        ch->getActualDamage(tmp_victim, NULL, dam, SPELL_GEYSER);
        act("You are seared by the boiling water!!", FALSE, ch, 0, tmp_victim, TO_VICT);
      } else {
        act("You are almost seared by the boiling water!!",
                FALSE, ch, 0, tmp_victim, TO_VICT);
      }
    }
  }
#endif
}

int conjureElemWater(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(WATER_ELEMENTAL, VIRTUAL))) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CONJURE_WATER, 0);

  if (caster->bSuccess(bKnown, SPELL_CONJURE_WATER)) {
    act("You summon the powers of the ocean!", 
            TRUE, caster, NULL, NULL, TO_CHAR, ANSI_BLUE_BOLD);
    act("$n summons the powers of the ocean!", 
            TRUE, caster, NULL, NULL, TO_ROOM, ANSI_BLUE_BOLD);

    /* charm them for a while */
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CONJURE_WATER;
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
    restrict_xp(caster, victim, aff.duration);

    /* Add hp for higher levels - Russ */
    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CONJURE_WATER)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CONJURE_WATER);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim, TO_ROOM, ANSI_BLUE_BOLD);
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
      victim->affectFrom(SPELL_CONJURE_WATER);
      victim->affectFrom(AFFECT_THRALL);
      return SPELL_FAIL;
    }
    caster->addFollower(victim);
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("When your eyes recover, you see $N standing before you!", TRUE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int conjureElemWater(TBeing * caster)
{
  TThing *t;
  int found=0;
  TPool *tp;
  TBaseCup *tbc;

  if (real_mobile(WATER_ELEMENTAL) < 0) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_CONJURE_WATER, NULL))
    return FALSE;

  if(caster->roomp->isWaterSector() || caster->roomp->isUnderwaterSector() ||
     caster->roomp->getWeather() == WEATHER_RAINY){
    found=1;
  } else {
    for(t = caster->roomp->getStuff(); t; t = t->nextThing) {
      if ((tp = dynamic_cast<TPool *>(t)) && tp->getDrinkUnits() >= 100 &&
	  (tp->getDrinkType() == LIQ_WATER ||
	   tp->getDrinkType() == LIQ_SALTWATER ||
	   tp->getDrinkType() == LIQ_HOLYWATER)) {
	tp->addToDrinkUnits(-100);
	found=1;
	break;
      }

      if ((tbc = dynamic_cast<TBaseCup *>(t)) && 
	  tbc->getDrinkUnits() >= 100 &&
	  (tbc->getDrinkType() == LIQ_WATER ||
	   tbc->getDrinkType() == LIQ_SALTWATER ||
	   tbc->getDrinkType() == LIQ_HOLYWATER)) {
	tbc->addToDrinkUnits(-100);
	found=1;
	break;
      }

      if (t->spec == SPEC_FOUNTAIN) {
        found = 1;
        break;
      }
    }
  }

  if (!found) {
    caster->sendTo("There doesn't seem to be enough water around to conjure a water elemental.\n\r"); 
    return FALSE;
  }


  lag_t rounds = discArray[SPELL_CONJURE_WATER]->lag;
  taskDiffT diff = discArray[SPELL_CONJURE_WATER]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CONJURE_WATER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castConjureElemWater(TBeing * caster)
{
  int ret,level; 

  level = caster->getSkillLevel(SPELL_CONJURE_WATER);
  int bKnown = caster->getSkillValue(SPELL_CONJURE_WATER);

  if ((ret=conjureElemWater(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    act("Hmmm...that didn't feel quite right.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

static bool canBeGilled(TBeing *caster, TBeing *victim)
{
  // casting on natural waterbreathers causes natural ability to
  // be lost when spell decays
  // but do allow it to be multiply cast (increase duration)
  if (victim->isAffected(AFF_WATERBREATH) &&
      !(victim->affectedBySpell(SPELL_GILLS_OF_FLESH) ||
       victim->affectedBySpell(SPELL_BREATH_OF_SARAHAGE))) {
    if (caster != victim)
      act("$N already has the ability to breathe underwater.",
          FALSE, caster, NULL, victim, TO_CHAR);
    else
      act("You already have the ability to breathe underwater.",
          FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return true;
  }
  return false;
}

int gillsOfFlesh(TBeing * caster, TBeing * victim, int level, byte bKnown)
{
  affectedData aff;

  if (canBeGilled(caster, victim))
    return FALSE;

  if (caster->bSuccess(bKnown, SPELL_GILLS_OF_FLESH)) {

    caster->reconcileHelp(victim,discArray[SPELL_GILLS_OF_FLESH]->alignMod);
    aff.type = SPELL_GILLS_OF_FLESH;
    aff.level = level;
    aff.duration = 6 * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.renew = aff.duration;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_WATERBREATH;

    switch (critSuccess(caster, SPELL_GILLS_OF_FLESH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_GILLS_OF_FLESH);
        aff.duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_NO)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("You become one with the fishes!", TRUE, victim, NULL, NULL, TO_CHAR, ANSI_BLUE);
    act("$N makes a face like a fish.", TRUE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
    if (victim != caster)
      act("You bestow upon $N the ability to breathe water!",
                TRUE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void gillsOfFlesh(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  gillsOfFlesh(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int gillsOfFlesh(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

  if (canBeGilled(caster, victim))
    return FALSE;

  if (!bPassMageChecks(caster, SPELL_GILLS_OF_FLESH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_GILLS_OF_FLESH]->lag;
  diff = discArray[SPELL_GILLS_OF_FLESH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_GILLS_OF_FLESH, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castGillsOfFlesh(TBeing * caster, TBeing * victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_GILLS_OF_FLESH);
  int bKnown= caster->getSkillValue(SPELL_GILLS_OF_FLESH);

  if ((ret=gillsOfFlesh(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int breathOfSarahage(TBeing * caster, int level, byte bKnown)
{
  TBeing *tmp_victim = NULL;
  affectedData aff;

  if (caster->bSuccess(bKnown, SPELL_BREATH_OF_SARAHAGE)) {
    aff.type = SPELL_GILLS_OF_FLESH;
    aff.level = level;
    aff.duration = 6 * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_WATERBREATH;
    TThing *t;
    int found = FALSE;
    for (t = caster->roomp->getStuff(); t; t = t->nextThing) {
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if (caster->inGroup(*tmp_victim)) {
        if (!tmp_victim->isAffected(AFF_WATERBREATH)) {
          caster->reconcileHelp(tmp_victim,discArray[SPELL_BREATH_OF_SARAHAGE]->alignMod);
          act("$n makes a face like a fish.", TRUE, tmp_victim, NULL, NULL, TO_ROOM, ANSI_BLUE_BOLD);
          act("You make a face like a fish.", TRUE, tmp_victim, NULL, NULL, TO_CHAR, ANSI_BLUE_BOLD);

          tmp_victim->affectTo(&aff);
          found = TRUE;
        }
      }
    }
    if (!found)
      caster->sendTo("But, there's nobody in your group.\n\r");
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int breathOfSarahage(TBeing * caster)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_BREATH_OF_SARAHAGE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_BREATH_OF_SARAHAGE]->lag;
  diff = discArray[SPELL_BREATH_OF_SARAHAGE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_BREATH_OF_SARAHAGE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castBreathOfSarahage(TBeing * caster)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_BREATH_OF_SARAHAGE);
  int bKnown = caster->getSkillValue(SPELL_BREATH_OF_SARAHAGE);

  if ((ret=breathOfSarahage(caster,level,bKnown)) == SPELL_SUCCESS) {
    caster->sendTo("You exhale a blue-green vapor around your group.\n\r");
    act("$n breaths forth a blue-green mist that surrounds $s group.", TRUE,caster,0,0,TO_ROOM, ANSI_GREEN);
  } else
    caster->nothingHappens();
  return TRUE;
}

int protectionFromWater(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  aff.type = SPELL_PROTECTION_FROM_WATER;
  aff.level = level;
  aff.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_WATER;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;
 
  if (caster->bSuccess(bKnown,SPELL_PROTECTION_FROM_WATER)) {
    act("$n glows with a faint blue-green aura for a brief moment.", FALSE, victim, NULL, NULL, TO_ROOM, ANSI_GREEN);
    act("You glow with a faint blue-green aura for a brief moment.", FALSE, victim, NULL, NULL, TO_CHAR, ANSI_GREEN);
    switch (critSuccess(caster, SPELL_PROTECTION_FROM_WATER)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_PROTECTION_FROM_WATER);
        aff.duration = (10 + (level / 2)) * UPDATES_PER_MUDHOUR;
        aff.modifier2 = (level * 2);
        break;
      case CRIT_S_NONE:
        break;
    }
 
    if (caster != victim) 
      aff.modifier2 /= 2;
 
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    caster->reconcileHelp(victim, discArray[SPELL_PROTECTION_FROM_WATER]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
void protectionFromWater(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  protectionFromWater(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int protectionFromWater(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_PROTECTION_FROM_WATER, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_PROTECTION_FROM_WATER]->lag;
  taskDiffT diff = discArray[SPELL_PROTECTION_FROM_WATER]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_PROTECTION_FROM_WATER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castProtectionFromWater(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_PROTECTION_FROM_WATER);
  int bKnown = caster->getSkillValue(SPELL_PROTECTION_FROM_WATER);
 
  int ret=protectionFromWater(caster,victim,level,bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}
 
int gusher(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  int rc;
  TThing *t;

  level = min(level, 10);

  int dam = caster->getSkillDam(victim, SPELL_GUSHER, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_GUSHER)) {
    caster->reconcileHurt(victim,discArray[SPELL_GUSHER]->alignMod);

    if ((critSuccess(caster, SPELL_GUSHER) ||
        critSuccess(caster, SPELL_GUSHER) ||
        critSuccess(caster,SPELL_GUSHER)) &&
        !caster->isNotPowerful(victim, level, SPELL_GUSHER, SILENT_YES)) {
      CS(SPELL_GUSHER);
      dam *= 2;
      act("A HUGE stream of water smacks into $N knocking $M over!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a HUGE stream of water at you, knocking you down!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a HUGE stream of water in $N's direction, knocking $M over!",
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
    } else if (victim->isLucky(caster->spellLuckModifier(SPELL_GUSHER))) {

      act("A tiny stream of water smacks into $N!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a tiny stream of water in your direction!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a tiny stream of water in $N's direction!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
      victim->dropPool(10, LIQ_WATER);

      SV(SPELL_GUSHER);
      dam /= 2;
    } else {
      act("A stream of water smacks into $N!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_BLUE);
      act("$n directs a stream of water in your direction!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_BLUE);
      act("$n directs a stream of water in $N's direction!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_BLUE);
      victim->dropPool(25, LIQ_WATER);
    }
    if (caster->reconcileDamage(victim, dam, SPELL_GUSHER) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("$n just tried to attack you.", FALSE, caster, 0, victim, TO_VICT, ANSI_BLUE);
    if (critFail(caster, SPELL_GUSHER) == CRIT_F_HITSELF) {
      CF(SPELL_GUSHER);
      act("You call forth a stream of water, but it leaves you all wet!",
           FALSE, caster, NULL, 0, TO_CHAR, ANSI_BLUE);
      act("$n calls forth a stream of water, but it leaves $m all wet!",
           FALSE, caster, NULL, 0, TO_ROOM, ANSI_BLUE);
      caster->dropPool(10, LIQ_WATER);
      if (caster->reconcileDamage(caster, dam, SPELL_GUSHER) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int gusher(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

   if (!bPassMageChecks(caster, SPELL_GUSHER, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_GUSHER]->lag;
    diff = discArray[SPELL_GUSHER]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_GUSHER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

      return TRUE;
}

int castGusher(TBeing * caster, TBeing * victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_GUSHER);
  int bKnown = caster->getSkillValue(SPELL_GUSHER);

  ret=gusher(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_GUSHER));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int gusher(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret=gusher(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

CDWater::CDWater() :
  CDiscipline(),
  skWateryGrave(),
  skTsunami(),
  skBreathOfSarahage(),
  skPlasmaMirror()
{
}

CDWater::CDWater(const CDWater &a) :
  CDiscipline(a),
  skWateryGrave(a.skWateryGrave),
  skTsunami(a.skTsunami),
  skBreathOfSarahage(a.skBreathOfSarahage),
  skPlasmaMirror(a.skPlasmaMirror)
{
}

CDWater & CDWater::operator=(const CDWater &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skWateryGrave = a.skWateryGrave;
  skTsunami = a.skTsunami;
  skBreathOfSarahage = a.skBreathOfSarahage;
  skPlasmaMirror = a.skPlasmaMirror;
  return *this;
}

CDWater::~CDWater()
{
}

CDWater * CDWater::cloneMe()
{
  return new CDWater(*this);
}

int plasmaMirror(TBeing *caster)
{
  if (caster->affectedBySpell(SPELL_PLASMA_MIRROR)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  if (!bPassMageChecks(caster, SPELL_PLASMA_MIRROR, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_PLASMA_MIRROR]->lag;
  taskDiffT diff = discArray[SPELL_PLASMA_MIRROR]->task;

  start_cast(caster, caster, NULL, caster->roomp, SPELL_PLASMA_MIRROR, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castPlasmaMirror(TBeing *caster)
{
  int ret,level;

  if (caster && caster->affectedBySpell(SPELL_PLASMA_MIRROR)) {
    act("You already possess a plasma mirror.",
           FALSE, caster, NULL, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_PLASMA_MIRROR);
  int bKnown = caster->getSkillValue(SPELL_PLASMA_MIRROR);

  ret=plasmaMirror(caster,level,bKnown);

  return ret;
}

int plasmaMirror(TBeing *caster, int level, byte bKnown)
{
  affectedData aff;

  // this gets checked for before here, but just for security
  if (caster->affectedBySpell(SPELL_PLASMA_MIRROR)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  aff.type = SPELL_PLASMA_MIRROR;
  aff.duration = max(min(level/10, 5), 1) * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = level;

  if (caster->bSuccess(bKnown, SPELL_PLASMA_MIRROR)) {
    switch (critSuccess(caster, SPELL_PLASMA_MIRROR)) {
      case CRIT_S_KILL:
        CS(SPELL_PLASMA_MIRROR);
        aff.duration *= 2;
        act("Great swirls of plasma swirl FIERCELY about you!",
            FALSE, caster, 0, 0, TO_CHAR, ANSI_GREEN);
        act("Great swirls of plasma swirl FIERCELY about $n!",
            FALSE, caster, 0, 0, TO_ROOM, ANSI_GREEN);
        break;
      default:
        act("Great swirls of plasma swirl about you!",
            FALSE, caster, 0, 0, TO_CHAR, ANSI_GREEN);
        act("Great swirls of plasma swirl about $n!",
            FALSE, caster, 0, 0, TO_ROOM, ANSI_GREEN);
        break;
    }
    caster->affectTo(&aff, -1);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int garmulsTail(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_GARMULS_TAIL, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_GARMULS_TAIL]->lag;
  taskDiffT diff = discArray[SPELL_GARMULS_TAIL]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_GARMULS_TAIL, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castGarmulsTail(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_GARMULS_TAIL);
  int bKnown = caster->getSkillValue(SPELL_GARMULS_TAIL);

  int ret=garmulsTail(caster,victim,level,bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

void garmulsTail(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  int ret = garmulsTail(caster, victim, obj->getMagicLevel(), obj->getMagicLearnedness());
  if (ret == SPELL_SUCCESS) {
  } else {
  }
}

int garmulsTail(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_GARMULS_TAIL]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_GARMULS_TAIL)) {
    aff.type = SPELL_GARMULS_TAIL;
    aff.level = level;
    aff.duration = (aff.level / 3) * UPDATES_PER_MUDHOUR;
    aff.location = APPLY_SPELL;
    aff.modifier = SKILL_SWIM;
    aff.modifier2 = bKnown/2;
    aff.bitvector = 0;

    switch (critSuccess(caster, SPELL_GARMULS_TAIL)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_GARMULS_TAIL);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("$N moves with greater fluidity!",
           FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Your movement is much more fluid now!",
           FALSE, victim, NULL, NULL, TO_CHAR);
    if (caster != victim)
      act("You have given $N the gift of fluid motion!",
           FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
