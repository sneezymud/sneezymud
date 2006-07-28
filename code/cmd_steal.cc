//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_looting.h"
#include "obj_open_container.h"

// We sometimes use steal as a mechanism to reposess items from mortals
// invisibly.  Not a great thing to do, but necessary and has a valid
// use in the "steal xx, swap xx to something, go invis, give xx"
// which is mostly a quest thing, but is a good way to make it magically
// appear that the item changed without them seeing the loss of possession,
// etc.
// Anyway, some of these checks check for POWER_WIZARD in order to
// bypass things that would otherwise block the steal.  Not positive
// that this should dual-purpose that power, but probably best to NOT
// have generic imms be able to steal at will.

static bool genericCanSteal(TBeing *thief, TBeing *victim)
{
  bool is_imp = thief->hasWizPower(POWER_WIZARD);

  if ((thief->equipment[HOLD_LEFT] || thief->equipment[HOLD_RIGHT]) && 
      !is_imp) {
    thief->sendTo("It is impossible to steal with your hand(s) already full!\n\r");
    return FALSE;
  }
  if (IS_SET(victim->specials.act, ACT_IMMORTAL) || victim->isImmortal()) {
    thief->sendTo("You can't steal from an immortal.\n\r");
    return FALSE;
  }
  if (!thief->doesKnowSkill(SKILL_STEAL)) {
    thief->sendTo("You know nothing about stealing.\n\r");
    return FALSE;
  }
  if (dynamic_cast<TMonster *>(thief) && dynamic_cast<TMonster *>(victim))
    return FALSE;

  if (!is_imp) { 
    if (thief->checkPeaceful("What if they caught you?\n\r"))
      return FALSE;
    if (thief->roomp->isRoomFlag(ROOM_NO_STEAL)) {
      thief->sendTo("Such actions are prevented here.\n\r");
      return FALSE;
    }
  }

  if (victim == thief) {
    thief->sendTo("Come on now, that's rather stupid!\n\r");
    return FALSE;
  }

  if (thief->riding) {
    thief->sendTo("Yeah... right... while mounted.\n\r");
    return FALSE;
  }

  if (thief->isFlying()) {
    thief->sendTo("The fact that you are flying makes you a bit too conspicuous to steal.\n\r");
    return FALSE;
  }

  if (victim->isShopkeeper() && !is_imp) {
    thief->sendTo("Oh, Bad Move.  Bad Move.\n\r");
    vlogf(LOG_CHEAT, fmt("%s just tried to steal from a shopkeeper! [%s]") % 
          thief->getName() % victim->getName());
    return FALSE;
  }

  return true;
}

static bool countersteal(TBeing *thief, TBeing *vict, int known)
{
  int bKnown;

  if(thief->isImmortal() || !vict->awake() || 
     !vict->doesKnowSkill(SKILL_COUNTER_STEAL))
    return FALSE;

  bKnown=vict->getSkillValue(SKILL_COUNTER_STEAL);
  bKnown+=vict->getSkillValue(SKILL_STEAL)/2;
  bKnown-=known;
  
  if(vict->bSuccess(bKnown, SKILL_COUNTER_STEAL)){
    act("Just when you think you've succeeded, $N reaches out and swats your hand away painfully.", 
	TRUE, thief, NULL, vict, TO_CHAR);
    act("You notice $n attempting to steal from you, so you wait until $e is almost successful and swat his hand away forcefully.", 
	TRUE, thief, NULL, vict, TO_VICT);
    return TRUE;
  }

  return FALSE;
}

// some of the prohibitions here are bypassed for isIMmortal in case we
// need to secretely repossess things.
static int steal(TBeing * thief, TBeing * victim)
{
  int modifier, gold, rc;
  TThing *t, *t2;
  TMonster *guard = NULL;
  int vict_lev = victim->GetMaxLevel();
  int level = thief->getSkillLevel(SKILL_STEAL);
  spellNumT skill = thief->getSkillNum(SKILL_SNEAK);

  if (victim->getPartMinHeight(ITEM_WEAR_WAIST) > (thief->getPosHeight() + 5)) {
    // victim riding a tall creature...
    act("You can't quite reach $N's pockets from here.",
        FALSE, thief, 0, victim, TO_CHAR);
    return FALSE;
  }

/* high modifier ---> easier to steal */
  modifier = (level - vict_lev)/3;

  modifier += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (!victim->awake())
    modifier += 50;

  if ((vict_lev > level) ||
      victim->isLucky(thief->spellLuckModifier(SKILL_STEAL)))
    modifier -= 45;

  modifier += victim->getCond(DRUNK)/4;

  if (!victim->isPc())
    modifier -= dynamic_cast<TMonster *>(victim)->susp()/2;

  int bKnown = thief->getSkillValue(SKILL_STEAL);

  modifier = max(min(modifier, 100 - bKnown), -100);

  if (!victim->awake() ||
      (!countersteal(thief, victim, bKnown+modifier) &&
      thief->bSuccess(bKnown+ modifier, SKILL_STEAL))) {
    /* Steal some money */
    gold = (int) ((victim->getMoney() * ::number(1, 10)) / 100);
    gold = min(5000, gold);
    LogDam(thief, SKILL_STEAL,gold);
    if (gold > 0) {
      victim->giveMoney(thief, gold, GOLD_INCOME);
      
      thief->sendTo(fmt("You have just stolen %d talen%s from %s.\n\r") % gold % 
            ((gold > 1) ? "s" : "") % victim->getName());

      if (victim->isPerceptive())
        victim->sendTo("You suddenly feel lighter in your moneypouch...\n\r");
    } else 
      thief->sendTo("You couldn't seem to find any talens...\n\r");
  } else {
    act("Oh ohhh...Busted!", FALSE, thief, 0, 0, TO_CHAR);

  
    if (thief->affectedBySpell(skill) || 
	thief->checkForSkillAttempt(skill)) {
      thief->sendTo("You are no longer sneaking.\n\r");
      thief->removeSkillAttempt(skill);
      if (thief->affectedBySpell(skill))
	thief->affectFrom(skill);
     }
  
    act("$n tries to steal money from $N.", TRUE, thief, 0, victim, TO_NOTVICT);
    if (victim->getPosition() == POSITION_SLEEPING && 
        !victim->isAffected(AFF_SLEEP) && victim->isLucky(thief->spellLuckModifier(SKILL_STEAL))) {
      victim->sendTo("You feel someone touching you and wake with a start.\n\r");
      act("$n wakes with a start.", TRUE, victim, 0, 0, TO_ROOM);
      victim->setPosition(POSITION_RESTING);
      victim->doLook("", CMD_LOOK);
    } else
      act("You discover that $n has $s hands in your moneypouch.", 
           FALSE, thief, 0, victim, TO_VICT);

    for (t = thief->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      if (!(guard = dynamic_cast<TMonster *>(t)))
        continue;
      if (!guard->isPolice()|| !guard->canSee(thief) || 
           !guard->awake() || guard == victim)
        continue;
      guard->doSay("Thief!  Villain!  Prepare to die!");
      if ((rc = guard->takeFirstHit(*thief)) == DELETE_VICT)
        return DELETE_THIS;
      else if (rc == DELETE_THIS) {
        delete guard;
        guard = NULL;
      }
    }
    if (critFail(thief, SKILL_STEAL) && !victim->isPc() && victim->awake())  {
      CF(SKILL_STEAL);
      act("$N reacts in anger to your arrogance!", TRUE, thief, NULL, victim, TO_CHAR);
      act("$N reacts in anger to $n arrogance!", TRUE, thief, NULL, victim, TO_NOTVICT);
      act("You get pissed at $n's arrogance!", TRUE, thief, NULL, victim, TO_VICT);
      thief->setCharFighting(victim);
      thief->setVictFighting(victim);
    } else 
      if (!victim->isPc() && victim->awake()) {
        if ((rc = dynamic_cast<TMonster *>(victim)->aiSteal(thief)) == DELETE_VICT)
          return DELETE_THIS;
        else if (rc == DELETE_THIS)
          return DELETE_VICT;
      }
  }
  return TRUE;
}

// bigger number makes it harder to steal
int TThing::stealModifier()
{
  return 0;
}

static int steal(TBeing * thief, TBeing * victim, const sstring &obj_name)
{
  int modifier;
  wearSlotT eq_pos;
  int vict_lev = victim->GetMaxLevel();
  int level = thief->getSkillLevel(SKILL_STEAL);
  TThing *t, *t2;
  TMonster *guard = NULL;
  int rc;
  int bKnown = thief->getSkillValue(SKILL_STEAL);
  bool is_imp = thief->hasWizPower(POWER_WIZARD);
  spellNumT skill = thief->getSkillNum(SKILL_SNEAK);
  
  if (bKnown < 75) {
    thief->sendTo("You don't have the ability to steal equipment. (yet...)\n\r");
    return FALSE;
  }

#if 0
  // i moved this code down past where eq_pos is defined.... hehe -dash
  if (victim->awake()) {
    if (!thief->isImmortal()) {
      if ((eq_pos != WEAR_NECK && eq_pos != WEAR_FINGER_R && eq_pos != WEAR_FINGER_L && 
   	   eq_pos != WEAR_WRIST_R && eq_pos != WEAR_WRIST_L) || 
	  (victim->getPosition() <= POSITION_SLEEPING && eq_pos != WEAR_FOOT_L && eq_pos != WEAR_FOOT_R &&
	   eq_pos != WEAR_HAND_L && eq_pos != WEAR_HAND_R && eq_pos != WEAR_HEAD)) {
	thief->sendTo("It is not possible to steal that without being noticed.\n\r");
	return FALSE;
      }
    }
  }
  // The above was added to make steal a bit more realistic at Peel's request --jh

#endif

/* high modifier ---> easier to steal */
  modifier = (level - vict_lev)/3;

  modifier -= 35;   /* tough to steal equipped stuff */

  modifier += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (!victim->awake())
    modifier += 100;

  if ((vict_lev > level) && victim->isLucky(thief->spellLuckModifier(SKILL_STEAL)))
    modifier -= 65;

  modifier += victim->getCond(DRUNK)/4;

  if (!victim->isPc())
    modifier -= dynamic_cast<TMonster *>(victim)->susp()/2;


  TThing *tt = searchLinkedListVis(thief, obj_name, victim->getStuff());
  TObj *obj = dynamic_cast<TObj *>(tt);
  if (!obj) {
    for (eq_pos = MIN_WEAR; (eq_pos < MAX_WEAR); eq_pos++) {
      obj = dynamic_cast<TObj *>(victim->equipment[eq_pos]);
      if (obj && (isname(obj_name, obj->name)) && thief->canSee(obj)) {
        break;
      }
    }

    if (!obj) {
      act("$E does not have that item.", FALSE, thief, 0, victim, TO_CHAR);
      return FALSE;
    } else {			/* It is equipment */
      modifier -= (int) obj->getWeight();      /* Make heavy harder */
      modifier -= obj->getVolume()/150; // make equip item harder per size
      modifier -= obj->stealModifier();
    }

    // ideally, find the ITEM_WEAR slot for eq-pos, and use it...
    if (victim->getPartMinHeight(ITEM_WEAR_FEET) > thief->getPosHeight()) {
      // victim riding a tall creature...
      act("You can't quite reach $N's $o from here.",
          FALSE, thief, obj, victim, TO_CHAR);
      return FALSE;
    }

  } else {
    eq_pos = WEAR_NOWHERE; // not equiped
  }


  if (victim->awake()) {
    if (!thief->isImmortal() && eq_pos != WEAR_NOWHERE) {
      if ((eq_pos != WEAR_NECK && eq_pos != WEAR_FINGER_R && eq_pos != WEAR_FINGER_L &&
           eq_pos != WEAR_WRIST_R && eq_pos != WEAR_WRIST_L) ||
          (victim->getPosition() <= POSITION_SLEEPING && eq_pos != WEAR_FOOT_L && 
           eq_pos != WEAR_FOOT_R && eq_pos != WEAR_HAND_L && eq_pos != WEAR_HAND_R && 
           eq_pos != WEAR_HEAD)) {
	thief->sendTo("It is not possible to steal that without being noticed.\n\r");
	return FALSE;
      }
    }
  }
  // The above was added to make steal a bit more realistic at Peel's request --jh
  // oh sure, blame me - peel


  if (!is_imp && obj->isObjStat(ITEM_NODROP)) {
    thief->sendTo("You can't steal it, it must be CURSED!\n\r");
    return FALSE;
  }
  if (!is_imp && obj->isMonogrammed()) {
    thief->sendTo("That item has been monogrammed making it worthless to steal.\n\r");
    return FALSE;
  }

  // mostly here for spellbags, but applies to other containers too...
  // too simplistic a means of getting all their wealth fast
  if(!is_imp && dynamic_cast<TOpenContainer *>(obj)){
    thief->sendTo("You can't seem to distract your victim enough to steal that.\n\r");
    return FALSE;
  }

  if (!victim->awake() || is_imp ||
      (!countersteal(thief, victim, bKnown+modifier) &&
       thief->bSuccess(bKnown + modifier, SKILL_STEAL))) {
    if (is_imp || ((thief->getCarriedVolume() + obj->getTotalVolume()) < thief->carryVolumeLimit())) {
      // obj-weight <= weight limit
      if (is_imp || (compareWeights(obj->getTotalWeight(TRUE), 
                thief->carryWeightLimit() - thief->getCarriedWeight()) != -1)) {
        if (eq_pos == WEAR_NOWHERE) {
	    --(*obj);
	    *thief += *obj;
	    thief->sendTo("Got it!\n\r");
        } else {
          act("You unequip $p and steal it.", FALSE, thief, obj, 0, TO_CHAR);
          *thief += *(victim->unequip(eq_pos));
        }

	/*
        if (thief->isPc() && victim->isPc() && !thief->isImmortal()) {
          affectedData tAff;

          tAff.type     = AFFECT_PLAYERLOOT;
          tAff.duration = (24 * UPDATES_PER_MUDHOUR);
          thief->affectJoin(thief, &tAff, AVG_DUR_NO, AVG_EFF_NO);
          vlogf(LOG_CHEAT, fmt("Adding PLoot Flag To: %s (4)") %  thief->getName());
        }
	*/

        thief->doSave(SILENT_YES);
        victim->doSave(SILENT_YES);
        if (!thief->hasWizPower(POWER_WIZARD))
          vlogf(LOG_MISC, fmt("%s stole %s from %s.") % thief->getName() %
                obj->getName() % victim->getName());

      if (victim->isPerceptive() && !thief->isImmortal())
        victim->sendTo("You suddenly feel like something is missing...\n\r");
      } else
        thief->sendTo("You can't carry that much weight.\n\r");
    } else
       thief->sendTo("You can't carry that much volume.\n\r");
  } else {
    act("You are caught in the act of stealing the $o!",FALSE,thief,obj,0,TO_CHAR);

    if (thief->affectedBySpell(skill) ||
        thief->checkForSkillAttempt(skill)) {
      thief->sendTo("You are no longer sneaking.\n\r");
      thief->removeSkillAttempt(skill);
      if (thief->affectedBySpell(skill))
        thief->affectFrom(skill);
    }

    act("$n fails to steal $N's $o.",FALSE,thief,obj,victim,TO_NOTVICT);

    if (victim->getPosition() == POSITION_SLEEPING && 
        !victim->isAffected(AFF_SLEEP) && victim->isLucky(thief->spellLuckModifier(SKILL_STEAL))) {
      victim->sendTo("You feel someone touching you and wake with a start.\n\r");
      act("$n wakes with a start.", TRUE, victim, 0, 0, TO_ROOM);
      victim->setPosition(POSITION_RESTING);
      victim->doLook("", CMD_LOOK);
    } else
      act("$n just tried to steal your $o!!",FALSE,thief,obj,victim,TO_VICT);

    for (t = thief->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      if (!(guard = dynamic_cast<TMonster *>(t)))
        continue;
      if (!guard->isPolice() || !guard->canSee(thief) || 
           !guard->awake() || guard == victim)
        continue;
      guard->doSay("Thief!  Villain!  Prepare to die!");
      if ((rc = guard->takeFirstHit(*thief)) == DELETE_VICT)
        return DELETE_THIS;
      else if (rc == DELETE_THIS) {
        delete guard;
        guard = NULL;
      }
    }

    if (critFail(thief, SKILL_STEAL) && !victim->isPc())  {
      CF(SKILL_STEAL);
      act("$N reacts in anger to your arrogance!", TRUE, thief, NULL, victim, TO_CHAR);
      act("$N reacts in anger to $n arrogance!", TRUE, thief, NULL, victim, TO_NOTVICT);
      act("You get pissed at $n's arrogance!", TRUE, thief, NULL, victim, TO_VICT);
      thief->setCharFighting(victim);
      thief->setVictFighting(victim);
    } else 
      if (!victim->isPc() && victim->awake()) {
        if ((rc = dynamic_cast<TMonster *>(victim)->aiSteal(thief)) == DELETE_VICT)
          return DELETE_THIS;
        else if (rc == DELETE_THIS)
          return DELETE_VICT;
      }
  }
  return TRUE;
}

int TBeing::doSteal(const sstring &argument, TBeing *vict)
{
  TBeing * victim;
  sstring victim_name, obj_name;
  int rc;

  obj_name=argument.word(0);
  victim_name=argument.word(1);


  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, victim_name))) {
      sendTo("Steal what from whom?\n\r");
      return FALSE;
    }
  } 
  if (!genericCanSteal(this, victim))
    return false;

  if (is_abbrev(obj_name, "coins") || is_abbrev(obj_name, "talens")) 
    rc = steal(this,victim);
  else 
    rc = steal(this,victim,obj_name);

  if (rc)
    addSkillLag(SKILL_STEAL, rc);

  if (rc == DELETE_VICT) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
  }
  return rc;
}
