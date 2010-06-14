//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "disc_air.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "obj_magic_item.h"
#include "combat.h"

int gust(TBeing * caster, TBeing * victim, int level, short bKnown, int adv_learn)
{
  int rc;
  TThing *ch;

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo(COLOR_SPELLS, "<W>Not much air down here under all this water.<z>\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 10);

  int dam = caster->getSkillDam(victim, SPELL_GUST, level, adv_learn);

  caster->reconcileHurt(victim,discArray[SPELL_GUST]->alignMod); 

  if (caster->bSuccess(bKnown,SPELL_GUST)) {
    if ((critSuccess(caster, SPELL_GUST) || 
        critSuccess(caster, SPELL_GUST) || 
        critSuccess(caster,SPELL_GUST)) &&
        !caster->isNotPowerful(victim, level, SPELL_GUST, SILENT_YES)) {
      CS(SPELL_GUST);
      dam *= 2;
      act("<W>A gust of wind picks $N up in the air and slams $M to the $G.<z>", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>A gust of wind picks you up in the air and slams you to the $G.<z>",
          FALSE, caster, NULL, victim, TO_VICT);
      act("<W>You call up a gust of wind that picks $N up in the air and slams $M to the $G.<z>", 
          FALSE, caster, NULL, victim, TO_CHAR);

      if (caster->isLucky(levelLuckModifier(victim->GetMaxLevel())))
        victim->addToDistracted(2, FALSE);
      else
        victim->addToDistracted(1, FALSE);

      if (victim->riding)
        victim->dismount(POSITION_STANDING);
      while ((ch = victim->rider)) {
        TBeing *tb = dynamic_cast<TBeing *>(ch);
        if (tb) {
          rc = tb->fallOffMount(victim, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tb;
            tb = NULL;
          }
        } else {
          ch->dismount(POSITION_DEAD);
        }
      }

      victim->setPosition(POSITION_SITTING);
      victim->addToWait(combatRound(1));
    } else if (victim->isLucky(caster->spellLuckModifier(SPELL_GUST))) {
      SV(SPELL_GUST);
      act("<W>A gust of wind buffets $N.<z>", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>A gust of wind buffets you.<z>",
          FALSE, caster, NULL, victim, TO_VICT);
      act("<W>You call up a gust of wind that buffets $N.<z>", 
          FALSE, caster, NULL, victim, TO_CHAR);
      dam /= 2;
    } else {
      act("<W>A gust of wind buffets $N fiercely!<z>", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>A gust of wind buffets you fiercely!<z>",
          FALSE, caster, NULL, victim, TO_VICT);
      act("<W>You call up a gust of wind that buffets $N fiercely!<z>", 
          FALSE, caster, NULL, victim, TO_CHAR);
    }
    if (caster->reconcileDamage(victim, dam, SPELL_GUST) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_GUST) == CRIT_F_HITSELF) {
      CF(SPELL_GUST);
      act("<W>Oh No!  You slam yourself into the $g!<z>", FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>A gust of wind picks you up in the air and slams you to the $g.<z>", FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>$n calls up a gust of wind that picks $mself up in the air and slams $mself to the $g.<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
      if (caster->riding)
        caster->dismount(POSITION_STANDING);
      while ((ch = caster->rider)) {
        TBeing *tb = dynamic_cast<TBeing *>(ch);
        if (tb) {
          rc = tb->fallOffMount(caster, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tb;
            tb = NULL;
          }
        } else {
          ch->dismount(POSITION_DEAD);
        }
      }
      caster->setPosition(POSITION_SITTING);
      if (caster->reconcileDamage(caster, dam, SPELL_GUST) == -1)
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_CRIT_FAIL;
    } else {
      act("<W>$n just tried to bowl you over.<z>", FALSE, caster, NULL, victim, TO_VICT);
      caster->nothingHappens();
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
      return SPELL_FAIL;
    }
  }
}

int gust(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_GUST, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_GUST]->lag;
  taskDiffT diff = discArray[SPELL_GUST]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_GUST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castGust(TBeing * caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_GUST);
  if (victim == caster->fight()) {
  } else {
    return FALSE;
  }
  int ret=gust(caster,victim,level,caster->getSkillValue(SPELL_GUST), caster->getAdvLearning(SPELL_GUST));

  int rc = 0;
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int gust(TBeing * caster, TBeing *victim,  TMagicItem * obj)
{
  if (victim->isImmortal()) {
    caster->sendTo(COLOR_SPELLS, "<W>Now that was pretty stupid of you...<z>\n\r");
    return FALSE;
  }

  int level = obj->getMagicLevel();

  int ret=gust(caster,victim,level,obj->getMagicLearnedness(), 0);

  int retCode = 0;
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(retCode, DELETE_THIS);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(retCode, DELETE_VICT);

  return retCode;
}

int immobilize(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  int rc;
  TThing *ch;
  int retCode = 0;

  if (victim->affectedBySpell(SPELL_IMMOBILIZE)) {
    act("<W>$N is already immobilized!<z>",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return retCode;
  }
  caster->reconcileHurt(victim,discArray[SPELL_IMMOBILIZE]->alignMod);

  // see the balance notes for details on what we are doing here
  // we are removing a certain number of combat hits and also adding
  // some command lock-out.
  // since spell is tasked, add one to the lag
  float rounds = 0.2233 * (discArray[SPELL_IMMOBILIZE]->lag + 1);
  // adjust for difficulty
  rounds = (rounds * 100.0 / getSkillDiffModifier(SPELL_IMMOBILIZE));
  // adjust for saving-throw penalty
  rounds = (rounds * 4 / 3);

  if (caster->bSuccess(bKnown, SPELL_IMMOBILIZE)) {
    retCode |= SPELL_SUCCESS;
    
    switch (critSuccess(caster, SPELL_FEATHERY_DESCENT)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
        rounds *= 3.0;
        CS(SPELL_IMMOBILIZE);
        retCode |= SPELL_CSUC_TRIPLE;
        break;
      case CRIT_S_DOUBLE:
        rounds *= 2.0;
        CS(SPELL_IMMOBILIZE);
        retCode |= SPELL_CSUC_DOUBLE;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_IMMOBILIZE))) {
          SV(SPELL_IMMOBILIZE);
          rounds /= 2.0;
          retCode |= SPELL_SAVE;
        }
        break;
    }

    int cr = victim->loseRound(rounds);
    victim->cantHit += cr;
    
    cr = combatRound(rounds);
    victim->addToWait(cr);

    if (victim->riding) {
      rc = victim->fallOffMount(victim->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return retCode | VICTIM_DEAD;
      }
    }
    while ((ch = victim->rider)) {
      rc = ch->fallOffMount(victim, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete ch;
        ch = NULL;
      }
    }
    if (victim->fight())
      victim->stopFighting();
    victim->setPosition(POSITION_SITTING);
    return retCode;
  } else {
    retCode |= SPELL_FAIL;

    if (critFail(caster, SPELL_IMMOBILIZE)) {
      CF(SPELL_IMMOBILIZE);
      act("<W>You immobilize yourself!<z>", FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>Uhoh!  Something went wrong.<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>$n just tried to immobilize you.<z>", FALSE, caster, NULL, victim, TO_VICT);

      int cr = caster->loseRound(rounds);
      caster->cantHit += cr;
      
      cr = combatRound(rounds);
      caster->addToWait(cr);

      if (caster->riding) {
        rc = caster->fallOffMount(caster->riding, POSITION_STANDING);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      }
      while ((ch = caster->rider)) {
        TBeing *tb = dynamic_cast<TBeing *>(ch);
        if (tb) {
          rc = tb->fallOffMount(caster, POSITION_STANDING);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tb;
            tb = NULL;
          }
        } else {
          ch->dismount(POSITION_DEAD);
        }
      }
      if (caster->fight())
        caster->stopFighting();

      caster->setPosition(POSITION_SITTING);

      retCode |= SPELL_CRIT_FAIL;
    }                                                                           

    return retCode;                                                          
  }                                                                             
} 

int immobilize(TBeing * caster, TBeing * victim)
{
#if 0
  // good idea, no such affect set though
  if (victim->affectedBySpell(SPELL_IMMOBILIZE)) {                              
    act("<W>$N is already immobilized!<z>",
        FALSE, caster, NULL, victim, TO_CHAR);                                  
    return FALSE;                                                               
  } 
#endif

  if (!bPassMageChecks(caster, SPELL_IMMOBILIZE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_IMMOBILIZE]->lag;
  taskDiffT diff = discArray[SPELL_IMMOBILIZE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_IMMOBILIZE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castImmobilize(TBeing * caster, TBeing * victim)
{
  int level,ret;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_IMMOBILIZE);
  if (caster->isNotPowerful(victim, level, SPELL_IMMOBILIZE, SILENT_NO)) {
    return FALSE;
  }

  ret=immobilize(caster,victim,level,caster->getSkillValue(SPELL_IMMOBILIZE));
  // logic assumes won't mix a save with criticals
  if (IS_SET(ret, SPELL_CSUC_TRIPLE)) {
    act("<W>You create a GIGANTIC wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_CHAR);
    act("<W>$n creates a GIGANTIC wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>$n creates a GIGANTIC wall of air around you.  You are knocked over and trapped!<z>", FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_CSUC_DOUBLE)) {
    act("<W>You create a HUGE wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_CHAR);
    act("<W>$n creates a HUGE wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>$n creates a HUGE wall of air around you.  You are knocked over and trapped!<z>", FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_SAVE)) {
    act("<W>You create a *tiny* wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_CHAR);
    act("<W>$n creates a *tiny* wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>$n creates a *tiny* wall of air around you.  You are knocked over and trapped!<z>", FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_SUCCESS)) {
    act("<W>You create a wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_CHAR);
    act("<W>$n creates a wall of air around $N, knocking $M over and trapping $M!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>$n creates a wall of air around you.  You are knocked over and trapped!<z>", FALSE, caster, NULL, victim, TO_VICT);
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  } else {
    caster->nothingHappens();
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int suffocate(TBeing * caster, TBeing * victim, int level, short bKnown) 
{
  affectedData aff;
  int duration;

  if (victim->affectedBySpell(SPELL_SUFFOCATE)) {
    act("<W>$N is already immobilized!<z>",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  caster->reconcileHurt(victim,discArray[SPELL_SUFFOCATE]->alignMod);
  duration = level / 5;
//  duration = modifyForSector(duration, caster, victim, SPELL_SUFFOCATE, 0);
  aff.type = AFFECT_DISEASE;
  aff.level = level;
  aff.duration = duration;
  aff.modifier = DISEASE_SUFFOCATE;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SILENT;

// duration = spellNum, pointer to caster, victim, flags 

  if (caster->bSuccess(bKnown, SPELL_SUFFOCATE)) {

    if (victim->isLucky(caster->spellLuckModifier(SPELL_SUFFOCATE))) {
      SV(SPELL_SUFFOCATE);
      aff.duration /= 2;
    } else if (critSuccess(caster, SPELL_SUFFOCATE) == CRIT_S_DOUBLE) {
      CS(SPELL_SUFFOCATE);
      aff.duration *= 3;
    }
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    if (critFail(caster, SPELL_SUFFOCATE) == CRIT_F_HITSELF) {
      CF(SPELL_SUFFOCATE);
      caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
      return SPELL_CRIT_FAIL;
    } 
    return SPELL_FAIL;
  }
}

void suffocate(TBeing * caster, TBeing *victim,  TMagicItem * obj)
{
  int level,ret;

  if (victim->isImmortal()) {
    caster->sendTo(COLOR_SPELLS, "<W>Now that was pretty stupid of you...<z>\n\r");
    return;
  }

  level = obj->getMagicLevel();

  ret=suffocate(caster,victim,level,obj->getMagicLearnedness());
  if ((ret==SPELL_SUCCESS)) {
    if (caster == victim) {
      act("<W>Don't you feel stupid. You begin to choke!<z>", 
          FALSE, caster, NULL, NULL, TO_CHAR);
      act("<W>The air around $N dissipates!<z>", 
          FALSE, caster, NULL, caster, TO_NOTVICT);
    } else {
      act("<W>You remove the air from around $N!<z>", 
            FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>The air around $N dissipates!<z>", 
            FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>You gasp for air as $n removes the air around you!<z>", 
            FALSE, caster, NULL, victim, TO_VICT);
    }
  } else {
    if (ret==SPELL_CRIT_FAIL) {
      act("<W>Woooops! You remove the air from around yourself!<z>", 
          FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>Oopsies! The air around $n dissipates!<z>", 
          FALSE, caster, NULL, victim, TO_NOTVICT);
      act("<W>$n just tried to suffocate you!<z>", 
          FALSE, caster, NULL, victim, TO_VICT);
    }
  }
}

int suffocate(TBeing * caster, TBeing * victim)
{
  if (victim->affectedBySpell(SPELL_SUFFOCATE)) {
    act("<W>$N is already suffocating!<z>",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_SUFFOCATE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_SUFFOCATE]->lag;
  taskDiffT diff = discArray[SPELL_SUFFOCATE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_SUFFOCATE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castSuffocate(TBeing * caster, TBeing * victim)
{
  if (victim->isImmortal()) {
    act("<W>You can't suffocate $M, $E's immortal.<z>", TRUE, caster, 0, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  int level = caster->getSkillLevel(SPELL_SUFFOCATE);

  int ret=suffocate(caster, victim, level, caster->getSkillValue(SPELL_SUFFOCATE));
  if (ret==SPELL_SUCCESS) {
    act("<W>You remove the air from around $N!<z>", FALSE, caster, NULL, victim, TO_CHAR);
    act("<W>The air around $N dissipates!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>You gasp for air as $n removes the air around you!<z>", FALSE, caster, NULL, victim, TO_VICT);
    return TRUE;
    } else {
    if (ret==SPELL_CRIT_FAIL) {
      act("<W>Woooops! You remove the air from around yourself!<z>", FALSE, caster, NULL, victim, TO_CHAR);
      act("<W>Oopsies! The air around $n dissipates!<z>", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("<W>$n just tried to suffocate you!<z>", FALSE, caster, NULL, victim, TO_VICT);
    }
    return TRUE;
  }
}

int dustStorm(TBeing * caster, int level, short bKnown, int adv_learn)
{
  TThing *t=NULL;
  TBeing *tmp_victim = NULL;


  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("Not much air down here under all this water.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  level = min(level, 15);

  int dam = caster->getSkillDam(NULL, SPELL_DUST_STORM, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_DUST_STORM)) {
    if (critSuccess(caster, SPELL_DUST_STORM) == CRIT_S_DOUBLE) {
      CS(SPELL_DUST_STORM);
      dam *= 2;
    }
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;

      if (!caster->inGroup(*tmp_victim) && caster != tmp_victim) {
        act("$N chokes on the dust!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
        act("You choke on the dust!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
        caster->reconcileHurt(tmp_victim, discArray[SPELL_DUST_STORM]->alignMod);
        if (caster->reconcileDamage(tmp_victim, dam, SPELL_DUST_STORM) == -1) {
          delete tmp_victim;
          tmp_victim = NULL;
          continue;
        }
      } else {
        //act("$N dodges the dust!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
        act("You dodge the dust!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_DUST_STORM)) {
      CF(SPELL_DUST_STORM);
      for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
        tmp_victim = dynamic_cast<TBeing *>(t);
        if (!tmp_victim)
          continue;

        if (caster->inGroup(*tmp_victim) && caster != tmp_victim) {
          act("$N chokes on the dust!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
          act("You choke on the dust!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
          caster->reconcileHurt(tmp_victim, discArray[SPELL_DUST_STORM]->alignMod);
          if (caster->reconcileDamage(tmp_victim, dam, SPELL_DUST_STORM) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
        } else {
          //act("$N dodges the dust!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
          act("You dodge the dust!", FALSE, tmp_victim, NULL, NULL, TO_CHAR);
        }
      }
      return SPELL_CRIT_FAIL;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int dustStorm(TBeing * caster)
{
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("Not much air down here under all this water.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (!bPassMageChecks(caster, SPELL_DUST_STORM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_DUST_STORM]->lag;
  taskDiffT diff = discArray[SPELL_DUST_STORM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_DUST_STORM, diff, 1, "", rounds, caster->in_room, 0, 0, TRUE, 0);

  return TRUE;
} 

int castDustStorm(TBeing * caster)
{
  int level,ret;

  level = caster->getSkillLevel(SPELL_DUST_STORM);

  act("You kick up a whirlwind of dust!", FALSE, caster, NULL, NULL, TO_CHAR);
  act("$n kicks up a whirlwind of dust!", FALSE, caster, NULL, caster, TO_NOTVICT);

  ret=dustStorm(caster, level, caster->getSkillValue(SPELL_DUST_STORM), caster->getAdvLearning(SPELL_DUST_STORM));
  if (ret==SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int tornado(TBeing * caster, int level, short bKnown, int adv_learn)
{
  TThing *t, *ch;
  TBeing *tb;
  int rc;
  level = min(level, 33);

  int dam = caster->getSkillDam(NULL, SPELL_TORNADO, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_TORNADO)) {
    if (critSuccess(caster, SPELL_TORNADO) == CRIT_S_DOUBLE) {
      CS(SPELL_TORNADO);
      dam *= 2;
    }
    act("You've created a tornado!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n has created a tornado!", FALSE, caster, NULL, NULL, TO_ROOM);
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
      t=*(it++);
      if(!(tb=dynamic_cast<TBeing *>(t)))
	continue;

      if (1) {
        if (!caster->inGroup(*tb) && !tb->isImmortal()) {
          caster->reconcileHurt(tb, discArray[SPELL_TORNADO]->alignMod);
          act("$n is blasted by the force of the wind!", FALSE, t, NULL, 0, TO_ROOM);
          act("You are blasted by the force of the wind!", FALSE, tb, NULL, NULL, TO_CHAR);
          if (tb->riding) {
            rc = tb->fallOffMount(t->riding, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
              continue;
            }
          }
          while ((ch = tb->rider)) {
            rc = ch->fallOffMount(tb, POSITION_STANDING);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete ch;
              ch = NULL;
            }
          }
          tb->setPosition(POSITION_SITTING);
          if (caster->reconcileDamage(tb, dam, SPELL_TORNADO) == -1) {
            delete tb;
            tb = NULL;
            continue;
          }
          caster->setCharFighting((TBeing *)tb);
          caster->setVictFighting((TBeing *)tb);

          // no teleport if the room doesnt allow it
          if (caster->roomp->isRoomFlag(ROOM_HAVE_TO_WALK|ROOM_NO_FLEE|ROOM_NO_ESCAPE))
            continue;

          // teleport them away
          if (!caster->isNotPowerful(tb, level, SPELL_TORNADO, SILENT_YES))
          {
            act("Uhoh, Toto...", FALSE, tb, NULL, NULL, TO_CHAR);
            act("$n is swept away...", FALSE, tb, NULL, NULL, TO_ROOM);

            rc = tb->genericTeleport(SILENT_YES);
            tb->doLook("", CMD_LOOK);
            act("A tiny vortex sweeps through the room, depositing $n.",
                   FALSE, tb, NULL, NULL, TO_ROOM);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }

            rc = tb->genericMovedIntoRoom(tb->roomp, -1);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }
          }
        } else {
          act("$n manages to duck the tornado!", FALSE, tb, NULL, 0, TO_ROOM);
          act("You duck the tornado!", FALSE, tb, NULL, NULL, TO_CHAR);
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_TORNADO)) {
      act("You've managed to create some kind of tornado!", FALSE, caster, NULL, NULL, TO_CHAR);
      act("$n has created a tornado!", FALSE, caster, NULL, NULL, TO_ROOM);

      CF(SPELL_TORNADO);
      for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
        t=*(it++);
      if(!(tb=dynamic_cast<TBeing *>(t)))
	continue;

        if (1) {
          if (caster->inGroup(*tb)) {
            caster->reconcileHurt(tb, discArray[SPELL_TORNADO]->alignMod);
            act("$n chokes on the dust!", FALSE, tb, NULL, 0, TO_ROOM);
            act("You choke on the dust!", FALSE, tb, NULL, NULL, TO_CHAR);
            if (tb->riding) {
              rc = tb->fallOffMount(tb->riding, POSITION_STANDING);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete tb;
                tb = NULL;
                continue;
              }
            }
            while ((ch = tb->rider)) {
              rc = ch->fallOffMount(tb, POSITION_STANDING);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete ch;
                ch = NULL;
              }
            }
            tb->setPosition(POSITION_SITTING);
            if (caster->reconcileDamage((TBeing *)tb, dam, SPELL_TORNADO) == -1) {
              delete tb;
              tb = NULL;
              continue;
            }

            if (caster->roomp->isRoomFlag(ROOM_HAVE_TO_WALK|ROOM_NO_FLEE|ROOM_NO_ESCAPE))
              continue;

            act("Uhoh, Toto...", FALSE, tb, NULL, NULL, TO_CHAR);
            act("$n is swept away...", FALSE, tb, NULL, NULL, TO_ROOM);

            rc = tb->genericTeleport(SILENT_YES);
            tb->doLook("", CMD_LOOK);
            act("A tiny vortex sweeps through the room, depositing $n.",
                   FALSE, tb, NULL, NULL, TO_ROOM);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }

            rc = tb->genericMovedIntoRoom(tb->roomp, -1);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete tb;
              tb = NULL;
            }
          } else {
            //act("$n dodges the vortex!", FALSE, t, NULL, 0, TO_ROOM);
            act("You dodge the vortex!", FALSE, tb, NULL, NULL, TO_CHAR);
          }
        }
      }
      return SPELL_CRIT_FAIL;
    }
    return SPELL_FAIL;
  }
}

void tornado(TBeing * caster, TMagicItem * obj)
{
  int ret, level;

  level = obj->getMagicLevel();

  if (!caster->outside()) {
    caster->sendTo("You can only cast that outside!\n\r");
    return;
  }

  act("You've created a tornado!", FALSE, caster, NULL, NULL, TO_CHAR);
  act("$n has created a tornado!", FALSE, caster, NULL, NULL, TO_ROOM);

  ret=tornado(caster,level,obj->getMagicLearnedness(), 0);
  if (ret==SPELL_SUCCESS) {
  } else {
  }
}

void tornado(TBeing * caster)
{
  if (!bPassMageChecks(caster, SPELL_TORNADO, NULL)) {
    caster->sendTo("You did not pass all your mage checks!\n\r");
    return;
  }

  if (!caster->outside()) {
    caster->sendTo("You can only cast that outside!\n\r");
    return;
  }
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You can't cast that underwater!\n\r");
    return;
  }

  lag_t rounds = discArray[SPELL_TORNADO]->lag;
  taskDiffT diff = discArray[SPELL_TORNADO]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_TORNADO, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
}

int castTornado(TBeing * caster)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_TORNADO);

  ret=tornado(caster,level,caster->getSkillValue(SPELL_TORNADO), caster->getAdvLearning(SPELL_TORNADO));
  if (ret==SPELL_SUCCESS) {
    return TRUE;
  } else {
    act("You feel a hint of a breeze but your spell fails!", FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
  }
  return FALSE;
}

int featheryDescent(TBeing * caster, TBeing * victim, int, affectedData * aff, short bKnown)
{
  caster->reconcileHelp(victim,discArray[SPELL_FEATHERY_DESCENT]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_FEATHERY_DESCENT)) {

    switch (critSuccess(caster, SPELL_FEATHERY_DESCENT)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FEATHERY_DESCENT);
        aff->duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }
    victim->affectJoin(caster, aff, AVG_DUR_NO, AVG_EFF_YES);
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void featheryDescent(TBeing * caster, TBeing *victim, TMagicItem * obj)
{
  featheryDescent(caster, victim, obj->getMagicLevel()/3,obj->getMagicLearnedness());
}

void featheryDescent(TBeing *caster, TBeing *victim, int level, int learn)
{
  affectedData aff;
  int ret;

  aff.type = SPELL_FEATHERY_DESCENT;
  aff.level = level;
  aff.duration = (aff.level) * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  if (victim->affectedBySpell(SPELL_FEATHERY_DESCENT)) {
    caster->nothingHappens();
    return;
  }

  ret = featheryDescent(caster,victim,level,&aff,learn);

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$N seems lighter on $S feet!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You feel much \"lighter\"!", FALSE, victim, NULL, NULL, TO_CHAR);
    victim->sendTo("You have been granted the gift of featherfall!\n\r");
  } else {
    caster->nothingHappens();
  }
}

int featheryDescent(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_FEATHERY_DESCENT, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_FEATHERY_DESCENT]->lag;
  taskDiffT diff = discArray[SPELL_FEATHERY_DESCENT]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_FEATHERY_DESCENT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}
int castFeatheryDescent(TBeing * caster, TBeing * victim)
{
  affectedData aff;

  int level = caster->getSkillLevel(SPELL_FEATHERY_DESCENT);

  aff.type = SPELL_FEATHERY_DESCENT;
  aff.level = level;
  aff.duration = aff.level * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;

  int ret=featheryDescent(caster,victim,level,&aff,caster->getSkillValue(SPELL_FEATHERY_DESCENT));
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$N seems lighter on $S feet!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You feel much \"lighter\"!", FALSE, victim, NULL, NULL, TO_CHAR);
    victim->sendTo("You have been granted the gift of featherfall!\n\r");
    if (caster != victim)
      act("You have given $N the gift of featherfall!", 
               FALSE, caster, NULL, victim, TO_CHAR);
   } else {
     caster->nothingHappens();
  }
  return TRUE;
}

int fly(TBeing * caster, TBeing * victim, int, affectedData * aff, short bKnown)
{
  caster->reconcileHelp(victim,discArray[SPELL_FLY]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_FLY)) {

    switch (critSuccess(caster, SPELL_FLY)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FLY);
        aff->duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }
    victim->affectJoin(caster, aff, AVG_DUR_NO, AVG_EFF_YES);
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

void weightCorrectDuration(const TBeing *victim, affectedData *aff)
{
  aff->duration = (int) (aff->duration * 170.0 / victim->getWeight());
}

void fly(TBeing * caster, TBeing *victim,  TMagicItem * obj)
{
  affectedData aff;
  int level;

  level = obj->getMagicLevel();

  aff.type = SPELL_FLY;
  aff.level = level;
  aff.duration = 1 * UPDATES_PER_MUDHOUR * level;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_FLYING;

  // correct for weight
  weightCorrectDuration(victim, &aff);

  int ret = fly(caster,victim,level,&aff,obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster == victim) {
      caster->sendTo("You feel much \"lighter\"!\n\r");
      act("$n seems lighter on $s feet!", FALSE, caster, NULL, 0, TO_ROOM);
    } else {
      victim->sendTo("You feel much \"lighter\"!\n\r");
      act("$n seems lighter on $s feet!", FALSE, victim, NULL, 0, TO_ROOM);
    }
  } else {
     caster->nothingHappens();
  }
}

int fly(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_FLY, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_FLY]->lag;
  taskDiffT diff = discArray[SPELL_FLY]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_FLY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castFly(TBeing * caster, TBeing * victim)
{
  affectedData aff;

  int level = caster->getSkillLevel(SPELL_FLY);

  aff.type = SPELL_FLY;
  aff.level = level;
  aff.duration = 3 * UPDATES_PER_MUDHOUR * level;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_FLYING;

  // correct for weight
  weightCorrectDuration(victim, &aff);

  int ret=fly(caster,victim,level,&aff,caster->getSkillValue(SPELL_FLY));
  if (IS_SET(ret, SPELL_SUCCESS)) {
    if (caster == victim) {
      caster->sendTo("You feel much \"lighter\"!\n\r");
      act("$n seems lighter on $s feet!", FALSE, caster, NULL, 0, TO_ROOM);
    } else {
      victim->sendTo("You feel much \"lighter\"!\n\r");
      act("$n seems lighter on $s feet!", FALSE, victim, NULL, 0, TO_ROOM);
    }
  } else {
     caster->nothingHappens();
  }
  return TRUE;
}

int antigravity(TBeing *caster, int, affectedData *aff, short bKnown)
{
  TThing *t=NULL;
  TBeing *vict = NULL;
  char buf[80];
  
  if (caster->bSuccess(bKnown, SPELL_ANTIGRAVITY)) {

    switch (critSuccess(caster, SPELL_ANTIGRAVITY)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ANTIGRAVITY);
        aff->duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;
      if ((caster == vict) || (caster->inGroup(*vict))) {
        if ( vict->isAffected(AFF_LEVITATING) || vict->canFly()) {
          
          if (caster == vict)
            sprintf(buf, "You are already capable of some form of flight!\n\r");
          else
            sprintf(buf, "%s is already capable of some form of flight!\n\r",vict->getName());
          
          caster->sendTo(buf);
          caster->nothingHappens(SILENT_YES);
          continue;
        }
        caster->reconcileHelp(vict,discArray[SPELL_ANTIGRAVITY]->alignMod);
        act("You begin to levitate! You're floating in the air!", 
                    TRUE, vict, NULL, NULL, TO_CHAR);
        act("With the grace of an angel, $n floats up off the $g!",
                    TRUE, vict, NULL, NULL, TO_ROOM);
        vict->affectJoin(caster, aff, AVG_DUR_NO, AVG_EFF_YES);
      }
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int antigravity(TBeing * caster)
{

  if (!bPassMageChecks(caster, SPELL_ANTIGRAVITY, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ANTIGRAVITY]->lag;
  taskDiffT diff = discArray[SPELL_ANTIGRAVITY]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ANTIGRAVITY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castAntigravity(TBeing * caster)
{
  affectedData aff;
  int ret,level;

  if (!caster->roomp)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ANTIGRAVITY);

  aff.type = SPELL_LEVITATE;
  aff.level = level;
  aff.duration = (caster->isImmortal() ? caster->GetMaxLevel() : 3) * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_LEVITATING;

  if ((ret=antigravity(caster,level,&aff,caster->getSkillValue(SPELL_ANTIGRAVITY))) == SPELL_SUCCESS) {
    act("$n makes a minor change to the laws of physics.", TRUE,caster,0,0,TO_ROOM);
    act("You alter the forces of space and time slightly.", TRUE,caster,0,0,TO_CHAR);
  } else 
    caster->nothingHappens();
    return TRUE;
}

int conjureElemAir(TBeing * caster, int level, short bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(Mob::AIR_ELEMENTAL, VIRTUAL))) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CONJURE_AIR, 0);

  if (caster->bSuccess(bKnown, SPELL_CONJURE_AIR)) {
     act("You summon the powers of the sky!", 
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of the sky!", 
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while 
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CONJURE_AIR;
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

    // Add hp for higher levels - Russ 
    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CONJURE_AIR)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CONJURE_AIR);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim, TO_ROOM);
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
      victim->affectFrom(SPELL_CONJURE_AIR);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("When your eyes recover, you see $N standing before you!", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int conjureElemAir(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::AIR_ELEMENTAL) < 0) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_CONJURE_AIR, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CONJURE_AIR]->lag;
  taskDiffT diff = discArray[SPELL_CONJURE_AIR]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CONJURE_AIR, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castConjureElemAir(TBeing * caster)
{
   int ret,level; 

   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_CONJURE_AIR);

   if ((ret=conjureElemAir(caster,level,caster->getSkillValue(SPELL_CONJURE_AIR))) == SPELL_SUCCESS) {
   } else {
     act("Hmmm...that didn't feel quite right.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}


int levitate(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  affectedData aff;

  aff.type = SPELL_LEVITATE;
  aff.level = level;
  aff.duration = 3 * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_LEVITATING;

  if (caster->bSuccess(bKnown, SPELL_LEVITATE)) {
    switch (critSuccess(caster, SPELL_LEVITATE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_LEVITATE);
        aff.duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES))
      return SPELL_FAIL;

    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int castLevitate(TBeing * caster, TBeing * victim)
{
  int level = caster->getSkillLevel(SPELL_LEVITATE);

  int ret=levitate(caster,victim,level,caster->getSkillValue(SPELL_LEVITATE));
  if (ret == SPELL_SUCCESS) {
    victim->sendTo("You feel much \"lighter\"!\n\r");
    victim->sendTo("You begin to levitate! You're floating in the air!\n\r");
    act("With the grace of an angel, $n floats up off the $g!",
         FALSE, victim, NULL, 0, TO_ROOM);
    act("$n seems lighter on $s feet!",
          FALSE, victim, NULL, 0, TO_ROOM);
    return TRUE;
  } else {
    caster->nothingHappens();
  }
  return FALSE;
}

void levitate(TBeing * caster, TBeing * victim)
{
  if (!bPassMageChecks(caster, SPELL_LEVITATE, victim))
    return;

  lag_t rounds = discArray[SPELL_LEVITATE]->lag;
  taskDiffT diff = discArray[SPELL_LEVITATE]->task;

  if (victim->getPosition() != POSITION_STANDING) {
    caster->sendTo("You can't induce levitation on someone that is not standing.\n\r");
    caster->nothingHappens(SILENT_YES);
    return;
  }

  if (victim->roomp->isUnderwaterSector()) {
    act("You can't seem to induce levitation underwater.",
           FALSE, caster, 0,0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return;
  }

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LEVITATE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return;
}

int falconWings(TBeing * caster, TBeing * victim, int level, short bKnown)
{
  affectedData aff;

  if (victim->affectedBySpell(SPELL_FLY) ||
      victim->affectedBySpell(SPELL_FALCON_WINGS)) {
    act("$N is already affected by a some type of flight spell.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  aff.type = SPELL_FALCON_WINGS;
  aff.level = level;
  aff.duration = 3 * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_FLYING;

  // correct for weight
  weightCorrectDuration(victim, &aff);

  if (caster->bSuccess(bKnown, SPELL_FALCON_WINGS)) {

    switch (critSuccess(caster, SPELL_FALCON_WINGS)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FALCON_WINGS);
        aff.duration >>= 1;
        break;
      case CRIT_S_NONE:
        break;
    }

    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    victim->sendTo("Feathers sprout from your arms!\n\r");
    victim->sendTo("You feel as if you could fly!!\n\r");
    act("Feathers sprout from $n's arms and $e leaps into the sky!",
         FALSE, victim, NULL, 0, TO_ROOM);
    act("$n seems capable of flight!",
          FALSE, victim, NULL, 0, TO_ROOM);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void falconWings(TBeing * caster, TBeing *victim, TMagicItem * obj)
{
  int level;

  level = obj->getMagicLevel();

  falconWings(caster,victim,level,obj->getMagicLearnedness());
}

int falconWings(TBeing * caster, TBeing * victim)
{
  taskDiffT diff;

  if (caster->roomp->isUnderwaterSector()) {
   caster->sendTo("Falcons can't fly under these wet conditions!\n\r");
   return FALSE;
  }
  if (victim->affectedBySpell(SPELL_FALCON_WINGS) || victim->affectedBySpell(SPELL_FLY)) {
    act("$N is already affected by a some type of flight spell.", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

     if (!bPassMageChecks(caster, SPELL_FALCON_WINGS, victim))
        return FALSE;

       lag_t rounds = discArray[SPELL_FALCON_WINGS]->lag;
       diff = discArray[SPELL_FALCON_WINGS]->task;

       start_cast(caster, victim, NULL, caster->roomp, SPELL_FALCON_WINGS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
       return TRUE;
}

int castFalconWings(TBeing * caster, TBeing * victim)
{
  int ret,level;

  caster->reconcileHelp(victim,discArray[SPELL_FALCON_WINGS]->alignMod);

  level = caster->getSkillLevel(SPELL_FALCON_WINGS);

  if ((ret=falconWings(caster,victim,level,caster->getSkillValue(SPELL_FALCON_WINGS))) == SPELL_SUCCESS) {
  } else {
  }
  return FALSE;
}

int protectionFromAir(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;
 
  aff.type = SPELL_PROTECTION_FROM_AIR;
  aff.level = level;
  aff.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_AIR;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;
 
  if (caster->bSuccess(bKnown,SPELL_PROTECTION_FROM_AIR)) {
    act("$n glows with a faint blue aura for a brief moment.", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a faint blue aura for a brief moment.", FALSE, victim, NULL, NULL, TO_CHAR);
    switch (critSuccess(caster, SPELL_PROTECTION_FROM_AIR)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_PROTECTION_FROM_AIR);
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
    caster->reconcileHelp(victim, discArray[SPELL_PROTECTION_FROM_AIR]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void protectionFromAir(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  protectionFromAir(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int protectionFromAir(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

     if (!bPassMageChecks(caster, SPELL_PROTECTION_FROM_AIR, victim))
        return FALSE;

     lag_t rounds = discArray[SPELL_PROTECTION_FROM_AIR]->lag;
     diff = discArray[SPELL_PROTECTION_FROM_AIR]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_PROTECTION_FROM_AIR, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
        return TRUE;
}

int castProtectionFromAir(TBeing *caster, TBeing *victim)
{
int ret,level;
 
  level = caster->getSkillLevel(SPELL_PROTECTION_FROM_AIR);
 
  if ((ret=protectionFromAir(caster,victim,level,caster->getSkillValue(SPELL_PROTECTION_FROM_AIR))) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}
 
CDAir::CDAir() :
  CDiscipline(),
  skImmobilize(),
  skSuffocate(),
  skFly(),
  skAntigravity()
{
}

CDAir::CDAir(const CDAir &a) :
  CDiscipline(a),
  skImmobilize(a.skImmobilize),
  skSuffocate(a.skSuffocate),
  skFly(a.skFly),
  skAntigravity(a.skAntigravity)
{
}

CDAir & CDAir::operator= (const CDAir &a)
{
  if (this == &a) return *this;

  CDiscipline::operator=(a);
  skImmobilize = a.skImmobilize;
  skSuffocate = a.skSuffocate;
  skFly = a.skFly;
  skAntigravity = a.skAntigravity;
  return *this;
}

CDAir::~CDAir()
{
}

CDAir * CDAir::cloneMe()
{
  return new CDAir(*this);
}
