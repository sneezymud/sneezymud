//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_animal.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disease.h"
#include "combat.h"
#include "disc_animal.h"

int beastSoother(TBeing * caster, TBeing * victim, int, byte bKnown)
{
  int rc;

  if (!caster->useComponent(caster->findComponent(SKILL_BEAST_SOOTHER), victim, CHECK_ONLY_NO))
    return FALSE;

  if (!victim->awake()) {
    act("$N looks sound asleep.  Oops!", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (victim->fight() || !victim->isDumbAnimal()) {
    act("$N seems indifferent.", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (bSuccess(caster, bKnown, caster->getPerc(), SKILL_BEAST_SOOTHER)) {
    if (IS_SET(victim->specials.act, ACT_HUNTING))
      REMOVE_BIT(victim->specials.act, ACT_HUNTING);

    TMonster *tmons = dynamic_cast<TMonster *>(victim);
    if (!tmons->isPc() && IS_SET(tmons->specials.act, ACT_AGGRESSIVE)) {
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

    if (critSuccess(caster, SKILL_BEAST_SOOTHER)) {
      CS(SKILL_BEAST_SOOTHER);
      if (IS_SET(tmons->specials.act, ACT_HATEFUL))
	REMOVE_BIT(tmons->specials.act, ACT_HATEFUL);

      act("$N seems to relax quite a bit.", FALSE, caster, NULL, tmons, TO_NOTVICT);
      act("$N seems to relax quite a bit.  Good job!", FALSE, caster, NULL, tmons, TO_CHAR);
      act("You feel very relaxed, and much less hostile.", FALSE, caster, NULL, tmons, TO_VICT);
      return SPELL_CRIT_SUCCESS;
    } else {
      act("$N seems to be more relaxed.", FALSE, caster, NULL, tmons, TO_NOTVICT);
      act("$N seems to relax a bit.", FALSE, caster, NULL, tmons, TO_CHAR);
      act("You feel relaxed, and less hostile.", FALSE, caster, NULL, tmons, TO_VICT);
      return SPELL_SUCCESS;
    }
  } else {
    if (critFail(caster, SKILL_BEAST_SOOTHER)) {
      CF(SKILL_BEAST_SOOTHER);
      act("$N turns towards $n, infuriated!", FALSE, caster, NULL, victim, TO_NOTVICT);
      act("$N turns towards you, infuriated!", FALSE, caster, NULL, victim, TO_CHAR);
      act("For some reason, you feel a deep hostility towards $n.", FALSE, caster, NULL, victim, TO_VICT);
      if (!caster->checkPeaceful("") && !victim->isPc()) {
        if ((rc = victim->hit(caster)) == DELETE_VICT) 
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        else if (rc == DELETE_THIS) 
          return SPELL_CRIT_FAIL + VICTIM_DEAD;
        
        return SPELL_CRIT_FAIL;
      }
    } else {
      caster->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_FAIL;
    }
  }
  return SPELL_FAIL;
}

int TBeing::doSoothBeast(const char *argument)
{
  int level, ret = 0;
  int rc = 0;
  TBeing *victim = NULL;
  char namebuf[256];

  if (!doesKnowSkill(SKILL_BEAST_SOOTHER)) {
    sendTo("You know nothing about soothing beasts.");
    return FALSE;
  }

  only_argument(argument, namebuf);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Sooth what?\n\r");
    return FALSE;
  }

  if (!sameRoom(victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  level = getSkillLevel(SKILL_BEAST_SOOTHER);

  ret=beastSoother(this,victim,level,getSkillValue(SKILL_BEAST_SOOTHER));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}


int beastSoother(TBeing * caster, TBeing * victim)
{
  int level, ret;
  int rc = 0;

  level = caster->getSkillLevel(SKILL_BEAST_SOOTHER);

  ret=beastSoother(caster,victim,level,caster->getSkillValue(SKILL_BEAST_SOOTHER));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int TBeing::doBefriendBeast(const char *argument)
{
  if (!doesKnowSkill(SKILL_BEFRIEND_BEAST)) {
    sendTo("You know nothing about befriending beasts.");
    return FALSE;
  }
  sendTo("This skill is not implemented yet.");
  return FALSE;
#if 0
  TBeing *victim = NULL;
  char namebuf[256];

  only_argument(argument, namebuf);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Befriend what?\n\r");
    return FALSE;
  }

  if (!sameRoom(victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  int level = getSkillLevel(SKILL_BEFRIEND_BEAST);
  int ret=befriendBeast(this,victim,level,caster->getSkillValue(SKILL_BEAST_SOOTHER));
  int rc = 0;
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
#endif
}

int TBeing::doCharmBeast(const char *argument)
{
  if (!doesKnowSkill(SKILL_BEAST_CHARM)) {
    sendTo("You know nothing about charming beasts.");
    return FALSE;
  }
  sendTo("This skill is not implemented yet.");
  return FALSE;
#if 0
  int level, ret = 0;
  int rc = 0;
  TBeing *victim = NULL;
  char namebuf[256];

  only_argument(argument, namebuf);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Charm what?\n\r");
    return FALSE;
  }

  if (!sameRoom(victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  level = getSkillLevel(SKILL_BEAST_CHARM);

  ret=beastCharm(this,victim,level,caster->getSkillValue(SKILL_BEAST_SOOTHER));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
#endif
}


int transfix(TBeing * caster, TBeing * victim, int level, byte bKnown)
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

  if (bSuccess(caster, bKnown, caster->getPerc(), SKILL_TRANSFIX) && 
           !victim->isLucky(caster->spellLuckModifier(SKILL_TRANSFIX))) {
    aff.type = SKILL_TRANSFIX;
    aff.level = level;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.duration = (level/10)* UPDATES_PER_TICK;
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

  only_argument(argument, namebuf);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Transfix what?\n\r");
    return FALSE;
  }

  if (!sameRoom(victim)) {
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

int beastSummon(TBeing * caster, const char * arg, int level, byte bKnown)
{
  int i;
  int max_dist, num_sum = 0, max_num;
  char targ_name[1024];
  TBeing *v;

  if (!caster || !arg)
    return FALSE;
#if 0
  if (!bPassMageChecks(caster,SKILL_BEAST_SUMMON, NULL))
    return FALSE;
#else
  if (!caster->useComponent(caster->findComponent(SKILL_BEAST_SUMMON), NULL, CHECK_ONLY_NO))
    return FALSE;
#endif

  if (sscanf(arg, "%s", targ_name) != 1) {
    caster->sendTo("It suddenly strikes you that you might want to summon something specific.\n\r");
    return FALSE;
  }

  if (bSuccess(caster, bKnown, caster->getPerc(), SKILL_BEAST_SUMMON)) {
    max_dist = caster->isImmortal() ? 1000 : level;
    max_num = caster->isImmortal() ? 100 : level / 2;
  
i=0;
    for (v = character_list; v; v = v->next) {
      if (!v->isPc() && v->awake() && isname(targ_name, v->name) &&
	  !v->fight() && (caster->isImmortal() || v->isDumbAnimal()) &&
	  !IS_SET(v->specials.act, ACT_HUNTING) && (v->in_room != -1) &&
	  (caster->isImmortal() || (caster->roomp->getZone() == v->roomp->getZone()))) {
        if (!v->isLucky(caster->spellLuckModifier(SKILL_BEAST_SUMMON))) {
          TMonster *tmons = dynamic_cast<TMonster *>(v);
	  SET_BIT(tmons->specials.act, ACT_HUNTING);
	  tmons->specials.hunting = caster;
	  tmons->hunt_dist = max_dist;
	  tmons->persist = max_dist + 5;
          tmons->oldRoom = v->in_room;
        }
        if (++num_sum > max_num)
	  break;
      }
      i++;
    }

    if (!num_sum) {
      caster->sendTo("You can't seem to make contact with a creature of that type.\n\r");
      act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    } else if (num_sum == 1)
      caster->sendTo("You succeed in summoning only a single creature.\n\r");
    else
      caster->sendTo("You succeed in summoning many creatures.\n\r");
    return SPELL_SUCCESS;
  } else {
    caster->sendTo("Nothing seems to happen.\n\r");
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }
}

int TBeing::doSummonBeast(const char *argument)
{
  int level, ret = 0;
  int rc = 0;

  if (!doesKnowSkill(SKILL_BEAST_SUMMON)) {
    sendTo("You know nothing about summoning beasts.");
    return FALSE;
  }

  level = getSkillLevel(SKILL_BEAST_SUMMON);

  ret=beastSummon(this,argument,level,getSkillValue(SKILL_BEAST_SUMMON));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int beastSummon(TBeing * caster, const char * arg)
{
  int level, ret;
  int rc = 0;

  level = caster->getSkillLevel(SKILL_BEAST_SUMMON);

  ret=beastSummon(caster,arg,level,caster->getSkillValue(SKILL_BEAST_SUMMON));
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

static const int LAST_SHAPESHIFT_MOB = 12;
struct PolyType shapeShiftList[LAST_SHAPESHIFT_MOB] =
{
  {"gopher", 30, 1, 25401, DISC_ANIMAL},
  {"deer", 31, 1, 14105, DISC_ANIMAL},
  {"wolf", 35, 10, 3400, DISC_ANIMAL},
  {"snake", 37, 20, 3412, DISC_ANIMAL},
  {"moose", 39, 30,10200, DISC_ANIMAL},
  {"dolphin", 40, 45, 12432, DISC_ANIMAL},
  {"bear", 42, 75, 3403, DISC_ANIMAL},
  {"crow", 44, 70, 14350, DISC_ANIMAL},
  {"shark", 40, 60,  12413, DISC_ANIMAL},
  {"hawk", 48, 100, 14440, DISC_ANIMAL},
  {"saberfish", 49, 80, 5503, DISC_ANIMAL},
  {"spider", 49, 80, 7717, DISC_ANIMAL},
};

int shapeShift(TBeing *caster, const char * buffer, int level, byte bKnown)
{
  int i, ret, duration = 0, nameFound = FALSE, found = FALSE;
  TBeing *mob;
  affectedData aff;
  affectedData aff2;

  if (!caster->isImmortal() && caster->checkForSkillAttempt(SPELL_SHAPESHIFT)) {
    act("You are not prepared to try to shapeshift yourself again so soon.",
         FALSE, caster, NULL, NULL, TO_CHAR);
    return FALSE;
  }
  
  discNumT das = getDisciplineNumber(SPELL_SHAPESHIFT, FALSE);
  if (das == DISC_NONE) {
    vlogf(5, "bad discipline for shapeshift");
    return SPELL_FAIL;
  }
  for (i = 0; (i < LAST_SHAPESHIFT_MOB); i++) {
    if (is_abbrev(buffer,shapeShiftList[i].name)) {
      nameFound = TRUE; 
    } else {
      continue;
    }
    if ((caster->getDiscipline(das)->getLearnedness() >= shapeShiftList[i].learning) && (level > 25))
      break;
  }

  if (i >= LAST_SHAPESHIFT_MOB) {
    if (nameFound) {
    caster->sendTo("You are not powerful enough yet to change into such a creature.\n\r");
    } else {
    caster->sendTo("Couldn't find any of those.\n\r");
    }
    return SPELL_FAIL;
  }
  if (!(mob = read_mobile(shapeShiftList[i].number, VIRTUAL))) {
    caster->sendTo("You couldn't summon an image of that creature.\n\r");
    return SPELL_FAIL;
  }
  thing_to_room(mob,ROOM_VOID);   // just so if extracted it isn't in NOWHERE 

  // Check to make sure that there is no snooping going on. 
  if (!caster->desc || caster->desc->snoop.snooping) {
    caster->sendTo("Nothing seems to happen.\n\r");
    vlogf(5,"PC tried to shapeshift while being snooped");
    delete mob;
    mob = NULL;
    return SPELL_FAIL;
  }
  if (caster->desc->original) {
    // implies they are switched, while already switched (as x switch)
    caster->sendTo("You already seem to be switched.\n\r");
    delete mob;
    mob = NULL;
    return SPELL_FAIL;
  }
  if (caster->desc->snoop.snoop_by)
    caster->desc->snoop.snoop_by->doSnoop(caster->desc->snoop.snoop_by->name);
  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.location = APPLY_NONE;
  aff.duration = (2 + (level/5)) * UPDATES_PER_TICK;
  aff.bitvector = 0;
  aff.modifier = SPELL_SHAPESHIFT;

  if (bSuccess(caster, bKnown, SPELL_SHAPESHIFT)) {
    switch (critSuccess(caster, SPELL_SHAPESHIFT)) {
      case CRIT_S_KILL:
      case CRIT_S_DOUBLE:
        CS(SPELL_SHAPESHIFT);
        ret = SPELL_CRIT_SUCCESS;
      default:
        break;
   }

    --(*mob);
    *caster->roomp += *mob;
    SwitchStuff(caster, mob);

    act("$n's flesh melts and flows into the shape of $N.", TRUE, caster, NULL, mob, TO_ROOM);
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
      act("Slowly you take on the shape of $N.", TRUE, caster, NULL, mob, TO_CHAR);
    }
  
    --(*caster);
    thing_to_room(caster, ROOM_POLY_STORAGE);

    // stop following whoever you are following.. 
    if (caster->master)
      caster->stopFollower(TRUE);

    // switch caster into mobile 
    caster->desc->character = mob;
    caster->desc->original = dynamic_cast<TPerson *>(caster);

    mob->desc = caster->desc;
    caster->desc = NULL;
    caster->polyed = POLY_TYPE_SHAPESHIFT;

// first add the attempt -- used to regulate attempts
    aff.duration = duration + ((2 + (level/5)) * UPDATES_PER_TICK);
    caster->affectTo(&aff);

    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.location = APPLY_NONE;
    aff2.duration = duration + ((2 + (level/5)) * UPDATES_PER_TICK);
    aff2.bitvector = 0;
    aff2.modifier = SPELL_SHAPESHIFT;
    mob->affectTo(&aff2);

    SET_BIT(mob->specials.act, ACT_POLYSELF);
    SET_BIT(mob->specials.act, ACT_NICE_THIEF);
    SET_BIT(mob->specials.act, ACT_SENTINEL);
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
    REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
    REMOVE_BIT(mob->specials.act, ACT_NOCTURNAL);

    mob->setMana(min((mob->getMana() - 15), 85));
    return SPELL_SUCCESS;
  } else {
    caster->affectTo(&aff);
    return SPELL_FAIL;
  }
}

void shapeShift(TBeing *caster, const char * buffer)
{
  int ret,level;

  if (!bPassMageChecks(caster, SPELL_SHAPESHIFT, NULL))
    return;

  level = caster->getSkillLevel(SPELL_SHAPESHIFT);
  int bKnown = caster->getSkillValue(SPELL_SHAPESHIFT);

  if ((ret=shapeShift(caster,buffer,level,bKnown)) == SPELL_SUCCESS) {
  } else 
    caster->sendTo("Nothing seems to happen.\n\r");
}
