////////////////////////////////////////////////////////////////////
// 
//  disc_shaman_control
//
///////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman.h"
#include "spelltask.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_magic_item.h"
#include "combat.h"

int resurrection(TBeing * caster, TObj * obj, int level, byte bKnown)
{
  affectedData aff;
  TThing *t, *n;
  TMonster * victim;
  TBaseCorpse *corpse;
  float shamLvl = caster->getLevel(SHAMAN_LEVEL_IND);

  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    caster->sendTo("You can't resurrect something that's not a corpse!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    caster->sendTo("You can't resurrect that - someone is sacrificing it!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  // note: we have this here to keep people from dissecting mobs (or butchering), then resurrecting agian
  if (corpse->isCorpseFlag(CORPSE_NO_DISSECT) || corpse->isCorpseFlag(CORPSE_NO_BUTCHER) ||
      corpse->isCorpseFlag(CORPSE_NO_SKIN)) {
    caster->sendTo("This corpse appears too mutilated to resurrect!\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->getMoney() < 2500) {
    caster->sendTo("You need 2500 talens to make the resurrection sacrifice.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  caster->addToMoney(-2500, GOLD_HOSPITAL);

  if (shamLvl < 1)
    shamLvl = caster->getLevel(caster->bestClass()) / 2;

  if (caster->bSuccess(bKnown, caster->getPerc(), SPELL_RESURRECTION) &&
    (victim = read_mobile(corpse->getCorpseVnum(), VIRTUAL))) {

    // switch to warrior - this way charmies dont use their procs/caster AI
    if (victim->player.Class != CLASS_WARRIOR)
    {
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
    caster->reconcileHelp(victim,discArray[SPELL_RESURRECTION]->alignMod);
      
    if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY, level)) {
      victim->setPosition(POSITION_STANDING);
      delete corpse;
      victim->doSay("Thank you soooooooo very much!");
      victim->doSay("How could I ever repay you for your deed?!");
      return SPELL_FALSE;
    } else if (caster->tooManyFollowers(victim, FOL_ZOMBIE)) {
      act("$N refuses to enter a group the size of yours!", TRUE, caster, NULL, victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!", TRUE, caster, NULL, victim, TO_ROOM);
      delete corpse;
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

    // Add the restrict XP affect, so that you cannot twink newbies with this skill
    // this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    caster->addFollower(victim);
    for (t = corpse->getStuff(); t; t = n) {
      n = t->nextThing;
      --(*t);
      *victim += *t;
    }
    act("With mystic power, $p is resurrected.", 
            TRUE, caster, corpse, 0, TO_CHAR);
    act("With mystic power, $p is resurrected.", 
            TRUE, caster, corpse, 0, TO_ROOM);
    delete corpse;
    return SPELL_SUCCESS;  // note, this indicates obj should go bye bye
  } else {
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    delete corpse;
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
  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    caster->sendTo("You can't resurrect that while someone is sacrificing it!\n\r");
    return SPELL_FAIL;
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

// ENTHRALL DEMON

int enthrallDemon(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(THRALL_DEMON, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_DEMON, 0);

  if (caster->bSuccess(bKnown, SPELL_ENTHRALL_DEMON)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_DEMON;
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

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_ENTHRALL_DEMON)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_DEMON);
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
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL,
victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int enthrallDemon(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(THRALL_DEMON) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_DEMON, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_DEMON]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_DEMON]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_DEMON, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castEnthrallDemon(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_ENTHRALL_DEMON);

   if
((ret=enthrallDemon(caster,level,caster->getSkillValue(SPELL_ENTHRALL_DEMON)))
== SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
TO_CHAR);
  }
  return TRUE;
}

// END ENTHRALL DEMON

// wood golem
int createWoodGolem(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(WOOD_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_WOOD_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_WOOD_GOLEM)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_WOOD_GOLEM;
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

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_WOOD_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_WOOD_GOLEM);
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
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createWoodGolem(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(WOOD_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_WOOD_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_WOOD_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_WOOD_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_WOOD_GOLEM, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castCreateWoodGolem(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_CREATE_WOOD_GOLEM);

   if ((ret=createWoodGolem(caster,level,caster->getSkillValue(SPELL_CREATE_WOOD_GOLEM))) == SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

/////////////////
// rock golem
int createRockGolem(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(ROCK_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_ROCK_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_ROCK_GOLEM)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_ROCK_GOLEM;
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

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_ROCK_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_ROCK_GOLEM);
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
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createRockGolem(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(ROCK_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_ROCK_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_ROCK_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_ROCK_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_ROCK_GOLEM, diff, 1, "", rounds, 
caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castCreateRockGolem(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_CREATE_ROCK_GOLEM);

   if ((ret=createRockGolem(caster,level,caster->getSkillValue(SPELL_CREATE_ROCK_GOLEM))) == 
SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

//////////////////
// iron golem
int createIronGolem(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(IRON_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_IRON_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_IRON_GOLEM)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_IRON_GOLEM;
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

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_IRON_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_IRON_GOLEM);
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
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createIronGolem(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(IRON_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_IRON_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_IRON_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_IRON_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_IRON_GOLEM, diff, 1, "", rounds, 
caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castCreateIronGolem(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_CREATE_IRON_GOLEM);

   if ((ret=createIronGolem(caster,level,caster->getSkillValue(SPELL_CREATE_IRON_GOLEM))) == 
SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}

//////////////////
// diamond golem
int createDiamondGolem(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(DIAMOND_GOLEM, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CREATE_DIAMOND_GOLEM, 0);

  if (caster->bSuccess(bKnown, SPELL_CREATE_DIAMOND_GOLEM)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CREATE_DIAMOND_GOLEM;
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

    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CREATE_DIAMOND_GOLEM)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CREATE_DIAMOND_GOLEM);
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
    act("<R>In a flash of red light you see $N standing before you!<1>", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("The gods are not pleased! $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    return SPELL_FAIL;
  }
}

int createDiamondGolem(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(DIAMOND_GOLEM) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_CREATE_DIAMOND_GOLEM, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_CREATE_DIAMOND_GOLEM]->lag;
  taskDiffT diff = discArray[SPELL_CREATE_DIAMOND_GOLEM]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CREATE_DIAMOND_GOLEM, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castCreateDiamondGolem(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_CREATE_DIAMOND_GOLEM);

   if ((ret=createDiamondGolem(caster,level,caster->getSkillValue(SPELL_CREATE_DIAMOND_GOLEM))) 
== SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL, TO_CHAR);
  }
  return TRUE;
}


