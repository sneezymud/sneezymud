//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_shaman.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_shaman.h"

int createGolem(TBeing * caster, int target, int power, int level, byte bKnown)
{
  TMonster *golem;
  affectedData aff;
  int rc;

  if (!(golem = read_mobile(target, VIRTUAL))) {
    vlogf(10, "Spell 'create golem' unable to load golem [bad!]...");
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

    aff.type = SPELL_ENSORCER;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime(); 
    aff.duration = (int) (caster->percModifier() * aff.duration);

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

int controlUndead(TBeing *caster,TBeing *victim,int level,byte bKnown)
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

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_CONTROL_UNDEAD)) {
    if (victim->master)
      victim->stopFollower(TRUE);
    caster->addFollower(victim);

    aff.type = SPELL_ENSORCER;
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

void controlUndead(TBeing * caster, TBeing * victim, TMagicItem *obj)
{
  int ret;

  if (caster != victim) {
    act("$p attempts to bend $N to your will.",
          FALSE, caster, obj, victim, TO_CHAR);
    act("$p attempts to bend you to $n's will.",
          FALSE, caster, obj, victim, TO_VICT);
    act("$p attempts to bend $N to $n's will.",
          FALSE, caster, obj, victim, TO_NOTVICT);
  } else {
    act("$p tries to get you to control yourself.",
          FALSE, caster, obj, 0, TO_CHAR);
    act("$p tries to get $n to control $mself.",
          FALSE, caster, obj, 0, TO_ROOM);
  }
 
  ret=controlUndead(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
 
  return;
}

void controlUndead(TBeing * caster, TBeing * victim)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_CONTROL_UNDEAD))
    return;

  level = caster->getSkillLevel(SPELL_CONTROL_UNDEAD);
  int bKnown = caster->getSkillValue(SPELL_CONTROL_UNDEAD);

  ret=controlUndead(caster,victim,level,bKnown);
}

// returns VICTIM_DEAD if corpse should be fried
int voodoo(TBeing * caster, TObj * obj, int level, byte bKnown)
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
    return SPELL_FAIL;
  }
 
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int) (3 * level / 5)) {
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
    vlogf(9, "FAILED Load!!  No mob (%d)", corpse->getCorpseVnum());
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

    aff.type = SPELL_ENSORCER;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int) (caster->percModifier() * aff.duration);

    aff.duration *= 2;   // zombie adjustment

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

int voodoo(TBeing * caster, TObj * corpse, TMagicItem *obj)
{
  int ret;

  act("$p directs ia strange beam of energy at $N.",
          FALSE, caster, obj, corpse, TO_CHAR);
  act("$p directs ia strange beam of energy at $N.",
          FALSE, caster, obj, corpse, TO_ROOM);
 
  ret=voodoo(caster,corpse,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;
 
  return FALSE;
}

int voodoo(TBeing * caster, TObj * corpse)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_VOODOO))
    return FALSE;

  level = caster->getSkillLevel(SPELL_VOODOO);
  int bKnown = caster->getSkillValue(SPELL_VOODOO);

  ret=voodoo(caster,corpse,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;   // nuke the corpse
  return FALSE;
}

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

    aff.type = SPELL_ENSORCER;
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
    } else {
      aff.type      = SPELL_ENSORCER;
      aff.duration = caster->followTime(); 
      aff.duration = (int) (caster->percModifier() * aff.duration);
      aff.modifier = 0;
      aff.location = APPLY_NONE;
      aff.bitvector = AFF_CHARM;
    }

    if (critSuccess(caster, SPELL_RESURRECTION)) {
      CS(SPELL_RESURRECTION);
      aff.duration *= 2;
    }
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

int resurrection(TBeing * caster, TObj * obj, TMagicItem *obj_mi)
{
  int ret;
  TBaseCorpse *corpse;

  if (!(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    return FALSE;
  }

  if (dynamic_cast<TPCorpse *>(corpse)) {
     /* corpse is a pc */
    caster->sendTo("Resurrection of players is not currently supported.\n\r");
    caster->sendTo("Bug Brutius is you want to see resurrection of players.\n\r");
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

  act("$p directs a strange beam of energy at $N.",
          FALSE, caster, obj_mi, corpse, TO_CHAR);
  act("$p directs a strange beam of energy at $N.",
          FALSE, caster, obj_mi, corpse, TO_ROOM);

  ret=resurrection(caster,corpse,obj_mi->getMagicLevel(),obj_mi->getMagicLearnedness());
  if (ret == SPELL_SUCCESS) {
    return DELETE_ITEM;  // delete corpse
  }
  return FALSE;
}

int resurrection(TBeing * caster, TObj * obj)
{
  int ret,level;
  TBaseCorpse *corpse;
    
  if (!obj || !(corpse = dynamic_cast<TBaseCorpse *>(obj))) 
    return FALSE;
  
  if (!bPassClericChecks(caster,SPELL_RESURRECTION))
    return FALSE;

  if (dynamic_cast<TPCorpse *>(corpse)) {
     /* corpse is a npc */
    caster->sendTo("Resurrection of players is not currently supported.\n\r");
    caster->sendTo("Bug Brutius is you want to see resurrection of players.\n\r");
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

  level = caster->getSkillLevel(SPELL_RESURRECTION);
  int bKnown = caster->getSkillValue(SPELL_RESURRECTION);

  ret=resurrection(caster,corpse,level,bKnown);
  if (ret == SPELL_SUCCESS) {
    return DELETE_ITEM;  // delete corpse
  }
  return FALSE;
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
    return SPELL_FAIL;
  }
 
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a corpse that can't be res'd  (body-part or something)
    caster->sendTo("You can't do that.  Nothing happens.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }
  if (corpse->getCorpseLevel() > (unsigned int) (3 * level / 5)) {
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
    vlogf(9, "FAILED Load!!  No mob (%d)", corpse->getCorpseVnum());
    caster->sendTo("Something screwed up.  Tell a god.\n\r");
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
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

  if (bSuccess(caster, bKnown, caster->getPerc(), SPELL_DANCING_BONES)) {
    affectedData aff;

    SET_BIT(mob->specials.affectedBy, AFF_CHARM );
    mob->setPosition(POSITION_STUNNED);    // make it take a little to wake up
    caster->addFollower(mob);
    act("$N slowly begins to move...it's slowly standing up!",
             FALSE, caster, NULL, mob, TO_CHAR);
    act("$N slowly begins to move...it's slowly standing up!", 
             FALSE, caster, NULL, mob, TO_ROOM);

    aff.type = SPELL_ENSORCER;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = AFF_CHARM;
    aff.duration = caster->followTime();
    aff.duration = (int) (caster->percModifier() * aff.duration);

    aff.duration *= 3;   // skeleton adjustment

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

int dancingBones(TBeing * caster, TObj * corpse, TMagicItem *obj)
{
  int ret;

  act("$p directs ia strange beam of energy at $N.",
          FALSE, caster, obj, corpse, TO_CHAR);
  act("$p directs ia strange beam of energy at $N.",
          FALSE, caster, obj, corpse, TO_ROOM);
 
  ret=dancingBones(caster,corpse,obj->getMagicLevel(),obj->getMagicLearnedness());
 
  // nuke corpse
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;

  return FALSE;
}

int dancingBones(TBeing * caster, TObj * corpse)
{
  int ret,level;

  if (!bPassClericChecks(caster,SPELL_DANCING_BONES))
    return FALSE;

  level = caster->getSkillLevel(SPELL_DANCING_BONES);
  int bKnown = caster->getSkillLevel(SPELL_DANCING_BONES);

  ret=dancingBones(caster,corpse,level,bKnown);
  if (IS_SET(ret, VICTIM_DEAD))
    return DELETE_ITEM;   // nuke the corpse
  return FALSE;
}
