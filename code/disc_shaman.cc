#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman.h"
#include "spelltask.h"


// returns VICTIM_DEAD if corpse should be fried
int voodoo(TBeing *caster, TObj *obj, int level, byte bKnown)
{
  TMonster *mob;
  TThing *t, *n;
  TBaseCorpse *corpse = dynamic_cast<TBaseCorpse *>(obj);
  char buf[256], capbuf[256];
  wearSlotT i;

  /* some sort of check for corpse hood */
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    caster->sendTo("You're invoking that on something that isn't a corpse!?\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
 
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int) caster->GetMaxLevel() - 3) {
    caster->sendTo("Your invokation lacks the power.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseVnum() <= 0) {
    // med mobs == -1, pcs == -2
    caster->sendTo("A strange power prevents anything from occurring.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (!(mob = read_mobile(corpse->getCorpseVnum(), VIRTUAL))) {
    vlogf(LOG_BUG, "FAILED Load!!  No mob (%d)", corpse->getCorpseVnum());
    caster->sendTo("Something screwed up.  Tell a god.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  
  mob->genericCharmFix();

  // make it a zombie
  SET_BIT(mob->specials.act, ACT_ZOMBIE);
  
  *caster->roomp += *mob;

  act("You channel some of the cosmic energy into $p!", FALSE, caster, corpse, NULL, TO_CHAR);
  act("$n channels some of the cosmic energy into $p!", TRUE, caster, corpse, NULL, TO_ROOM);

  caster->roomp->playsound(SOUND_SPELL_ANIMATE_DEAD, SOUND_TYPE_MAGIC);

  // adjust stats : somewhat weaker
  mob->setMaxHit(max(1, mob->hitLimit() * 2 / 10));
  mob->setHit((int) (mob->hitLimit() >> 1));
  mob->setSex(SEX_NEUTER);

  // take all from corpse, and give to zombie 
  for (t = corpse->stuff; t; t = n) {
    n = t->nextThing;
    --(*t);
    *mob += *t;
  }

  for (i=MIN_WEAR;i < MAX_WEAR;i++) {
    if (mob->hasPart(i)) 
      mob->setCurLimbHealth(i, mob->getMaxLimbHealth(i));
  }

  // set up descriptions and such 
  mob->swapToStrung();
  sprintf(buf, "zombie %s", mob_index[mob->getMobIndex()].name);
  delete [] mob->name;
  mob->name = mud_str_dup(buf);
  sprintf(buf, "a zombie of %s", mob_index[mob->getMobIndex()].short_desc);
  delete [] mob->shortDescr;
  mob->shortDescr = mud_str_dup(buf);

  strcpy(capbuf, mob->getName());
  sprintf(buf, "%s is here, obediently following its master.\n\r", cap(capbuf));
  delete [] mob->player.longDescr;
  mob->player.longDescr = mud_str_dup(buf);

  if (caster->tooManyFollowers(mob, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!", 
          TRUE, caster, NULL, mob, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!", 
          TRUE, caster, NULL, mob, TO_ROOM);
    return SPELL_FAIL + VICTIM_DEAD;
  }

  REMOVE_BIT(mob->specials.affectedBy, AFF_CHARM );

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_VOODOO)) {
    affectedData aff;

    SET_BIT(mob->specials.affectedBy, AFF_CHARM );
    mob->setPosition(POSITION_STUNNED);    // make it take a little to wake up
    caster->addFollower(mob);
    act("$N slowly begins to move...it's slowly standing up!",
             FALSE, caster, NULL, mob, TO_CHAR);
    act("$N slowly begins to move...it's slowly standing up!", 
             FALSE, caster, NULL, mob, TO_ROOM);

    aff.type = SPELL_VOODOO;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int) (caster->percModifier() * aff.duration);

    aff.duration *= 2;   // zombie adjustment

    mob->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    mob->affectTo(&aff);

    return SPELL_SUCCESS + VICTIM_DEAD;
  } else {
    act("You've created a monster; $N hates you!",
            FALSE, caster, NULL, mob, TO_CHAR);
    caster->setCharFighting(mob);
    caster->setVictFighting(mob);
    return SPELL_FAIL + VICTIM_DEAD;
  }
}

void voodoo(TBeing *caster, TObj *corpse, TMagicItem *obj)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, obj, corpse, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, obj, corpse, TO_ROOM);
  ret=voodoo(caster,corpse,level,bKnown);
}

int castVoodoo(TBeing * caster, TObj * corpse)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, corpse, 0, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, corpse, 0, TO_ROOM);
  if ((ret=voodoo(caster,corpse,level,bKnown)) == SPELL_SUCCESS) {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;
  return TRUE;
}

int voodoo(TBeing * caster, TObj * corpse)
{
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_VOODOO, corpse))
    return FALSE;

  lag_t rounds = discArray[SPELL_VOODOO]->lag;
  diff = discArray[SPELL_VOODOO]->task;

  start_cast(caster, NULL, corpse, caster->roomp, SPELL_VOODOO, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int resurrection(TBeing * caster, TObj * obj, int level, byte bKnown)
{
  affectedData aff;
  TThing *t, *n;
  TMonster * victim;
  TBaseCorpse *corpse;

  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    caster->sendTo("You can't resurrect something that's not a corpse!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->getMoney() < 25000) {
    caster->sendTo("You don't have enough talens to make the resurrection sacrifice.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  caster->addToMoney(-25000, GOLD_HOSPITAL);

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_RESURRECTION)) {
    victim = read_mobile(corpse->getCorpseVnum(), VIRTUAL);
    *caster->roomp += *victim;
    victim->genericCharmFix();
    victim->setHit(1);
    victim->setPosition(POSITION_STUNNED);
    act("$N slowly rises from the $g.", FALSE, caster, 0, victim, TO_ROOM);
    caster->reconcileHelp(victim,discArray[SPELL_RESURRECTION]->alignMod);
      
    if (victim->isImmune(IMMUNE_CHARM, level)) {
      victim->setPosition(POSITION_STANDING);
      victim->doSay("Thank you");
      return SPELL_FALSE;
    } else if (caster->tooManyFollowers(victim, FOL_ZOMBIE)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM);
      return SPELL_FALSE;
    }

    aff.type      = SPELL_RESURRECTION;
    aff.duration = caster->followTime(); 
    aff.duration = (int) (caster->percModifier() * aff.duration);
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;

    if (critSuccess(caster, SPELL_RESURRECTION)) {
      CS(SPELL_RESURRECTION);
      aff.duration *= 2;
    }
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    caster->addFollower(victim);
    victim->setCarriedWeight(0.0);
    victim->setCarriedVolume(0);
    for (t = corpse->stuff; t; t = n) {
      n = t->nextThing;
      --(*t);
      *victim += *t;
    }
    act("With mystic power, $p is resurrected.", 
            TRUE, caster, corpse, 0, TO_CHAR);
    act("With mystic power, $p is resurrected.", 
            TRUE, caster, corpse, 0, TO_ROOM);
    return SPELL_SUCCESS;  // note, this indicates obj should go bye bye
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
}

void resurrection(TBeing *caster, TObj *obj, TMagicItem * obj_mi)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, obj_mi, obj, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, obj_mi, obj, TO_ROOM);
  ret=resurrection(caster,obj,level,bKnown);
}

int castResurrection(TBeing * caster, TObj * obj)
{
  int ret, level;
  TBaseCorpse *corpse;

  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    return FALSE;
  }

  if (dynamic_cast<TPCorpse *>(corpse)) {
     /* corpse is a pc */
    caster->sendTo("Resurrection of players is not currently supported.\n\r");
    return FALSE;
  }
  if (corpse->getCorpseVnum() == -1) {
     /* corpse is a MEDIT mob */
    caster->sendTo("You can't resurrect that.\n\r");
    return FALSE;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
     /* corpse is a body part, pile of dust, etc */
    caster->sendTo("There isn't enough left to resurrect.\n\r");
    return FALSE;
  }
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, obj, 0, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, obj, 0, TO_ROOM);

  level = caster->getSkillLevel(SPELL_RESURRECTION);
  int bKnown = caster->getSkillValue(SPELL_RESURRECTION);
  ret=resurrection(caster,corpse,level,bKnown);
  if (ret == SPELL_SUCCESS) {
    return DELETE_ITEM;  // delete corpse
  }
  return TRUE;
}

int resurrection(TBeing * caster, TObj * obj)
{
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_RESURRECTION, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_RESURRECTION]->lag;
  diff = discArray[SPELL_RESURRECTION]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_RESURRECTION, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

// returns VICTIM_DEAD if corpse should be fried
int dancingBones(TBeing * caster, TObj * obj, int level, byte bKnown)
{
  TMonster *mob;
  TThing *t, *n;
  TBaseCorpse *corpse;
  char buf[256], capbuf[256];
  wearSlotT i;

  /* some sort of check for corpse hood */
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    caster->sendTo("You're invoking that on something that isn't a corpse!?\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    vlogf(LOG_JESUS, "Someone tried to invoke dancing bones on a non-corpse object.");
    return SPELL_FAIL;
  }
 
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    vlogf(LOG_JESUS, "Someone tried to invoke dancing bones on a corpse object flagged NO_REGEN.");
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int) (3 * level / 5)) {
    caster->sendTo("Your invokation lacks the power.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    vlogf(LOG_JESUS, "Someone tried to invoke dancing bones on a corpse of > 3*lev / 5.");
    return SPELL_FAIL;
  }
  if (corpse->getCorpseVnum() <= 0) {
    // med mobs == -1, pcs == -2
    caster->sendTo("A strange power prevents anything from occurring.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    vlogf(LOG_JESUS, "Someone tried to invoke dancing bones on a corpse with vnum less than or 0.");
    return SPELL_FAIL;
  }
  if (!(mob = read_mobile(corpse->getCorpseVnum(), VIRTUAL))) {
    vlogf(LOG_BUG, "FAILED Load!!  No mob (%d)", corpse->getCorpseVnum());
    caster->sendTo("Something screwed up.  Tell a god.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    vlogf(LOG_JESUS, "Dancing Bones: FAILED Load!!  No mob (%d)", corpse->getCorpseVnum());
    return SPELL_FAIL;
  }
  
  // make it a skeleton
  SET_BIT(mob->specials.act, ACT_SKELETON);
  
  mob->genericCharmFix();
  *caster->roomp += *mob;

  act("You channel some of the cosmic energy into $p!", FALSE, caster, corpse, NULL, TO_CHAR);
  act("$n channels some of the cosmic energy into $p!", TRUE, caster, corpse, NULL, TO_ROOM);

  // adjust stats : somewhat weaker
  mob->setMaxHit(max(1, mob->hitLimit() * 4 / 10));
  mob->setHit((int) (mob->hitLimit() >> 1));
  mob->setSex(SEX_NEUTER);
  mob->setCarriedWeight(0.0);
  mob->setCarriedVolume(0);

  // take all from corpse, and give to mob
  for (t = corpse->stuff; t; t = n) {
    n = t->nextThing;
    --(*t);
    *mob += *t;
  }

  for (i=MIN_WEAR;i < MAX_WEAR;i++) {
    if (mob->hasPart(i)) {
      mob->setCurLimbHealth(i, mob->getMaxLimbHealth(i));
    }
  }

 /* set up descriptions and such */
  mob->swapToStrung();
  sprintf(buf, "skeleton %s", mob_index[mob->getMobIndex()].name);
  delete [] mob->name;
  mob->name = mud_str_dup(buf);
  sprintf(buf, "a skeleton of %s", mob_index[mob->getMobIndex()].short_desc);
  delete [] mob->shortDescr;
  mob->shortDescr = mud_str_dup(buf);
  strcpy(capbuf, mob->getName());
  sprintf(buf, "%s is here, enthralled by it's master.\n\r", cap(capbuf));
  delete [] mob->player.longDescr;
  mob->player.longDescr = mud_str_dup(buf);

  if (caster->tooManyFollowers(mob, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!", 
          TRUE, caster, NULL, mob, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!", 
          TRUE, caster, NULL, mob, TO_ROOM);
    return SPELL_FAIL + VICTIM_DEAD;
  }

  REMOVE_BIT(mob->specials.affectedBy, AFF_CHARM );

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_DANCING_BONES)) {
    affectedData aff;

    SET_BIT(mob->specials.affectedBy, AFF_CHARM );
    mob->setPosition(POSITION_STUNNED);    // make it take a little to wake up
    caster->addFollower(mob);
    act("$N slowly begins to move...it's slowly standing up!",
             FALSE, caster, NULL, mob, TO_CHAR);
    act("$N slowly begins to move...it's slowly standing up!", 
             FALSE, caster, NULL, mob, TO_ROOM);

    aff.type = SPELL_DANCING_BONES;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int) (caster->percModifier() * aff.duration);

    aff.duration *= 3;   // skeleton adjustment

    mob->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    mob->affectTo(&aff);

    return SPELL_SUCCESS + VICTIM_DEAD;
    vlogf(LOG_JESUS, "Someone succesfully casted Dancing Bones.");
  } else {
    act("You've created a monster; $N hates you!",
            FALSE, caster, NULL, mob, TO_CHAR);
    caster->setCharFighting(mob);
    caster->setVictFighting(mob);
    return SPELL_FAIL + VICTIM_DEAD;
  }
}

void dancingBones(TBeing *caster, TObj *corpse, TMagicItem *obj)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_DANCING_BONES);
  int bKnown = caster->getSkillValue(SPELL_DANCING_BONES);
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, obj, corpse, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, obj, corpse, TO_ROOM);
  ret=dancingBones(caster,corpse,level,bKnown);
}

int castDancingBones(TBeing * caster, TObj * corpse)
{
  int ret, level;

  level = caster->getSkillLevel(SPELL_DANCING_BONES);
  int bKnown = caster->getSkillValue(SPELL_DANCING_BONES);
  act("You direct a strange beam of energy at $p.",
          FALSE, caster, corpse, 0, TO_CHAR);
  act("$n directs a strange beam of energy at $p.",
          FALSE, caster, corpse, 0, TO_ROOM);
  if ((ret=dancingBones(caster,corpse,level,bKnown)) == SPELL_SUCCESS) {
  }
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;
  return TRUE;
}

int dancingBones(TBeing * caster, TObj * corpse)
{
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DANCING_BONES, corpse))
    return FALSE;

  lag_t rounds = discArray[SPELL_DANCING_BONES]->lag;
  diff = discArray[SPELL_DANCING_BONES]->task;

  start_cast(caster, NULL, corpse, caster->roomp, SPELL_DANCING_BONES, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int shieldOfMists(TBeing *caster, TBeing *victim, int level, byte bKnown)
{
  affectedData aff;

  aff.type = SPELL_SHIELD_OF_MISTS;
  aff.level = level;
  aff.duration = (3 + (aff.level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_ARMOR;
  aff.modifier = -80;
  aff.bitvector = 0;

  if (bSuccess(caster,bKnown,SPELL_SHIELD_OF_MISTS)) {
    switch (critSuccess(caster, SPELL_SHIELD_OF_MISTS)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_SHIELD_OF_MISTS);
        aff.duration = (12 + (level / 2)) * UPDATES_PER_MUDHOUR;
        if (caster != victim)
          aff.modifier *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster != victim) 
      aff.modifier /= 5;

    //Second argument FALSE causes it to add new duration to old
    //Third argument TRUE causes it to average the old and newmodifier

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }


    act("<G>$n is enveloped by a thick green mist!<z>", FALSE, victim, NULL,
NULL, TO_ROOM);
    act("<G>You are enveloped by a thick green mist!<z>", FALSE, victim,
NULL, NULL, TO_CHAR);

    caster->reconcileHelp(victim, discArray[SPELL_SHIELD_OF_MISTS]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void shieldOfMists(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
int ret;

ret=shieldOfMists(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int shieldOfMists(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

    if (!bPassMageChecks(caster, SPELL_SHIELD_OF_MISTS, victim))
      return FALSE;

    lag_t rounds = discArray[SPELL_SHIELD_OF_MISTS]->lag;
    diff = discArray[SPELL_SHIELD_OF_MISTS]->task;

    start_cast(caster, victim, NULL, caster->roomp, SPELL_SHIELD_OF_MISTS,
diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
      return TRUE;
}

int castShieldOfMists(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_SHIELD_OF_MISTS);
  int bKnown = caster->getSkillValue(SPELL_SHIELD_OF_MISTS);

  if ((ret=shieldOfMists(caster,victim,level,bKnown)) == SPELL_SUCCESS) {
  }
  return TRUE;
}

int enthrallSpectre(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(THRALL_SPECTRE, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_SPECTRE, 0);

  if (bSuccess(caster, bKnown, SPELL_ENTHRALL_SPECTRE)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_SPECTRE;
    aff.level = level;
    aff.duration  = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_SPECTRE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_SPECTRE);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim, TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
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
      act("Your loa is displeased! $N hates you!",
             FALSE, caster, NULL, victim, TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_SPECTRE);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallSpectre(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(THRALL_SPECTRE) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_ENTHRALL_SPECTRE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_SPECTRE]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_SPECTRE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_SPECTRE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castEnthrallSpectre(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_ENTHRALL_SPECTRE);

   if
((ret=enthrallSpectre(caster,level,caster->getSkillValue(SPELL_ENTHRALL_SPECTRE))) == SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

int enthrallGhast(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(THRALL_GHAST, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_GHAST, 0);

  if (bSuccess(caster, bKnown, SPELL_ENTHRALL_GHAST)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_GHAST;
    aff.level = level;
    aff.duration  = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_GHAST)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_GHAST);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim, TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
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
      act("Your loa is displeased! $N hates you!",
             FALSE, caster, NULL, victim, TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_GHAST);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL,
victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallGhast(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet
conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(THRALL_GHAST) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_ENTHRALL_GHAST, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_GHAST]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_GHAST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_GHAST, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castEnthrallGhast(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_ENTHRALL_GHAST);

   if ((ret=enthrallGhast(caster,level,caster->getSkillValue(SPELL_ENTHRALL_GHAST))) == SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

void TThing::sacrificeMe(TBeing *ch, const char *arg)
{
  TBaseCorpse *corpse;
  TObj *obj;
  TBeing *dummy;

  // Check to see if argument passed exists in room
  if (!generic_find(arg, FIND_OBJ_ROOM, ch, &dummy, &obj)) {
    ch->sendTo("You do not see a %s here.\n\r", arg);
    return;
  }
  // Check to see if corpse is a corpse
  
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    ch->sendTo(COLOR_OBJECTS, "You cannot sacrifice %s.\n\r", obj->getName());
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    act("Someone must be sacrificing $p currently.",
          FALSE, ch, corpse, 0, TO_CHAR);    
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("You aren't able to sacrifice that $p.",
          FALSE, ch, corpse, 0, TO_CHAR);    
    return;
  }
  if (!corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    corpse->addCorpseFlag(CORPSE_SACRIFICE);
  }
  ch->sendTo("You start sacrificing a corpse.\n\r");
  act("$n begins to chant over a corpse.", FALSE, ch, NULL, 0, TO_ROOM);
}

void TTool::sacrificeMe(TBeing *ch, const char *arg)
{
  TObj *obj;
  TBaseCorpse *corpse;
  TBeing *dummy;

  if (getToolType() != TOOL_TOTEM) {
    ch->sendTo("You must be holding a totem in your primary hand to perform this ritual.\n\r");
    return;
  }

  // Check to see if argument passed exists in room
  if (!generic_find(arg, FIND_OBJ_ROOM, ch, &dummy, &obj)) {
    ch->sendTo("You do not see a %s here.\n\r", arg);
    return;
  }
  // Check to see if corpse is a corpse
  
  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    ch->sendTo(COLOR_OBJECTS, "You cannot sacrifice %s.\n\r", obj->getName());
    return;
  }
  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    act("Someone must be sacrificing $p currently.",
          FALSE, ch, corpse, 0, TO_CHAR);    
    return;
  }

  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    act("You aren't able to sacrifice $p.",
          FALSE, ch, corpse, 0, TO_CHAR);    return;
  }
  if (!corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    corpse->addCorpseFlag(CORPSE_SACRIFICE);
  }
  ch->sendTo("You start the sacrificial ritual.\n\r");
  act("$n begins to chant over a corpse.", FALSE, ch, NULL, 0, TO_ROOM);
  start_task(ch, corpse, 0, TASK_SACRIFICE, "", 2, ch->inRoom(), 0, 0, 5);
}

void TBeing::doSacrifice(const char *arg)
{
  TThing *tobj;

  for (; isspace(*arg); arg++);

  if (getPosition() != POSITION_STANDING) {
    sendTo("Have some respect! Stand to perform the sacrifice!\n\r");
    return;
  }
  if (!doesKnowSkill(SKILL_SACRIFICE)) {
    sendTo("You don't have a clue about sacrificing anything.\n\r");
    return;
  }

  tobj = heldInPrimHand();
  if (!(tobj = heldInPrimHand())) {
    sendTo("You must be holding a totem in your primary hand to perform the ritual.\n\r");
    return;
  }
  if (!tobj) {
    sendTo("You must be holding a totem in your primary hand to perform the ritual.\n\r");
    return;
  }
  tobj->sacrificeMe(this, arg);
}

int vampiricTouch(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!",
             FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  int num = ::number(1,30);
  int num2 = ::number(1,16);
  int num3 = ::number(1,30);

  if (bSuccess(caster, bKnown,SPELL_VAMPIRIC_TOUCH)) {
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N groans in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You groan in pain as life is drawn from your body!", FALSE, caster, NULL, victim, TO_VICT);
    victim->addToHit(-num);
    caster->addToHit(num2);
    caster->addToLifeforce(num3);
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_VAMPIRIC_TOUCH)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_VAMPIRIC_TOUCH);
        act("$n's body glows a dark, evil-looking red!", 
               FALSE, caster, NULL, NULL, TO_ROOM);
        act("You sang the invokation incorrectly! The ancestors are EXTREMELY pissed!", 
               FALSE, caster, NULL, NULL, TO_CHAR);
        victim->addToHit(num);
        caster->addToHit(-num);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  caster->updatePos();
}

int vampiricTouch(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_VAMPIRIC_TOUCH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_VAMPIRIC_TOUCH]->lag;
  taskDiffT diff = discArray[SPELL_VAMPIRIC_TOUCH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_VAMPIRIC_TOUCH, diff, 1, "", rounds, caster->in_room, 0, 
0,TRUE, 
0);

  return TRUE; 
}

int castVampiricTouch(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_VAMPIRIC_TOUCH);
  int bKnown = caster->getSkillValue(SPELL_VAMPIRIC_TOUCH);

  ret=vampiricTouch(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_VAMPIRIC_TOUCH));
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

int vampiricTouch(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = vampiricTouch(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}

int lifeLeech(TBeing *caster, TBeing *victim, int level, byte bKnown, int adv_learn)
{
  if (victim->isImmortal()) {
    act("You can't touch a god in that manner!",
             FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FAIL;
  }
  int num = ::number(1,10);
  int num2 = ::number(1,8);
  int num3 = ::number(1,10);

  if (bSuccess(caster, bKnown,SPELL_LIFE_LEECH)) {
    act("$N buckles in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$N buckles in pain as life is drawn from $S body!", FALSE, caster, NULL, victim, TO_CHAR);
    act("You buckle in pain as life is drawn from your body!", FALSE, caster, NULL, victim, TO_VICT);
    victim->addToHit(-num);
    caster->addToHit(num2);
    caster->addToLifeforce(num3);
    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_LIFE_LEECH)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_LIFE_LEECH);
        act("<r>$n's body glows a dark, evil-looking red!<z>", 
               FALSE, caster, NULL, NULL, TO_ROOM);
        act("<r>You sang the invokation incorrectly! The ancestors are<z> <R>EXTREMELY<z> <r>pissed!<z>", 
               FALSE, caster, NULL, NULL, TO_CHAR);
        victim->addToHit(num);
        caster->addToHit(-num);
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
  caster->updatePos();
}

int lifeLeech(TBeing *caster, TBeing *victim)
{
  if (!bPassShamanChecks(caster, SPELL_LIFE_LEECH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_LIFE_LEECH]->lag;
  taskDiffT diff = discArray[SPELL_LIFE_LEECH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_LIFE_LEECH, diff, 1, "", rounds, caster->in_room, 0, 
0,TRUE, 
0);

  return TRUE; 
}

int castLifeLeech(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_LIFE_LEECH);
  int bKnown = caster->getSkillValue(SPELL_LIFE_LEECH);

  ret=lifeLeech(caster,victim,level,bKnown, caster->getAdvLearning(SPELL_LIFE_LEECH));
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

int lifeLeech(TBeing *tMaster, TBeing *tSucker, TMagicItem *tMagItem)
{
  int tRc = FALSE,
      tReturn;

  tReturn = lifeLeech(tMaster, tSucker, tMagItem->getMagicLevel(), tMagItem->getMagicLearnedness(), 0);

  if (IS_SET(tReturn, VICTIM_DEAD))
    ADD_DELETE(tRc, DELETE_VICT);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tRc, DELETE_THIS);

  return tRc;
}




