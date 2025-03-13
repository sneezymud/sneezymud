#include "room.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman_control.h"
#include "spelltask.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_magic_item.h"
#include "combat.h"

// ENTHRALL GHOUL

int enthrallGhoul(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::THRALL_GHOUL, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_GHOUL, 0);

  if (caster->bSuccess(bKnown, SPELL_ENTHRALL_GHOUL)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_GHOUL;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_GHOUL)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_GHOUL);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_GHOUL);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallGhoul(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::THRALL_GHOUL) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_GHOUL, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_GHOUL]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_GHOUL]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_GHOUL, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castEnthrallGhoul(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ENTHRALL_GHOUL);

  if ((ret = enthrallGhoul(caster, level,
         caster->getSkillValue(SPELL_ENTHRALL_GHOUL))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

// END ENTHRALL GHOUL

int enthrallSpectre(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::THRALL_SPECTRE, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_SPECTRE, 0);

  if (caster->bSuccess(bKnown, SPELL_ENTHRALL_SPECTRE)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_SPECTRE;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_SPECTRE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_SPECTRE);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_SPECTRE);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallSpectre(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::THRALL_SPECTRE) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_SPECTRE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_SPECTRE]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_SPECTRE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_SPECTRE, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castEnthrallSpectre(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ENTHRALL_SPECTRE);

  if ((ret = enthrallSpectre(caster, level,
         caster->getSkillValue(SPELL_ENTHRALL_SPECTRE))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

int enthrallGhast(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::THRALL_GHAST, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_GHAST, 0);

  if (caster->bSuccess(bKnown, SPELL_ENTHRALL_GHAST)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_GHAST;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_GHAST)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_GHAST);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_GHAST);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);

    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallGhast(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::THRALL_GHAST) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_GHAST, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_GHAST]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_GHAST]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_GHAST, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castEnthrallGhast(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ENTHRALL_GHAST);

  if ((ret = enthrallGhast(caster, level,
         caster->getSkillValue(SPELL_ENTHRALL_GHAST))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

// returns VICTIM_DEAD if corpse should be fried
int voodoo(TBeing* caster, TObj* obj, int level, short bKnown) {
  TMonster* mob;
  TThing* t;
  TBaseCorpse* corpse = dynamic_cast<TBaseCorpse*>(obj);
  char buf[256], capbuf[256];
  wearSlotT i;

  /* some sort of check for corpse hood */
  if (!(corpse = dynamic_cast<TBaseCorpse*>(obj))) {
    caster->sendTo(
      "You're invoking that on something that isn't a corpse!?\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int)caster->GetMaxLevel() - 3) {
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
    vlogf(LOG_BUG,
      format("FAILED Load!!  No mob (%d)") % corpse->getCorpseVnum());
    caster->sendTo("Something screwed up.  Tell a god.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  mob->genericCharmFix();

  // make it a zombie
  SET_BIT(mob->specials.act, ACT_ZOMBIE);

  *caster->roomp += *mob;

  act("You channel some of the cosmic energy into $p!", FALSE, caster, corpse,
    NULL, TO_CHAR);
  act("$n channels some of the cosmic energy into $p!", TRUE, caster, corpse,
    NULL, TO_ROOM);

  caster->roomp->playsound(SOUND_SPELL_ANIMATE_DEAD, SOUND_TYPE_MAGIC);

  // adjust stats : somewhat weaker
  mob->setMaxHit(max(1, mob->hitLimit() * 2 / 7));
  mob->setHit((int)(mob->hitLimit() >> 1));
  mob->setSex(SEX_NEUTER);

  // take all from corpse, and give to zombie
  for (StuffIter it = corpse->stuff.begin(); it != corpse->stuff.end();) {
    t = *(it++);
    --(*t);
    *mob += *t;
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (mob->hasPart(i))
      mob->setCurLimbHealth(i, mob->getMaxLimbHealth(i));
  }

  // set up descriptions and such
  mob->swapToStrung();
  sprintf(buf, "zombie %s", mob_index[mob->getMobIndex()].name);
  mob->name = buf;
  sprintf(buf, "a zombie of %s", mob_index[mob->getMobIndex()].short_desc);
  mob->shortDescr = buf;

  strcpy(capbuf, mob->getName().c_str());
  sprintf(buf, "%s is here, obediently following its master.\n\r",
    sstring(capbuf).cap().c_str());
  mob->player.longDescr = buf;

  if (caster->tooManyFollowers(mob, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
      mob, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
      mob, TO_ROOM);
    return SPELL_FAIL + VICTIM_DEAD;
  }

  REMOVE_BIT(mob->specials.affectedBy, AFF_CHARM);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_VOODOO)) {
    affectedData aff;

    SET_BIT(mob->specials.affectedBy, AFF_CHARM);
    mob->setPosition(POSITION_STUNNED);  // make it take a little to wake up
    caster->addFollower(mob);
    //    delete corpse;
    act("$N slowly begins to move...it's slowly standing up!", FALSE, caster,
      NULL, mob, TO_CHAR);
    act("$N slowly begins to move...it's slowly standing up!", FALSE, caster,
      NULL, mob, TO_ROOM);

    aff.type = SPELL_VOODOO;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int)(caster->percModifier() * aff.duration);

    aff.duration *= 2;  // zombie adjustment

    mob->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    mob->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, mob, PERMANENT_DURATION);

    delete corpse;
    return SPELL_SUCCESS + VICTIM_DEAD;
  } else {
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, mob,
      TO_CHAR);
    caster->setCharFighting(mob);
    caster->setVictFighting(mob);
    delete corpse;
    return SPELL_FAIL + VICTIM_DEAD;
  }
}

void voodoo(TBeing* caster, TObj* corpse, TMagicItem* obj) {
  int level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.", FALSE, caster, obj, corpse,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, obj, corpse,
    TO_ROOM);
  voodoo(caster, corpse, level, bKnown);
}

int castVoodoo(TBeing* caster, TObj* corpse) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.", FALSE, caster, corpse, 0,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, corpse, 0,
    TO_ROOM);
  if ((ret = voodoo(caster, corpse, level, bKnown)) == SPELL_SUCCESS) {}
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;
  return TRUE;
}

int voodoo(TBeing* caster, TObj* corpse) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_VOODOO, corpse))
    return FALSE;

  lag_t rounds = discArray[SPELL_VOODOO]->lag;
  diff = discArray[SPELL_VOODOO]->task;

  start_cast(caster, NULL, corpse, caster->roomp, SPELL_VOODOO, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

// returns VICTIM_DEAD if corpse should be fried
int dancingBones(TBeing* caster, TObj* obj, int level, short bKnown) {
  TMonster* mob;
  TThing* t;
  TBaseCorpse* corpse;
  char buf[256], capbuf[256];
  wearSlotT i;

  /* some sort of check for corpse hood */
  if (!(corpse = dynamic_cast<TBaseCorpse*>(obj))) {
    caster->sendTo(
      "You're invoking that on something that isn't a corpse!?\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int)(3 * level / 5)) {
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
    vlogf(LOG_BUG,
      format("FAILED Load!!  No mob (%d)") % corpse->getCorpseVnum());
    caster->sendTo("Something screwed up.  Tell a god.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  // make it a skeleton
  SET_BIT(mob->specials.act, ACT_SKELETON);

  mob->genericCharmFix();
  *caster->roomp += *mob;

  act("You channel some of the cosmic energy into $p!", FALSE, caster, corpse,
    NULL, TO_CHAR);
  act("$n channels some of the cosmic energy into $p!", TRUE, caster, corpse,
    NULL, TO_ROOM);

  // adjust stats : somewhat weaker
  mob->setMaxHit(max(1, mob->hitLimit() * 4 / 7));
  mob->setHit((int)(mob->hitLimit() >> 1));
  mob->setSex(SEX_NEUTER);

  // take all from corpse, and give to mob
  for (StuffIter it = corpse->stuff.begin(); it != corpse->stuff.end();) {
    t = *(it++);
    --(*t);
    *mob += *t;
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (mob->hasPart(i)) {
      mob->setCurLimbHealth(i, mob->getMaxLimbHealth(i));
    }
  }

  /* set up descriptions and such */
  mob->swapToStrung();
  sprintf(buf, "skeleton %s", mob_index[mob->getMobIndex()].name);
  mob->name = buf;
  sprintf(buf, "a skeleton of %s", mob_index[mob->getMobIndex()].short_desc);
  mob->shortDescr = buf;
  strcpy(capbuf, mob->getName().c_str());
  sprintf(buf, "%s is here, enthralled by it's master.\n\r",
    sstring(capbuf).cap().c_str());
  mob->player.longDescr = buf;

  if (caster->tooManyFollowers(mob, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
      mob, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
      mob, TO_ROOM);
    return SPELL_FAIL + VICTIM_DEAD;
  }

  REMOVE_BIT(mob->specials.affectedBy, AFF_CHARM);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_DANCING_BONES)) {
    affectedData aff;

    SET_BIT(mob->specials.affectedBy, AFF_CHARM);
    mob->setPosition(POSITION_STUNNED);  // make it take a little to wake up
    caster->addFollower(mob);
    //    delete corpse;
    act("$N slowly begins to move...it's slowly standing up!", FALSE, caster,
      NULL, mob, TO_CHAR);
    act("$N slowly begins to move...it's slowly standing up!", FALSE, caster,
      NULL, mob, TO_ROOM);

    aff.type = SPELL_DANCING_BONES;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int)(caster->percModifier() * aff.duration);

    aff.duration *= 3;  // skeleton adjustment

    mob->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    mob->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, mob, PERMANENT_DURATION);

    delete corpse;
    return SPELL_SUCCESS + VICTIM_DEAD;
  } else {
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, mob,
      TO_CHAR);
    caster->setCharFighting(mob);
    caster->setVictFighting(mob);
    delete corpse;
    return SPELL_FAIL + VICTIM_DEAD;
  }
}

void dancingBones(TBeing* caster, TObj* corpse, TMagicItem* obj) {
  int level;

  level = caster->getSkillLevel(SPELL_DANCING_BONES);
  int bKnown = caster->getSkillValue(SPELL_DANCING_BONES);
  act("You direct a strange beam of energy at $p.", FALSE, caster, obj, corpse,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, obj, corpse,
    TO_ROOM);
  dancingBones(caster, corpse, level, bKnown);
}

int castDancingBones(TBeing* caster, TObj* corpse) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_DANCING_BONES);
  int bKnown = caster->getSkillValue(SPELL_DANCING_BONES);
  act("You direct a strange beam of energy at $p.", FALSE, caster, corpse, 0,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, corpse, 0,
    TO_ROOM);
  if ((ret = dancingBones(caster, corpse, level, bKnown)) == SPELL_SUCCESS) {}
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;
  return TRUE;
}

int dancingBones(TBeing* caster, TObj* corpse) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_DANCING_BONES, corpse))
    return FALSE;

  lag_t rounds = discArray[SPELL_DANCING_BONES]->lag;
  diff = discArray[SPELL_DANCING_BONES]->task;

  start_cast(caster, NULL, corpse, caster->roomp, SPELL_DANCING_BONES, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int shieldOfMists(TBeing* caster, TBeing* victim, int level, short bKnown) {
  affectedData aff;

  aff.type = SPELL_SHIELD_OF_MISTS;
  aff.level = level;
  aff.duration = caster->durationModify(SPELL_SHIELD_OF_MISTS,
    (3 + (aff.level / 2)) * Pulse::UPDATES_PER_MUDHOUR);
  aff.location = APPLY_ARMOR;
  aff.modifier = -60;
  aff.bitvector = 0;

  if (caster->bSuccess(bKnown, SPELL_SHIELD_OF_MISTS)) {
    switch (critSuccess(caster, SPELL_SHIELD_OF_MISTS)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_SHIELD_OF_MISTS);
        aff.duration *= 2;
        break;
      case CRIT_S_NONE:
        break;
    }

    // Second argument FALSE causes it to add new duration to old
    // Third argument TRUE causes it to average the old and newmodifier

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      caster->nothingHappens();
      return FALSE;
    }

    act("<G>$n <G>is enveloped by a thick green mist!<z>", FALSE, victim, NULL,
      NULL, TO_ROOM);
    act("<G>You are enveloped by a thick green mist!<z>", FALSE, victim, NULL,
      NULL, TO_CHAR);

    caster->reconcileHelp(victim, discArray[SPELL_SHIELD_OF_MISTS]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int resurrection(TBeing* caster, TObj* obj, int level, short bKnown) {
  affectedData aff;
  TThing* t;
  TMonster* victim;
  TBaseCorpse* corpse;

  if (!(corpse = dynamic_cast<TBaseCorpse*>(obj))) {
    caster->sendTo("You can't resurrect something that's not a corpse!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    caster->sendTo("You can't resurrect that - someone is sacrificing it!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  // note: we have this here to keep people from dissecting mobs (or
  // butchering), then resurrecting agian
  if (corpse->isCorpseFlag(CORPSE_NO_DISSECT) ||
      corpse->isCorpseFlag(CORPSE_NO_BUTCHER) ||
      corpse->isCorpseFlag(CORPSE_NO_SKIN)) {
    caster->sendTo("This corpse appears too mutilated to resurrect!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->getMoney() < 2500) {
    caster->sendTo(
      "You need 2500 talens to make the resurrection sacrifice.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  caster->addToMoney(-2500, GOLD_HOSPITAL);

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_RESURRECTION) &&
      (victim = read_mobile(corpse->getCorpseVnum(), VIRTUAL))) {
    // switch to warrior - this way charmies dont use their procs/caster AI
    if (victim->player.Class != CLASS_WARRIOR) {
      ubyte oldLevel = victim->GetMaxLevel();
      victim->fixLevels(0);
      victim->setClass(CLASS_WARRIOR);
      victim->fixLevels(oldLevel);
      delete victim->discs;
      victim->discs = NULL;
      victim->assignDisciplinesClass();
    }

    victim->setExp(0);
    victim->spec = 0;
    victim->elementalFix(caster, SPELL_RESURRECTION, 0);

    *caster->roomp += *victim;

    victim->setHit(1);
    victim->setPosition(POSITION_STUNNED);

    act("$N slowly rises from the $g.", FALSE, caster, 0, victim, TO_ROOM);
    caster->reconcileHelp(victim, discArray[SPELL_RESURRECTION]->alignMod);

    if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY, level)) {
      victim->setPosition(POSITION_STANDING);
      delete corpse;
      victim->doSay("Thank you soooooooo very much!");
      victim->doSay("How could I ever repay you for your deed?!");
      return SPELL_FALSE;
    } else if (caster->tooManyFollowers(victim, FOL_ZOMBIE)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      delete corpse;
      return SPELL_FALSE;
    }

    aff.type = SPELL_RESURRECTION;
    aff.duration = caster->followTime();
    aff.duration = (int)(caster->percModifier() * aff.duration);
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;

    if (critSuccess(caster, SPELL_RESURRECTION)) {
      CS(SPELL_RESURRECTION);
      aff.duration *= 2;
    }
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    caster->addFollower(victim);
    for (StuffIter it = corpse->stuff.begin(); it != corpse->stuff.end();) {
      t = *(it++);
      --(*t);
      *victim += *t;
    }
    act("With mystic power, $p is resurrected.", TRUE, caster, corpse, 0,
      TO_CHAR);
    act("With mystic power, $p is resurrected.", TRUE, caster, corpse, 0,
      TO_ROOM);
    delete corpse;
    return SPELL_SUCCESS;  // note, this indicates obj should go bye bye
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    delete corpse;
    return SPELL_FAIL;
  }
}

void resurrection(TBeing* caster, TObj* obj, TMagicItem* obj_mi) {
  int level;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);
  act("You direct a strange beam of energy at $p.", FALSE, caster, obj_mi, obj,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, obj_mi, obj,
    TO_ROOM);
  resurrection(caster, obj, level, bKnown);
}

int castResurrection(TBeing* caster, TObj* obj) {
  int ret, level;
  TBaseCorpse* corpse;

  if (!(corpse = dynamic_cast<TBaseCorpse*>(obj))) {
    return FALSE;
  }
  if (dynamic_cast<TPCorpse*>(corpse)) {
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
  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    caster->sendTo(
      "You can't resurrect that while someone is sacrificing it!\n\r");
    return SPELL_FAIL;
  }

  act("You direct a strange beam of energy at $p.", FALSE, caster, obj, 0,
    TO_CHAR);
  act("$n directs a strange beam of energy at $p.", FALSE, caster, obj, 0,
    TO_ROOM);

  level = caster->getSkillLevel(SPELL_RESURRECTION);
  int bKnown = caster->getSkillValue(SPELL_RESURRECTION);
  ret = resurrection(caster, corpse, level, bKnown);
  if (ret == SPELL_SUCCESS) {
    return DELETE_ITEM;  // delete corpse
  }
  return TRUE;
}

int resurrection(TBeing* caster, TObj* obj) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_RESURRECTION, obj))
    return FALSE;

  lag_t rounds = discArray[SPELL_RESURRECTION]->lag;
  diff = discArray[SPELL_RESURRECTION]->task;

  start_cast(caster, NULL, obj, caster->roomp, SPELL_RESURRECTION, diff, 1, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

// ENTHRALL DEMON

int enthrallDemon(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::THRALL_DEMON, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_DEMON, 0);

  if (caster->bSuccess(bKnown, SPELL_ENTHRALL_DEMON)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_DEMON;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_DEMON)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_DEMON);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_ENTHRALL_DEMON);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);
    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallDemon(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::THRALL_DEMON) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_DEMON, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_DEMON]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_DEMON]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_DEMON, diff, 1,
    "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castEnthrallDemon(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_ENTHRALL_DEMON);

  if ((ret = enthrallDemon(caster, level,
         caster->getSkillValue(SPELL_ENTHRALL_DEMON))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

// END ENTHRALL DEMON

// wood golem
int createWoodGolem(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::WOOD_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_WOOD_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_WOOD_GOLEM)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_WOOD_GOLEM;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_WOOD_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_WOOD_GOLEM);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_CREATE_WOOD_GOLEM);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);
    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createWoodGolem(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::WOOD_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_WOOD_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_WOOD_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_WOOD_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_WOOD_GOLEM, diff,
    1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castCreateWoodGolem(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_CREATE_WOOD_GOLEM);

  if ((ret = createWoodGolem(caster, level,
         caster->getSkillValue(SPELL_CREATE_WOOD_GOLEM))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

/////////////////
// rock golem
int createRockGolem(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::ROCK_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_ROCK_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_ROCK_GOLEM)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_ROCK_GOLEM;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_ROCK_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_ROCK_GOLEM);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_CREATE_ROCK_GOLEM);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);
    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }

    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createRockGolem(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::ROCK_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_ROCK_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_ROCK_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_ROCK_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_ROCK_GOLEM, diff,
    1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castCreateRockGolem(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_CREATE_ROCK_GOLEM);

  if ((ret = createRockGolem(caster, level,
         caster->getSkillValue(SPELL_CREATE_ROCK_GOLEM))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

//////////////////
// iron golem
int createIronGolem(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::IRON_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_IRON_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_IRON_GOLEM)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_IRON_GOLEM;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_IRON_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_IRON_GOLEM);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_CREATE_IRON_GOLEM);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);
    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createIronGolem(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::IRON_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_IRON_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_IRON_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_IRON_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_IRON_GOLEM, diff,
    1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castCreateIronGolem(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_CREATE_IRON_GOLEM);

  if ((ret = createIronGolem(caster, level,
         caster->getSkillValue(SPELL_CREATE_IRON_GOLEM))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}

//////////////////
// diamond golem
int createDiamondGolem(TBeing* caster, int level, short bKnown) {
  affectedData aff;
  TMonster* victim;

  if (!(victim = read_mobile(Mob::DIAMOND_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_DIAMOND_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_DIAMOND_GOLEM)) {
    act("You call upon the powers of your ancestors!", TRUE, caster, NULL, NULL,
      TO_CHAR);
    act("$n summons the powers of $s ancestors!", TRUE, caster, NULL, NULL,
      TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_DIAMOND_GOLEM;
    aff.level = level;
    aff.duration = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing*>((void*)mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this
    // skill this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_DIAMOND_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_DIAMOND_GOLEM);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim,
          TO_ROOM);
        caster->sendTo("The gods have blessed your wishes greatly!\n\r");
        victim->setMaxHit((int)(victim->hitLimit() * 1.5));
        victim->setHit((int)(victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL,
        victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL,
        victim, TO_ROOM);
      act("Your loa is displeased! $N hates you!", FALSE, caster, NULL, victim,
        TO_CHAR);
      victim->affectFrom(SPELL_CREATE_DIAMOND_GOLEM);
      victim->affectFrom(AFFECT_THRALL);
    } else
      caster->addFollower(victim);
    if (!caster->isPc()) {
      SET_BIT(caster->specials.affectedBy, AFF_GROUP);
      SET_BIT(victim->specials.affectedBy, AFF_GROUP);
    }
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *victim;
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE,
      caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim,
      TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createDiamondGolem(TBeing* caster) {
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo(
      "You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(Mob::DIAMOND_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_DIAMOND_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_DIAMOND_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_DIAMOND_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_DIAMOND_GOLEM,
    diff, 1, "", rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castCreateDiamondGolem(TBeing* caster) {
  int ret, level;

  if (!caster)
    return TRUE;

  level = caster->getSkillLevel(SPELL_CREATE_DIAMOND_GOLEM);

  if ((ret = createDiamondGolem(caster, level,
         caster->getSkillValue(SPELL_CREATE_DIAMOND_GOLEM))) == SPELL_SUCCESS) {
  } else {
    act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
      TO_CHAR);
  }
  return TRUE;
}
