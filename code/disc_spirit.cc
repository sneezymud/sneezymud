//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_spirit.h"
#include "obj_magic_item.h"
#include "combat.h"

int knot(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_KNOT, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_KNOT]->lag;
  diff = discArray[SPELL_KNOT]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_KNOT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castKnot(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_KNOT);
  int bKnown = caster->getSkillValue(SPELL_KNOT);

  ret = knot(caster,victim,level,bKnown);

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int knot(TBeing *caster, TBeing *victim, int, byte bKnown)
{
  int rc;
  TThing *t;

  if (caster->bSuccess(bKnown,SPELL_KNOT)) {
  // I added this to prevent pkillers from seeking refuge in the 
  // knot...jesus
    if (caster->affectedBySpell(AFFECT_PLAYERKILL) ||
        victim->affectedBySpell(AFFECT_PLAYERKILL)){
      act("The Knot will not provide refuge to a murderer.",
          TRUE, caster, NULL, NULL, TO_CHAR);
      act("Nothing seems to happen.",
          FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }
    if (caster->roomp->isRoomFlag(ROOM_NO_ESCAPE)) {
      caster->sendTo("The defenses of this area are too strong.\n\r");
      caster->nothingHappens(SILENT_YES);
      return SPELL_FAIL;
    }

    act("$n <r>tears a gap in reality and steps through.<1>", FALSE, caster, NULL, NULL, TO_ROOM);
    act("<r>You tear a gap in reality and step through.<1>", FALSE, caster, NULL, NULL, TO_CHAR);
    
    while ((t = caster->rider)) {
      rc = t->fallOffMount(caster, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
	delete t;
	t = NULL; 
      }
    }
    
    if (caster->riding) {
      rc = caster->fallOffMount(caster->riding, POSITION_STANDING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
	return SPELL_SUCCESS + CASTER_DEAD;
    }
    
    TRoom *room = real_roomp(2387);
    --(*caster);
    *room += *caster;

    act("$n <r>steps into <1><k>the knot<1><r>.<1>", FALSE, caster, NULL, NULL, TO_ROOM);
    act("<r>You step into <1><k>the knot<1><r>.<1>", FALSE, caster, NULL, NULL, TO_CHAR);

    TBeing *tbt = dynamic_cast<TBeing *>(caster);
    if (tbt) {
      tbt->doLook("", CMD_LOOK);
      rc = tbt->genericMovedIntoRoom(room, -1);
      if (IS_SET_DELETE(rc, DELETE_THIS))
	return SPELL_SUCCESS + CASTER_DEAD;
    }


    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

}


int silence(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  if (caster->isNotPowerful(victim, level, SPELL_SILENCE, SILENT_YES) ||
      (victim->isLucky(caster->spellLuckModifier(SPELL_SILENCE)))) {
    act("$N resists your attempts to silence $M.",
          FALSE, caster, NULL, victim, TO_CHAR);
    act("That dumbass $n just tried to silence you!",
          FALSE, caster, NULL, victim, TO_VICT);
    return SPELL_FAIL;
  }
  caster->reconcileHurt(victim,discArray[SPELL_SILENCE]->alignMod);

  aff.type = SPELL_SILENCE;
  aff.level = level;
  aff.duration =  aff.level * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_SILENT;

  if (caster->bSuccess(bKnown, SPELL_SILENCE)) {
    switch (critSuccess(caster, SPELL_SILENCE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SILENCE);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_SILENCE))) {
          SV(SPELL_SILENCE);
          aff.duration /= 2;
        }
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_SILENCE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_SILENCE);
        caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    return SPELL_FAIL;
  }
}

int silence(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

   if (!bPassMageChecks(caster, SPELL_SILENCE, victim))
     return FALSE;

   lag_t rounds = discArray[SPELL_SILENCE]->lag;
   diff = discArray[SPELL_SILENCE]->task;

   start_cast(caster, victim, NULL, caster->roomp, SPELL_SILENCE, diff, 1, "",
 rounds, caster->in_room, 0, 0,TRUE, 0);

     return TRUE;
}

int castSilence(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_SILENCE);
  int bKnown = caster->getSkillValue(SPELL_SILENCE);

  if ((ret=silence(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
    act("A gag order has been placed upon $n!", TRUE, victim, NULL, NULL, TO_ROOM, ANSI_WHITE_BOLD);
    act("You have been muzzled!", TRUE, victim, NULL, NULL, TO_CHAR, ANSI_WHITE_BOLD);
  } else {
    if (ret==SPELL_CRIT_FAIL) {
      caster->sendTo("Oops! The magical muzzle attempts to find its home on your face!\n\r");
      caster->nothingHappens(SILENT_YES);
    } else {
      caster->nothingHappens();
    }
  }
  return TRUE;
}

int slumber(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  int ret= FALSE, rc = FALSE;
  bool found = false;
  int crit = 1;
  saveTypeT save = SAVE_NO;
  TObj *primary = dynamic_cast<TObj *>(caster->heldInPrimHand());
  TObj *secondary = dynamic_cast<TObj *>(caster->heldInSecHand());

  if ((primary && (primary->objVnum() == OBJ_SLEEPTAG_STAFF)) ||
      (secondary && (secondary->objVnum() == OBJ_SLEEPTAG_STAFF)))
    found = TRUE;

// sleep tag
  if (victim->GetMaxLevel() < GOD_LEVEL1) {
    if (found) {
      if (caster->inRoom() == ROOM_SLEEPTAG_CONTROL) {
        caster->sendTo("You can not use that staff here.\n\r");
        return SPELL_FAIL;
      }
      vlogf(LOG_MISC, fmt("Sleep Tag Staff: %s just got slept by %s") % 
               victim->getName() % caster->getName());
      rc = victim->rawSleep(level, (4 + level/2) * UPDATES_PER_MUDHOUR, crit, save);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return SPELL_SUCCESS | VICTIM_DEAD;
      }
    }
  }

  if ((victim->isAffected(AFF_SLEEP)) || 
      (victim->getPosition() == POSITION_SLEEPING)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  if (victim->isImmortal() || 
      caster->isNotPowerful(victim, level, SPELL_SLUMBER, SILENT_YES) ||
      (!caster->isImmortal() &&
      (victim->isLucky(caster->spellLuckModifier(SPELL_SLUMBER))))) {
    act("$N resists your attempts to sleep $M.",
              FALSE, caster, NULL, victim, TO_CHAR);
    act("That nitwit $n just tried to sleep you!",
              FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
    return SPELL_FAIL;
  }

  if (victim->isImmune(IMMUNE_SLEEP, WEAR_BODY) && !caster->isImmortal())  {
    victim->sendTo("You are able to fend off the sleepy cosmic powers!\n\r");
    victim->failSleep(caster);
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim,discArray[SPELL_SLUMBER]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SLUMBER)) {
    switch (critSuccess(caster, SPELL_SLUMBER)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_SLUMBER);
        crit = 2;
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SLUMBER);
        crit = 3;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_SLUMBER))) {
          SV(SPELL_SLUMBER);
          save = SAVE_YES;
        }
        break;
    }
    ret = SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_SLUMBER)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_SLUMBER);
        if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY))  {
          caster->nothingHappens();
          return SPELL_CRIT_FAIL;
        } else {
          victim = caster;
        }
        ret = SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        return SPELL_FAIL;
    }
  }
  rc = victim->rawSleep(level, (4 + level/2) * UPDATES_PER_MUDHOUR, crit, save);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    if (victim == caster)
      ret |= CASTER_DEAD;
    else 
      ret |= VICTIM_DEAD;
  }
  if (caster->fight() == victim) {
    caster->stopFighting();
  }

  return ret;
}

int slumber(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret, rc = FALSE;

  ret=slumber(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (ret== SPELL_SUCCESS) {
    act("$n puts $N to sleep!", TRUE, caster, NULL, victim, TO_NOTVICT, ANSI_WHITE_BOLD);
    act("You put $N to sleep!", TRUE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    act("You drift of to never-never land! Zzzzzzz!", TRUE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
  } else if (ret==SPELL_CRIT_FAIL) {
      caster->sendTo("Oops! Your attempt to put someone to sleep homes on you!\n\r");
      act("$n puts $mself to sleep!", TRUE, caster, NULL, victim, TO_ROOM, ANSI_WHITE_BOLD);
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int slumber(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

   if (!bPassMageChecks(caster, SPELL_SLUMBER, victim))
     return FALSE;

   lag_t rounds = discArray[SPELL_SLUMBER]->lag;
   diff = discArray[SPELL_SLUMBER]->task;

   start_cast(caster, victim, NULL, caster->roomp, SPELL_SLUMBER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

     return TRUE;
}

int castSlumber(TBeing *caster, TBeing *victim)
{
  int ret = FALSE,level, rc = FALSE;

  level = caster->getSkillLevel(SPELL_SLUMBER);
  int bKnown = caster->getSkillValue(SPELL_SLUMBER);

  if ((ret=slumber(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
    act("$n puts $N to sleep!", TRUE, caster, NULL, victim, TO_ROOM, ANSI_WHITE_BOLD);
    act("You drift off to never-never land! Zzzzzzz!", TRUE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
  } else {
    if (ret==SPELL_CRIT_FAIL) {
      caster->sendTo("Oops! Your attempt to put someone to sleep homes on you!\n\r");
      caster->nothingHappens(SILENT_YES);
    } else
      caster->nothingHappens();
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int ensorcer(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;
  int again;
  char buf[256];

  if (victim == caster) {
    sprintf(buf, "You tell yourself, \"Gosh darnit! I'm a pretty okay %s!\"", (!caster->getSex() ? "eunuch" : (caster->getSex() == 1 ? "guy" : "gal")));
    act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  if (victim->isLinkdead()) {
    act("Ha Ha, real funny!", FALSE, caster, 0, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't charm $N -- you're busy taking orders yourself!");
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }

  if (victim->isAffected(AFF_CHARM)) {
    again = (victim->master == caster);
    sprintf(buf, "You can't charm $N%s -- $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
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
#if 0
  if (!victim->isPc()) {
    caster->sendTo("You can't charm that.\n\r");
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
#endif
  if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY) || victim->GetMaxLevel() > caster->GetMaxLevel() ||
      (!victim->isPc() && dynamic_cast<TMonster *>(victim)->Hates(caster, NULL)) ||
      caster->isNotPowerful(victim, level, SPELL_ENSORCER, SILENT_YES) ||
      (victim->isLucky(caster->spellLuckModifier(SPELL_ENSORCER)))) {

      victim->failCharm(caster);
      act("Something went wrong!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
      act("All you did was piss $N off!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
      caster->nothingHappens(SILENT_YES);
      act("$n just tried to charm you!", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
      return SPELL_FAIL;
  }

  caster->reconcileHurt(victim,discArray[SPELL_ENSORCER]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_ENSORCER)) {
    if (victim->master)
      victim->stopFollower(TRUE);
    caster->addFollower(victim);

    aff.type = SPELL_ENSORCER;
    aff.level = level;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    aff.duration  =  3 * level * UPDATES_PER_MUDHOUR;

    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_CHARM));
    aff.duration /= 100;

    switch (critSuccess(caster, SPELL_ENSORCER)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_ENSORCER);
        aff.duration *= 2;
        break;
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
        CS(SPELL_ENSORCER);
        aff.duration *= 3;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_ENSORCER))) {
          SV(SPELL_ENSORCER);
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
    act("Something went wrong!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    act("All you did was piss $N off!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    act("$n just tried to charm you!", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
    caster->nothingHappens(SILENT_YES);
    victim->failCharm(caster);
    return SPELL_FAIL;
  }
}

void ensorcer(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;
  
  if (caster != victim) {
    act("$p attempts to bend $N to your will.",
          FALSE, caster, obj, victim, TO_CHAR, ANSI_WHITE_BOLD);
    act("$p attempts to bend you to $n's will.",
          FALSE, caster, obj, victim, TO_VICT, ANSI_WHITE_BOLD);
    act("$p attempts to bend $N to $n's will.",
          FALSE, caster, obj, victim, TO_NOTVICT, ANSI_WHITE_BOLD);
  } else {
    act("$p tries to get you to control yourself.",
          FALSE, caster, obj, 0, TO_CHAR, ANSI_WHITE_BOLD);
    act("$p tries to get $n to control $mself.",
          FALSE, caster, obj, 0, TO_ROOM, ANSI_WHITE_BOLD);
  }
  ret=ensorcer(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  return;
}

int ensorcer(TBeing *caster, TBeing *victim)
{
  char buf[256];
  taskDiffT diff;
  int level, again;

  level = caster->getSkillLevel(SPELL_ENSORCER);

  if (victim == caster) {
    sprintf(buf, "You tell yourself, \"Gosh darnit! I'm a pretty okay %s!\"", (!caster->getSex() ? "eunuch" : (caster->getSex() == 1 ? "guy" : "gal")));
    act(buf, FALSE, caster, NULL, NULL, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }

  if (caster->isAffected(AFF_CHARM)) {
    sprintf(buf, "You can't charm $N -- you're busy taking orders yourself!");
    caster->nothingHappens(SILENT_YES);
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }
  if (victim->isAffected(AFF_CHARM)) {
    again = (victim->master == caster);
    sprintf(buf, "You can't charm $N%s -- $E's busy following %s!", (again ? " again" : ""), (again ? "you already" : "somebody else"));
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

  if (!bPassMageChecks(caster, SPELL_ENSORCER, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENSORCER]->lag;
  diff = discArray[SPELL_ENSORCER]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_ENSORCER, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castEnsorcer(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_ENSORCER);
  int bKnown = caster->getSkillValue(SPELL_ENSORCER);

  if ((ret=ensorcer(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
    act("You feel an overwhelming urge to follow $n!", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
    act("You decide to do whatever $e says!", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
    act("$N has become charmed by $n!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_WHITE_BOLD);
    act("$N has become charmed by you!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);

  } else {
  }
  return TRUE;
}

int cloudOfConcealment(TBeing *caster, int level, byte bKnown)
{
  TBeing *tmp_victim;
  TThing *t, *t2;
  affectedData aff;
  int found = FALSE;
  if (caster->bSuccess(bKnown, SPELL_CLOUD_OF_CONCEALMENT)) {
    caster->sendTo("You focus your powers and cause a cloud to materialize around your group.\n\r");
    act("$n invokes some magic and produces huge volumes of vaporous smoke.",
        TRUE,caster,0,0,TO_ROOM,ANSI_GREEN);

    aff.type = SPELL_INVISIBILITY;
    aff.level = level;
    aff.duration = 24 * UPDATES_PER_MUDHOUR;
    aff.modifier = -40;
    aff.location = APPLY_ARMOR;
    aff.bitvector = AFF_INVISIBLE;
    for (t = caster->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      tmp_victim = dynamic_cast<TBeing *>(t);
      if (!tmp_victim)
        continue;
      if (caster != tmp_victim && !tmp_victim->isImmortal()) {
        if (caster->inGroup(*tmp_victim)) {
          if (!tmp_victim->isAffected(AFF_INVISIBLE)) {
            caster->reconcileHelp(tmp_victim,discArray[SPELL_CLOUD_OF_CONCEALMENT]->alignMod);
#if 0
// setting the affect sends similar text, so don't be redundant
            act("$n dissolves out of sight!", TRUE, tmp_victim, NULL, NULL, TO_ROOM,ANSI_GREEN);
            tmp_victim->sendTo(fmt("You vanish!\n\r") %ANSI_GREEN);
#endif

            tmp_victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
            found = TRUE;
          }
        }
      } 
    }
    if (!caster->isAffected(AFF_INVISIBLE)) {
#if 0
      act("$n dissolves out of sight!", TRUE, caster, NULL, NULL, TO_ROOM, ANSI_GREEN);
      caster->sendTo(fmt("You vanish!\n\r") %ANSI_GREEN);
#endif
      caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
      found = TRUE;
    }
    if (!found) {
      caster->sendTo("But, there's nobody visible in your group.\n\r");
      return SPELL_FAIL;
    }
    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int cloudOfConcealment(TBeing *caster)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_CLOUD_OF_CONCEALMENT, NULL))
    return TRUE;

  lag_t rounds = discArray[SPELL_CLOUD_OF_CONCEALMENT]->lag;
  diff = discArray[SPELL_CLOUD_OF_CONCEALMENT]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CLOUD_OF_CONCEALMENT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castCloudOfConcealment(TBeing *caster)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_CLOUD_OF_CONCEALMENT);
  int bKnown = caster->getSkillValue(SPELL_CLOUD_OF_CONCEALMENT);

  if ((ret=cloudOfConcealment(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

int dispelInvisible(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  if (caster->isNotPowerful(victim, level, SPELL_DISPEL_INVISIBLE, SILENT_NO)) {
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim,discArray[SPELL_DISPEL_INVISIBLE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_DISPEL_INVISIBLE)) {
    if (victim->affectedBySpell(SPELL_INVISIBILITY)) {
#if 0
      act("$n slowly becomes visible again.", TRUE, victim, NULL, NULL, TO_ROOM, ANSI_WHITE_BOLD);
      victim->sendTo(fmt("You slowly become visible again.\n\r") %ANSI_WHITE_BOLD);
#endif

      victim->affectFrom(SPELL_INVISIBILITY);
      if (IS_SET(victim->specials.affectedBy, AFF_INVISIBLE))
	victim->specials.affectedBy -= AFF_INVISIBLE;
    } else if (victim->isAffected(AFF_INVISIBLE)) {
      act("$n slowly becomes visible again.", TRUE, victim, NULL, NULL, TO_ROOM,ANSI_WHITE_BOLD);
      victim->sendTo("You slowly become visible again.\n\r");     
      REMOVE_BIT(victim->specials.affectedBy, AFF_INVISIBLE);
    } else {
      act("Umm...$N is already visible, bud.", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
      caster->nothingHappens(SILENT_YES);
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_DISPEL_INVISIBLE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_DISPEL_INVISIBLE);
        if (caster->affectedBySpell(SPELL_INVISIBILITY)) {
          caster->spellMessUp(SPELL_DISPEL_INVISIBLE);
          act("Oh no! You have dispelled your own invisibility!", FALSE, caster, NULL, NULL, TO_CHAR, ANSI_WHITE_BOLD);
          caster->affectFrom(SPELL_INVISIBILITY);
          if (IS_SET(caster->specials.affectedBy, AFF_INVISIBLE))
          caster->specials.affectedBy -= AFF_INVISIBLE;
          return SPELL_CRIT_FAIL;
        }
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void dispelInvisible(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  dispelInvisible(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int dispelInvisible(TBeing *caster, TBeing *victim)
{
  if (!victim->isAffected(AFF_INVISIBLE)) {
    act("Do you need glasses or something?  $N is already visible!", FALSE, caster, NULL, victim, TO_CHAR,ANSI_WHITE);
    act("Humor the pitiful mage and pretend that $N just magically appeared.  Clap or something.", FALSE, caster, NULL, victim, TO_ROOM, ANSI_WHITE);
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_DISPEL_INVISIBLE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DISPEL_INVISIBLE]->lag;
  taskDiffT diff = discArray[SPELL_DISPEL_INVISIBLE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DISPEL_INVISIBLE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castDispelInvisible(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_DISPEL_INVISIBLE);
  int bKnown = caster->getSkillValue(SPELL_DISPEL_INVISIBLE);

  dispelInvisible(caster,victim,level,bKnown);

  return FALSE;
}

int dispelInvisible(TBeing *caster, TObj * obj, int, byte bKnown)
{
  if (caster->bSuccess(bKnown, SPELL_DISPEL_INVISIBLE)) {
    if (obj->isObjStat(ITEM_INVISIBLE)) {
      obj->remObjStat(ITEM_INVISIBLE);
      act("$p loses its cloak of invisibility!",
                  FALSE, caster, obj, NULL, TO_CHAR);
      act("$p loses its cloak of invisibility!",
                  FALSE, caster, obj, NULL, TO_ROOM);
    } else {
      caster->sendTo(fmt("Uhm, that item wasn't invisible.\n\r"));
      caster->nothingHappens(SILENT_YES);
    }

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void dispelInvisible(TBeing *caster, TObj* targ_obj, TMagicItem * obj)
{
  dispelInvisible(caster,targ_obj,obj->getMagicLevel(),obj->getMagicLearnedness());
}

void dispelInvisible(TBeing *caster, TObj* obj)
{
  if (!bPassMageChecks(caster, SPELL_DISPEL_INVISIBLE, obj))
    return;

  int level = caster->getSkillLevel(SPELL_DISPEL_INVISIBLE);
  int bKnown = caster->getSkillValue(SPELL_DISPEL_INVISIBLE);

  dispelInvisible(caster,obj,level,bKnown);
}

static struct PolyType PolyList[] =
//   name,    level, learning, vnum, discipline, race
{
  {"orc"       ,  8,   1,   941, DISC_SPIRIT, RACE_NORACE}, // L  4
  {"frog"      ,  8,   1,   917, DISC_SPIRIT, RACE_NORACE}, // L  5
  {"cockatrice", 10,   1,   911, DISC_SPIRIT, RACE_NORACE}, // L  6
  {"komodo"    , 12,  30,  7501, DISC_SPIRIT, RACE_NORACE}, // L 25
  {"minotaur"  , 15,  40,   937, DISC_SPIRIT, RACE_NORACE}, // L  8
  {"lamia"     , 25,  50,   934, DISC_SPIRIT, RACE_NORACE}, // L 13
  {"reindeer"  , 25,  55, 10212, DISC_SPIRIT, RACE_NORACE}, // L 15
  {"chimera"   , 30,  60,   910, DISC_SPIRIT, RACE_NORACE}, // L 15
  {"dragonne"  , 35,  70,   915, DISC_SPIRIT, RACE_NORACE}, // L 21
  {"tiger"     , 40,  85, 23630, DISC_SPIRIT, RACE_NORACE}, // L 24
  {"ettin"     , 45, 100,   916, DISC_SPIRIT, RACE_NORACE}, // L 27
  {"arch"      , 60, 100, 28813, DISC_SPIRIT, RACE_NORACE}, // L 70 (god only)
  {"\n"        , -1,  -1,    -1, DISC_SPIRIT, RACE_NORACE}
};

int polymorph(TBeing *caster, int level, byte bKnown)
{
  int i, ret = 0;
  bool nameFound = FALSE;
  bool found = FALSE;
  TMonster *mob;
  const char * buffer;
  affectedData aff;
  affectedData aff2;

  buffer = caster->spelltask->orig_arg;
 
  discNumT das = getDisciplineNumber(SPELL_POLYMORPH, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, "Bad disc for SPELL_POLYMORPH");
    return SPELL_FAIL;
  }
  for (i = 0; *PolyList[i].name != '\n'; i++) {
    if (is_abbrev(buffer,PolyList[i].name)) {
      nameFound = TRUE;
    } else {
      continue;
    }
    if ((caster->getSkillValue(SPELL_POLYMORPH) >= PolyList[i].learning) && (level > 25) && (level > PolyList[i].level))
      break;
  }

  if (*PolyList[i].name == '\n') {
    if (nameFound) {
      caster->sendTo("You are not powerful enough yet to change into such a creature.\n\r");
    } else { // This case is already taken care of earlier
      caster->sendTo("Couldn't find any of those.\n\r");
    }
    return SPELL_FAIL;
  }
  if (!(mob = read_mobile(PolyList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't summon an image of that creature.\n\r");
    return SPELL_FAIL;
  }
  thing_to_room(mob,ROOM_VOID);   // just so if extracted it isn't in NOWHERE 
  mob->swapToStrung();
  
 int duration;

  if (caster->bSuccess(bKnown, SPELL_POLYMORPH)) {
    switch (critSuccess(caster, SPELL_POLYMORPH)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        duration = (2 + level / 5) * UPDATES_PER_MUDHOUR;
        CS(SPELL_POLYMORPH);
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
  aff.modifier = SPELL_POLYMORPH;
  caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    --(*mob);
    *caster->roomp += *mob;
    SwitchStuff(caster, mob);
    setCombatStats(caster, mob, PolyList[i], SPELL_POLYMORPH);

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
    aff2.type = SPELL_POLYMORPH;
    aff2.location = APPLY_NONE;
    aff2.duration = duration;
    aff2.bitvector = 0;
    aff2.modifier = SPELL_POLYMORPH;
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

    // fix name so poly'd mage can be found
    appendPlayerName(caster, mob);

    return SPELL_SUCCESS;
  } else {
    return SPELL_FAIL;
  }
}

int polymorph(TBeing *caster, const char * buffer)
{
  taskDiffT diff;
//  int level;
//  int i;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SPELL_POLYMORPH)) {
    act("You are not prepared to try to polymorph yourself again so soon.",
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
    caster->desc->snoop.snoop_by->doSnoop(caster->desc->snoop.snoop_by->name);

  if (!bPassMageChecks(caster, SPELL_POLYMORPH, caster))
    return FALSE;

  lag_t rounds = discArray[SPELL_POLYMORPH]->lag;
  diff = discArray[SPELL_POLYMORPH]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_POLYMORPH, diff, 1, buffer, rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castPolymorph(TBeing *caster)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_POLYMORPH);
  int bKnown = caster->getSkillValue(SPELL_POLYMORPH);

  if ((ret=polymorph(caster,level,bKnown)) == SPELL_SUCCESS) {
  } else 
    caster->nothingHappens();
  return TRUE;
}

int stealth(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_STEALTH]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_STEALTH)) {
    aff.type = SPELL_STEALTH;
    aff.level = level;
    aff.duration = (aff.level / 3) * UPDATES_PER_MUDHOUR;
    aff.modifier = - aff.level;
    aff.location = APPLY_NOISE;
    aff.bitvector = 0;

    switch (critSuccess(caster, SPELL_STEALTH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_STEALTH);
        aff.duration *= 2;
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

void stealth(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;

  ret=stealth(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$n seems more stealthy!", FALSE, victim, NULL, 0, TO_ROOM);
    act("You feel much more stealthy!", FALSE, victim, NULL, NULL, TO_CHAR);
  } else {
    caster->nothingHappens();
  }
}

int stealth(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_STEALTH, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_STEALTH]->lag;
     diff = discArray[SPELL_STEALTH]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_STEALTH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
        return TRUE;
}

int castStealth(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_STEALTH);
  int bKnown = caster->getSkillValue(SPELL_STEALTH);

  ret=stealth(caster,victim,level,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$N seems more stealthy!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_GREEN);
    act("You feel much more stealthy!", FALSE, victim, NULL, NULL, TO_CHAR, ANSI_GREEN);
    if (caster != victim)
      act("You have given $N the gift of stealth!", 
              FALSE, caster, NULL, victim, TO_CHAR, ANSI_GREEN);
  } else {
    act("Your attempt to give $N the gift of stealth fails.", 
                   FALSE, caster, NULL, victim, TO_CHAR, ANSI_GREEN);
    caster->nothingHappens(SILENT_YES);
  }
  return TRUE;
}

int accelerate(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_ACCELERATE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_ACCELERATE)) {
    aff.type = SPELL_ACCELERATE;
    aff.level = level;
    aff.duration = (aff.level / 3) * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    switch (critSuccess(caster, SPELL_ACCELERATE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ACCELERATE);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }
    victim->roomp->playsound(SOUND_SPELL_ACCELERATE, SOUND_TYPE_MAGIC);

    act("$N seems more nimble on $S feet!",
        FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_WHITE_BOLD);
    act("You seem to be able to move with more ease!", 
        FALSE, victim, NULL, NULL, TO_CHAR, ANSI_WHITE_BOLD);
    if (caster != victim)
      act("You have given $N the gift of speed!", 
             FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    return SPELL_SUCCESS;
  } else {
    act("Your attempt to give $N the gift of speed fails!",
        FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
}

void accelerate(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  accelerate(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int accelerate(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_ACCELERATE, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_ACCELERATE]->lag;
     diff = discArray[SPELL_ACCELERATE]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_ACCELERATE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
       return TRUE;
}

int castAccelerate(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_ACCELERATE);
  int bKnown = caster->getSkillValue(SPELL_ACCELERATE);

  if ((ret=accelerate(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int haste(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_HASTE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_HASTE)) {
    aff.type = SPELL_HASTE;
    aff.level = level;
    aff.duration = (aff.level / 3) * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = 0;
    switch (critSuccess(caster, SPELL_HASTE)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_HASTE);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    victim->roomp->playsound(SOUND_SPELL_HASTE, SOUND_TYPE_MAGIC);

    act("$N has gained a bounce in $S step!",
           FALSE, caster, NULL, victim, TO_NOTVICT);
    act("You seem to be able to move with the greatest of ease!",
           FALSE, victim, NULL, NULL, TO_CHAR);
    if (caster != victim)
      act("You have given $N the speed of the wind!",
           FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void haste(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  haste(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int haste(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_HASTE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_HASTE]->lag;
  taskDiffT diff = discArray[SPELL_HASTE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_HASTE, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castHaste(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_HASTE);
  int bKnown = caster->getSkillValue(SPELL_HASTE);

  int ret=haste(caster,victim,level,bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int calm(TBeing *caster, TBeing *victim, int, byte bKnown)
{
  affectedData aff;

  if (caster->bSuccess(bKnown,SPELL_CALM )) {
    if (victim->isLucky(caster->spellLuckModifier(SPELL_CALM))) {
      SV(SPELL_CALM);
      act("$N seems to resist the calming!", FALSE, caster, NULL, victim, TO_ROOM, ANSI_WHITE_BOLD);
      act("$N seems to resist the calming!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
      return SPELL_FAIL;
    }

    if (!victim->isPc()) {
      TMonster *tmons = dynamic_cast<TMonster *>(victim);
      if (IS_SET(tmons->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(tmons->specials.act, ACT_AGGRESSIVE);
        tmons->setDefMalice(tmons->defmalice() - min(10, tmons->defmalice()));
        tmons->setMalice(tmons->defmalice());
      }
      tmons->setDefAnger(tmons->defanger() - min(30, tmons->defanger()));
      tmons->setAnger(tmons->defanger());
      tmons->setDefMalice(tmons->defmalice() - min(10, tmons->defmalice()));
      tmons->setMalice(tmons->defmalice());
      tmons->setDefSusp(tmons->defsusp() - min(15, tmons->defsusp()));
      tmons->setSusp(tmons->defsusp());
    }

    aff.type = SPELL_CALM;
    aff.level = caster->getSkillLevel(SPELL_CALM);
    aff.duration =  aff.level * UPDATES_PER_MUDHOUR / 3;
    aff.modifier = 0;
    aff.location = APPLY_NONE;

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    return SPELL_SUCCESS;
  } else {
    int ret = 0;

    switch (critFail(caster, SPELL_CALM)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        if (!victim->isPc()) {
          TMonster *tmons = dynamic_cast<TMonster *>(victim);
          tmons->setDefAnger(tmons->defanger() + min(28, 100 - tmons->defanger()));
          tmons->setAnger(tmons->defanger());
          tmons->setDefMalice(tmons->defmalice() + min(15, 100 - tmons->defmalice()));
          tmons->setMalice(tmons->defmalice());
          tmons->setDefSusp(tmons->defsusp() + min(30, 100 - tmons->defsusp()));
          tmons->setSusp(tmons->defsusp());
        }
        ret |= SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }

    victim->failCalm(caster);
    ret |= SPELL_FAIL;
    return ret;
  }
}

int calm(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_CALM, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_CALM]->lag;
  taskDiffT diff = discArray[SPELL_CALM]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_CALM, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castCalm(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_CALM);
  int bKnown = caster->getSkillValue(SPELL_CALM);

  int ret=calm(caster,victim,level,bKnown);
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$N has come to terms with $S anger and now looks quite peaceful!",
              FALSE, caster, NULL, victim, TO_ROOM, ANSI_RED);
    act("$N has come to terms with $S anger and now looks quite peaceful!",
              FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("$n succeeds only in angering $N more!", 
              FALSE, caster, NULL, victim, TO_ROOM, ANSI_RED);
    act("Oops! Now you really pissed $M off!", 
              FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

static bool invisibilityCheck(TBeing *caster, TObj *obj)
{
  if (obj->isObjStat(ITEM_INVISIBLE)) {
    act("$p is already invisible.", FALSE, caster, obj, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return true;
  }
  return false;
}

int invisibility(TBeing *caster, TObj * obj, int, byte bKnown)
{
  if (invisibilityCheck(caster, obj))
    return SPELL_FAIL;

  if (caster->bSuccess(bKnown, SPELL_INVISIBILITY)) {
    act("$p vanishes into the thin ether!",
                FALSE, caster, obj, NULL, TO_CHAR, ANSI_GREEN);
    act("$p vanishes into the thin ether!",
                FALSE, caster, obj, NULL, TO_ROOM, ANSI_GREEN);
    obj->addObjStat(ITEM_INVISIBLE);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void invisibility(TBeing *caster, TObj* targ_obj, TMagicItem * obj)
{
  if (invisibilityCheck(caster, targ_obj))
    return;

  int ret=invisibility(caster,targ_obj,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (ret == SPELL_SUCCESS) {
  } else {
  }
}

int invisibility(TBeing *caster, TObj * obj)
{
  if (!obj)
    return FALSE;

  if (invisibilityCheck(caster, obj))
    return FALSE;

  if (!bPassMageChecks(caster, SPELL_INVISIBILITY, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_INVISIBILITY]->lag;
  taskDiffT diff = discArray[SPELL_INVISIBILITY]->task;
  start_cast(caster, NULL, obj, caster->roomp, SPELL_INVISIBILITY, diff,2 
, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castInvisibility(TBeing *caster, TObj *obj)
{
  if (!obj)
    return FALSE;

  if (invisibilityCheck(caster, obj))
    return FALSE;

  int level = caster->getSkillLevel(SPELL_INVISIBILITY);
  int bKnown = caster->getSkillValue(SPELL_INVISIBILITY);

  int ret=invisibility(caster,obj,level,bKnown);
  if (ret == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int invisibility(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

#if 0
  if (victim->affectedBySpell(SPELL_INVISIBILITY)) {
    char buf[256];
    sprintf(buf, "You realize with much embarrassment that %s already invisible!", ((caster== victim) ? "you are" : "$N is"));
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
#endif

  caster->reconcileHelp(victim, discArray[SPELL_INVISIBILITY]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_INVISIBILITY)) {
    aff.type = SPELL_INVISIBILITY;
    aff.level = level;
    aff.duration = 24 * UPDATES_PER_MUDHOUR;
    aff.modifier = -40;
    aff.location = APPLY_ARMOR;
    aff.bitvector = AFF_INVISIBLE;

    switch (critSuccess(caster, SPELL_INVISIBILITY)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_INVISIBILITY);
        aff.duration = 36 * UPDATES_PER_MUDHOUR;
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

void invisibility(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret=invisibility(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());

  if (ret == SPELL_SUCCESS) {
#if 0
    act("$n vanishes into the thin ether!", FALSE, victim, NULL, NULL,
                 TO_ROOM, ANSI_GREEN);
    act("You vanish into the thin ether!", FALSE, victim, NULL, NULL, TO_CHAR,
                 ANSI_GREEN);
#endif
  } else {
    caster->nothingHappens();
  }
}

int invisibility(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_INVISIBILITY, victim))
     return FALSE;

#if 0
  if (victim->affectedBySpell(SPELL_INVISIBILITY)) {
    char buf[256];
    sprintf(buf, "You realize with much embarrassment that %s already invisible!", ((caster== victim) ? "you are" : "$N is"));
    act(buf, FALSE, caster, NULL, victim, TO_CHAR);
    return FALSE;
  }
#endif

  lag_t rounds = discArray[SPELL_INVISIBILITY]->lag;
  diff = discArray[SPELL_INVISIBILITY]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_INVISIBILITY, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castInvisibility(TBeing *caster, TBeing *victim)
{
  int level = caster->getSkillLevel(SPELL_INVISIBILITY);
  int bKnown = caster->getSkillValue(SPELL_INVISIBILITY);

  int ret=invisibility(caster,victim,level,bKnown);
  if (ret== SPELL_SUCCESS) {
#if 0
    act("$n vanishes into the thin ether!", FALSE, victim, NULL, NULL, TO_ROOM, ANSI_GREEN);
    act("You vanish into the thin ether!", FALSE, victim, NULL, NULL, TO_CHAR, ANSI_GREEN);
#endif
  } else {
    caster->nothingHappens();
  }
  return TRUE;
}

int senseLife(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_SENSE_LIFE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_SENSE_LIFE)) {
    aff.type = SPELL_SENSE_LIFE;
    aff.duration = level * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_SENSE_LIFE;

    switch (critSuccess(caster, SPELL_SENSE_LIFE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_SENSE_LIFE);
        aff.duration *= 2;
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

void senseLife(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;

  ret = senseLife(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (ret == SPELL_SUCCESS) {
    victim->sendTo("You feel more aware of the world about you.\n\r");
    act("$n's eyes flicker a faint aqua blue.", FALSE, victim, NULL, NULL, TO_ROOM, ANSI_CYAN);
  } else { 
    caster->nothingHappens();
  }
}

int senseLife(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_SENSE_LIFE, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_SENSE_LIFE]->lag;
     diff = discArray[SPELL_SENSE_LIFE]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_SENSE_LIFE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
       return TRUE;
}

int castSenseLife(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_SENSE_LIFE);
  int bKnown = caster->getSkillValue(SPELL_SENSE_LIFE);

  ret = senseLife(caster,victim,level,bKnown);
  if (ret == SPELL_SUCCESS) {
    victim->sendTo("You feel more aware of the world about you.\n\r");
    act("$n's eyes flicker a faint aqua blue.", FALSE, victim, NULL, NULL, TO_ROOM, ANSI_CYAN);
  } else 
    caster->nothingHappens();

  return TRUE;
}

int detectInvisibility(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  caster->reconcileHelp(victim, discArray[SPELL_DETECT_INVISIBLE]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_DETECT_INVISIBLE)) {
    aff.type = SPELL_DETECT_INVISIBLE;
    aff.duration = ((level * 3 * UPDATES_PER_MUDHOUR) / 2);
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_DETECT_INVISIBLE;
 
    switch (critSuccess(caster, SPELL_DETECT_INVISIBLE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_DETECT_INVISIBLE);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    act("$n's eyes briefly glow yellow.", FALSE, victim, 0, 0, TO_ROOM, ANSI_YELLOW);
    act("Your eyes tingle.", FALSE, victim, 0, 0, TO_CHAR, ANSI_YELLOW);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void detectInvisibility(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;

  ret=detectInvisibility(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int detectInvisibility(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_DETECT_INVISIBLE, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_DETECT_INVISIBLE]->lag;
  diff = discArray[SPELL_DETECT_INVISIBLE]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_DETECT_INVISIBLE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
    return TRUE;
}

int castDetectInvisibility(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_DETECT_INVISIBLE);
  int bKnown = caster->getSkillValue(SPELL_DETECT_INVISIBLE);

  ret=detectInvisibility(caster,victim,level,bKnown);

  if (ret == SPELL_SUCCESS) {
  }
  return TRUE;
}

int trueSight(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;
  caster->reconcileHelp(victim, discArray[SPELL_TRUE_SIGHT]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_TRUE_SIGHT)) {
    aff.type = SPELL_TRUE_SIGHT;
    aff.duration = level / 2 * UPDATES_PER_MUDHOUR;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_TRUE_SIGHT;

    switch (critSuccess(caster, SPELL_TRUE_SIGHT)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_TRUE_SIGHT);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return SPELL_FALSE;
    }

    victim->sendTo("Your eyes glow silver for a moment.\n\r");
    act("$n's eyes take on a silvery hue.", FALSE, victim, 0, 0, TO_ROOM);

    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void trueSight(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  trueSight(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int trueSight(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_TRUE_SIGHT, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_TRUE_SIGHT]->lag;
     diff = discArray[SPELL_TRUE_SIGHT]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_TRUE_SIGHT, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castTrueSight(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_TRUE_SIGHT);
  int bKnown = caster->getSkillValue(SPELL_TRUE_SIGHT);

  ret=trueSight(caster,victim,level,bKnown);
    return TRUE;
}

int telepathy(TBeing *caster, int, byte bKnown)
{
  Descriptor *i;
  const char *msg = caster->spelltask->orig_arg;
  sstring garbled, pgbuf;

  for (; isspace(*msg); msg++);

  if (caster->isPc() && 
     ((caster->desc && 
      IS_SET(caster->desc->autobits, AUTO_NOSHOUT)) || caster->isPlayerAction(PLR_GODNOSHOUT))) {
    caster->sendTo("You aren't allowed to cast this spell at this time!!\n\r");
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_TELEPATHY)) {
    if (!*msg) {
      caster->sendTo("Telepathy is a nice spell, but you need to send some sort of message!\n\r");
      caster->nothingHappens(SILENT_YES);
    } else {
      garbled = caster->garble(NULL, msg, SPEECH_SHOUT, GARBLE_SCOPE_EVERYONE);
      caster->sendTo(COLOR_SPELLS, fmt("You telepathically send the message, \"%s<z>\"\n\r") % msg);
      for (i = descriptor_list; i; i = i->next) {
        if (i->character && (i->character != caster) &&
            !i->connected && !i->character->checkSoundproof() &&
            (dynamic_cast<TMonster *>(i->character) ||
              (!IS_SET(i->autobits, AUTO_NOSHOUT)) ||
              !i->character->isPlayerAction(PLR_GODNOSHOUT))) {

          i->character->sendTo(COLOR_SPELLS, fmt("Your mind is flooded with a telepathic message from %s.\n\r") % caster->getName());
          pgbuf = caster->garble(i->character, garbled, SPEECH_SHOUT, GARBLE_SCOPE_INDIVIDUAL);
          i->character->sendTo(COLOR_SPELLS, fmt("The message is, \"%s%s\"\n\r") % pgbuf % i->character->norm());
          if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
          i->clientf(fmt("%d|%s|%s") % CLIENT_TELEPATHY % colorString(i->character, i, caster->getName(), NULL, COLOR_NONE, FALSE) % colorString(i->character, i, pgbuf, NULL, COLOR_NONE, FALSE));

        }
      }
      caster->addToMove(-5);
    }
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int telepathy(TBeing *caster, const char * msg)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_TELEPATHY, NULL))
       return FALSE;

     lag_t rounds = discArray[SPELL_TELEPATHY]->lag;
     diff = discArray[SPELL_TELEPATHY]->task;

     start_cast(caster, NULL, NULL, caster->roomp, SPELL_TELEPATHY, diff, 1, msg, rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castTelepathy(TBeing *caster)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_TELEPATHY);
  int bKnown = caster->getSkillValue(SPELL_TELEPATHY);

  if ((ret=telepathy(caster,level,bKnown)) == SPELL_SUCCESS) {
  }
  return TRUE;
}

int fear(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  int rc;

  if (caster->isNotPowerful(victim, level, SPELL_FEAR, SILENT_NO)) {
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_FEAR]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_FEAR)) {
    if (victim->isLucky(caster->spellLuckModifier(SPELL_FEAR)) || victim->isImmune(IMMUNE_FEAR, WEAR_BODY)) {
      SV(SPELL_FEAR);
      act("Hmmm...nothing seems to happen.", FALSE, caster, NULL, NULL, TO_CHAR);
      act("You feel afraid for a second, but the effect fades.", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
    } else {
      act("$N is afraid of something! Look at $M run!", FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_WHITE_BOLD);
      act("$N is afraid! Look at $M run!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
      act("You are afraid of $n! Run for your life!", FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);

      // run away immediately
      rc = victim->doFlee("");
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return SPELL_SUCCESS + VICTIM_DEAD;
      }

      // this will keep them running away
      if (!victim->isPc() && level >= (int) (victim->GetMaxLevel() * 1.1)) {
        dynamic_cast<TMonster *>(victim)->addFeared(caster);
      }

      // but since once at full HP (and fearing) they will grow a hatred
      // we need a way to make the fear last a little while....
      affectedData aff;
      aff.type = SPELL_FEAR;
      aff.duration = level * UPDATES_PER_MUDHOUR / 2;
      aff.renew = aff.duration;  // renewable immediately

    // we've made raw immunity check, but allow it to reduce effects too
    aff.duration *= (100 - victim->getImmunity(IMMUNE_FEAR));
    aff.duration /= 100;

      victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    }
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_FEAR)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_FEAR);
        act("What the... $n is afraid of $mself!?",
              FALSE, caster, NULL, NULL, TO_ROOM, ANSI_WHITE_BOLD);
        act("Oops! Now you're afraid of yourself!  Run for your life!",
              FALSE, caster, NULL, victim, TO_CHAR, ANSI_WHITE_BOLD);
        act("Hey, $n was casting that on you!",
              FALSE, caster, NULL, victim, TO_VICT, ANSI_WHITE_BOLD);
        rc = caster->doFlee("");
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        }
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int fear(TBeing *caster, TBeing *victim)
{
  if (!bPassMageChecks(caster, SPELL_FEAR, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_FEAR]->lag;
  taskDiffT diff = discArray[SPELL_FEAR]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_FEAR, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castFear(TBeing *caster, TBeing *victim)
{
  int rc = 0;

  int level = caster->getSkillLevel(SPELL_FEAR);
  int bKnown = caster->getSkillValue(SPELL_FEAR);

  int ret=fear(caster,victim,level,bKnown);
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

int fumble(TBeing *caster, TBeing *victim, int level, byte bKnown) 
{
  bool both = FALSE, done = FALSE;
  TObj *tobj;

//  if (caster->isNotPowerful(victim, level, SPELL_DISPEL_INVISIBLE, SILENT_NO)) {
  if (caster->isNotPowerful(victim, level, SKILL_DISARM, SILENT_NO)) {
    return SPELL_FAIL;
  }

  caster->reconcileHurt(victim, discArray[SPELL_FUMBLE]->alignMod);

  if (caster->bSuccess(bKnown,SPELL_FUMBLE)) {
    switch (critSuccess(caster, SPELL_FUMBLE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FUMBLE);
        both = TRUE;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (both) {
      if (!(victim->heldInPrimHand() && victim->heldInSecHand()))
        both = FALSE;    /* doesn't have something in both hands */
    }
    if (both) {
      tobj = dynamic_cast<TObj *>(victim->heldInPrimHand());
      if (tobj && tobj->isPaired())
        both = FALSE;
    }

    act("Your fingers begin to tingle as magical forces ripple over your hands!",
            FALSE, victim, NULL, NULL, TO_CHAR, ANSI_CYAN);
    act("A glowing blue ball of light poofs out around $n's hands!",
            FALSE, victim, NULL, NULL, TO_ROOM, ANSI_CYAN);

    if (victim->heldInSecHand()) {
      victim->dropWeapon(victim->getSecondaryHold());
      done = TRUE;
    }
    if (victim->heldInPrimHand() && (both || !done)) {
      victim->dropWeapon(victim->getPrimaryHold());
      done = TRUE;
    }
    if (!done) {
      act("But $N isn't holding anything!", FALSE, caster, NULL, victim, TO_CHAR, ANSI_CYAN);
    }

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_FUMBLE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_FUMBLE);

        act("Oops! You have a terrible feeling something went horribly wrong!",
                FALSE, caster, NULL, NULL, TO_CHAR, ANSI_CYAN);
        act("You've cast your 'fumble' on yourself!",
                FALSE, caster, NULL, NULL, TO_CHAR, ANSI_CYAN);

        tobj = dynamic_cast<TObj *>(victim->heldInSecHand());
        if (tobj && !tobj->isPaired()) {
          victim->dropWeapon(victim->getSecondaryHold());
          done = TRUE;
        }
        if (victim->heldInPrimHand() && (both || !done)) {
          victim->dropWeapon(victim->getPrimaryHold());
          done = TRUE;
        }
        return SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int fumble(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_FUMBLE, victim))
       return FALSE;

     lag_t rounds = discArray[SPELL_FEAR]->lag;
     diff = discArray[SPELL_FEAR]->task;

     start_cast(caster, victim, NULL, caster->roomp, SPELL_FUMBLE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;

     return TRUE;
}

int castFumble(TBeing *caster, TBeing *victim)
{
  int ret,level;

  level = caster->getSkillLevel(SPELL_FUMBLE);
  int bKnown = caster->getSkillValue(SPELL_FUMBLE);

  ret=fumble(caster,victim,level,bKnown);

#if 1
  if (ret == SPELL_SUCCESS || ret == SPELL_CRIT_FAIL)
    if ((ret = victim->hit(caster)))
      return ret;
    else
      victim->setVictFighting(caster);
#else
  if (ret == SPELL_SUCCESS) {
  } else {
    if (ret==SPELL_CRIT_FAIL) {
    }
  }
#endif

  return TRUE;
}
