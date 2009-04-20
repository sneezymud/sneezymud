#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "disc_animal.h"
#include "obj_magic_item.h"

int beastSoother(TBeing *caster, TBeing *victim, TMagicItem *tObj)
{
  int tKnown = tObj->getMagicLearnedness(),
      tReturn = 0;

  tReturn = beastSoother(caster, victim, 1, tKnown);

  if (IS_SET(tReturn, CASTER_DEAD))
    ADD_DELETE(tReturn, DELETE_THIS);

  return tReturn;
}


int beastSoother(TBeing * caster, TBeing * victim, int tWand, short bKnown)
{
  int rc;

  if (!tWand)
    if (!caster->useComponent(caster->findComponent(SKILL_BEAST_SOOTHER), victim, CHECK_ONLY_NO))
      return FALSE;

  TMonster *tmons = dynamic_cast<TMonster *>(victim);

  if (victim->fight() || !victim->isDumbAnimal() || !tmons || tmons->isPc()) {
    act("$N seems indifferent.", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (!victim->awake()) {
    act("$N looks sound asleep.  Oops!", TRUE, caster, NULL, victim, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, NULL, NULL, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, caster->getPerc(), SKILL_BEAST_SOOTHER)) {
    if (IS_SET(victim->specials.act, ACT_HUNTING))
      REMOVE_BIT(victim->specials.act, ACT_HUNTING);

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

  strcpy(namebuf, argument);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Sooth what?\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  level = getSkillLevel(SKILL_BEAST_SOOTHER);

  ret=beastSoother(this,victim,0,getSkillValue(SKILL_BEAST_SOOTHER));
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

  ret=beastSoother(caster,victim,0,caster->getSkillValue(SKILL_BEAST_SOOTHER));
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

  strcpy(namebuf, argument);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Befriend what?\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
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

int TBeing::doSkySpirit(const char *argument)
{
  if (!doesKnowSkill(SPELL_SKY_SPIRIT)) {
    sendTo("You do not know how to summon the spirits of the sky.\n\r");
    return FALSE;
  }

  if (this->roomp->isIndoorSector()) {
    sendTo("You cannot summon the spirits of the sky unless you are outdoors!\n\r");
    return FALSE;
  }

  char    tTarget[256];
  TObj   *tObj    = NULL;
  TBeing *victim = NULL;
  sstring spirit;
  
  if (checkBusy())
    return FALSE;


  if (getMana() < 0) {
    sendTo("You lack mana to summon the spirits of the sky.\n\r");
    return FALSE;
  }

  if (argument && *argument) {
    strcpy(tTarget, argument);
    generic_find(tTarget, FIND_CHAR_ROOM, this, &victim, &tObj);
  } else {
    if (!fight()) {
      sendTo("Upon whom do you wish to unleash the spirit of the sky?\n\r");
      return FALSE;
    } else
      victim = fight();
  }

  if (victim == NULL) {
    sendTo("There is no one by that name here.\n\r");
    return FALSE;
  } else if (victim == this) {
    this->sendTo("Do you really want to summon the spirits of the sky to attack you?\n\r");
    return FALSE;
  }

  if (noHarmCheck(victim))
    return FALSE;

  int lev = getSkillLevel(SPELL_SKY_SPIRIT);
  int bKnown= getSkillValue(SPELL_SKY_SPIRIT);
  
  int percent = bKnown + ::number(-5,5);

  if (percent <= 0)
    spirit="<k>gnat";
  else if (percent <= 5)
    spirit="pigeon";
  else if (percent <= 10)
    spirit="sparrow";
  else if (percent <= 15)
    spirit="<k>crow";
  else if (percent <= 20)
    spirit="<k>raven";
  else if (percent <= 25)
    spirit="<r>vulture";
  else if (percent <= 30)
    spirit="<W>owl";
  else if (percent <= 40)
    spirit="<B>couatl";
  else if (percent <= 49)
    spirit="<o>falcon";
  else if (percent <= 55)
    spirit="<g>hawk";
  else if (percent <= 60)
    spirit="<Y>dragonne";
  else if (percent <= 70)
    spirit="<o>condor";
  else if (percent <= 75)
    spirit="<W>eagle";
  else if (percent <= 80)
    spirit="<o>roc";
  else if (percent <= 85)
    spirit="<G>wyvern";
  else if (percent <= 90)
    spirit="<Y>drake";
  else if (percent >= MAX_SKILL_LEARNEDNESS)
    spirit="<Y>dragon";
  else
    spirit="<R>phoenix";



  int dam = getSkillDam(victim, SPELL_SKY_SPIRIT, lev, getAdvLearning(SPELL_SKY_SPIRIT));


  if (!useComponent(findComponent(SPELL_SKY_SPIRIT), this, CHECK_ONLY_NO))
    return FALSE;

  addToWait((int)combatRound(discArray[SPELL_SKY_SPIRIT]->lag));
  reconcileHurt(victim,discArray[SPELL_SKY_SPIRIT]->alignMod);
  if (bSuccess(bKnown, SPELL_SKY_SPIRIT)) {
    
    if (critSuccess(this, SPELL_SKY_SPIRIT) == CRIT_S_DOUBLE) {
      CS(SPELL_SKY_SPIRIT);
      dam *= 2;
      spirit += " of incredible size";
    }
    
    act("You summon a spirit of the sky!", FALSE, this, NULL, victim, TO_CHAR);
    act("$n summons a spirit of the sky!", FALSE, this, NULL, victim, TO_ROOM);
    char buf[256];

    sprintf(buf, "<c>A phantasmal %s<1><c> swoops down from above and strikes $N!<1>", spirit.c_str());

    act(buf, FALSE, this, NULL, victim, TO_CHAR);
    act(buf, FALSE, this, NULL, victim, TO_NOTVICT);

    sprintf(buf, "<C>A phantasmal %s<1><c> swoops down from above and strikes you!<1>", spirit.c_str());

    act(buf, FALSE, this, NULL, victim, TO_VICT);


    if (this->reconcileDamage(victim, dam, SPELL_SKY_SPIRIT) == -1) {
      delete victim;
      return SPELL_SUCCESS + VICTIM_DEAD + DELETE_VICT;
    }
    return SPELL_SUCCESS;

  } else {
    
    act("You fail to summon a spirit of the sky.", FALSE, this, NULL, victim, TO_CHAR);
    act("$n fails to summon a spirit of the sky.", FALSE, this, NULL, victim, TO_ROOM);

    return SPELL_FAIL;
  }


  return SPELL_FAIL;
}


int TBeing::doFeralWrath(const char *argument)
{
  if (!doesKnowSkill(SPELL_FERAL_WRATH)) {
    sendTo("You do not know the secrets of feral wrath.\n\r");
    return FALSE;
  }

  if (affectedBySpell(SPELL_FERAL_WRATH)) {
    sendTo("You are already affected by feral wrath.\n\r");
    return FALSE;
  }

  int level = getSkillLevel(SPELL_FERAL_WRATH);
  int bKnown = getSkillValue(SPELL_FERAL_WRATH);

  // not technically a spell, but needs a component anyway
  if (!useComponent(findComponent(SPELL_FERAL_WRATH), this, CHECK_ONLY_NO))
    return FALSE;

  int which = ::number(1,3);

  affectedData aff, aff2;
  aff.type = SPELL_FERAL_WRATH;
  aff.location = APPLY_ARMOR;
  aff.duration = max(min(level/5, 5), 1) * UPDATES_PER_MUDHOUR;
  aff.bitvector = 0;
  aff.modifier = 200;

  int modifier = (level * ::number(80,125))/100;

  switch (which) {
    case 1:
      aff2.location = APPLY_STR;
      aff2.modifier = modifier;
      break;
    case 2:
      aff2.location = APPLY_DEX;
      aff2.modifier = modifier;
      break;
    case 3:
      aff2.location = APPLY_SPE;
      aff2.modifier = modifier;
      break;
      // removed this - dumb to give hp in a hitter spell - Maror
//    case 4:
//      aff2.location = APPLY_HIT;
//      aff2.modifier = modifier * 2;
//      break;
  }
  aff2.type = SPELL_FERAL_WRATH;
  aff2.duration = aff.duration;
  aff2.bitvector = 0;

  if (bSuccess(bKnown, this->getPerc(), SPELL_FERAL_WRATH)) {
    if (critSuccess(this, SPELL_FERAL_WRATH)) {
      CS(SPELL_FERAL_WRATH);
      aff2.modifier *= 2;
      aff.duration *= 2;
      aff2.duration *= 2;
    }

    if (!this->affectJoin(this, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      return SPELL_FALSE;
    }

    if (!this->affectJoin(this, &aff2, AVG_DUR_NO, AVG_EFF_YES)) {
      return SPELL_FALSE;
    }

    if (aff2.location == APPLY_STR) {
      act("The misty shape of a large bear settles on you.\n\rYou feel stronger.", FALSE, this, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a large bear settles on $n.",
          FALSE, this, NULL, NULL, TO_ROOM);
    } else if (aff2.location == APPLY_DEX) {
      act("The misty shape of a great cat settles on you.\n\rYou feel your reflexes quicken.", FALSE, this, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a great cat settles on $n.",
          FALSE, this, NULL, NULL, TO_ROOM);
    } else if (aff2.location == APPLY_SPE) {
      act("The misty shape of a snake settles on you.\n\rYou feel you can strike with great speed.", FALSE, this, NULL, NULL, TO_CHAR);
      act("A light mist in the shape of a snake settles on $n.",
          FALSE, this, NULL, NULL, TO_ROOM);
    }

    act("Your blood boils with feral rage!",
        FALSE, this, NULL, NULL, TO_CHAR);
    act("$n's eyes narrow with anger as $e takes on an aura of feral rage.",
        TRUE, this, NULL, NULL, TO_ROOM);

    return SPELL_SUCCESS;
  } else {
    if (critFail(this, SPELL_FERAL_WRATH)) {
      CF(SPELL_FERAL_WRATH);
      act("You feel the rage build inside you, and then suddenly you get a peaceful, easy feeling...", FALSE, this, NULL, NULL, TO_CHAR);
      act("$n looks angry for a moment, then suddenly becomes extremely relaxed.", TRUE, this, NULL, NULL, TO_ROOM);
      this->affectTo(&aff);
    } else {
      this->sendTo("Nothing seems to happen.\n\r");
      act("Nothing seems to happen.", FALSE, this, NULL, NULL, TO_ROOM);
    }
    return SPELL_FAIL;
  }
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

  strcpy(namebuf, argument);
  if (!(victim = get_char_room_vis(this, namebuf))) {
    sendTo("Charm what?\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
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



int beastSummon(TBeing * caster, const char * arg, int level, short bKnown)
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

  if (caster->bSuccess(bKnown, caster->getPerc(), SKILL_BEAST_SUMMON)) {
    max_dist = caster->isImmortal() ? 1000 : level;
    max_num = caster->isImmortal() ? 100 : level / 2;
  
i=0;
    for (v = character_list; v; v = v->next) {
      if (!v->isPc() && v->awake() && isname(targ_name, v->name) &&
	  !v->fight() && (caster->isImmortal() || v->isDumbAnimal()) &&
	  !IS_SET(v->specials.act, ACT_HUNTING) && (v->in_room != -1) &&
	  (caster->isImmortal() || (caster->roomp->getZoneNum() == v->roomp->getZoneNum()))) {
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
    sendTo("You know nothing about summoning beasts.\n\r");
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

