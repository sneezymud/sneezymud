//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_steal.cc,v $
// Revision 1.3  1999/10/12 00:33:00  lapsos
// Added block to prevent stealing from shopkeepers.
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_looting.h"

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
  if (victim->isImmortal()) {
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

  if (victim->spec == SPEC_SHOPKEEPER && !is_imp) {
    thief->sendTo("Oh, Bad Move.  Bad Move.\n\r");
    vlogf(10, "%s just tried to steal from a shopkeeper! [%s]",
          thief->getName(), victim->getName());
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
  
  if(bSuccess(vict, bKnown, SKILL_COUNTER_STEAL)){
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

  if (victim->getPartMinHeight(ITEM_WEAR_WAISTE) > (thief->getPosHeight() + 5)) {
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

  if ((vict_lev > level) &&
      victim->isLucky(thief->spellLuckModifier(SKILL_STEAL)))
    modifier -= 45;

  modifier += victim->getCond(DRUNK)/4;

  if (!victim->isPc())
    modifier -= dynamic_cast<TMonster *>(victim)->susp()/2;

  int bKnown = thief->getSkillValue(SKILL_STEAL);

  modifier = max(min(modifier, 100 - bKnown), -100);

  if (!victim->awake() ||
      (!countersteal(thief, victim, bKnown+modifier) &&
      bSuccess(thief, bKnown+ modifier, SKILL_STEAL))) {
    /* Steal some money */
    gold = (int) ((victim->getMoney() * ::number(1, 10)) / 100);
    gold = min(4000, gold);
    LogDam(thief, SKILL_STEAL,gold);
    if (gold > 0) {
      thief->addToMoney(gold, GOLD_INCOME);
      victim->addToMoney(-gold, GOLD_INCOME);
      
      thief->sendTo("Bingo! You got %d talen%s.\n\r", gold, 
            (gold > 1) ? "s" : "");
    } else 
      thief->sendTo("You couldn't seem to find any talens...\n\r");
  } else {
    act("Oops..", FALSE, thief, 0, 0, TO_CHAR);
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

    for (t = thief->roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      if (!(guard = dynamic_cast<TMonster *>(t)))
        continue;
      if (!guard->isPolice()|| !guard->canSee(thief) || 
           !guard->awake() || guard == victim)
        continue;
      guard->doSay("Thief!  Villain!  Prepare to die!");
      if ((rc = guard->takeFirstHit(thief)) == DELETE_VICT)
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

static int steal(TBeing * thief, TBeing * victim, char * obj_name)
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

  if (bKnown < 75) {
    thief->sendTo("You don't have the ability to steal equipment. (yet...)\n\r");
    return FALSE;
  }

/* high modifier ---> easier to steal */
  modifier = (level - vict_lev)/3;

  modifier -= 25;   /* tough to steal equipped stuff */

  modifier += thief->plotStat(STAT_CURRENT, STAT_DEX, -70, 15, 0);

  if (!victim->awake())
    modifier += 100;

  if ((vict_lev > level) && victim->isLucky(thief->spellLuckModifier(SKILL_STEAL)))
    modifier -= 55;

  modifier += victim->getCond(DRUNK)/4;

  if (!victim->isPc())
    modifier -= dynamic_cast<TMonster *>(victim)->susp()/2;


  TThing *tt = searchLinkedListVis(victim, obj_name, victim->stuff);
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
  if(!is_imp && dynamic_cast<TRealContainer *>(obj)){
    thief->sendTo("You can't seem to distract your victim enough to steal that.\n\r");
    return FALSE;
  }

  if (!victim->awake() || is_imp ||
      (!countersteal(thief, victim, bKnown+modifier) &&
       bSuccess(thief, bKnown + modifier, SKILL_STEAL))) {
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
        thief->doSave(SILENT_YES);
        victim->doSave(SILENT_YES);
        if (!thief->hasWizPower(POWER_WIZARD))
          vlogf(0,"%s stole %s from %s.",thief->getName(),
                obj->getName(), victim->getName());
      } else
        thief->sendTo("You can't carry that much weight.\n\r");
    } else
       thief->sendTo("You can't carry that much volume.\n\r");
  } else {
    act("You are caught in the act of stealing the $o!",FALSE,thief,obj,0,TO_CHAR);
    act("$n fails to steal $N's $o.",FALSE,thief,obj,victim,TO_NOTVICT);

    if (victim->getPosition() == POSITION_SLEEPING && 
        !victim->isAffected(AFF_SLEEP) && victim->isLucky(thief->spellLuckModifier(SKILL_STEAL))) {
      victim->sendTo("You feel someone touching you and wake with a start.\n\r");
      act("$n wakes with a start.", TRUE, victim, 0, 0, TO_ROOM);
      victim->setPosition(POSITION_RESTING);
      victim->doLook("", CMD_LOOK);
    } else
      act("$n just tried to steal your $o!!",FALSE,thief,obj,victim,TO_VICT);

    for (t = thief->roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      if (!(guard = dynamic_cast<TMonster *>(t)))
        continue;
      if (!guard->isPolice() || !guard->canSee(thief) || 
           !guard->awake() || guard == victim)
        continue;
      guard->doSay("Thief!  Villain!  Prepare to die!");
      if ((rc = guard->takeFirstHit(thief)) == DELETE_VICT)
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

int TBeing::doSteal(const char *argument, TBeing *vict)
{
  TBeing * victim;
  char victim_name[240];
  char obj_name[240];
  int rc;

  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);

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
    addSkillLag(SKILL_STEAL);

  if (rc == DELETE_VICT) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
  }
  return rc;
}
