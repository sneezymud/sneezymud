// DISC_WRATH.cc
// Part of SneezyMUD

#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_wrath.h"
#include "obj_magic_item.h"

int plagueOfLocusts(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  TMonster *locusts;
  affectedData aff; 
  int rc, swarm;
  followData *k, *n;

  if (!caster->outside()) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    caster->sendTo("You need to be outside to summon locusts!\n\r");
    return SPELL_FAIL;
  }
  if (caster->roomp->getWeather() == WEATHER_RAINY) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    caster->sendTo("How do you expect to summon locusts in the rain!\n\r");
    return SPELL_FAIL;
  }
  if (level < 26) 
    swarm = MOB_LOCUSTS25;
  else if (level < 31) 
    swarm = MOB_LOCUSTS30;
  else if (level < 36) 
    swarm = MOB_LOCUSTS35;
  else if (level < 41) 
    swarm = MOB_LOCUSTS40;
  else 
    swarm = MOB_LOCUSTS50;

  if (!(locusts = read_mobile(swarm, VIRTUAL))) {
    vlogf(LOG_BUG, "Spell PLAGUE_LOCUSTS unable to load mob...");
    caster->sendTo("Unable to create the locusts, please report this.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if ((caster->followers) && (level < GOD_LEVEL1))  {
    for (k = caster->followers; k; k = n) {
      n = k->next;
      if (isname("locust", k->follower->name)) {
        act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
        caster->sendTo("You would have to be a god to summon another locust swarm!\n\r");
        delete locusts;
        locusts=NULL;
        return SPELL_FAIL;
      }
    }
  }

  level = min(level, 75);

  float lvl = max(1.0, level/2.0);
  locusts->setDamPrecision(20);

  locusts->setExp(0);
  locusts->setMoney(0);
  locusts->setHitroll(0);

  *caster->roomp += *locusts;

  act("You hear a loud humming sound as a swarm of locusts decends from above.", TRUE, locusts, NULL, caster, TO_ROOM);
  act("You hear a loud humming sound as a swarm of locusts decends from above.", TRUE, locusts, NULL, caster, TO_CHAR);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_PLAGUE_LOCUSTS)) {
    switch(critSuccess(caster, SPELL_PLAGUE_LOCUSTS)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_PLAGUE_LOCUSTS);
        act("The swarm seems to be especially angry and to be circling especially fast!", TRUE, caster, 0, locusts, TO_ROOM);
        caster->sendTo("The swarm seems to be especially angry and to be circling especially fast!\n\r");
        lvl *= 2;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_PLAGUE_LOCUSTS))) {
          SV(SPELL_PLAGUE_LOCUSTS);
          lvl /= 2;
        }
        break;
    }
    if (!caster->fight()) {
    }

    locusts->setLevel(WARRIOR_LEVEL_IND, (byte)lvl);
    locusts->setHPLevel(lvl);
    locusts->setHPFromHPLevel();
    locusts->setACLevel(lvl);
    locusts->setACFromACLevel();
    locusts->setDamLevel(lvl);

    if (!IS_SET(locusts->specials.act, ACT_SENTINEL))
      SET_BIT(locusts->specials.act, ACT_SENTINEL);

    if (!locusts->master)
      caster->addFollower(locusts);

    act("After following $n, the swarm descends on and attacks $N with all of the vengeance of $d!", TRUE, caster, 0, victim, TO_NOTVICT);
    act("After following you, the swarm descends on $N attacking with all the vengeance of $d!", TRUE, caster, 0, victim, TO_CHAR);
    act("After following $n, the swarm descends on you with all the fury of $d!", TRUE, caster, 0, victim, TO_VICT);

    aff.type = SPELL_PLAGUE_LOCUSTS;
    aff.level = level;
    aff.duration = 32000;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = 0;
    locusts->affectTo(&aff);

    return SPELL_SUCCESS;
  } else {
    locusts->setLevel(WARRIOR_LEVEL_IND, (byte)lvl);
    locusts->setHPLevel(lvl);
    locusts->setHPFromHPLevel();
    locusts->setACLevel(lvl);
    locusts->setACFromACLevel();
    locusts->setDamLevel(lvl);

    switch (critFail(caster, SPELL_PLAGUE_LOCUSTS)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_PLAGUE_LOCUSTS);
        act("The swarm hums angrily as it attacks $n instead of $s enemy!", TRUE, caster, 0, locusts, TO_ROOM);
        act("The swarm of locusts hums angrily as it begins to attack you!", TRUE, caster, 0, locusts, TO_CHAR);
        if ((rc = locusts->hit(caster)) == DELETE_VICT) {
          return SPELL_CRIT_FAIL | CASTER_DEAD;
          }
          if (rc == DELETE_THIS) {
            delete locusts;
            locusts = NULL;
          }
          return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
       
    caster->sendTo("The swarm circles once but then begins to dissipate into thin air.\n\r");
    act("The swarm circles once but then begins to dissipate into thin air!", TRUE, caster, 0, locusts, TO_ROOM);
    caster->sendTo("Finally, the swarm disappears entirely and the humming stops.\n\r");
    act("Finally, the swarm disappears totally and the humming stops.", TRUE, caster, 0, locusts, TO_ROOM);
    delete locusts;
    locusts = NULL;
    return SPELL_FAIL;
  }
}

int plagueOfLocusts(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  int ret = 0;
  int rc=0;
  ret=plagueOfLocusts(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
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

int plagueOfLocusts(TBeing * caster, TBeing * victim)
{

  int level,ret;
  int rc = 0;


  level = caster->getSkillLevel(SPELL_PLAGUE_LOCUSTS);
  int bKnown = caster->getSkillValue(SPELL_PLAGUE_LOCUSTS);

  if (!bPassClericChecks(caster, SPELL_PLAGUE_LOCUSTS)) {
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  
  ret=plagueOfLocusts(caster, victim, level, bKnown);

  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}
 
int pillarOfSalt(TBeing * caster, TBeing * victim, int level, byte bKnown, int adv_learn)
{
  wearSlotT slot = WEAR_NOWHERE;
  char buf[256], limb[256];

  if (!caster->roomp || caster->roomp->isWaterSector() || caster->roomp->isUnderwaterSector()) {
    caster->sendTo("The water surrounding you dissolves all the spell's salt!\n\r");
    return FALSE;
  }

  level = min(level, 50);

  caster->reconcileHurt(victim, discArray[SPELL_PILLAR_SALT]->alignMod);

  int dam = caster->getSkillDam(victim, SPELL_PILLAR_SALT, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_PILLAR_SALT)) {
    switch(critSuccess(caster, SPELL_PILLAR_SALT)) {
      case CRIT_S_KILL:
        CS(SPELL_PILLAR_SALT);
        dam *= 2;
        act("The $g rumbles and an enormous pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_ROOM);
        act("The $g rumbles and an enormous pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("The $g rumbles and an enormous pillar of salt erupts from beneath your feet", FALSE, caster, NULL, victim, TO_VICT);

        if (victim->isImmune(IMMUNE_PARALYSIS)) {
          break;
        }
        for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
          if (notBreakSlot(slot, false))
            continue;
          if (!victim->slotChance(slot) ||
            victim->isLimbFlags(slot, PART_MISSING))
            continue;
          break;
        }

        sprintf(limb, "%s", victim->describeBodySlot(slot).c_str());
        victim->addToLimbFlags(slot, PART_MISSING);
        sprintf(buf, "Suddenly the feeling disappears from your %s as it is turned into salt and then falls to the $g to be dispersed by the elements!", limb);
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        sprintf(buf, "$N's %s is caught in $n's prayer and is turned into salt flaring briefly then dropping to the $g to be dispersed by the elements!", limb);
        act(buf, FALSE, caster, NULL, victim, TO_ROOM);
        sprintf(buf, "$N's %s is caught in your prayer and is turned into salt flaring briefly then dropping to the $g to be dispersed by the elements!", limb);
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);

        victim->dropWeapon(slot);
        *victim->roomp += *read_object(OBJ_SALTPILE, VIRTUAL);

        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_PILLAR_SALT);
        dam *= 2;
        act("The $g rumbles and an enormous pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("The $g rumbles and an enormous pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("The $g rumbles and an enormous pillar of salt erupts from beneath your feet", FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        act("The $g shakes and a large pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("The $g shakes and a large pillar of salt erupts from the $g encasing $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("The $g erupts beneath your feet and your skin starts to burn from a pillar of salt $n has called forth!", FALSE, caster, NULL, victim, TO_VICT);
        break;
    }

    if (victim->awake()) {
      if ((victim->isLucky(caster->spellLuckModifier(SPELL_PILLAR_SALT))) ||
          (caster->isNotPowerful(victim, (level+8), SPELL_PILLAR_SALT, SILENT_YES))) {
        SV(SPELL_PILLAR_SALT);
        act("$N manages to avoid some of the pelting salt particles!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$N manages to avoid some of the salt particles!", FALSE, caster, NULL, victim, TO_CHAR);
        act("Luckily, you manage to avoid some of the salt particles!", FALSE, caster, NULL, victim, TO_VICT); 
        dam /= 2;
      }
    }

    victim->roomp->playsound(SOUND_SPELL_PILLAR_OF_SALT, SOUND_TYPE_MAGIC);

    if (caster->reconcileDamage(victim, dam, SPELL_PILLAR_SALT) == -1)
      return SPELL_SUCCESS | VICTIM_DEAD;

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_PILLAR_SALT)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_PILLAR_SALT);
        act("$n is pelted by salt from the very pillar $e has called forth!", FALSE, caster, NULL, victim, TO_VICT);
        act("Your skin burns from salt particles from a pillar of salt intended for $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("A pillar of salt misses $N and encases $n!", FALSE, caster, NULL, victim, TO_NOTVICT);

        // don't be obscene about it
        dam /= 4;

        if (caster->reconcileDamage(caster, dam, SPELL_PILLAR_SALT) == -1)
          return SPELL_CRIT_FAIL | CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
	break;
    }
    act("You hear a muffled sound coming from the $g but nothing seems to happen!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("You hear a muffled sound coming from the $g but nothing seems to happen!", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }
}

int pillarOfSalt(TBeing * caster, TBeing * victim, TMagicItem * obj)
{
  int ret = 0;
  int rc=0;

  ret=pillarOfSalt(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
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

int pillarOfSalt(TBeing * caster, TBeing * victim)
{
  int ret, level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_PILLAR_SALT)) {
    return FALSE;
  }

  level = caster->getSkillLevel(SPELL_PILLAR_SALT);
  int bKnown = caster->getSkillValue(SPELL_PILLAR_SALT);

  ret=pillarOfSalt(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_PILLAR_SALT));
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

int rainBrimstone(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  level = min(level, 10);

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    switch(critSuccess(caster, spell)) {
      case CRIT_S_KILL: 
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(spell);
        dam *= 2;
        act("You hear a loud cracking sound and white hot brimstone rains down on $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("You hear a loud cracking sound and white hot brimstone rains down on $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("You hear a loud cracking sound and then your skin starts to burn from white hot brimstone $n has called down upon your head!", FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        act("You hear a rending sound and red hot brimstone rains down on $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("You hear a rending sound and red hot brimstone rains down on $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("You hear a rending sound and then your skin starts to burn from red hot brimstone $n has called down upon your head!", FALSE, caster, NULL, victim, TO_VICT);
        break;
    }

#if 0
    if ((victim->isLucky(caster->spellLuckModifier(spell))) ||
        (caster->isNotPowerful(victim, (level+3), spell, SILENT_YES))) {
#else
    if (victim->isLucky(caster->spellLuckModifier(spell))) {
#endif
      SV(spell);
      act("$N manages to avoid some of the sulfur particles!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("$N manages to avoid some of the sulfur particles!", FALSE, caster, NULL, victim, TO_CHAR);
      act("Luckily, you manage to avoid some of the sulfur particles!", FALSE, caster, NULL, victim, TO_VICT); 
      dam *= 2;
      dam /= 3;
    }

    victim->roomp->playsound(SOUND_SPELL_RAIN_BRIMSTONE, SOUND_TYPE_MAGIC);

    if (caster->reconcileDamage(victim, dam, spell) == -1)
      return SPELL_SUCCESS | VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(spell);
        act("A whining sound issues from above you as $n is burned by the very brimstone $e has called forth to help $m!", FALSE, caster, NULL, victim, TO_NOTVICT);
        act("A whining sound issues from above you.  Your skin starts to burn from the hot brimstone you intended for $N!", FALSE, caster, NULL, victim, TO_CHAR);
        act("A whining sound issues from above you as hot brimstone misses your head and descends on $n!", FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, spell) == -1)
          return SPELL_CRIT_FAIL | CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
	break;
    }
    act("You hear a slight rending sound but nothing seems to happen!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("You hear a slight rending sound but nothing seems to happen!", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }
}

int rainBrimstone(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret = 0;
  int rc=0;

  ret=rainBrimstone(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
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

int rainBrimstone(TBeing * caster, TBeing * victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_RAIN_BRIMSTONE);


  if (!bPassClericChecks(caster,spell)) {
    return FALSE;
  }

  int level = caster->getSkillLevel(spell);
  int bKnown = caster->getSkillValue(spell);

  int ret=rainBrimstone(caster,victim,level,bKnown, spell, caster->getAdvLearning(spell));
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

int curse(TBeing * caster, TObj * obj, int, byte bKnown, spellNumT spell)
{
  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    obj->addObjStat(ITEM_NODROP);

    obj->curseMe();
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void curse(TBeing * caster, TObj * target, TMagicItem * obj, spellNumT spell)
{
  int ret;

  ret=curse(caster,target,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (IS_SET(ret,SPELL_SUCCESS)) {
    act("$n glows briefly with a dark forboding red!",
       TRUE, target, NULL, 0, TO_ROOM);
   act("You glow briefly with a dark forboding red!",
       FALSE, target, NULL, 0, TO_CHAR);
  }
}

void curse(TBeing * caster, TBeing * victim, TMagicItem * obj, spellNumT spell)
{
  int ret;
 
  ret=curse(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell);
  if (IS_SET(ret,SPELL_SUCCESS)) {
    act("$n glows evilly with a dark forboding red!", 
       TRUE, victim, NULL, 0, TO_ROOM);
    act("You glow evilly with a dark forboding red!", 
       FALSE, victim, NULL, 0, TO_VICT);
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

void curse(TBeing * caster, TObj * obj)
{
  spellNumT spell = caster->getSkillNum(SPELL_CURSE);

  if (!obj)
    return;

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);
  int bKnown = caster->getSkillValue(spell);

  int ret=curse(caster,obj,level,bKnown, spell);
  if (ret == SPELL_SUCCESS) {
    act("$p glows evilly with a dark forboding light.", FALSE, caster, obj, NULL, TO_ROOM);
    act("$p glows evilly with a dark forboding light.", FALSE, caster, obj, NULL, TO_CHAR);
    act("You succeed in summoning a minor demon spirit to take residence in $p.", FALSE, caster, obj, NULL, TO_CHAR);
  } else 
    caster->sendTo("Nothing seems to happen.\n\r");
}

int curse(TBeing * caster, TBeing * victim, int level, byte bKnown, spellNumT spell)
{
  if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) 
    return SPELL_FAIL;

  caster->reconcileHurt(victim,discArray[spell]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    // made it violent - Maror
/*    if (!victim->isLucky(caster->spellLuckModifier(spell)) &&
        !victim->affectedBySpell(spell)) {
*/
      genericCurse(caster, victim, level, spell);
/*
      // this spell is non-violent, cause it to piss off mobs though
      if (!victim->isPc()) {
        TMonster *tmons = dynamic_cast<TMonster *>(victim);
        tmons->UM(4);
        tmons->US(5);
        tmons->UA(7);
        tmons->aiTarget(caster);
      }*/

      return SPELL_SUCCESS;
/*    } else {
      return SPELL_FAIL;
    }*/
  } else {
    if (critFail(caster, spell)) {
      CF(spell);
      genericCurse(caster, caster, level, spell);
      return SPELL_CRIT_FAIL;
    } else
      return SPELL_FAIL;
  }
}

void curse(TBeing * caster, TBeing * victim)
{
  spellNumT spell = caster->getSkillNum(SPELL_CURSE);

  if (!bPassClericChecks(caster,spell))
    return;

  int level = caster->getSkillLevel(spell);
  int bKnown = caster->getSkillValue(spell);

  int ret=curse(caster,victim,level,bKnown, spell);
  if (ret == SPELL_SUCCESS) {
    act("$N glows evilly with a dark forboding red!", TRUE, caster, NULL, victim, TO_ROOM);
    act("You glow evilly with a dark forboding red!", FALSE, caster, NULL, victim, TO_VICT);
    act("You succeed in summoning a minor demon spirit to inhabit $N's body.", FALSE, caster, NULL, victim, TO_CHAR);
    act("You feel extremely uncomfortable, as if you don't have complete control over your body!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else {
    if (ret==SPELL_CRIT_FAIL) {
      caster->spellMessUp(spell);
      act("$n seems to have cursed $mself!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("Oh no! You seem to have cursed yourself!", FALSE, caster, NULL, NULL, TO_CHAR);
    } else
      act("Nothing seems to happen...", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
  return;
}

int earthquake(TBeing *caster, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int rc;
  TBeing *tmp_victim, *temp;

  if (caster->roomp->isWaterSector() || caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You can't earthquake in all this water.\n\r");
    return SPELL_FAIL;
  }

  level = min(level, 50);

  int dam = caster->getSkillDam(NULL, spell, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    act("The $g below you starts to shake and tremble! Earthquake!!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n causes the $g below you to shake and tremble! Earthquake!!", FALSE, caster, NULL, NULL, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      int act_dam = dam;
      temp = tmp_victim->next;

      if (caster->sameRoom(*tmp_victim) && (caster != tmp_victim)) {
	if (tmp_victim->isImmortal())
	  act("As an immortal, of course you aren't bothered by a measly little earthquake!", FALSE, caster, NULL, tmp_victim, TO_VICT);
	else if (tmp_victim->isFlying())
	  act("You fly above the danger!", FALSE, caster, NULL, tmp_victim, TO_VICT);
	else if (tmp_victim->isLevitating())
	  act("You float above the danger!", FALSE, caster, NULL, tmp_victim, TO_VICT);
	else if (caster->inGroup(*tmp_victim))
	  act("The earthquake has no effect on you!", FALSE, caster, NULL, tmp_victim, TO_VICT);
        else {
	  caster->reconcileHurt(tmp_victim, discArray[spell]->alignMod);

          if (tmp_victim->riding) {
            rc = tmp_victim->fallOffMount(tmp_victim->riding, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tmp_victim;
              tmp_victim = NULL;
              continue;
            }
          }

	  if (critSuccess(caster, spell))  {
            act_dam *= 2;
            if (tmp_victim->getPosition() >= POSITION_STANDING) {
               act("$n falls down and hurts $mself in the earthquake!", 
                   FALSE, tmp_victim, NULL, NULL, TO_ROOM);
               act("You fall down and hurt yourself in the earthquake!", 
                   FALSE, tmp_victim, NULL, NULL, TO_CHAR);
	      tmp_victim->setPosition(POSITION_SITTING);
            } else {
               act("$n hurts $mself in the earthquake!", 
                   FALSE, tmp_victim, NULL, NULL, TO_ROOM);
               act("You hurt yourself in the earthquake!", 
                   FALSE, tmp_victim, NULL, NULL, TO_CHAR);
            }
          } else {
            // a non-critsuccess
            if (tmp_victim->isLucky(caster->spellLuckModifier(spell))) {
              act("The earthquake slightly injures $n!", 
                   FALSE, tmp_victim, NULL, NULL, TO_ROOM);
              act("The earthquake slightly injures you!", 
                   FALSE, tmp_victim, NULL, NULL, TO_CHAR);
              act_dam /= 2;
            } else if (tmp_victim->getPosition() >= POSITION_STANDING) {
              act("The earth shakes and smashes $n into the $g!", 
                   FALSE, tmp_victim, NULL, NULL, TO_ROOM);
              act("The earth shakes and smashes you into the $g!",
                   FALSE, tmp_victim, NULL, NULL, TO_CHAR);
              tmp_victim->setPosition(POSITION_SITTING);
            } else {
              act("The earth shakes causing $n injury!", 
                   FALSE, tmp_victim, NULL, NULL, TO_ROOM);
              act("The earth shakes causing you injury!",
                   FALSE, tmp_victim, NULL, NULL, TO_CHAR);
            }
          }
          if (caster->reconcileDamage(tmp_victim, act_dam, spell) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
          }
        }
      } else if ((caster != tmp_victim) &&
        	 (tmp_victim->in_room != ROOM_NOWHERE) &&
		 (caster->roomp->getZoneNum() == tmp_victim->roomp->getZoneNum())) {
	tmp_victim->sendTo("The earth shakes for a moment...\n\r");
	if ((tmp_victim->getPosition() > POSITION_SITTING) &&
	    !tmp_victim->isImmortal() &&
            !tmp_victim->isFlying() &&
            !tmp_victim->isLevitating() &&
            caster->isNotPowerful(tmp_victim, level, spell, SILENT_YES) &&
	    (!tmp_victim->isLucky(caster->spellLuckModifier(spell)) &&    // two chances to save  
	    !tmp_victim->isLucky(caster->spellLuckModifier(spell)))) {
          if (tmp_victim->riding) {
            rc = tmp_victim->fallOffMount(tmp_victim->riding,
POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tmp_victim;
              tmp_victim = NULL;
              continue;
            }
          }
	  act("You lose your balance and tumble to the $g!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
	  act("$n tumbles to the $g!", FALSE, tmp_victim, NULL, NULL, TO_ROOM);
	  tmp_victim->setPosition(POSITION_SITTING);
	}
      }
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int earthquake(TBeing *caster, TMagicItem *obj, spellNumT spell)
{
  earthquake(caster,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  return FALSE;
}

int earthquake(TBeing *caster)
{
  spellNumT spell = caster->getSkillNum(SPELL_EARTHQUAKE);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);
  int bKnown = caster->getSkillValue(spell);

  int ret=earthquake(caster,level,bKnown, spell, caster->getAdvLearning(spell));
  if (ret == SPELL_SUCCESS) {
  } else {
    act("You get the feeling that something big was supposed to happen...", FALSE, caster, NULL, NULL, TO_ROOM);
    act("Nothing happened!", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return FALSE;
}

int callLightning(TBeing *caster, TBeing *victim, int level, byte bKnown, spellNumT spell, int adv_learn)
{
  int rc;

  //if (caster->isNotPowerful(victim, level, spell, SILENT_NO)) {
  //  return SPELL_FAIL;
  //}

  if (!((victim->roomp->getWeather() == WEATHER_RAINY) ||
      (victim->roomp->getWeather() == WEATHER_LIGHTNING))) {
    caster->sendTo("You fail to call upon the lightning from the sky!\n\r");
    if (!victim->outside())
      caster->sendTo("There is no sky here...you're not outdoors!\n\r");
    //    else if (victim->roomp->getWeather() == WEATHER_SNOWY)
    //      caster->sendTo("It's too cold to produce lightning.\n\r");
    else
      caster->sendTo("There is no lightning here...it's not storming!\n\r");
    return SPELL_FAIL;
  }

  level = min(level, 50);

  caster->reconcileHurt(victim, discArray[spell]->alignMod);

  int dam = caster->getSkillDam(victim, spell, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), spell)) {
    if (victim->isLucky(caster->spellLuckModifier(spell)))
      dam /= 2;		// half damage 

    if (critSuccess(caster, spell)) {
      CS(spell);
      dam *= 2;
    }

    victim->roomp->playsound(SOUND_SPELL_CALL_LIGHTNING, SOUND_TYPE_MAGIC);

    act("$n summons a lightning bolt from the stormy skies, guiding it down upon $N!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You summon a lightning bolt from the stormy skies, guiding it down upon $N!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n summons a lightning bolt from the stormy skies, guiding it down upon you!", FALSE, caster, NULL, victim, TO_VICT);
    if (caster->reconcileDamage(victim, dam, spell) == -1) 
      return SPELL_SUCCESS | VICTIM_DEAD;
    rc = victim->lightningEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return SPELL_SUCCESS | VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, spell)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(spell);
        dam /= 3;

        act("$n summons lightning from the stormy skies, bringing down a bolt upon $mself!", FALSE, caster, NULL, NULL, TO_ROOM);
        act("You summon lightning from the stormy skies, bringing down a bolt upon yourself!", FALSE, caster, NULL, NULL, TO_CHAR);
        act("That lightning bolt was intended for you!", FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam,spell) == -1)
          return SPELL_CRIT_FAIL | CASTER_DEAD;
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->sendTo("Nothing seems to happen.\n\r");
    return SPELL_FAIL;
  }
}

int callLightning(TBeing *caster, TBeing *victim, TMagicItem *obj, spellNumT spell)
{
  int ret = 0;
  int rc = 0;

  ret=callLightning(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), spell, 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int callLightning(TBeing *caster, TBeing *victim)
{
  int rc = 0;

  spellNumT spell = caster->getSkillNum(SPELL_CALL_LIGHTNING);

  if (!bPassClericChecks(caster,spell))
    return FALSE;

  int level = caster->getSkillLevel(spell);
  int bKnown = caster->getSkillValue(spell);

  int ret=callLightning(caster,victim,level,bKnown, spell, caster->getAdvLearning(spell));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (IS_SET(ret,SPELL_CRIT_FAIL)) {
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int spontaneousCombust(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  int rc;

  //  if (caster->isNotPowerful(victim, level, SPELL_SPONTANEOUS_COMBUST, SILENT_NO)) {
  //    return SPELL_FAIL;
  //  }

  if (victim->roomp->isUnderwaterSector()) {
    caster->sendTo("Your attempt fizzels and sputters in the water!\n\r");
    act("$n causes the water to warm up around you.  You wonder why?", 
          FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FALSE;
  }

  level = min(level, 60);

  caster->reconcileHurt(victim, discArray[SPELL_SPONTANEOUS_COMBUST]->alignMod);

  int dam = caster->getSkillDam(victim, SPELL_SPONTANEOUS_COMBUST, level, adv_learn);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_SPONTANEOUS_COMBUST)) {

    if (critSuccess(caster, SPELL_SPONTANEOUS_COMBUST)) {
      CS(SPELL_SPONTANEOUS_COMBUST);
      dam *= 2;
      act("<R>$N is immolated in an enourmous burst of fire!<1>",
          FALSE, caster, NULL, victim, TO_NOTVICT);
      if (caster != victim) {
	act("<R>$N is immolated in an enourmous burst of fire!<1>",
	    FALSE, caster, NULL, victim, TO_CHAR);
	act("<R>You are immolated in an enourmous burst of fire!<1>",
	    FALSE, caster, NULL, victim, TO_VICT);
      } else {
	act("<R>You are immolated in an enourmous burst of fire!<1>",
	    FALSE, caster, NULL, victim, TO_VICT);
      }
      
    } else if (victim->isLucky(caster->spellLuckModifier(SPELL_SPONTANEOUS_COMBUST))) {
      SV(SPELL_SPONTANEOUS_COMBUST);
      dam /= 2;		// half damage 
      act("<r>$N is immolated in a <Y>rather pitiful<1><r> burst of fire!<1>",
          FALSE, caster, NULL, victim, TO_NOTVICT);
      if (caster != victim) {
        act("<r>$N is immolated in a <Y>rather pitiful<1><r> burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_CHAR);
        act("<r>You are immolated in a <Y>rather pitiful<1><r> burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_VICT);
      } else {
        act("<r>You are immolated in a <Y>rather pitiful<1><r> burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_VICT);
      }
      
    } else {
      act("<r>$N is immolated in a burst of fire!<1>",
          FALSE, caster, NULL, victim, TO_NOTVICT);
      if (caster != victim) {
        act("<r>$N is immolated in a burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_CHAR);
        act("<r>You are immolated in a burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_VICT);
      } else {
        act("<r>You are immolated in a burst of fire!<1>",
            FALSE, caster, NULL, victim, TO_VICT);
	
      }
    }
      victim->roomp->playsound(SOUND_SPELL_SPONTANEOUS_COMBUST, SOUND_TYPE_MAGIC);
      
    act("$N howls in pain!", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
    if (caster != victim) {
      act("$N howls in pain!", 
          FALSE, caster, NULL, victim, TO_CHAR);
      act("You howl in pain!<1>", 
          FALSE, caster, NULL, victim, TO_VICT);
    } else {
      act("You howl in pain!<1>", 
          FALSE, caster, NULL, victim, TO_VICT);
    }
    
    if (caster->reconcileDamage(victim, dam, SPELL_SPONTANEOUS_COMBUST) == -1)
      return SPELL_SUCCESS | VICTIM_DEAD;
    
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return SPELL_SUCCESS | VICTIM_DEAD;
    
    return SPELL_SUCCESS;
    } else {
    switch (critFail(caster, SPELL_SPONTANEOUS_COMBUST)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_SPONTANEOUS_COMBUST);
        act("Something goes dramatically wrong.",
              FALSE, caster, NULL, NULL, TO_ROOM);
        act("Something goes dramatically wrong.",
              FALSE, caster, NULL, NULL, TO_CHAR);
        act("$n suddenly erupts in a tower of flames!", 
              FALSE, caster, NULL, NULL, TO_ROOM);
        act("You are suddenly consumed in a tower of flames! <R>FIRE!!!<1>", 
              FALSE, caster, NULL, NULL, TO_CHAR);

        if (caster->reconcileDamage(caster, dam,SPELL_SPONTANEOUS_COMBUST) == -1)
          return SPELL_CRIT_FAIL | CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    act("Phantom sparks drift hazily about $n before fizzling out!", 
          FALSE, victim, NULL, NULL, TO_ROOM);
    act("Phantom sparks drift hazily about you before fizzling out!",
          FALSE, victim, NULL, NULL, TO_CHAR);

    return SPELL_FAIL;
  } 
}

int spontaneousCombust(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  int ret = 0;
  int rc=0;

  ret=spontaneousCombust(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
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

int spontaneousCombust(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_SPONTANEOUS_COMBUST))
    return FALSE;

  level = caster->getSkillLevel(SPELL_SPONTANEOUS_COMBUST);
  int bKnown = caster->getSkillValue(SPELL_SPONTANEOUS_COMBUST);

  ret=spontaneousCombust(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_SPONTANEOUS_COMBUST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (IS_SET(ret,SPELL_CRIT_FAIL)) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int flamestrike(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  int ret = 0;

  level = min(level, 30);

  int dam = caster->getSkillDam(victim, SPELL_FLAMESTRIKE, level, adv_learn);

  caster->reconcileHurt(victim, discArray[SPELL_FLAMESTRIKE]->alignMod);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_FLAMESTRIKE)) {
    act("The blazing column descends toward $n.",
          FALSE, victim, 0, 0, TO_ROOM);
    act("The blazing column descends toward you! <R>FIRE!!!<1>",
          FALSE, victim, 0, 0, TO_CHAR);

    switch (critSuccess(caster, SPELL_FLAMESTRIKE)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_FLAMESTRIKE);
        dam *= 2;
        ret = SPELL_CRIT_SUCCESS;
        act("The blazing column is incredibly hot!",
              FALSE, victim, 0, 0, TO_ROOM);
        act("The blazing column is incredibly hot!",
              FALSE, victim, 0, 0, TO_CHAR);
        break;
      case CRIT_S_NONE:
        ret=SPELL_SUCCESS;
        break;
    }
    if (victim->isNotPowerful(victim, level + 5, SPELL_FLAMESTRIKE, SILENT_YES)) {
      dam /=2;
    }
    if (victim->isLucky(caster->spellLuckModifier(SPELL_FLAMESTRIKE))) {
      dam /= 2;
      SV(SPELL_FLAMESTRIKE);
      act("$n ducks and manages to avoid some of the fire!",
          FALSE, victim, 0, 0, TO_ROOM);
      act("You manage to avoid some of the fire! <R>FIRE!!!<1>",
          FALSE, victim, 0, 0, TO_CHAR);
      ret |= SPELL_SAVE;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_FLAMESTRIKE) == -1)
      return (ret | VICTIM_DEAD);
    return ret;
  } else {
    switch (critFail(caster, SPELL_FLAMESTRIKE)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        act("Oops!  The blazing column twists and makes a bee-line for $n!",
              FALSE, caster, NULL, 0, TO_ROOM);
        act("Oops!  The blazing column twists and makes a bee-line for you! <R>FIRE!!!<1>",
              FALSE, caster, NULL, 0, TO_CHAR);
        act("Hey!  That spell was meant for you!",
              FALSE, victim, NULL, 0, TO_CHAR);
        CF(SPELL_FLAMESTRIKE);
        dam /= 2;
        ret = SPELL_CRIT_FAIL;
        if (caster->reconcileDamage(caster, dam, SPELL_FLAMESTRIKE) == -1)
          return (ret | CASTER_DEAD);
        return ret;
      case CRIT_F_NONE:
        act("You get a sensation of heat but nothing seems to happen!",
              FALSE, caster, NULL, NULL, TO_ROOM);
        act("You get a sensation of heat but nothing seems to happen!",
              FALSE, caster, NULL, NULL, TO_CHAR);
    }
    return SPELL_FAIL;
  }
}

int flamestrike(TBeing *caster, TBeing *victim)
{
  int ret = 0;
  int level;
  int rc = 0;

  if (!bPassClericChecks(caster,SPELL_FLAMESTRIKE))
    return FALSE;

  level = caster->getSkillLevel(SPELL_FLAMESTRIKE);
  int bKnown = caster->getSkillValue(SPELL_FLAMESTRIKE);

  act("$n calls forth a blazing column of fire!",
          FALSE, caster, NULL, victim, TO_ROOM);
  act("You call forth a blazing column of fire!",
          FALSE, caster, NULL, victim, TO_CHAR);

  ret=flamestrike(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_FLAMESTRIKE));

  if (IS_SET(ret, SPELL_SUCCESS)) {
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= VICTIM_DEAD;
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= VICTIM_DEAD;
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    rc = caster->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= CASTER_DEAD;
  } else if (IS_SET(ret, SPELL_FAIL)) {
  } else {
  }
  rc = 0;

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int flamestrike(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  int ret = 0;
  int rc = 0;

  act("$p calls forth a blazing column of fire!",
          FALSE, caster, obj, victim, TO_ROOM);
  act("You call forth a blazing column of fire!",
          FALSE, caster, obj, victim, TO_CHAR);

  ret = flamestrike(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (IS_SET(ret,SPELL_SAVE)) {
    }
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= VICTIM_DEAD;
  } else if (IS_SET(ret,SPELL_CRIT_SUCCESS)) {
    if (IS_SET(ret,SPELL_SAVE)) {
    }
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= VICTIM_DEAD;
  } else if (IS_SET(ret,SPELL_CRIT_FAIL)) {
    rc = caster->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ret |= CASTER_DEAD;
  } else if (IS_SET(ret,SPELL_FAIL)) {
  } else {
  }
  rc = 0;

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

    return rc;
}



