#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman.h"
#include "spelltask.h"

// ENTHRALL GHOUL

int enthrallGhoul(TBeing * caster, int level, byte bKnown)
{
  affectedData aff;
  TMonster * victim;

  if (!(victim = read_mobile(THRALL_GHOUL, VIRTUAL))) {
    caster->sendTo("You cannot summon a being of that type.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_ENTHRALL_GHOUL, 0);

  if (bSuccess(caster, bKnown, SPELL_ENTHRALL_GHOUL)) {
     act("You call upon the powers of your ancestors!",
            TRUE, caster, NULL, NULL, TO_CHAR);
     act("$n summons the powers of $s ancestors!",
            TRUE, caster, NULL, NULL, TO_ROOM);

    // charm them for a while
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_ENTHRALL_GHOUL;
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

    switch (critSuccess(caster, SPELL_ENTHRALL_GHOUL)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_ENTHRALL_GHOUL);
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
      victim->affectFrom(SPELL_ENTHRALL_GHOUL);
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

int enthrallGhoul(TBeing * caster)
{
  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot dance the ritual under these wet conditions!\n\r");
    return FALSE;
  }

  if (real_mobile(THRALL_GHOUL) < 0) {
    caster->sendTo("You cannot call upon a being of that type.\n\r");
    return FALSE;
  }

  if (!bPassShamanChecks(caster, SPELL_ENTHRALL_GHOUL, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ENTHRALL_GHOUL]->lag;
  taskDiffT diff = discArray[SPELL_ENTHRALL_GHOUL]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ENTHRALL_GHOUL, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castEnthrallGhoul(TBeing * caster)
{
   int ret,level;


   if (!caster)
     return TRUE;

   level = caster->getSkillLevel(SPELL_ENTHRALL_GHOUL);

   if
((ret=enthrallGhoul(caster,level,caster->getSkillValue(SPELL_ENTHRALL_GHOUL)))
== SPELL_SUCCESS) {
   } else {
     act("You feel the ancestors are not pleased.", FALSE, caster, NULL, NULL,
TO_CHAR);
  }
  return TRUE;
}

// END ENTHRALL GHOUL
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

  if (bSuccess(caster, bKnown, SPELL_ENTHRALL_DEMON)) {
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
    caster->sendTo("You cannot dance the ritual under these wet
conditions!\n\r");
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
// CACAODEMON

int cacaodemon(TBeing * caster, const char * buffer, int level, byte bKnown)
{
  affectedData aff;
  int mob, obj, foundit;
  TObj *sac = NULL;
  TMonster *el;

const int DEMON_TYPE_I     = 10;
const int DEMON_TYPE_II    = 11;
const int DEMON_TYPE_III   = 12;
const int DEMON_TYPE_IV    = 13;
const int DEMON_TYPE_V     = 14;
const int DEMON_TYPE_VI    = 15;

const int OBJ_1  = 9963;  // sacrificial knife
const int OBJ_2  = 11311; // orcish blade
const int OBJ_3  = 6101;  // staff of the serpent 
const int OBJ_4  = 10703; // obsidian flail
const int OBJ_5  = 11105; // runed hammer
const int OBJ_6  = 10702; // runed obsidian sword 

  if (!strcasecmp(buffer, "one")) {
    mob = DEMON_TYPE_I;
    obj = OBJ_1;
  } else if (!strcasecmp(buffer, "two")) {
    mob = DEMON_TYPE_II;
    obj = OBJ_2;
  } else if (!strcasecmp(buffer, "three")) {
    mob = DEMON_TYPE_III;
    obj = OBJ_3;
  } else if (!strcasecmp(buffer, "four")) {
    mob = DEMON_TYPE_IV;
    obj = OBJ_4;
  } else if (!strcasecmp(buffer, "five")) {
    mob = DEMON_TYPE_V;
    obj = OBJ_5;
  } else if (!strcasecmp(buffer, "six")) {
    mob = DEMON_TYPE_VI;
    obj = OBJ_6;
  } else {
    caster->sendTo("There are no demons of that type available.\n\r");
    return SPELL_FAIL;
  }

  if ((!caster->equipment[HOLD_RIGHT]) && (!caster->equipment[HOLD_LEFT])) {
    caster->sendTo("You must be wielding the correct item.\n\r");
    return SPELL_FAIL;
  }

  foundit = FALSE;

  sac = dynamic_cast<TObj *>(caster->equipment[HOLD_RIGHT]);
  if (sac) {
    if (sac->objVnum() == obj) {
      caster->unequip(HOLD_RIGHT);
      foundit = TRUE;
    }
  }

  if (!foundit) {
    sac = dynamic_cast<TObj *>(caster->equipment[HOLD_LEFT]);
    if (sac) {
      if (sac->objVnum() == obj) {
        caster->unequip(HOLD_LEFT);
        foundit = TRUE;
      }
    }
  }

  if (!foundit) {
    caster->sendTo("You must be wielding the sacrifical item.\n\r");
    return SPELL_FAIL;
  }

  *caster += *sac;
  el = read_mobile(mob, VIRTUAL);

  if (!el) {
    caster->sendTo("There are no demons of that type available. None in the database.\n\r");
    return SPELL_FAIL;
  }

  *caster->roomp += *el;

  act("$p bursts into flame and burns to a pile of ashes!", TRUE, caster, sac, NULL, TO_ROOM);
  act("$p bursts into flame and burns to a pile of ashes!", TRUE, caster, sac, NULL, TO_CHAR);
  act("With an evil laugh, $N emerges from the smoke.", TRUE, caster, NULL, el, TO_NOTVICT);

  --(*sac);
  delete sac;
  sac = NULL;

  /* charm them for a while */
  if (el->master)
    el->stopFollower(TRUE);

  el->genericCharmFix();

  if (caster->tooManyFollowers(el, FOL_ZOMBIE)) {
    act("$N refuses to enter a group the size of yours!",
        TRUE, caster, NULL, el, TO_CHAR);
    act("$N refuses to enter a group the size of $n's!",
        TRUE, caster, NULL, el, TO_ROOM);
    return SPELL_FAIL;
  }

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_CACAODEMON)) {
    caster->addFollower(el);

    aff.type = SPELL_CACAODEMON;
    aff.level = level;
    aff.duration = caster->followTime(); 
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;

    aff.duration = (int) (caster->percModifier() * aff.duration);

    if (critSuccess(caster, SPELL_CACAODEMON)) {
      CS(SPELL_CACAODEMON);
      aff.duration *= 3;
      aff.duration /= 2;
    }

    el->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    el->affectTo(&aff);

    return SPELL_SUCCESS;
  } else {
    caster->setCharFighting(el);
    caster->setVictFighting(el);
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, el, TO_CHAR);
    return SPELL_FAIL;
  }
}

void cacaodemon(TBeing * caster, const char * buffer )
{
  int ret,level;

  if (!caster)
    return;

  if (!bPassClericChecks(caster,SPELL_CACAODEMON))
    return;

  level = caster->getSkillLevel(SPELL_CACAODEMON);
  int bKnown = caster->getSkillValue(SPELL_CACAODEMON);

  act("A cloud of black smoke billows forth from $n!", TRUE, caster, NULL, NULL, TO_ROOM);
  act("A cloud of black smoke billows forth!", TRUE, caster, NULL, NULL, TO_CHAR);

  if ((ret=cacaodemon(caster,buffer,level,bKnown)) == SPELL_SUCCESS) {

  } else {
  }
}

// END CACAODEMON
// CREATE GOLEM

int createGolem(TBeing * caster, int target, int power, int level, byte bKnown)
{
  TMonster *golem;
  affectedData aff;
  int rc;

  if (!(golem = read_mobile(target, VIRTUAL))) {
    vlogf(LOG_BUG, "Spell 'create golem' unable to load golem [bad!]...");
    caster->sendTo("Unable to create the golem.  Please report this.\n\r.");
    return SPELL_FAIL;
  }

  golem->setMaxHit((int) ((golem->hitLimit() + power) * caster->percModifier()));
  golem->setHit(golem->hitLimit());

  golem->genericCharmFix();

  *caster->roomp += *golem;
  act("$n arrives in a puff of blue smoke!", TRUE, golem, NULL, caster, TO_ROOM);

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_CREATE_GOLEM)) {
    if (golem->master)
      golem->stopFollower(TRUE);
    caster->addFollower(golem);

    aff.type = SPELL_CREATE_GOLEM;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime(); 
    aff.duration = (int) (caster->percModifier() * aff.duration);

    golem->affectTo(&aff);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    golem->affectTo(&aff);

    if (critSuccess(caster, SPELL_CREATE_GOLEM)) {
      CS(SPELL_CREATE_GOLEM);
      act("$N flexes $S overly strong muscles.", TRUE, caster, 0, golem, TO_ROOM);
      caster->sendTo("You have created an unusually strong golem!\n\r");
      golem->setHit((int) (golem->hitLimit() * 1.5));
      golem->setMaxHit((int) (golem->hitLimit() * 1.5));
    }

    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_CREATE_GOLEM)) {
      CF(SPELL_CREATE_GOLEM);
      act("$n loses control of the magic $e has unleashed!", TRUE, caster, 0, golem, TO_ROOM);
      act("You lose control of the magic you have unleashed!", TRUE, caster, 0, golem, TO_CHAR);
      if ((rc = golem->hit(caster)) == DELETE_VICT) {
        return SPELL_CRIT_FAIL + CASTER_DEAD;
      } else if (rc == DELETE_THIS) {
        delete golem;
        golem = NULL;
      }
      return SPELL_CRIT_FAIL;
    } else {
      caster->sendTo("You don't seem to have control of the golem.\n\r");
      act("The golem seems to stare off into space!", TRUE, caster, 0, golem, TO_ROOM);
      return SPELL_FAIL;
    }
  }
}

int createGolem(TBeing * caster)
{
  int target = 0;
  int ret,level;
  TThing *comp;
  int adjBKnown;      // this is used to make diffrent golems easier to make
  int rc = 0;

  comp = NULL;
  int bKnown = caster->getSkillValue(SPELL_CREATE_GOLEM);

  if (!bPassClericChecks(caster,SPELL_CREATE_GOLEM))
    return FALSE;

  if ((comp = get_thing_char_using(caster, NULL, WOOD_COMPONENT, 0, 1))) {
    target = WOOD_GOLEM;
    adjBKnown = min((int) MAX_SKILL_LEARNEDNESS, bKnown* 4);
  } else if ((comp = get_thing_char_using(caster, NULL,  ROCK_COMPONENT, 0, 1))) {  
    target = ROCK_GOLEM;
    adjBKnown = min((int) MAX_SKILL_LEARNEDNESS, bKnown* 3);
  } else if ((comp = get_thing_char_using(caster, NULL, IRON_COMPONENT, 0, 1))) {  
    target = IRON_GOLEM;
    adjBKnown = min((int) MAX_SKILL_LEARNEDNESS, bKnown* 2);
  } else if ((comp = get_thing_char_using(caster, NULL, DIAMOND_COMPONENT, 0, 1))) {
    target = DIAMOND_GOLEM;
    adjBKnown = min((int) MAX_SKILL_LEARNEDNESS, bKnown * 1);
  } else {
    // comp == NULL
    caster->sendTo("You have nothing to create the golem with!\n\r");
    act("$n looks around stupidly, as if trying to find something.", TRUE, caster, 0, NULL, TO_ROOM);
    return FALSE;
  }

  // add some text about how component is used here
  act("You throw the $p up into the air and it begins to transform!", TRUE, caster, comp, NULL, TO_CHAR);
  act("$n throws the $p up into the air and it begins to transform!", TRUE, caster, comp, NULL, TO_ROOM);

  if (comp->parent)
    --(*comp);
  else if (comp->eq_pos != WEAR_NOWHERE)
    // this assumes comp is equipped by caster, a safe bet here
    caster->unequip(comp->eq_pos);
  else
    --(*comp);

  delete comp;
  comp = NULL;

  level = caster->getSkillLevel(SPELL_CREATE_GOLEM);

  ret=createGolem(caster,target,0,level,adjBKnown);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

// END CREATE GOLEM

