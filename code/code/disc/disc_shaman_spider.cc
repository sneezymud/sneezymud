/*******************************************************************
 *                                                                 *
 *                                                                 *
 *   disc_shaman_spider.cc             Shaman Discipline Disc      *
 *                                                                 *
 *   SneezyMUD Development - All Rights Reserved                   *
 *                                                                 *
 *******************************************************************/
 
#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_shaman_spider.h"
#include "obj_magic_item.h"
#include "combat.h"

int transfix(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  int dif, rc;
  affectedData aff;

#if 0
  if (!bPassMageChecks(caster,SKILL_TRANSFIX, victim))
    return SPELL_FAIL;
#else
  if (!caster->useComponent(caster->findComponent(SKILL_TRANSFIX), victim, CHECK_ONLY_NO))
    return FALSE;
#endif

  if (victim->affectedBySpell(SKILL_TRANSFIX)) {
    act("$N doesn't look any more transfixed than before.", FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (!victim->awake()) {
    act("$N looks sound asleep.  Oops!", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (victim->fight() || !victim->isDumbAnimal()) {
    act("$N pays no attention to you.", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  dif = level - victim->GetMaxLevel();

  if (caster->bSuccess(bKnown, caster->getPerc(), SKILL_TRANSFIX) && 
           !victim->isLucky(caster->spellLuckModifier(SKILL_TRANSFIX))) {
    aff.type = SKILL_TRANSFIX;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.duration = (level/10)* UPDATES_PER_MUDHOUR;
    aff.bitvector = 0;

    if (critSuccess(caster, SKILL_TRANSFIX)) {
      CS(SKILL_TRANSFIX);
      aff.duration *= 2;
      rc = SPELL_CRIT_SUCCESS;
    } else
      rc = SPELL_SUCCESS;

    act("$N stares transfixed into $n's eyes.", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N stares transfixed into your eyes.", FALSE, caster, NULL, victim, TO_CHAR);
    act("Aren't $n's eyes beautiful?", FALSE, caster, NULL, victim, TO_VICT);
    victim->affectTo(&aff);
    return rc;
  } else {
    if (critFail(caster, SKILL_TRANSFIX)) {
      CF(SKILL_TRANSFIX);
      act("$N turns towards $n, infuriated!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("$N turns towards you, infuriated!", FALSE, caster, NULL, victim, TO_CHAR);
      act("For some reason, you feel a deep hostility towards $n.", FALSE, caster, NULL, victim, TO_VICT);
      if (!caster->checkPeaceful("") && !victim->isPc()) {
        if ((rc = victim->hit(caster)) == DELETE_VICT) {
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        } else if (rc == DELETE_THIS) {
          return SPELL_CRIT_FAIL + VICTIM_DEAD;
        }
      }
      return SPELL_CRIT_FAIL;
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    }
  }
  return SPELL_FAIL;
}

// LIVING VINES

int livingVines(TBeing *caster, TBeing *victim,int level, short bKnown)
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
   
  if (caster->bSuccess(bKnown, SPELL_LIVING_VINES)) {
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

// END LIVING VINES

int rootControl(TBeing *caster, TBeing *victim, TMagicItem *tObj)
{
  int tLevel = tObj->getMagicLevel(),
    tKnown = tObj->getMagicLearnedness(),
    tReturn = 0;

  tReturn = rootControl(caster, victim, 0, tLevel, tKnown);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}

int rootControl(TBeing * caster, TBeing * victim, int, int dam, short bKnown)
{
  dam = max(1,dam);
 
  if (!victim) {
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (victim->getPosition() <= POSITION_SLEEPING) {
    act("$N seems to already be on the $g!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->roomp->notRangerLandSector() && !caster->roomp->isForestSector()) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    caster->sendTo("You need to be in nature or on land to cast this spell!\n\r");
    return SPELL_FAIL;
  }
  caster->reconcileHurt(victim,discArray[SPELL_ROOT_CONTROL]->alignMod);
 
  act("Large tree roots start to form in front of $N.", FALSE, caster , NULL, victim, TO_CHAR);
  act("Large tree roots start to form in front of $N.", TRUE, caster , NULL, victim, TO_NOTVICT);
  act("Large tree roots start to form in front of you.", TRUE, caster , NULL, victim, TO_VICT);

  if (caster->bSuccess(bKnown,SPELL_ROOT_CONTROL)) {
    act("$n commands the tree roots to trip $N, causing $M to fall to the $g.", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n commands the tree roots to trip you!", FALSE, caster, NULL, victim, TO_VICT);
    act("You command the tree roots to trip $N, causing $M to fall to the $g.", FALSE, caster, NULL, victim, TO_CHAR);
    if (victim->isLucky(caster->spellLuckModifier(SPELL_ROOT_CONTROL))) {
      SV(SPELL_ROOT_CONTROL);
      dam /= 2;
    } else if (critSuccess(caster, SPELL_ROOT_CONTROL) == CRIT_S_DOUBLE) {
      CS(SPELL_ROOT_CONTROL);
      dam *= 2;
    }
    victim->setPosition(POSITION_SITTING);
    victim->addToWait(combatRound(1));
    if (caster->reconcileDamage(victim, dam, SPELL_ROOT_CONTROL) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_ROOT_CONTROL) == CRIT_F_HITSELF) {
      CF(SPELL_ROOT_CONTROL);
      act("The roots turn on you, and cause you to trip!", FALSE, caster, NULL, victim, TO_CHAR);
      act("The roots that $n summoned turn against $m and cause $m to trip!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("The roots $n summoned to aid $m, trip $m instead!", FALSE, caster, NULL, victim, TO_VICT);
      caster->setPosition(POSITION_SITTING);
      if (caster->reconcileDamage(caster, dam, SPELL_ROOT_CONTROL) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_CRIT_FAIL;
    } else {
      act("The roots you summoned ignore you!", FALSE, caster, NULL, victim, TO_CHAR);
      act("The roots that $n summoned ignore $s commands.", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("The roots ignore $n's request to trip you.", FALSE, caster, NULL, caster, TO_VICT);
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_FAIL;
    }
  }
}
 
int rootControl(TBeing * caster, TBeing * victim)
{
  int ret;
  int rc = 0;
 
  if (!bPassMageChecks(caster, SPELL_ROOT_CONTROL, victim))
    return FALSE;

  int level = caster->getSkillLevel(SPELL_ROOT_CONTROL);
  int bKnown = caster->getSkillValue(SPELL_ROOT_CONTROL);
  int dam = caster->getSkillDam(victim, SPELL_ROOT_CONTROL, level, caster->getAdvLearning(SPELL_ROOT_CONTROL));
 
  ret=rootControl(caster,victim,level,dam,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}
 



int TBeing::doTransfix(const char *argument)
{
  int level, ret;
  int rc = 0;
  TBeing *victim = NULL;
  char namebuf[256];

  if (!doesKnowSkill(SKILL_TRANSFIX)) {
    sendTo("You know nothing about transfixing beasts.");
    return FALSE;
  }

  strcpy(namebuf, argument);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Transfix what?\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  level = getSkillLevel(SKILL_TRANSFIX);

  ret=transfix(this,victim,level,getSkillValue(SKILL_TRANSFIX));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int transfix(TBeing * caster, TBeing * victim)
{
  int rc = 0;
  int level, ret;

  level = caster->getSkillLevel(SKILL_TRANSFIX);

  ret=transfix(caster,victim,level,caster->getSkillValue(SKILL_TRANSFIX));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// STICKS TO SNAKES

int sticksToSnakes(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  TMonster *snake;
  affectedData aff; 
  int rc, mobile;
  followData *k, *n;

  if (level < 26) {
     mobile = Mob::Mob::SNAKES25;
  } else if (level < 31) {
     mobile = Mob::Mob::SNAKES30;
  } else if (level < 36) {
     mobile = Mob::Mob::SNAKES35;
  } else if (level < 41) {
     mobile = Mob::Mob::SNAKES40;
  } else {
     mobile = Mob::Mob::SNAKES50;
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

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_STICKS_TO_SNAKES)) {
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

    snake->setLevel(WARRIOR_LEVEL_IND, (byte)lvl);
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
    snake->setLevel(WARRIOR_LEVEL_IND, (byte)lvl);
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
    *snake->roomp += *read_object(Obj::INERT_STICK, VIRTUAL);
    delete snake;
    snake = NULL;
    return SPELL_FAIL;
  }
}

void sticksToSnakes(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  sticksToSnakes(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int sticksToSnakes(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassShamanChecks(caster, SPELL_STICKS_TO_SNAKES, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_STICKS_TO_SNAKES]->lag;
     diff = discArray[SPELL_STICKS_TO_SNAKES]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_STICKS_TO_SNAKES, diff, 1, "", 
rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castSticksToSnakes(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_STICKS_TO_SNAKES);
  int bKnown = caster->getSkillValue(SPELL_STICKS_TO_SNAKES);

  ret=sticksToSnakes(caster,victim,level,bKnown);
    return TRUE;
}

// END STICKS TO SNAKES
// CONTROL UNDEAD

int controlUndead(TBeing *caster,TBeing *victim,int level,short bKnown)
{
  affectedData aff;
  char buf[256];

  if (!victim->isUndead()) {
    caster->sendTo("Umm...that's not an undead creature. You can't charm it.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  } 

  if (caster->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't charm $N -- you're busy taking orders yourself!");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }

#if 0
  // all undead are immune charm, bypass this
  if (victim->isAffected(AFF_CHARM)) {
    again = (victim->master == caster);
    sprintf(buf, "You can't charm $N%s -- $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }
#endif

  if (caster->tooManyFollowers(victim, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM);
    return SPELL_FAIL;
  }

  if (victim->circleFollow(caster)) {
    caster->sendTo("Umm, you probably don't want to follow each other around in circles.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  // note: no charm immune check : undead always immune
  if ((!victim->isPc() && dynamic_cast<TMonster *>(victim)->Hates(caster, NULL)) ||
       caster->isNotPowerful(victim, level, SPELL_CONTROL_UNDEAD, SILENT_YES)) {

      act("Something went wrong!  All you did was piss $N off!", 
                FALSE, caster, NULL, victim, TO_CHAR);
      act("Nothing seems to happen.", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("$n just tried to charm you!", FALSE, caster, NULL, victim, TO_VICT);
      victim->failCharm(caster);
      return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_CONTROL_UNDEAD)) {
    if (victim->master)
      victim->stopFollower(TRUE);
    caster->addFollower(victim);

    aff.type = SPELL_CONTROL_UNDEAD;
    aff.level = level;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime(); 
    aff.duration = (int) (caster->percModifier() * aff.duration);

    switch (critSuccess(caster, SPELL_CONTROL_UNDEAD)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_CONTROL_UNDEAD);
        aff.duration *= 2;
        break;
      case CRIT_S_TRIPLE:
        CS(SPELL_CONTROL_UNDEAD);
        aff.duration *= 3;
        break;
      default:
        break;
    } 
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this skill
    // this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, aff.duration);

    if (!victim->isPc())
      dynamic_cast<TMonster *>(victim)->genericCharmFix();

    act("You feel an overwhelming urge to follow $n!",
            FALSE, caster, NULL, victim, TO_VICT);
    act("You decide to do whatever $e says!",
            FALSE, caster, NULL, victim, TO_VICT);
    act("$N has become charmed by $n!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N is now controlled by you.", 0, caster, 0, victim, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    act("Something went wrong!", FALSE, caster, NULL, victim, TO_CHAR);
    act("All you did was piss $N off!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    act("$n just tried to charm you!", FALSE, caster, NULL, victim, TO_VICT);
    victim->failCharm(caster);
    return SPELL_FAIL;
  }
}

void controlUndead(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  controlUndead(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int castControlUndead(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_CONTROL_UNDEAD);
  int bKnown = caster->getSkillValue(SPELL_CONTROL_UNDEAD);

  ret=controlUndead(caster,victim,level,bKnown);
    return TRUE;
}

int controlUndead(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassShamanChecks(caster, SPELL_CONTROL_UNDEAD, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_CONTROL_UNDEAD]->lag;
     diff = discArray[SPELL_CONTROL_UNDEAD]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_CONTROL_UNDEAD, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

// END CONTROL UNDEAD
// START CLARITY

int clarity(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_CLARITY]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_CLARITY)) {
    aff.type = SPELL_CLARITY;
    aff.duration = 6+level / 3 * UPDATES_PER_MUDHOUR;
    aff.modifier = 10;
    aff.location = APPLY_VISION;
    aff.bitvector = AFF_CLARITY;

    switch (critSuccess(caster, SPELL_CLARITY)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CLARITY);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    victim->sendTo("Your eyes flash.\n\r");
    act("$n's eyes glow <G>green<1>.", FALSE, victim, 0, 0, TO_ROOM);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void clarity(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  clarity(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int clarity(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassShamanChecks(caster, SPELL_CLARITY, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_CLARITY]->lag;
     diff = discArray[SPELL_CLARITY]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_CLARITY, diff, 1, "", 
rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castClarity(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_CLARITY);
  int bKnown = caster->getSkillValue(SPELL_CLARITY);

  ret=clarity(caster,victim,level,bKnown);
    return TRUE;
}
// END CLARITY

int hypnosis(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;
  int again;
  char buf[256];

  if (victim == caster) {
    sprintf(buf, "Doing this to yourself? Why?");
    act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (victim->isLinkdead()) {
    act("Cant do that to someone who is linkdead.", FALSE, caster, 0, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't hypnotize $N while under the same affects.");
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }

  if (victim->isAffected(AFF_CHARM)) {
    again = (victim->master == caster);
    sprintf(buf, "You can't hypnotize $N%s while $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }

  if (caster->tooManyFollowers(victim, FOL_CHARM)) {
    act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, 
TO_CHAR, ANSI_RED_BOLD);
    act("$N refuses to enter $ group the size of $n's!", TRUE, caster, NULL, victim, 
TO_ROOM, ANSI_RED_BOLD);
    return SPELL_FAIL;
  }

  if (victim->circleFollow(caster)) {
    caster->sendTo("Umm, you probably don't want to follow each other around in circles.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
#if 0
  if (!victim->isPc()) {
    caster->sendTo("You can't hypnotize that.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
#endif
  if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY, level) || victim->GetMaxLevel() > caster->GetMaxLevel() ||
      (!victim->isPc() && dynamic_cast<TMonster *>(victim)->Hates(caster, NULL)) ||
      caster->isNotPowerful(victim, level, SPELL_HYPNOSIS, SILENT_YES) ||
      (victim->isLucky(caster->spellLuckModifier(SPELL_HYPNOSIS)))) {

      victim->failCharm(caster);
      act("You have failed in this important ritual! Your victim is immune!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED_BOLD);
      caster->nothingHappens(SILENT_YES);
      act("$n just tried to hypnotize you!", FALSE, caster, NULL, victim, TO_VICT, 
ANSI_RED_BOLD);
      return SPELL_FAIL;
  }

  caster->reconcileHurt(victim,discArray[SPELL_HYPNOSIS]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_HYPNOSIS)) {
    if (victim->master)
      victim->stopFollower(TRUE);
    caster->addFollower(victim);

    aff.type = SPELL_HYPNOSIS;
    aff.level = level;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    aff.duration  =  2 * level * UPDATES_PER_MUDHOUR;

    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_CHARM));
    aff.duration /= 100;

    switch (critSuccess(caster, SPELL_HYPNOSIS)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_HYPNOSIS);
        aff.duration *= 2;
        break;
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
        CS(SPELL_HYPNOSIS);
        aff.duration *= 3;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_HYPNOSIS))) {
          SV(SPELL_HYPNOSIS);
          aff.duration /= 2;
        } 
        break;
    } 

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    aff.type = AFFECT_CHARM;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this skill
    // this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, aff.duration);

    if (!victim->isPc())
      dynamic_cast<TMonster *>(victim)->genericCharmFix();

    // don't hurt the one you love
    if (victim->fight() == caster && caster->fight())
      caster->stopFighting();

    // and don't let the charm hurt anyone that we didn't order them to hurt
    if (victim->fight())
      victim->stopFighting();

    return SPELL_SUCCESS;
  } else {
    act("You have invoked this important ritual incorrectly!", FALSE, caster, NULL, 
victim, TO_CHAR, ANSI_RED_BOLD);
    act("$n just tried to hypnotize you!", FALSE, caster, NULL, victim, TO_VICT, 
ANSI_RED_BOLD);
    caster->nothingHappens(SILENT_YES);
    victim->failCharm(caster);
    return SPELL_FAIL;
  }
}

void hypnosis(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;
  
  if (caster != victim) {
    act("$p attempts to hypnotize $N to your will.",
          FALSE, caster, obj, victim, TO_CHAR, ANSI_RED_BOLD);
    act("$p attempts to hypnotize you to $n's will.",
          FALSE, caster, obj, victim, TO_VICT, ANSI_RED_BOLD);
    act("$p attempts to hypnotize $N.",
          FALSE, caster, obj, victim, TO_NOTVICT, ANSI_RED_BOLD);
  } else {
    act("$p tries to get you to control yourself.",
          FALSE, caster, obj, 0, TO_CHAR, ANSI_RED_BOLD);
    act("$p tries to get $n to control $mself.",
          FALSE, caster, obj, 0, TO_ROOM, ANSI_RED_BOLD);
  }
  ret=hypnosis(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  return;
}

int hypnosis(TBeing *caster, TBeing *victim)
{
  char buf[256];
  taskDiffT diff;
  int level;

  level = caster->getSkillLevel(SPELL_HYPNOSIS);

  if (victim == caster) {
    sprintf(buf, "You don't want to hypnotize yourself...that would be dumb.");
    act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (victim->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't hypnotize $N while YOU are hypnotized!");
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }
  if (caster->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't hypnotize $N while under the same affects.");
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }

  if (caster->tooManyFollowers(victim, FOL_CHARM)) {
    act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    act("$N refuses to enter $ group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM, ANSI_WHITE_BOLD);
    return SPELL_FAIL;
  }

  if (victim->circleFollow(caster)) {
    caster->sendTo("Umm, you probably don't want to follow each other around in circles.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (!bPassShamanChecks(caster, SPELL_HYPNOSIS, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_HYPNOSIS]->lag;
  diff = discArray[SPELL_HYPNOSIS]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_HYPNOSIS, diff, 1, "", rounds, 
caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castHypnosis(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_HYPNOSIS);
  int bKnown = caster->getSkillValue(SPELL_HYPNOSIS);

  if ((ret=hypnosis(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
    act("You feel an overwhelming urge to follow $n!", FALSE, caster, NULL, victim, 
TO_VICT, ANSI_RED_BOLD);
    act("You decide to do whatever $e says!", FALSE, caster, NULL, victim, TO_VICT, 
ANSI_RED_BOLD);

  } else {
  }
  return TRUE;
}

int raze(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't do that to an immortal being.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 90);

  int dam = caster->getSkillDam(victim, SPELL_RAZE, level, adv_learn);

  caster->reconcileHurt(victim, discArray[SPELL_RAZE]->alignMod);


  if (victim->getImmunity(IMMUNE_ENERGY) >= 100) {
    act("$N is immune to energy rituals!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$N ignores $n's weak ritual!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n's ritual fails because of your immunity!", FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown,SPELL_RAZE)) {
    act("$n calls the spirits to erase $N's existance!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You call upon the loa to erase any memory of $N!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n calls upon the spirits to erase your existance!", FALSE, caster, NULL, victim, TO_VICT);
    switch(critSuccess(caster, SPELL_RAZE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_RAZE);
        dam <<= 1;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_RAZE))) {
          SV(SPELL_RAZE);
          dam /= 2;
        }
    }
    if (caster->reconcileDamage(victim, dam, SPELL_RAZE) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_RAZE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        act("$n nearly erases!", FALSE, caster, NULL, NULL, TO_ROOM);
        caster->sendTo("ACK!  The loa are extremely angry with you!\n\r");
        act("$n fails in $s ritual!", FALSE, caster, NULL, victim, TO_VICT);
        if (caster->isLucky(caster->spellLuckModifier(SPELL_RAZE))) {
          SV(SPELL_RAZE);
          dam /= 2;
        }
        if (caster->reconcileDamage(caster, dam/3, SPELL_RAZE) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        act("Oops! You nearly disintegrated yourself on that one!", 
            FALSE, caster, NULL, victim, TO_CHAR);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int raze(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_RAZE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_RAZE]->lag;
  taskDiffT diff = discArray[SPELL_RAZE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_RAZE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castRaze(TBeing *caster, TBeing *victim)
{
  int rc = 0;

  int level = caster->getSkillLevel(SPELL_RAZE);
  int bKnown = caster->getSkillValue(SPELL_RAZE);

  int ret=raze(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_RAZE));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int raze(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = raze(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}
 


