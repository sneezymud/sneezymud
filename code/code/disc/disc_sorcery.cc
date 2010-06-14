//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include <cmath>

#include "room.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_sorcery.h"
#include "obj_armor.h"
#include "obj_magic_item.h"
#include "obj_worn.h"
#include "person.h"

int mysticDarts(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  char buf[256];
  sstring misBuf;

  if (caster->isNotPowerful(victim, level, SPELL_MYSTIC_DARTS, SILENT_NO))
    return SPELL_FAIL;

  level = min(level, 10);

  int dam = caster->getSkillDam(victim, SPELL_MYSTIC_DARTS, level, adv_learn);
  // Lets make the missiles at least partly dependant on damage.
  int missiles = (dam / 3) + ::number(0, (caster->GetMaxLevel() / 10));
  missiles = max(missiles, 1);

  caster->reconcileHurt(victim, discArray[SPELL_MYSTIC_DARTS]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_MYSTIC_DARTS)) {
    switch (critSuccess(caster, SPELL_MYSTIC_DARTS)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_MYSTIC_DARTS);
        dam *= 2;
        missiles *= 2;
        sprintf(buf, "%d", missiles);
        misBuf = buf;
        misBuf += " gigantic mystical dart";
        if (missiles != 1)
          misBuf += "s stream";
        else
          misBuf += " streams";

        sprintf(buf, "%s from $n's fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's fingertips, blasting you!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_MYSTIC_DARTS);
        dam *= 3;
        missiles *=3;

        sprintf(buf, "%d", missiles);
        misBuf = buf;
        misBuf += " HUMONGOUS mystical dart";
        if (missiles != 1)
          misBuf += "s stream";
        else
          misBuf += " streams";

        sprintf(buf, "%s from $n's fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's fingertips, blasting you!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        break;
      case CRIT_S_NONE:
        sprintf(buf, "%d", missiles);
        misBuf = buf;
        misBuf += " mystical dart";
        if (missiles != 1)
          misBuf += "s stream";
        else
          misBuf += " streams";

        sprintf(buf, "%s from $n's fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your fingertips, blasting $N!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's fingertips, blasting you!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        if (victim->isLucky(caster->spellLuckModifier(SPELL_MYSTIC_DARTS))) {
          SV(SPELL_MYSTIC_DARTS);
          dam /= 2;
        }
    }


    victim->roomp->playsound(SOUND_SPELL_MYSTIC_DART, SOUND_TYPE_MAGIC);

    if (caster->reconcileDamage(victim, dam, SPELL_MYSTIC_DARTS) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_MYSTIC_DARTS)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_MYSTIC_DARTS);
        sprintf(buf, "%d", missiles);
        misBuf = buf;
        misBuf += " mystical dart";
        if (missiles != 1)
          misBuf += "s stream";
        else
          misBuf += " streams";

        sprintf(buf, "%s from $n's fingertips and blow up in $n's face!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_NOTVICT);
        sprintf(buf, "%s from your fingertips and blow up in your face!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_CHAR);
        sprintf(buf, "%s from $n's fingertips and blow up in $n's face!", misBuf.c_str());
        act(buf, FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, SPELL_MYSTIC_DARTS) == -1)
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

int mysticDarts(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int rc = 0;
  int ret = 0;

  ret = mysticDarts(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD)) 
    ADD_DELETE(rc, DELETE_VICT);
  
  if (IS_SET(ret, CASTER_DEAD)) 
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int mysticDarts(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_MYSTIC_DARTS, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_MYSTIC_DARTS]->lag;
  diff = discArray[SPELL_MYSTIC_DARTS]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_MYSTIC_DARTS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castMysticDarts(TBeing *caster, TBeing *victim)
{
  int level;
  int rc = 0;
  int ret = 0;

  level = caster->getSkillLevel(SPELL_MYSTIC_DARTS);
  int bKnown = caster->getSkillValue(SPELL_MYSTIC_DARTS);

  ret=mysticDarts(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_MYSTIC_DARTS));

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int stunningArrow(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't arrow $N -- $E's a god!", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 25);

  int dam = caster->getSkillDam(victim, SPELL_STUNNING_ARROW, level, adv_learn);
  caster->reconcileHurt(victim, discArray[SPELL_STUNNING_ARROW]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_STUNNING_ARROW)) {
    switch (critSuccess(caster, SPELL_STUNNING_ARROW)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        act("A **HUGE** <k>silver<1>-<o>orange<1> bolt leaps from your arms aimed at $N.", 
               FALSE,caster,0,victim, TO_CHAR);
        act("A **HUGE** <k>silver<1>-<o>orange<1> bolt leaps from $n's arms aimed at you!", 
               FALSE, caster,0,victim, TO_VICT);
        act("A **HUGE** <k>silver<1>-<o>orange<1> bolt leaps from $n's arms aimed at $N.", 
               FALSE,caster,0,victim, TO_NOTVICT);
        CS(SPELL_STUNNING_ARROW);
        dam <<= 1;
        break;
      case CRIT_S_NONE:
        act("A <k>silver<1>-<o>orange<1> bolt leaps from your arms aimed at $N.", 
               FALSE,caster,0,victim, TO_CHAR);
        act("A <k>silver<1>-<o>orange<1> bolt leaps from $n's arms aimed at you!", 
               FALSE, caster,0,victim, TO_VICT);
        act("A <k>silver<1>-<o>orange<1> bolt leaps from $n's arms aimed at $N.", 
               FALSE,caster,0,victim, TO_NOTVICT);
        if (victim->isLucky(caster->spellLuckModifier(SPELL_STUNNING_ARROW))) {
          SV(SPELL_STUNNING_ARROW);
          dam /= 2;
        }
    }

    victim->roomp->playsound(SOUND_SPELL_STUNNING_ARROW, SOUND_TYPE_MAGIC);

    if (caster->reconcileDamage(victim, dam, SPELL_STUNNING_ARROW) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;

    return SPELL_SUCCESS;
  } else {
    switch(critFail(caster, SPELL_STUNNING_ARROW)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_STUNNING_ARROW);
      act("$n stiffens as an electric bolt backfires at $mself!", 
               FALSE, caster, NULL, 0, TO_ROOM);
      act("You stiffen as an electric bolt backfires!", 
               FALSE, caster, NULL, victim, TO_CHAR);
        if (caster->reconcileDamage(caster, dam, SPELL_STUNNING_ARROW) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int stunningArrow(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  ret = stunningArrow(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int stunningArrow(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_STUNNING_ARROW, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_STUNNING_ARROW]->lag;
  diff = discArray[SPELL_STUNNING_ARROW]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_STUNNING_ARROW, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castStunningArrow(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_STUNNING_ARROW);
  int bKnown = caster->getSkillValue(SPELL_STUNNING_ARROW);

  ret=stunningArrow(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_STUNNING_ARROW));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int blastOfFury(TBeing *caster, TBeing *victim, TMagicItem *tObj)
{
  int tLevel = tObj->getMagicLevel(),
      tKnown = tObj->getMagicLearnedness(),
      tReturn = 0;

  tReturn = blastOfFury(caster, victim, tLevel, tKnown, 0);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}

int blastOfFury(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't blast $N -- $E's a god! ", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  level = min(level, 45);

  int dam = caster->getSkillDam(victim, SPELL_BLAST_OF_FURY, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_BLAST_OF_FURY)) {
    caster->reconcileHurt(victim, discArray[SPELL_BLAST_OF_FURY]->alignMod);

    switch (critSuccess(caster, SPELL_BLAST_OF_FURY)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BLAST_OF_FURY);
        dam <<= 1;
	
	if(!strcmp(caster->name, "Coppern")){
	  act("$n unleashes all $s LATENT HOMOSEXUAL ANGER AND FRUSTRATION on $N!", 
	      FALSE, caster, NULL, victim, TO_NOTVICT);
	  act("You unleash all your PENT UP ANGER AND FRUSTRATION on $N!", 
	      FALSE, caster, NULL, victim, TO_CHAR);
	  act("$n unleashes all $s LATENT HOMOSEXUAL ANGER AND FRUSTRATION on you!", 
	      FALSE, caster, NULL, victim, TO_VICT);
	} else {
	  act("$n unleashes all $s PENT UP ANGER AND FRUSTRATION on $N!", 
	      FALSE, caster, NULL, victim, TO_NOTVICT);
	  act("You unleash all your PENT UP ANGER AND FRUSTRATION on $N!", 
	      FALSE, caster, NULL, victim, TO_CHAR);
	  act("$n unleashes all $s PENT UP ANGER AND FRUSTRATION on you!", 
	      FALSE, caster, NULL, victim, TO_VICT);
	}
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_BLAST_OF_FURY))) {
          SV(SPELL_BLAST_OF_FURY);
          dam /= 2;
          act("$n unleashes a small portion of $s pent up anger on $N!", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("You unleash a small portion of your pent up anger on $N!", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("$n unleashes a small portion of $s pent up anger on you!", 
                  FALSE, caster, NULL, victim, TO_VICT);
        } else {
          act("$n unleashes all $s pent up anger on $N!", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
          act("You unleash all your pent up anger on $N!", 
                  FALSE, caster, NULL, victim, TO_CHAR);
          act("$n unleashes all $s pent up anger on you!", 
                  FALSE, caster, NULL, victim, TO_VICT);
        }
        break;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_BLAST_OF_FURY) == -1)
      return SPELL_FAIL + VICTIM_DEAD;
    return SPELL_FAIL;
  } else {
    switch(critFail(caster, SPELL_BLAST_OF_FURY)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_BLAST_OF_FURY);
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
        act("$n get's really pissed at $mself for screwing up this spell!", 
           FALSE, caster, NULL, victim, TO_NOTVICT);
        act("You get mad at yourself!", 
           FALSE, caster, NULL, victim, TO_CHAR);
        act("$n just tried to get pissed at you!", 
           FALSE, caster, NULL, victim, TO_VICT);
        if (caster->reconcileDamage(caster, dam, SPELL_BLAST_OF_FURY) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int blastOfFury(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_BLAST_OF_FURY, victim))
       return FALSE;

  lag_t rounds = discArray[SPELL_BLAST_OF_FURY]->lag;
  diff = discArray[SPELL_BLAST_OF_FURY]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_BLAST_OF_FURY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

     return TRUE;
}

int castBlastOfFury(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_BLAST_OF_FURY);
  int bKnown = caster->getSkillValue(SPELL_BLAST_OF_FURY);

  ret=blastOfFury(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_BLAST_OF_FURY));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int colorSpray(TBeing *caster, int level, short bKnown, int adv_learn)
{
  TBeing *tmp_victim = NULL;
  TThing *t;

  level = min(level, 15);

  int orig_dam = caster->getSkillDam(NULL, SPELL_COLOR_SPRAY, level, adv_learn);

  if (caster->bSuccess(bKnown,SPELL_COLOR_SPRAY)) {
    switch (critSuccess(caster, SPELL_COLOR_SPRAY)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_COLOR_SPRAY);
        orig_dam *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
      t=*(it++);
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim || (tmp_victim->isPc() && !tmp_victim->desc))
        continue;

      if ((caster != tmp_victim) && !tmp_victim->isImmortal()) {
        if (!caster->inGroup(*tmp_victim)) {
          caster->reconcileHurt(tmp_victim, discArray[SPELL_COLOR_SPRAY]->alignMod);
          int dam = orig_dam;

          act("$n sprays a huge blazing fan of bright clashing colors at $N!", 
              FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
          act("You spray a huge blazing fan of bright clashing colors at $N!", 
              FALSE, caster, NULL, tmp_victim, TO_CHAR);
          act("$n sprays a huge blazing fan of bright clashing colors at you!", 
              FALSE, caster, NULL, tmp_victim, TO_VICT);

          if (tmp_victim->isLucky(caster->spellLuckModifier(SPELL_COLOR_SPRAY)))
            dam/= 2;

          if (caster->reconcileDamage(tmp_victim, dam, SPELL_COLOR_SPRAY) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
          }
        } else {
          act("You are able to dodge the wave of colors!", FALSE, tmp_victim, 0, 0, TO_CHAR);
          //act("$n is able to dodge the wave of colors!", FALSE, tmp_victim, 0, 0, TO_ROOM);
        }
      }
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_COLOR_SPRAY)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_COLOR_SPRAY);
        act("A bright splash of clashing color explodes in $n's face!", 
                FALSE, caster, NULL, NULL, TO_ROOM);
        act("A bright splash of clashing color explodes in your face!", 
                FALSE, caster, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(caster, orig_dam, SPELL_COLOR_SPRAY) == -1)
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

int colorSpray(TBeing *caster, TMagicItem * obj)
{
  int ret = 0;
  int rc = 0;

  act("Prismatic ribbons of light leap from $p and circle the room.",
        TRUE, caster, obj, 0, TO_CHAR);
  act("Prismatic ribbons of light leap from $p and circle the room.",
        TRUE, caster, obj, 0, TO_ROOM);

  ret = colorSpray(caster,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int colorSpray(TBeing *caster)
{
  TThing *t;
  TBeing *victim, *vict = NULL;
  taskDiffT diff;
  bool found=FALSE;

  if (!bPassMageChecks(caster, SPELL_COLOR_SPRAY, vict))
    return TRUE;

  lag_t rounds = discArray[SPELL_COLOR_SPRAY]->lag;
  diff = discArray[SPELL_COLOR_SPRAY]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_COLOR_SPRAY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
    t=*(it++);
    victim = dynamic_cast<TBeing *>(t);
    if (!victim || (victim->isPc() && !victim->desc))
      continue;
    if (!caster->inGroup(*victim) && !victim->isImmortal()) {
    }
    found = TRUE;
  }
  return TRUE;
}

int castColorSpray(TBeing *caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_COLOR_SPRAY);
  int bKnown = caster->getSkillValue(SPELL_COLOR_SPRAY);

  act("Your hands become prismatic as you wave them around the room.",
        TRUE, caster, 0, 0, TO_CHAR);
  act("$n's hands become prismatic as $e waves them around the room.",
        TRUE, caster, 0, 0, TO_ROOM);

  ret=colorSpray(caster,level,bKnown, caster->getAdvLearning(SPELL_COLOR_SPRAY));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int energyDrain(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't drain $N's energy -- $E's a god!",
             FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  int dam = caster->getSkillDam(victim, SPELL_ENERGY_DRAIN, level, adv_learn);
  caster->reconcileHurt(victim, discArray[SPELL_ENERGY_DRAIN]->alignMod);
  bool save = victim->isLucky(caster->spellLuckModifier(SPELL_ENERGY_DRAIN));
  int vit = dice(number(1,level),4);

  if (victim->getImmunity(IMMUNE_DRAIN) >= 100) {
    act("$N appears to be immune!", FALSE, caster, 0, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }

  if (caster->bSuccess(bKnown,SPELL_ENERGY_DRAIN)) {
    act("$N screams in agony as energy pours from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N screams in agony as energy pours from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You scream in agony as energy pours from your body!", FALSE, caster, NULL, victim, TO_VICT);
    TPerson *pers;
    switch (critSuccess(caster, SPELL_ENERGY_DRAIN)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
        CS(SPELL_ENERGY_DRAIN);
        dam *=2;
        vit *=2;
        break;
      case CRIT_S_KILL:
        CS(SPELL_ENERGY_DRAIN);
        // Real nasty stuff for critical success - Russ 
        pers = dynamic_cast<TPerson *>(victim);
        if (pers && !save) {
          pers->dropLevel(pers->bestClass());
          pers->setTitle(false);
          vlogf(LOG_MISC, format("%s just lost a level from energy drain! :-)") %  pers->getName());
        }
        break;
      case CRIT_S_NONE:
        break;
    }
    if (save) {
      SV(SPELL_ENERGY_DRAIN);
      dam /= 2;
      vit /= 2;
    }
    if (!victim->isImmortal())
      victim->addToMove(-vit);
    if (caster->reconcileDamage(victim, dam, SPELL_ENERGY_DRAIN) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_ENERGY_DRAIN)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_ENERGY_DRAIN);
        act("$n moans in agony as energy drains from $s body!", 
               FALSE, caster, NULL, NULL, TO_ROOM);
        act("You moan in agony and feel your own energy drain from your body!", 
               FALSE, caster, NULL, NULL, TO_CHAR);
        caster->addToMove(-vit);
        dam /= 3;
        if (caster->reconcileDamage(caster, dam, SPELL_ENERGY_DRAIN) == -1)
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int energyDrain(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_ENERGY_DRAIN, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENERGY_DRAIN]->lag;
  taskDiffT diff = discArray[SPELL_ENERGY_DRAIN]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_ENERGY_DRAIN, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE; 
}

int castEnergyDrain(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_ENERGY_DRAIN);
  int bKnown = caster->getSkillValue(SPELL_ENERGY_DRAIN);

  ret=energyDrain(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_ENERGY_DRAIN));
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int energyDrain(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = energyDrain(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

int acidBlast(TBeing *caster, int level, short bKnown, int adv_learn)
{
  TThing *t;
  TBeing *b = NULL;

  level = min(level, 33);

  int orig_dam = caster->getSkillDam(NULL, SPELL_ACID_BLAST, level, adv_learn);

  if (caster->bSuccess(bKnown,SPELL_ACID_BLAST)) {
    switch (critSuccess(caster, SPELL_ACID_BLAST)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ACID_BLAST);
        orig_dam *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
      t=*(it++);
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt && (caster != tbt) && !tbt->isImmortal()) {
        if (!caster->inGroup(*tbt)) {
          caster->reconcileHurt(tbt, discArray[SPELL_ACID_BLAST]->alignMod);
          int dam = orig_dam;

          act("A large shower of acid sprays over $N!", 
              FALSE, caster, NULL, tbt, TO_NOTVICT);
          act("You shower $N in an acid spray!", 
              FALSE, caster, NULL, tbt, TO_CHAR);
          act("$n showers you in acid!", FALSE, caster, NULL, tbt, TO_VICT);
          if (tbt->isLucky(caster->spellLuckModifier(SPELL_ACID_BLAST)))
            dam /= 2;

          if (caster->reconcileDamage(tbt, dam, SPELL_ACID_BLAST) == -1) {
            delete tbt;
            tbt = NULL;
            continue;
          }
        }
      } else if (tbt) {
        act("$n is able to avoid the blast of acid!", 
            FALSE, tbt, 0, 0, TO_ROOM);
        act("You are able to avoid the blast of acid!", 
            FALSE, tbt, 0, 0, TO_CHAR);
      }
    }
    caster->acidRoom();
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_ACID_BLAST)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_ACID_BLAST);
          for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
            t=*(it++);
            b = dynamic_cast<TBeing *>(t);
            if (b && (caster != b) && (!b->isImmortal())) {
               if (caster->inGroup(*b)) {
                 caster->reconcileHurt(b,discArray[SPELL_ACID_BLAST]->alignMod);
                 int dam = orig_dam;

                 act("A large shower of acid sprays over $n!", FALSE, caster, NULL, NULL, TO_ROOM);
                 act("Something went wrong! Acid sprays all over you!", FALSE, caster, NULL, t, TO_CHAR);
                 act("Hey, that acid was intended for you!", FALSE, caster, NULL, t, TO_VICT);
                 if (b->isLucky(caster->spellLuckModifier(SPELL_ACID_BLAST))) 
                   dam/= 2;

                 if (caster->reconcileDamage(b, dam, SPELL_ACID_BLAST) == -1) {
                   delete b;
                   b = NULL;
                 }
               }
             }
          }
          caster->acidRoom();
          return SPELL_CRIT_FAIL;
          break;
        case CRIT_F_NONE:
          break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int acidBlast(TBeing *caster)
{
  if (!bPassMageChecks(caster, SPELL_ACID_BLAST, NULL))
    return TRUE;

  lag_t rounds = discArray[SPELL_ACID_BLAST]->lag;
  taskDiffT diff = discArray[SPELL_ACID_BLAST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ACID_BLAST, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castAcidBlast(TBeing *caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_ACID_BLAST);
  int bKnown = caster->getSkillValue(SPELL_ACID_BLAST);

  ret=acidBlast(caster,level,bKnown, caster->getAdvLearning(SPELL_ACID_BLAST));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } 
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);

  return rc;
}

int atomize(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't atomize $N -- $E's a god!", FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  level = min(level, 75);

  int dam = caster->getSkillDam(victim, SPELL_ATOMIZE, level, adv_learn);

  caster->reconcileHurt(victim, discArray[SPELL_ATOMIZE]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_ATOMIZE)) {
    act("Whoah! $n disperses the atoms from $N's body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("Whoah! You disperse the atoms from $N's body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("Whoah! $n disperses the atoms from your body!", FALSE, caster, NULL, victim, TO_VICT);
    switch(critSuccess(caster, SPELL_ATOMIZE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ATOMIZE);
        dam <<= 1;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_ATOMIZE))) {
          SV(SPELL_ATOMIZE);
          dam /= 2;
        }
    }
    if (caster->reconcileDamage(victim, dam, SPELL_ATOMIZE) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_BIND)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        act("$n nearly pulls $mself apart with $s spell!", FALSE, caster, NULL, NULL, TO_ROOM);
        caster->sendTo("ACK!  Something has gone horrendously wrong and you lose control of the spell!\n\r");
        act("Hey, $n was trying to cast that on you!", FALSE, caster, NULL, victim, TO_VICT);
        if (caster->isLucky(caster->spellLuckModifier(SPELL_ATOMIZE))) {
          SV(SPELL_ATOMIZE);
          dam /= 2;
        }
        if (caster->reconcileDamage(caster, dam/3, SPELL_ATOMIZE) == -1)
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

int atomize(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_ATOMIZE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_ATOMIZE]->lag;
  taskDiffT diff = discArray[SPELL_ATOMIZE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_ATOMIZE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castAtomize(TBeing *caster, TBeing *victim)
{
  int rc = 0;

  int level = caster->getSkillLevel(SPELL_ATOMIZE);
  int bKnown = caster->getSkillValue(SPELL_ATOMIZE);

  int ret=atomize(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_ATOMIZE));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int atomize(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = atomize(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

int animate(TBeing *caster, int level, short bKnown)
{
  int count = 0, armor;
  TMonster *gol;
  TObj *helm = NULL, *jacket = NULL, *l_legging = NULL, *r_legging = NULL,
  *l_sleeve = NULL, *r_sleeve = NULL, *l_glove = NULL, *r_glove = NULL,
  *l_boot = NULL, *r_boot = NULL, *o;
  bool paired_leg = FALSE;

  act("$n waves $s hand over the pile of armor on the $g...", FALSE, caster, NULL, NULL, TO_ROOM);
  act("You wave your hands over the pile of armor on the $g...", FALSE, caster, NULL, NULL, TO_CHAR);
  if (caster->bSuccess(bKnown,SPELL_ANIMATE)) {
    // you need:  helm, jacket, 2 leggings, 2 sleeves, 2 gloves, 2 boots 

    TThing *obj=NULL;
    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (obj=*it);++it) {
      o = dynamic_cast<TObj *>(obj);
      if (!o)
        continue;
      if (dynamic_cast<TWorn *>(o) || dynamic_cast<TArmor *>(o)) {
        if (o->canWear(ITEM_WEAR_HEAD)) {
          if (!helm) {
            count++;
            helm = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          }
        }
        if (o->canWear(ITEM_WEAR_FEET)) {
          if (!l_boot) {
            count++;
            l_boot = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          } else if (!r_boot) {
            count++;
            r_boot = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;
          }
        }
        if (o->canWear(ITEM_WEAR_BODY)) {
          if (!jacket) {
            count++;
            jacket = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          }
        }
        if (o->canWear(ITEM_WEAR_LEGS)) {
          if (o->usedAsPaired() && (!l_legging || (!r_legging && !paired_leg))) {
            if (!l_legging)
              count+=2;
            else
              count+=1;
           
            paired_leg = TRUE;
            l_legging = o;
            r_legging = NULL;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
          } else if (!l_legging) {
            count++;
            l_legging = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          } else if (!r_legging) {
            count++;
            r_legging = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;
          }
        }
        if (o->canWear(ITEM_WEAR_ARMS)) {
          if (!l_sleeve) {
            count++;
            l_sleeve = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          } else if (!r_sleeve) {
            count++;
            r_sleeve = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;
          }
        }
        if (o->canWear(ITEM_WEAR_HANDS)) {
          if (!l_glove) {
            count++;
            l_glove = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;		/* next item */
          } else if (!r_glove) {
            count++;
            r_glove = o;
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_CHAR);
            act("$p begins to make a strange crackling sound...", FALSE, caster, o, NULL, TO_ROOM);
            continue;
          }
        }
      }
    }

    if (count < 10) {
      act("Hmm...you must be missing an item.", FALSE, caster, NULL, NULL, TO_CHAR);
      caster->nothingHappens(SILENT_YES);
      return SPELL_FAIL;
    }
    if (count > 10) {
      act("Smells like an error to me!", FALSE, caster, NULL, NULL, TO_CHAR);
      caster->nothingHappens(SILENT_YES);
      vlogf(LOG_BUG, "greater than 10 eq pieces counted for monster creation!!");
      return SPELL_FAIL;
    }
    if (!l_boot || !r_boot ||
        !l_sleeve || !r_sleeve ||
        !l_glove || !r_glove ||
        !helm || !jacket ||
        !l_legging || (!r_legging && !paired_leg)) {
      caster->sendTo("Hmm...you must be missing an item.\n\r");
      caster->nothingHappens(SILENT_YES);
      return SPELL_FAIL;
    }
    gol = read_mobile(Mob::ANIMATION, VIRTUAL);
    if (!gol) {
      vlogf(LOG_BUG, format("ERROR! spell 'animate' (in code as create_monster) tried to load mob vnum #%d -- doesn't exist!") %  Mob::ANIMATION);
      caster->sendTo("Oops. Buggy spell. Error logged. Sorry.\n\r");
      caster->nothingHappens(SILENT_YES);
      return SPELL_FAIL;
    }

    gol->genericCharmFix();

    act("The crackling armor jumps together to form an armored figure!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("The crackling armor jumps together to form an armored figure!", FALSE, caster, NULL, NULL, TO_ROOM);
    act("You have created an armored monster!", FALSE, caster, NULL, NULL, TO_CHAR);
    act("$n has created an armored monster!", FALSE, caster, NULL, NULL, TO_ROOM);
    *caster->roomp += *gol;

    /* add up the armor values in the pieces */
    armor = l_boot->itemAC();
    armor += r_boot->itemAC();
    armor += l_legging->itemAC();
    if (!paired_leg)
      armor += r_legging->itemAC();
    armor += l_glove->itemAC();
    armor += r_glove->itemAC();
    armor += l_sleeve->itemAC();
    armor += r_sleeve->itemAC();
    armor += jacket->itemAC();
    armor += helm->itemAC();

    // the more negative armor is, the better the stuff used was
    // armor = 0 : -10 levels
    // armor = -2000 : +80 levels
    gol->setLevel(WARRIOR_LEVEL_IND, 
       gol->getLevel(WARRIOR_LEVEL_IND) +
       level/10 -
       ((armor + 400)/30));
    gol->calcMaxLevel();

    // 11 + Ld8, but make consistant
    int hp_amt = (int) (gol->GetMaxLevel() * 15.5);

    // sort of counterintuitive, but set the AC appropriate to the level
    // that is obj AC -> mob level -> mob AC
    gol->setArmor(60 - 2*gol->GetMaxLevel());

    gol->setMaxHit(hp_amt);
    gol->setHit(gol->hitLimit());

    // damage per round * 1.1 = lev
    double av_dam = gol->baseDamage() + gol->getDamroll();
    gol->setMult( ((((double) gol->GetMaxLevel()) / 1.10) / av_dam));

    gol->player.Class = CLASS_WARRIOR;

    gol->addAffects(l_boot);
    gol->addAffects(r_boot);
    gol->addAffects(l_glove);
    gol->addAffects(r_glove);
    gol->addAffects(jacket);
    gol->addAffects(l_sleeve);
    gol->addAffects(r_sleeve);
    gol->addAffects(l_legging);
    gol->addAffects(r_legging);
    gol->addAffects(helm);

    // here's the thing, without this, the auto gets an AC of 40 (tiny)
    // minus all the AC of the armor used so chain gives it AC -36
    // then folks pile on equipment and the automaton rocks!
    // let's keep things in control somewhat...
    // afterthought: guess its better to give them this but deny them
    // wearing stuff...
    //    gol->setArmor(100);
    //    gol->setHitroll(0);  // likewise...

    delete helm;
    delete l_boot;
    delete r_boot;
    delete l_glove;
    delete r_glove;
    delete l_legging;
    delete r_legging;
    delete l_sleeve;
    delete r_sleeve;
    delete jacket;

    helm = l_boot = r_boot = l_glove = r_glove = NULL;
    l_legging = r_legging = l_sleeve = r_sleeve = jacket = NULL;

    SET_BIT(gol->specials.affectedBy, AFF_CHARM);
    caster->addFollower(gol);
    act("$n is assembled from the pieces!", FALSE, gol, NULL, NULL, TO_ROOM);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int animate(TBeing *caster)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_ANIMATE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ANIMATE]->lag;
  diff = discArray[SPELL_ANIMATE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ANIMATE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castAnimate(TBeing *caster)
{
  int ret,level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ANIMATE);
  int bKnown = caster->getSkillValue(SPELL_ANIMATE);

  if ((ret=animate(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int sorcerersGlobe(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;

  aff.type = SPELL_SORCERERS_GLOBE;
  aff.level = level;
  aff.duration = (3 + (aff.level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_ARMOR;
  aff.modifier = -100;
  aff.bitvector = 0;

  if (caster->bSuccess(bKnown,SPELL_SORCERERS_GLOBE)) {
    switch (critSuccess(caster, SPELL_SORCERERS_GLOBE)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_SORCERERS_GLOBE);
        aff.duration = (12 + (level / 2)) * UPDATES_PER_MUDHOUR;
        if (caster != victim)
          aff.modifier *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster != victim) 
      aff.modifier /= 5;

    // I changed this to use affectJoin, it was just adding
    // new affs every cast - Russ 12/18/97
    //Second argument FALSE causes it to add new duration to old
    //Third argument TRUE causes it to average the old and newmodifier

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }

    victim->roomp->playsound(SOUND_SPELL_SORCERERS_GLOBE, SOUND_TYPE_MAGIC);

    act("$n is instantly surrounded by a hardened wall of air!", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You are instantly surrounded by a hardened wall of air!", FALSE, victim, NULL, NULL, TO_CHAR);

    caster->reconcileHelp(victim, discArray[SPELL_SORCERERS_GLOBE]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void sorcerersGlobe(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
int ret;
  ret=sorcerersGlobe(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int sorcerersGlobe(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_SORCERERS_GLOBE, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_SORCERERS_GLOBE]->lag;
    diff = discArray[SPELL_SORCERERS_GLOBE]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_SORCERERS_GLOBE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castSorcerersGlobe(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_SORCERERS_GLOBE);
  int bKnown = caster->getSkillValue(SPELL_SORCERERS_GLOBE);

  if ((ret=sorcerersGlobe(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  }
  return TRUE;
}

int bind(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff1, aff2;

  if (caster->isNotPowerful(victim, level, SPELL_BIND, SILENT_NO)) 
    return SPELL_FAIL;

  aff1.type = SPELL_BIND;
  aff1.level = level;
  aff1.bitvector = AFF_WEB;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = (level / 2) + 5;
  aff1.duration = level * UPDATES_PER_MUDHOUR;

  aff2.type = SPELL_BIND;
  aff2.level = level;
  aff2.bitvector = AFF_WEB;
  aff2.location = APPLY_SPELL_HITROLL;
  aff2.modifier = (-level * 2);
  aff2.duration = level * UPDATES_PER_MUDHOUR;

  if (caster->bSuccess(bKnown, SPELL_BIND)) {
    caster->reconcileHurt(victim, discArray[SPELL_BIND]->alignMod);

    act("$n traps $N in a mass of sticky, web-like substance!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You trap $N in a mass of sticky, web-like substance!", FALSE, caster, NULL, victim, TO_CHAR);
    act("$n traps you in a mass of sticky, web-like substance!", FALSE, caster, NULL, victim, TO_VICT);
    switch(critSuccess(caster, SPELL_BIND)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_BIND);
        aff1.duration *= 2;
        aff2.modifier *= 2;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_BIND))) {
          SV(SPELL_BIND);
          aff1.duration /= 2;
          aff2.modifier /= 2;
        }
    }

    if (!victim->affectJoin(caster, &aff1, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }
    if (!victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_BIND)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_BIND);
        act("$n traps $mself in a sticky, web-like substance!", FALSE, caster, NULL, NULL, TO_ROOM);
        act("Oops! You trap yourself in a sticky, web-like substance!", FALSE, caster, NULL, victim, TO_CHAR);
        act("Hey, $n was trying to trap you in that stuff!", FALSE, caster, NULL, victim, TO_VICT);
        caster->affectTo(&aff1);
        caster->affectTo(&aff2);
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void bind(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  bind(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int bind(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

   if (!bPassMageChecks(caster, SPELL_BIND, victim))
     return FALSE;

   lag_t rounds = discArray[SPELL_BIND]->lag;
   diff = discArray[SPELL_BIND]->task;

   start_cast(caster, victim, NULL, caster->roomp, SPELL_BIND, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

     return TRUE;
}

int castBind(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_BIND);
  int bKnown = caster->getSkillValue(SPELL_BIND);

  if ((ret=bind(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    } else {
    }
  }
  return TRUE;
}

int teleport(TBeing *caster, TBeing *victim, int, short bKnown) 
{
  int rc;
  TMonster *tmons = dynamic_cast<TMonster *>(victim);

  if ((caster != victim) && victim->isImmortal()) {
    act("You can't do that to $N -- $E's a god!", FALSE,caster,NULL,victim,TO_CHAR);
      caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown,SPELL_TELEPORT)) {
    if (caster->roomp->isRoomFlag(ROOM_NO_ESCAPE)) {
      caster->sendTo("The defenses of this area are too strong.\n\r");
      caster->nothingHappens(SILENT_YES);
      return SPELL_FAIL;
    }

    if (victim != caster) {
      if (!victim->isLucky(caster->spellLuckModifier(SPELL_TELEPORT))) {
        rc = victim->genericTeleport(SILENT_NO);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return SPELL_SUCCESS + VICTIM_DEAD;
    
        return SPELL_SUCCESS;
      } else {
        SV(SPELL_TELEPORT);
        caster->nothingHappens(SILENT_YES);
	if (tmons) {
	  if (tmons->isAttackerMultiplay(caster)) {
	    caster->nothingHappens(SILENT_NO);
	    return SPELL_FAIL;
	  }
	  caster->reconcileHurt(victim, discArray[SPELL_TELEPORT]->alignMod);
	  if (!victim->isPc()) {
	    // piss the mob off for shooting at it 
            if ((rc = victim->hit(caster)) == DELETE_VICT) 
              return SPELL_SUCCESS + CASTER_DEAD;
            else if (rc == DELETE_THIS) 
              return SPELL_SUCCESS + VICTIM_DEAD;
          }
	} else 
	  victim->sendTo("You feel a strange gut-wrenching, but the effect fades.\n\r");
	return SPELL_SUCCESS;
      }
    }
    rc = caster->genericTeleport(SILENT_NO);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return SPELL_SUCCESS + CASTER_DEAD;
    
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int teleport(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int rc = 0;
  int ret = 0;

  ret = teleport(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int teleport(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_TELEPORT, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_TELEPORT]->lag;
  diff = discArray[SPELL_TELEPORT]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_TELEPORT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castTeleport(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_TELEPORT);
  int bKnown = caster->getSkillValue(SPELL_TELEPORT);

  ret = teleport(caster,victim,level,bKnown);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}
int protectionFromElements(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff,aff2;
 
  aff.type = SPELL_PROTECTION_FROM_ELEMENTS;
  aff.level = level;
  aff.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_ACID;
  aff.modifier2 = ((level * 2) / 3);
  aff.bitvector = 0;

  aff2.type = SPELL_PROTECTION_FROM_ELEMENTS;
  aff2.level = level;
  aff2.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR; 
  aff2.location = APPLY_IMMUNITY;
  aff2.modifier = IMMUNE_ELECTRICITY;
  aff2.modifier2 = ((level * 2) / 3);
  aff2.bitvector = 0;
 
  if (caster->bSuccess(bKnown,SPELL_PROTECTION_FROM_ELEMENTS)) {
    act("$n glows with a faint orange aura for a brief moment.", FALSE, victim, NULL, NULL, TO_ROOM);
    act("You glow with a faint orange aura for a brief moment.", FALSE, victim, NULL, NULL, TO_CHAR);
    switch (critSuccess(caster, SPELL_PROTECTION_FROM_ELEMENTS)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_PROTECTION_FROM_ELEMENTS);
        aff.duration = (10 + (level / 2)) * UPDATES_PER_MUDHOUR;
        aff.modifier2 = (level * 2);
        aff2.duration = aff.duration;
        aff2.modifier2 = (level * 2);
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster != victim) {
      aff.modifier2 /= 2;
      aff2.modifier2 /= 2;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }
    if (!victim->affectJoin(caster, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }
    caster->reconcileHelp(victim, discArray[SPELL_PROTECTION_FROM_ELEMENTS]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}
void protectionFromElements(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
int ret;
  ret=protectionFromElements(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int protectionFromElements(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_PROTECTION_FROM_ELEMENTS, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_PROTECTION_FROM_ELEMENTS]->lag;
    diff = discArray[SPELL_PROTECTION_FROM_ELEMENTS]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_PROTECTION_FROM_ELEMENTS, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castProtectionFromElements(TBeing *caster, TBeing *victim)
{
int ret,level;
 
  level = caster->getSkillLevel(SPELL_PROTECTION_FROM_ELEMENTS);
  int bKnown = caster->getSkillValue(SPELL_PROTECTION_FROM_ELEMENTS);
 
  if ((ret=protectionFromElements(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}
 


