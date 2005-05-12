///////////////////////////////////////////////////////////////////////////
//
//     SneezyMUD++ - All rights reserved, SneezyMUD++ Coding Team
//
//     "offense.cc" - All offensive functions and routines
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"
#include "combat.h"
#include "obj_spellbag.h"
#include "obj_open_container.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "obj_base_cup.h"
#include "obj_base_clothing.h"
#include "obj_bag.h"
#include "liquids.h"

// DELETE_THIS implies this needs to be deleted
int TBeing::doHit(const sstring &argument, TBeing *vict)
{
  sstring arg;
  TBeing *victim, *tmp;
  int rc = FALSE;
  bool shouldHit = FALSE;

  spellNumT skill = getSkillNum(SKILL_SWITCH_OPP);

  if (checkBlackjack()) {
    gBj.Hit(this);
    return FALSE;
  }
  if (checkBaccarat()){
    gBaccarat.Hit(this);
    return FALSE;
  }
  if (getPosition() <= POSITION_SITTING) {
    sendTo("Try standing up first.\n\r");
    return FALSE;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  arg=argument.word(0);

  if (vict) {
    victim = vict;
    if (!canSee(victim) && (victim != fight())) {
      sendTo("Just whom do you want to hit?\n\r");
      return FALSE;
    }
  } else {
    if ((tmp = get_best_char_room(this, arg.c_str()))) {
      victim = tmp; 
    } else if (fight() && isAffected(AFF_ENGAGER)) {
      victim = fight();
      if (!canSee(victim)) {
        act("You can't see $N to start hitting $M.",
             FALSE, this, 0, victim, TO_CHAR);
        return FALSE;
      }
    } else {
      sendTo("Just whom do you want to hit?\n\r");
      return FALSE;
    } 
  } 

// if pc is already in battle but not swinging and wishes to hit-Cos 3/1/97
  if (fight() && isAffected(AFF_ENGAGER)) {
    if (doEngagedHit(argument.c_str(), victim)) {

      // remove engager after stopFighting so attackers kept up with properly
      if (fight())
        stopFighting();
      REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);

      // OK, potential problem here...
      // !fight() and set for one of the AUTO_ENGAGE things means first
      // call to reconcile_damage will result in becoming engaged.  This is
      // bad, so we kludge our way around this by setting ourselves fighting
      // we pass parm 3 as false so that we DON'T set engaged
      setFighting(victim, 0, false);

      addToWait(combatRound(1));
      rc = hit(victim);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        if (vict) 
          return rc;
        victim->reformGroup();
        delete victim;
        victim = NULL;
      }
      if (!rc) 
        setVictFighting(victim);
      return rc;
    } else {
      return FALSE;
    }
  }


  if (victim == this) {
    sendTo("You hit yourself..OUCH!\n\r");
    act("$n hits $mself, and says OUCH!", FALSE, this, 0, victim, TO_ROOM);
    return FALSE;
  } else if (isAffected(AFF_CHARM) && (master == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", 
            FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  } 
  if (noHarmCheck(victim))
    return FALSE;

  if ((getPosition() >= POSITION_CRAWLING) && !fight()) {
    TThing      * tThing;
    TBaseWeapon * tWeapon;

    if (isPc())
      if ((tThing   = equipment[getPrimaryHold()]) &&
          !(tWeapon = dynamic_cast<TBaseWeapon *>(tThing)) &&
          (tThing   = equipment[getSecondaryHold()]) &&
          !(tWeapon = dynamic_cast<TBaseWeapon *>(tThing)) &&
          !IS_SET(desc->autobits, AUTO_ENGAGE) &&
          !IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
        sendTo("You are wielding no weapons and intend to attack?\n\r");
        sendTo("I'm afraid that's rather hard, use engage instead.\n\r");
        return FALSE;
      }


//    if (!victim->fight()) 
// put if statement back in if doesnt work right
    if (!isAffected(AFF_AGGRESSOR))
      SET_BIT(specials.affectedBy, AFF_AGGRESSOR);
// this line is only if they have been sent back engaged by doEngagedhit
// this will stopthemfighting and return it o here
    if (isAffected(AFF_ENGAGER)) {
      act("You engage $N in battle.", TRUE, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act("$n engages you in battle.", TRUE, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("$n engages $N in battle.", TRUE, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
    } else {
      act("You attack $N.", TRUE, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act("$n attacks you.", TRUE, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("$n attacks $N.", TRUE, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      shouldHit = TRUE;

      soundNumT snd = pickRandSound(SOUND_BANZAI_01, SOUND_BANZAI_04);
      roomp->playsound(snd, SOUND_TYPE_COMBAT);
    }
    rc = hit(victim);
    addToWait(combatRound(1));
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      if (vict)
        return rc;
      victim->reformGroup();
      delete victim;
      victim = NULL;
    } else if (!rc) {
      if (!isAffected(AFF_AGGRESSOR))
        SET_BIT(specials.affectedBy, AFF_AGGRESSOR);
      if (isAffected(AFF_ENGAGER)) {
        setCharFighting(victim);
      }
      // hit failed (non-weapon in primary, etc)
      // force the fight to start
      setVictFighting(victim);
      if (shouldHit && isAffected(AFF_ENGAGER)) {
        REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
        if (fight())
          fight()->attackers++;
      }
    }
    return rc;
  } else {
    if (victim != fight()) {
      if (doesKnowSkill(skill)) {
        if (bSuccess(skill)) {
          stopFighting();
          if (victim->attackers < MAX_COMBAT_ATTACKERS) {
            if (isAffected(AFF_ENGAGER)) {
              doEngage("", victim);
              sendTo("You switch opponents.\n\r");
              act("$n switches targets.", FALSE, this, 0, 0, TO_ROOM);
              addToWait(combatRound(1));
            } else {
              setCharFighting(victim);
              sendTo("You switch opponents.\n\r");
              act("$n switches targets.", FALSE, this, 0, 0, TO_ROOM);
              addToWait(combatRound(1));
            }
          } else {
            sendTo("There's no room to switch!\n\r");
            addToWait(combatRound(1));
          }
        } else {
          sendTo("You try to switch opponents, but you become confused!\n\r");
          stopFighting();
          addToWait(combatRound(2));
        }
      } else {
        sendTo("You do the best you can!\n\r");
      }
    } else {
      sendTo("You do the best you can!\n\r");
    }
  }

  return FALSE;
}

int TBeing::doEngagedHit(const char *argument, TBeing *vict)
{
  char arg[80];
  TBeing *victim;

// used when the person is both engaged and tries to hit
// will start them hitting

  spellNumT skill = getSkillNum(SKILL_SWITCH_OPP);

  if (!fight()) {
    vlogf(LOG_BUG,fmt("DoEngagedHit called without pc (%s) fighting") %  getName());
    return FALSE;
  }

  strcpy(arg, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, arg, NULL, EXACT_NO, INFRA_YES))) {
      sendTo("Hit whom?\n\r");
      return FALSE;
    }
  }

  if (victim == this) {
    sendTo("You hit yourself..OUCH!.\n\r");
    act("$n hits $mself, and says OUCH!", FALSE, this, 0, victim, TO_ROOM);
    return FALSE;
  } else if (isAffected(AFF_CHARM) && (master == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", 
            FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  } 
  if (noHarmCheck(victim))
    return FALSE;

  if (fight() == victim) {
    if (victim->attackers < MAX_COMBAT_ATTACKERS) {
      act("You start to physically attack $N.", TRUE, this, 0, victim, TO_CHAR);
      act("$n starts to physically attack you.", TRUE, this, 0, victim, TO_VICT);
      act("$n attacks $N.", TRUE, this, 0, victim, TO_NOTVICT);
      stopFighting();
      // remove engager after stopFight so attackers kept up with OK
      REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
      return TRUE;
    } else {
      act("There is no room to begin physically attacking $N.",
           FALSE, this, 0, victim, TO_CHAR);
      return FALSE;
    
    }
  } else {
      if (doesKnowSkill(getSkillNum(skill))) {
        if (bSuccess(skill)) {
          if (victim->attackers < MAX_COMBAT_ATTACKERS) {
            stopFighting();
            // remove engager after stopFight so attackers kept up with OK
            REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
            sendTo("You switch opponents.\n\r");
            act("$n switches targets.", FALSE, this, 0, 0, TO_ROOM);
            addToWait(combatRound(1));
            return TRUE;
          } else {
            act("There is no room to switch and begin attacking $N.", 
                FALSE, this, 0, victim, TO_CHAR);
            addToWait(combatRound(1));
          }
        } else {
          sendTo("You try to switch opponents, but you become confused!\n\r");
          stopFighting();
          addToWait(combatRound(2));
        }
      } else {
        sendTo("You are already occupied in fighting someone else!\n\r");
      }
  }
  return FALSE;
}

// DELETE_THIS return implies this needs to be deleted
int TBeing::doEngage(const char *argument, TBeing *vict)
{
  char arg[MAX_INPUT_LENGTH];
  TBeing *victim, *tmp;

  if (checkPeaceful("You feel too peaceful to contemplate violence!\n\r"))
    return FALSE;

//  if (fight() && isAffected(AFF_ENGAGER)) {
//    sendTo("You are already engaged in combat.\n\r");
//    return FALSE;
//  }


// target setting

  strcpy(arg, argument);

  if (vict) {
    victim = vict;
    if (!canSee(victim)) {
      sendTo("Just whom do you want to engage?\n\r");
      return FALSE;
    }
  } else {
      if ((tmp = get_char_room_vis(this,arg, NULL, EXACT_NO, INFRA_YES))) {
        victim = tmp; 
      } else if (fight() && !isAffected(AFF_ENGAGER)) {
        victim = fight();
      } else {
        sendTo("Just whom do you want to engage?\n\r");
        return FALSE;
      } 
  } 

  if (this == victim) {
    sendTo("How are you going to engage yourself in combat.\n\r");
    return FALSE;
  }

// result setting

  if (fight()) {
    if (fight() == victim) {
      if (isAffected(AFF_ENGAGER)) {
        act("You are already engaged in combat with $N.",
            TRUE, this, 0, victim, TO_CHAR);
        return FALSE; 
      } else {
        SET_BIT(specials.affectedBy, AFF_ENGAGER);
        act("You stop physically attacking $N.",
            TRUE, this, 0, victim, TO_CHAR);
        act("$n stops physically attacking you.",
            TRUE, this, 0, victim, TO_VICT);
        act("$n stops physically attacking $N.",
            TRUE, this, 0, victim, TO_NOTVICT);

        // setFighting added to attackers, so allow for this
        fight()->attackers--;
        return TRUE;
      } 
    } else {
      if (isTanking()) {
        act("You can not simply engage another while you are tanking.",
              TRUE, this, 0, 0, TO_CHAR);
        return FALSE;
      }
      if (isAffected(AFF_ENGAGER)) {
        addToWait(combatRound(2));
      } else {
        addToWait(combatRound(3));
      }
      tmp = fight();
      act("You turn your attention from $N.",
            TRUE, this, 0, tmp, TO_CHAR);
      act("$n disengages $s attention from the battle against you.",
            TRUE, this, 0, tmp, TO_VICT);
      act("$n disengages from the attack on $N.",
            TRUE, this, 0, tmp, TO_NOTVICT);
      stopFighting();
      SET_BIT(specials.affectedBy, AFF_ENGAGER);
      return doHit(argument, victim);
    }
  } else {
    if (!victim)
      return FALSE;

    if (getPosition() <= POSITION_SITTING) {
      sendTo("Try standing up first.\n\r");
      return FALSE;
    }
    if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
      return FALSE;

    if (noHarmCheck(victim) || victim->isImmortal()) {
      sendTo("You can't engage that target in combat.\n\r");
      return FALSE;
    }
    if (victim == this) {
      sendTo("You dance around and engage yourself..OUCH!.\n\r");
      act("$n engages $mself, and stares $eself down!", FALSE, this, 0, victim, TO_ROOM); 
      return FALSE;
    } else if (isAffected(AFF_CHARM) && (master == victim)) {
      act("$N is just such a good friend, you simply can't engage $M.",
          FALSE, this, 0, victim, TO_CHAR);
      return FALSE;
    }

    SET_BIT(specials.affectedBy, AFF_ENGAGER);
    SET_BIT(specials.affectedBy, AFF_AGGRESSOR);

    setCharFighting(victim);
    act("You engage $N in battle.", TRUE, this, 0, victim, TO_CHAR);
    act("$n engages you in battle.", TRUE, this, 0, victim, TO_VICT);
    act("$n engages $N in battle.", TRUE, this, 0, victim, TO_NOTVICT);
    addToWait(combatRound(1));
    setVictFighting(victim);
    return FALSE;
  }
}

int TBeing::doDisengage()
{
  if (!fight()) {
    act("Why do you need to disengage. You are not even fighting.", 
        FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }

  if (isTanking() && isAffected(AFF_ENGAGER) && (fight()->getPosition() > POSITION_SLEEPING)) {
    act("You can not fully disengage while you are tanking.", 
        FALSE, this, 0, 0, TO_CHAR);
    return FALSE;
  }

  if (isAffected(AFF_ENGAGER)) {
    addToWait(combatRound(2));
    act("You disengage totally from the fight.", FALSE, this, 0, 0, TO_CHAR);
    stopFighting();
    REMOVE_BIT(specials.affectedBy, AFF_ENGAGER); 
  } else {
    SET_BIT(specials.affectedBy, AFF_ENGAGER);
    addToWait(combatRound(1));
    fight()->attackers--;
    act("You disengage from physical participation in the fight.", 
        FALSE, this, 0, 0, TO_CHAR);
  }
  if (!isTanking() && (cantHit > 0)) {
    if (cantHit <=2) {
      cantHit = 0; 
      act("You finish orienting yourself.", FALSE, this, 0, 0, TO_CHAR);
      addToWait(combatRound(2));
    } else if (!fight()) {
      addToWait(combatRound(2 + (cantHit/5)));
      act("You finish orienting yourself.", FALSE, this, 0, 0, TO_CHAR);
      cantHit = 0; 
    }  else {
      act("By disengaging, you are able focus on orienting yourself.", FALSE, this, 0, 0, TO_CHAR);
    }
  }
  return TRUE;
}

// DELETE_THIS return implies this needs to be deleted
int TBeing::doKill(const char *argument, TBeing *vict)
{
  char arg[MAX_INPUT_LENGTH];
  TBeing *v;

  if (checkPeaceful("You feel too peaceful to contemplate violence!\n\r"))
    return FALSE;

  if (!hasWizPower(POWER_SLAY)) {
    return doHit(argument, vict);
  }

  strcpy(arg, argument);

  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, arg, NULL, EXACT_NO, INFRA_YES))) {
      sendTo("They aren't here.\n\r");
      return FALSE;
    }
  }

  if (this == v)
    sendTo("Your mother would be so sad.. :(\n\r");
  else if (!noHarmCheck(v)) {
    if ((v->GetMaxLevel() < GetMaxLevel()) || !v->isPc()) {
      act("You chop $M to pieces! Ah! The blood!", 0, this, 0, v, TO_CHAR);
      act(msgVariables(MSG_SLAY_TARG, (TThing *)NULL),
          FALSE, v, 0, this, TO_CHAR);
      act(msgVariables(MSG_SLAY, v), 
          FALSE, this, 0, v, TO_NOTVICT);
      v->rawKill(DAMAGE_NORMAL, this);
      if (vict)
        return DELETE_VICT;
      v->reformGroup();
      delete v;
      v = NULL;
    } else {
      sendTo(COLOR_MOBS, fmt("You can't kill %s!\n\r") % v->getName());
    }
  }
  return FALSE;
}

bool TBeing::isOrderAllowed(const char *argument)
{
  return TRUE;
}

// returns DELETE_VICT (vict)
static int applyOrder(TBeing *ch, TBeing *vict, const char * message, silentTypeT silent)
{
  int rc;

  if (!silent)
    ch->sendTo("OK.\n\r");

  if (vict->getWait() <= 1) {
    vlogf(LOG_SILENT, fmt("%s ordering %s to '%s' at %d") % 
          ch->getName() % vict->getName() % message % ch->inRoom());
    if (!strncmp(message, "ret", 3)) {
      rc = vict->checkDecharm(FORCE_YES);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    } else {
      rc = vict->addCommandToQue(message);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
    }
  }
  return FALSE;
}

static bool orderDenyCheck(const char * cmd_buf)
{
  return (is_abbrev(cmd_buf, "kill") ||
      is_abbrev(cmd_buf, "hit") ||
      is_abbrev(cmd_buf, "engage") ||
      is_abbrev(cmd_buf, "assist") ||
      is_abbrev(cmd_buf, "attack") ||
      is_abbrev(cmd_buf, "kick") ||
      is_abbrev(cmd_buf, "bash") ||
      is_abbrev(cmd_buf, "deathstroke") ||
      is_abbrev(cmd_buf, "grapple") ||
      is_abbrev(cmd_buf, "shove") ||
      is_abbrev(cmd_buf, "bodyslam") ||
      is_abbrev(cmd_buf, "rescue") ||
      is_abbrev(cmd_buf, "kneestrike") ||
      is_abbrev(cmd_buf, "sleep") || // order sleep, backstab....
      is_abbrev(cmd_buf, "emote") ||
      is_abbrev(cmd_buf, "mount") ||
      is_abbrev(cmd_buf, "headbutt") ||
      is_abbrev(cmd_buf, "open") ||
      is_abbrev(cmd_buf, "lower") ||
      is_abbrev(cmd_buf, "lift") ||
      is_abbrev(cmd_buf, "raise"));
}

// returns DELETE_THIS
int TBeing::doOrder(const char *argument)
{
  char caName[100], message[256];
  char buf[256];
  int found = FALSE;
  int org_room, i;
  char cmd_buf[40];
  int rc;

  if (applySoundproof())
    return FALSE;

  if (roomp && roomp->isRoomFlag(ROOM_NO_ORDER)) {
    sendTo("A mystical force prevents your order from being uttered here.\n\r");
    return FALSE;
  }
  half_chop(argument, caName, message);

  if (!*caName || !*message) {
    sendTo("Order whom to do what?\n\r");
    return FALSE;
  }
/*  TBeing *v = get_char_room_vis(this, caName, NULL, EXACT_NO, INFRA_YES);*/
  TBeing *v = get_best_char_room(this, caName, VISIBLE_YES, INFRA_NO);
  if (!v)
    v = get_best_char_room(this, caName, VISIBLE_YES, INFRA_YES);

  if (!v && !is_abbrev(caName, "followers") && strcmp("all", caName)) {
    sendTo("That person isn't here.\n\r");
    return FALSE;
  }
  if (this == v) {
    sendTo("You obviously suffer from Multiple Personality Disorder.\n\r");
    return FALSE;
  }
  if (isAffected(AFF_CHARM)) {
    sendTo("Your superior would not approve of you giving orders.\n\r");
    return FALSE;
  }
  if (v) {
    sprintf(buf, "$N orders you to '%s'", message);
    act(buf, FALSE, v, 0, this, TO_CHAR);
    act("$n gives $N an order.", FALSE, this, 0, v, TO_NOTVICT);

    for (i = 0; message[i] && message[i] != ' '; i++)
      cmd_buf[i] = message[i];
    cmd_buf[i] = '\0';

    // legitimate situations
    bool legitimate = false;
    bool messSent = false;

    // I am a charm taking orders from my master
    if (v->master == this && v->isAffected(AFF_CHARM)) {
      // pets aren't too eager to leap into the fray, but will do so on their
      // own.  Charms and zombies do what they are told
      if (v->isPet(PETTYPE_PET)) {
        if (!orderDenyCheck(cmd_buf))
          legitimate = true;
      } else
        legitimate = true;
    }

    // I am a captive, taking orders from my capter
    if (!legitimate &&
        v->getCaptiveOf() == this)
      legitimate = true;

    // I am a horse, taking orders from my master
    if (!legitimate &&
      v->horseMaster() == this) {
      int check = MountEgoCheck(this, v);

      // mounts are not gung-ho supporters of their riders
      if (orderDenyCheck(cmd_buf))
        check = 3;   // force indifference

      // if mount has no ego (check <=0) do command
      // if mount is VERY egotistical, and rider not skilled, buck off
      if (check > 5) {
        if (!rideCheck(-5)) {
          act("$n gets pissed and bucks $N off!", FALSE, v, 0, this, TO_NOTVICT);
          act("$n gets pissed and bucks you off!", FALSE, v, 0, this, TO_VICT);
          dismount(POSITION_SITTING);
          messSent = true;
        }
      } else if (check <= 0)
        legitimate = true;
    }

    if (legitimate) {
      rc = applyOrder(this, v, message, SILENT_NO);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete v;
        v = NULL;
      }
    } else if (!messSent) {
      act("$n has an indifferent look.", FALSE, v, 0, 0, TO_ROOM);
    }
  } else {
    // This is order "followers" 
    sprintf(buf, "$n issues the order '%s'.", message);
    act(buf, FALSE, this, 0, v, TO_ROOM);

    // there is a possibility that the order would change our room
    // which might have drastic consequences, so we do this check
    org_room = in_room;

    // compare to specific-mob order above
    // horses are skipped (intentional) due to !AFF_CHARM
    // captives also skipped
    followData *k, *k2;
    for (k = followers; k; k = k2) {
      k2 = k->next;
      TBeing *kfol = k->follower;
      if (kfol && org_room == kfol->inRoom()) {
        if (kfol->isAffected(AFF_CHARM)) {
          found = TRUE;
          rc = applyOrder(this, kfol, message, SILENT_YES);
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            delete kfol;
            kfol = NULL;
          }
        }
      }
    }
    if (found)
      sendTo("Ok.\n\r");
    else {
      sendTo("Nobody here is a loyal subject of yours!\n\r");
      return FALSE;
    }
  }
  doSave(SILENT_YES);
  return TRUE;
}

static bool canFleeThisWay(TBeing *ch, dirTypeT dir)
{
  if (!ch->canGo(dir))
    return false;

  TRoom *rp2 = real_roomp(ch->exitDir(dir)->to_room);
  if (!rp2)
    return false;

  if ((rp2->isAirSector() || rp2->isVertSector()) && !ch->isFlying())
    return false;

#if 0
     // this is dumb
  if (rp2->isUnderwaterSector() && !ch->isAffected(AFF_WATERBREATH))
    return false;
#endif

  if (ch->isAquatic() && 
      !(rp2->isWaterSector() || rp2->isUnderwaterSector()))
    return false;

  if (rp2->getMoblim() && MobCountInRoom(rp2->getStuff()) >= rp2->getMoblim())
    return false;

#if 0
  // out of the frying pan, into the fire...
  // this should be allowed, that's the way it worked in original DIKU
  // and us old timers just knew to turn off wimpy near DTs 
  if (rp2->isRoomFlag(ROOM_DEATH))
    return false;
#endif

  return true;
}

static void fleeFail(const TBeing *ch)
{
#if 0
  // can fail for lack of moves or too tall for door...
  ch->sendTo("You try to flee but are too exhausted.\n\r");
  act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
#else
  ch->sendTo("You try to flee but fail.\n\r");
  act("$n tries to flee!", TRUE, ch, 0, 0, TO_ROOM);
#endif
}

// returns DELETE_THIS
int TBeing::doFlee(const char *arg)
{
  int i, iDie, percent;
  bool panic = FALSE;
  int rc = FALSE;
  TRoom *rp = roomp;
  TThing *t;
  TBeing *vict;

  spellNumT skill = getSkillNum(SKILL_RETREAT);
  spellNumT skill2 = getSkillNum(SKILL_RIDE);

  // automatic flee attempts should fail if lagging
  // remember that wait comes in as 1 naturaly i think
  if (getWait() > 1) {
    sendTo("You can't flee while orienting yourself.\n\r");
    return TRUE;
  }
  // This is used to prevent 'insta-fleeing' on mobiles
  if (isAffected(AFF_SHOCKED) && !isPc()) {
    sendTo("You are still recovering from something!\n\r");
    return FALSE;
  } if (!isPc()) {
    affectedData tAff;

    tAff.type     = AFFECT_COMBAT;
    tAff.duration = 1; // Short and sweet
    tAff.modifier = AFF_SHOCKED;
    tAff.location = APPLY_NONE;

    affectTo(&tAff, -1);
  }

  if (isAffected(AFF_PARALYSIS)) {
    sendTo("You can't flee while paralyzed.\n\r");
    return FALSE;
  }
  if (isAffected(AFF_STUNNED)) {
    sendTo("You can't flee while stunned.\n\r");
    return FALSE;
  }
  if (getPosition() <= POSITION_STUNNED)
    return FALSE;

  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You are berserking!  There is no way to flee!\n\r");
    return FALSE;
  }
  if (bothLegsHurt() && !isFlying()) {
    sendTo("It is futile to try to flee without use of your legs!\n\r");
    act("$n tries to flee, but $s legs just won't move!", TRUE, this, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  if (eitherLegHurt() && ::number(0,2) && !isFlying()) {
    sendTo("Your injured leg makes the attempt fail!\n\r");
    act("$n tries to flee, but $s legs just won't move!", 
         TRUE, this, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  if (!isHumanoid() && eitherArmHurt() && ::number(0,2) && !isFlying()) {
    // non-humanoid arms are considered legs
    sendTo("Your injured leg makes the attempt fail!\n\r");
    act("$n tries to flee, but $s legs just won't move!", 
         TRUE, this, NULL, NULL, TO_ROOM);
    return FALSE;
  }
  if (isAffected(AFF_CHARM) && master && sameRoom(*master)) {
    if (!::number(0, 5))
      act("$n bursts into tears.", TRUE, this, 0, 0, TO_ROOM);
    act("You burst into tears at the thought of leaving $N.", 
            FALSE, this, 0, master, TO_CHAR);
    return FALSE;
  }
  if (roomp->isRoomFlag(ROOM_NO_FLEE)) {
    act("A strange power prevents $n from escaping.",
            FALSE, this, NULL, NULL, TO_ROOM);
    act("A strange power prevents you from escaping.",
            FALSE, this, NULL, NULL, TO_CHAR, ANSI_RED);
    return FALSE;
  }
  if (riding) {
    // Let's make retreat skill have some chance of keeping you on the mount - Brutius 07/26/1999
    if (!(doesKnowSkill(skill) && doesKnowSkill(skill2)) || 
	!(bSuccess(skill) && bSuccess(skill2))) { 
      sendTo("Your panic causes you to fall.\n\r");
      rc = fallOffMount(riding, POSITION_SITTING);
      panic = TRUE;
      if (IS_SET_DELETE(rc, DELETE_THIS)) 
        return DELETE_THIS;
    }
  } else if (getPosition() <= POSITION_SITTING) {
    addToMove(-10);
    act("$n scrambles madly to $s feet!", TRUE, this, 0, 0, TO_ROOM);
    act("Panic-stricken, you scramble to your feet.", TRUE, this, 0, 0, TO_CHAR);
    doStand();
    addToWait(combatRound(1));

    panic = TRUE;
  } else if ((t = rider)) {
    t->sendTo("Your mount panics and attempts to flee.\n\r");
    rc = t->fallOffMount(this, POSITION_SITTING);
    panic = TRUE;
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      TBeing *tbt = dynamic_cast<TBeing *>(t);
      if (tbt)
        tbt->reformGroup();
      delete t;
      t = NULL;
    }
  }
   
  
  dirTypeT chosenDir = getDirFromChar(arg);

  soundNumT snd = pickRandSound(SOUND_FLEE_01, SOUND_FLEE_03);
  playsound(snd, SOUND_TYPE_COMBAT);

  if (!(vict = fight())) {
    for (i = 0; i < 20; i++) {
      dirTypeT attempt = dirTypeT(::number(MIN_DIR, MAX_DIR-1));        // Select a random direction 
      
      // not fighting, so encourage flight to be in dir PC selected
      if (chosenDir != DIR_NONE && !panic) {
	attempt = chosenDir;
      }
      if (canFleeThisWay(this, attempt)) {
        act("$n panics, and attempts to flee.", TRUE, this, 0, 0, TO_ROOM);
        TBeing *tbt = dynamic_cast<TBeing *>(rider);
        if (tbt) {
          act("You turn tail and attempt to run away.", 
	      TRUE, this, 0, 0, TO_CHAR);
          loseSneak();
          iDie = tbt->moveOne(attempt);
          if (IS_SET_DELETE(iDie, DELETE_THIS)) {
            tbt->reformGroup();
            delete tbt;
            tbt = NULL;
            return FALSE;
          }
          if (iDie == TRUE) {
            sendTo("You fled head over heels.\n\r");
            return TRUE;
          } else {
            fleeFail(this);
            return TRUE;
          }
        } else {
          act("You turn tail and attempt to run away.", 
	      TRUE, this, 0, 0, TO_CHAR);
          loseSneak();
          iDie = moveOne(attempt);
          if (IS_SET_DELETE(iDie, DELETE_THIS))
            return DELETE_THIS;
	  
          if (iDie == TRUE) {
            sendTo(fmt("You nearly hurt yourself as you fled madly %swards.\n\r") % dirs[attempt]);
            REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
	    
            return TRUE;
          } else {
            fleeFail(this);
            return TRUE;
          }
        }
      }
    }
    // No exits was found 
    sendTo("PANIC! You couldn't escape!\n\r");
    return TRUE;
  }
  for (i = 0; i < 20; i++) {
    dirTypeT attempt = dirTypeT(::number(MIN_DIR, MAX_DIR-1));        // Select a random direction 
    
    // fighting, so give slight chance of letting PC direct the direction
    if (chosenDir != DIR_NONE && !panic && !::number(0,99) < (30+getSkillValue(skill)/2))
      attempt = chosenDir;
    
    if (canFleeThisWay(this, attempt)) {
      if (panic || !doesKnowSkill(skill) || 
          !bSuccess(skill)) {
        act("$n panics, and attempts to flee.", TRUE, this, 0, 0, TO_ROOM);
        panic = TRUE;
      } else {
        act("$n skillfully retreats from battle", TRUE, this, 0, 0, TO_ROOM);
        panic = FALSE;
      }
      loseSneak();
      TBeing *tbt = dynamic_cast<TBeing *>(rider);
      if (tbt) {
        iDie = tbt->moveOne(attempt);
        if (IS_SET_DELETE(iDie, DELETE_THIS)) {
          tbt->reformGroup();
          delete tbt;
          tbt = NULL;
          return FALSE;
        }
      } else {
        iDie = moveOne(attempt);
        if (IS_SET_DELETE(iDie, DELETE_THIS))
          return DELETE_THIS;
      }
      if (iDie == 1) {
        double lose;
        if (GetMaxLevel() > 3) {
          // IDEALLY:
          // get agg'd by a trainer and flee = lose minimal
          // attack super high lev and flee for xp = lose a lot
          // flee from lower level = lose moderate
          if (vict->GetMaxLevel() > GetMaxLevel()) {
            if (isAffected(AFF_AGGRESSOR)) {
              // flee from higher level that I attacked...
              lose = 3 * mob_exp(GetMaxLevel());
            } else {
              // flee from higher level that attacked me...
              lose = mob_exp(GetMaxLevel());
              lose /= 4.0;
            }
          } else {
            // flee from lower level...
            // We'll use vict's XP so you can't lose more than mob was worth.
            lose = mob_exp(vict->GetMaxLevel());
          }
           // Tone this way down, we're making a lot of changes and
          // this is drawing complaints - Bat 12/98
          lose /= 1000;
        } else
          lose = 0;

        if (lose < 0)
          lose = 1;

        // a crash was here somehow, i rearranged a bit but net was
        // dynamic_cast was NULL, but !isPc occurred (tmons = 0x0 in addFeared)
        // bizarre!! bat 09-19-98
        TMonster *tmons = dynamic_cast<TMonster *>(this);
        if (!isPc() && tmons)
          tmons->addFeared(vict);

        if (!(vict->isPc())) {
          // I'm fleeing, make my opponent hunt me
          tmons = dynamic_cast<TMonster *>(vict);
          percent = (int) (100 *(double) tmons->getHit() /
           (double) tmons->hitLimit());
          if (::number(1, 101) < percent) {
            if (tmons->Hates(this, NULL) ||
                isOppositeFaction(tmons)) {
              tmons->setHunting(this);
            }
          }
        }
        if (isPc() && panic) {
          if (hasClass(CLASS_MONK) || hasClass(CLASS_WARRIOR) ||
              hasClass(CLASS_DEIKHAN) || hasClass(CLASS_RANGER))
            addToExp(-min(lose, getExp()));
        }
        if (panic)
          sendTo(fmt("Panic-stricken, you flee %s.\n\r") % dirs[attempt]);
        else
          sendTo(fmt("You retreat skillfully %s.\n\r") % dirs[attempt]);

        // do this before lookForEngaged to get attackers check to work OK
        if (fight())
          stopFighting();

        TThing *t2;
        for (t = rp->getStuff(); t; t = t2) {
          t2 = t->nextThing;
          TBeing *tbt = dynamic_cast<TBeing *>(t);
          if (!tbt)
            continue;
          if (tbt->fight() == this) {
            tbt->stopFighting();

            TMonster *tmon = dynamic_cast<TMonster *>(tbt);
            if (tmon) {
              rc = tmon->lookForEngaged(this);
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                delete tmon;
                tmon = NULL;
              }
              continue;
            }
          }
        }
        if (cantHit > 0)
          cantHit = 0;

        REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);

        return TRUE;
      } else {
        fleeFail(this);
        return TRUE;
      }
    }
  }                                // for 
  sendTo("PANIC! You couldn't escape!\n\r");
  return FALSE;
}


// return DELETE_THIS if this should die
int TBeing::doAssist(const char *argument, TBeing *vict, bool flags)
{
  TBeing *v = NULL, *tmp_ch = NULL;
  char v_name[240];
  int rc = FALSE;

  if (checkPeaceful("No one should need assistance here.\n\r"))
    return FALSE;

  strcpy(v_name, argument);

  if (!(v = vict)) {
    // Default to the first PC group member who is currently tanking, or the first mobile group member if no PC is.
    if (isAffected(AFF_GROUP) && (!argument || !*argument) && (followers || (master && master->followers))) {
      followData * tFData    = (master ? master->followers : followers);
      TBeing     * tAssistMe = NULL,
	         * tMaybeMe  = NULL;

      if (master && master->isAffected(AFF_GROUP) && sameRoom(*master) && canSee(master) && master->fight() &&
          (master->fight()->fight() == master))
	tAssistMe = master;
      else
	for (; tFData; tFData = tFData->next)
	  if (tFData->follower && tFData->follower->isAffected(AFF_GROUP) && canSee(tFData->follower) &&
              tFData->follower->fight() && (tFData->follower->fight()->fight() == tFData->follower) &&
              (tFData->follower != this) && sameRoom(*tFData->follower)) {
	    if (tFData->follower->isPc()) {
	      tAssistMe = tFData->follower;
	      break;
	    } else if (!tMaybeMe)
	      tMaybeMe = tFData->follower;
	  }

      if (!tAssistMe && tMaybeMe)
	tAssistMe = tMaybeMe;

      v = tAssistMe; // Since this is NULL'ed above we are safe doing this without a check.
    }

    if (!v)
      if (!(v = get_char_room_vis(this, v_name, NULL, EXACT_NO, INFRA_YES))) {
        sendTo("Whom do you want to assist?\n\r");
        return FALSE;
      }
  }
  if (v == this) {
    sendTo("Oh, by all means, help yourself...\n\r");
    return FALSE;
  }
  if (dynamic_cast<TMonster *>(v) && (v->master != this) && !flags) {
    sendTo("You can't assist non-charmed monsters sorry.\n\r");
    return FALSE;
  }
  if (fight() == v) {
    sendTo("That would be counterproductive?\n\r");
    return FALSE;
  }
  if (fight()) {
    sendTo("You have your hands full right now.\n\r");
    return FALSE;
  }
  if ((v->attackers >= MAX_COMBAT_ATTACKERS) && !flags) {
    sendTo("You can't get close enough to them to assist!\n\r");
    return FALSE;
  }
  if (!(tmp_ch = v->fight())) {
    act("But $E's not fighting anyone.", FALSE, this, 0, v, TO_CHAR);
    return FALSE;
  }
  if (isAffected(AFF_CHARM) && (master == v->fight())) {
    act("You can't assist $N against your beloved master.", 
            FALSE, this, 0, v, TO_CHAR);
    return FALSE;
  } 
// See if they have chosen to be an engager or if they are currently engaged
  if (checkEngagementStatus()) {
    act("You turn to face $N's enemy.",
        TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
    act("$n assists you.",
        TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
    act("$n turns to face you in battle.",
        TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
    act("$n turns to face $N's enemy in battle.",
        TRUE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
    doEngage("", v->fight());
    return TRUE;
  }

  // this checks to verify an attack could actually happen.
  // i.e. they need to be wielding a weapon or be barehanded
  if ((equipment[HOLD_LEFT] && equipment[HOLD_RIGHT] &&
      !dynamic_cast<TBaseWeapon *>(equipment[HOLD_LEFT]) &&
      !dynamic_cast<TBaseWeapon *>(equipment[HOLD_RIGHT])) ||
      !equipment[HOLD_LEFT] || !equipment[HOLD_RIGHT]) {
  } else {
    reconcileHelp(v, 0.01);
  }

  // This should be difficult, considering what they are trying to do.  But even blind mobs should have a chance
  // to assist in due situations, but not a guaranteed assist.  This also applies to any situation in which a mobile
  // wanting to assist can not see the person in trouble and/or the aggressor.   -Lapsos
  if ((!canSee(v) || !canSee(tmp_ch)) && ((::number(-100, getStat(STAT_CURRENT, STAT_FOC)) < 0) || (::number(-100, getStat(STAT_CURRENT, STAT_PER)) < 0))) {
    act("You try to assist $N but you can not see well enough to join the fray.", TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
    act("$n fails to assist you, look at them stumble around.", TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
    act("$n tried to attack you but their inability to see clearly stops them.", TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
    act("$n tries to assist against $N, but fails.", TRUE, this, 0, tmp_ch, TO_NOTVICT, ANSI_BLUE);
    return FALSE;
  }
  
  switch (::number(1,6)) {
    case 1:
      act("You jump into the battle attacking $N's enemy.", 
           TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n jumps into the battle, assisting you.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n jumps into the battle, attacking you.", 
           TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
      act("$n jumps into the battle, attacking $N.", 
           TRUE, this, 0, tmp_ch, TO_NOTVICT, ANSI_BLUE);
      break;
    case 2:
      act("You leap to the aid of $N.", 
           TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n leaps into the fray, assisting you.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n leaps into the fray, attacking you.", 
           TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
      act("$n leaps into the fray, attacking $N.", 
           TRUE, this, 0, tmp_ch, TO_NOTVICT, ANSI_BLUE);
      break;
    case 3:
      act("You add the strength of your arms to $N's cause.", 
           TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n joins the fray, assisting you.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n joins the fray, attacking you.", 
           TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
      act("$n joins the battle attacking $N.", 
           TRUE, this, 0, tmp_ch, TO_NOTVICT, ANSI_BLUE);
      break;
    case 4:
      act("You charge ahead in order to assist $N in battle.", 
           TRUE, this, 0, v,  TO_CHAR, ANSI_ORANGE);      
      act("$n charges into the fray, assisting you.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n charges into the fray, attacking you.", 
           TRUE, this, 0, tmp_ch, TO_VICT, ANSI_RED);
      act("$n charges into the battle and begins to attack $N.", 
           TRUE, this, 0, tmp_ch, TO_NOTVICT, ANSI_BLUE);
      break;
    case 5:
      act("You join forces with $N in battle.", 
           TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n joins forces with you against your enemy.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n joins forces with $N in battle coming to $S aid.", 
           TRUE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
      break;
    default:
      act("You come to the assistance of $N in $S time of need.", 
           TRUE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n assists you against your enemy.", 
           TRUE, this, 0, v, TO_VICT, ANSI_GREEN);
      act("$n enters the battle coming to the assistance of $N.", 
           TRUE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
      break;
  }

  rc = hit(tmp_ch);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    tmp_ch->reformGroup();
    delete tmp_ch;
    tmp_ch = NULL;
  }

  if (tmp_ch && !fight()) {
    // force the fight to occur.  Hit may abort for variety of reasons
    // or, hitter may have too few attacks that round (speed).
    setCharFighting(tmp_ch);
  }
#if 0
  // trap a problem report
  if (!fight())
    vlogf(LOG_BUG, fmt("%s not fighting after assist %s") %  getName() % v->getName());
#endif

  return rc;
}

// return DELETE_THIS
// return FALSE if item avoided flame, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::burnObject(TBeing *ch, int perc)
{
  int rc = 0;
  TThing *t, *t2;
  char buf[256];

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  if (getMaxStructPoints() < 0)
      return FALSE;
  if (getStructPoints() < 0)
    return FALSE;
  if (::number(1,100) > perc)
    return FALSE;
  if (dynamic_cast<TSpellBag *>(this))
    return FALSE;
  if (dynamic_cast<TBag *>(this))
    return FALSE;

  if (ch && (ch == equippedBy) &&
      material_nums[getMaterial()].conductivity) {
    int dam = 1 + getVolume() / 2000;
    if ((dam = ch->getActualDamage(ch, 0, dam, DAMAGE_FIRE)))
      act("Your $o turns red hot and burns you!",TRUE,ch,this,0,TO_CHAR);
    else
      act("Your $o turns red hot.",TRUE,ch,this,0,TO_CHAR);
    rc = ch->applyDamage(ch,dam,DAMAGE_FIRE);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_VICT;
    }
  }

  if(material_nums[getMaterial()].flammability<=0)
    return FALSE;

  TOpenContainer *trc = dynamic_cast<TOpenContainer *>(this);
  for (t = getStuff(); t; t = t2) {
    int perc2;
    t2 = t->nextThing;
    if (trc) {
      if (trc->isClosed())
        perc2 = 5 * perc / 100;   // 5% chance if closed
      else if (trc->isCloseable())
        perc2 = 6 * perc / 10;     // 60% chance if open and closable
      else
        perc2 = 5 * perc / 10;     // 50% chance for unclosable containers
    } else
      perc2 = 5 * perc / 10;     // 50% chance for unclosable containers
    TObj * tot = dynamic_cast<TObj *>(t);


    if (tot) {
      rc = tot->burnObject(ch, perc2);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = NULL;
      }
    }
  }



  int orig = getMaxStructPoints();
  int burndam = ::number(0,(int)((double)orig * 1.00 * 
				 ((double)(material_nums[getMaterial()].flammability))/1000.0) +1);
  burndam/=5;
  if(burndam<=0) burndam=1;

  if(material_nums[getMaterial()].flammability &&
     burndam > getStructPoints()){
    // destroyed
    setStructPoints(0);
    if (ch) {
      sprintf(buf, "Your $o $q consumed in <r>flame<1>.");
      act(buf,TRUE,ch,this,0,TO_CHAR, ANSI_YELLOW);
      sprintf(buf, "$n's $o $q consumed in the flames.");
      act(buf,TRUE,ch,this,0,TO_ROOM);
    } else {
      act("The flame consumes $n.",TRUE,this,0,0,TO_ROOM);
    }
    // since bag went poof, lets (re)check contents 
    // with an even higher percentage, and empty bag into owner
    while ((t = getStuff())) {
      (*t)--;
      if (parent)
	*parent += *t;
      else if (roomp)
	*roomp += *t;
      else
	vlogf(LOG_BUG, fmt("Bad struct on burnObj %s") %  t->name);
      TObj * tot = dynamic_cast<TObj *>(t);
      if (tot) {
	rc = tot->burnObject(ch, 100);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  delete tot;
	  tot = NULL;
	}
      }
    }
    return DELETE_THIS; 
  } else {
    setStructPoints(getStructPoints()-burndam);
    
    if (ch) {
      sprintf(buf, "Your $o $q singed slightly, but look$Q intact.");
      act(buf,TRUE,ch,this,0,TO_CHAR);
    } else {
      act("The flame slightly singes $n, but it looks intact.",
           TRUE,this,0,0,TO_ROOM);
    }

    //i want to give objs only a CHANCE to burn
    // flamability is usually 1-10 so if we use this as a
    // modifier
    // and give flammable objects only a 0-50% chance of burning, it should be good

    if(::number(0,100) < (int)(0.050*(double)material_nums[getMaterial()].flammability) &&
       !isObjStat(ITEM_BURNING) && !isObjStat(ITEM_PAIRED)){
      setBurning(ch);
      if(ch){
	sprintf(buf, "Your $o start$Q to burn!\a");
	act(buf,TRUE,ch,this,0,TO_CHAR);
      }
    }

    return TRUE;
  }

  return FALSE;
}

int TBeing::flameEngulfed()
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int i;
  int res = 0;

  for (i = MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t))) 
      continue;
    
    res = obj->burnObject(this, getImmunity(IMMUNE_HEAT)?1:50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    res = obj->burnObject(this, getImmunity(IMMUNE_HEAT)?1:50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  return 1;
}

// return DELETE_THIS
// return FALSE if item avoided frost, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::freezeObject(TBeing *ch, int perc)
{
  int rc = 0;
  TThing *t = NULL, *t2 = NULL;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  if (getMaxStructPoints() < 0)
      return FALSE;
  if (getStructPoints() < 0)
    return FALSE;
  if (::number(1,100) > perc)
    return FALSE;

  if (ch && (ch == equippedBy) && material_nums[getMaterial()].conductivity) {
    int dam = 1 + getVolume() / 2000;
    if ((dam = ch->getActualDamage(ch, 0, dam, SPELL_FROST_BREATH)))
      act("Your $o turns bitterly cold and freeze-burns you!",TRUE,ch,this,0,TO_CHAR);
    else
        act("Your $o turns bitterly cold.",TRUE,ch,this,0,TO_CHAR);

    rc = ch->applyDamage(ch,dam,SPELL_FROST_BREATH);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  TOpenContainer *trc = dynamic_cast<TOpenContainer *>(this);
  for (t = getStuff(); t; t = t2) {
    int perc2;
    t2 = t->nextThing;
    if (trc) {
      if (trc->isClosed())
        perc2 = 10 * perc / 100;   // 10% chance if closed
      else if (trc->isCloseable())
        perc2 = 8 * perc / 10;     // 80% chance if open and closable
      else
        perc2 = 6 * perc / 10;     // 60% chance for unclosable containers
    } else
      perc2 = 6 * perc / 10;     // 60% chance for unclosable containers

    TObj * tot = dynamic_cast<TObj *>(t);
    if (tot) {
      rc = tot->freezeObject(ch, perc2);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = NULL;
      }
    }
  }
  switch (getMaterial()) {
    case MAT_GLASS:
    case MAT_WATER:
      if (ch) {
        act("The chill freezes your $o and it shatters.",TRUE,ch,this,0,TO_CHAR, ANSI_BLUE);
      } else {
        act("The chill freezes $n and it shatters.",TRUE,this,0,0,TO_ROOM);
      }
      makeScraps();
      return DELETE_THIS;
    case MAT_MICA:
    case MAT_AMETHYST:
    case MAT_QUARTZ:
    case MAT_CRYSTAL:
      if (ch) {
        act("The chill causes your $o to freeze and crack severely.",TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
      } else {
        act("The chill causes $n to freeze and crack severely.",TRUE,this,0,0,TO_ROOM);
      }
      addToStructPoints(- ::number(1,12));
      if (getStructPoints() <= 0) {
        if (ch) {
          act("..The mineral splits and $p $q destroyed.",TRUE,ch,this,0,TO_CHAR, ANSI_GREEN);
        } else {
          act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
        }
        makeScraps();
        return DELETE_THIS;
      }
      return TRUE;
    default:
      break;
  }
  return FALSE;
}

int TBaseCup::freezeObject(TBeing *ch, int perc)
{
  // do recursive stuff first
  int rc = TObj::freezeObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;
  
  // more than 1/2 full
  if (2 * getDrinkUnits() >= getMaxDrinkUnits()) {
    char buf [256];
    if (ch) {
      sprintf(buf, "The chill causes the %s in your $o to freeze.",
           liquidInfo[getDrinkType()]->name);
      act(buf,TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
      act("$p is damaged serverely by the expanding ice!",TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
    } else {
      sprintf(buf, "The chill causes the %s in $n to freeze.",
           liquidInfo[getDrinkType()]->name);
      act(buf,TRUE,this,0,0,TO_ROOM);
      act("$n is damaged serverely by the expanding ice!",TRUE,this,0,0,TO_ROOM);
    }
    addToStructPoints(- ::number(1,12));
    if (getStructPoints() <= 0) {
      if (ch) {
        act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
      } else {
        act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
      }
      makeScraps();
      return DELETE_THIS;
    }
  }
  return FALSE;
}

// returns DELETE_THIS
int TBeing::frostEngulfed()
{
  TObj *obj = NULL;
  int i;
  int res;
  TThing *t = NULL, *t2 = NULL;
// Need to account for worn containers
  for (i = MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t)))
      continue;
    
    res = obj->freezeObject(this, getImmunity(IMMUNE_COLD)?1:50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    res = obj->freezeObject(this, getImmunity(IMMUNE_COLD)?1:50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  return 1;
}

// return DELETE_VICT if ch should be deleted
int TObj::meltObject(TBeing *ch, int perc)
{
  int rc = 0;
  int orig = getStructPoints();
  TThing *t, *t2;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  if (!canRust())
    return FALSE;
  if (::number(1,100) > perc)
    return FALSE;

  TOpenContainer *trc = dynamic_cast<TOpenContainer *>(this);
  for (t = getStuff(); t; t = t2) {
    int perc2;
    t2 = t->nextThing;
    if (trc) {
      if (trc->isClosed())
        perc2 = 10 * perc / 100;   // 10% chance if closed
      else if (trc->isCloseable())
        perc2 = 8 * perc / 10;     // 80% chance if open and closable
      else
        perc2 = 6 * perc / 10;     // 60% chance for unclosable containers
    } else
      perc2 = 6 * perc / 10;     // 60% chance for unclosable containers
    TObj * tot = dynamic_cast<TObj *>(t);
    if (tot) {
      rc = tot->meltObject(ch, max(5, perc2 - (ch ? ch->getImmunity(IMMUNE_ACID) : 0)));
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = NULL;
      }
    }
  }
  while (::number(0,101) <= material_nums[getMaterial()].acid_susc) {
    addToStructPoints(-1);
    if (::number(0,1))
      addToMaxStructPoints(-1);
    if (getStructPoints() <= 0) {
      if (ch) {
        act("Your $o $q destroyed by the corrosive acid.",
                TRUE,ch,this,0,TO_CHAR, ANSI_GREEN);
      } else {
        act("$n $r destroyed by acid.",TRUE,this,0,0,TO_ROOM);
      }
      makeScraps();
      return DELETE_THIS;
    }
  }
  if (orig != getStructPoints()) {
    if (ch) {
      act("Your $o $q corroded, but look$Q intact.",
             TRUE,ch,this,0,TO_CHAR);
    } else {
      act("$n decays slightly in the corrosive acid.",TRUE,this,NULL,0,TO_ROOM);
    }
    return TRUE;
  }
  return FALSE;
}

int TBeing::acidEngulfed()
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int i, res;

  for (i = MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t)))
      continue;
    res = obj->meltObject(this, getImmunity(IMMUNE_ACID)?1:50);
    if (IS_SET_ONLY(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    res = obj->meltObject(this, getImmunity(IMMUNE_ACID)?1:50);
    if (IS_SET_ONLY(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
  return FALSE;
}

// returns DELETE_THIS
int TBeing::lightningEngulfed()
{
  int rc;
  TObj *obj = NULL;
  TThing *t;
  int i;

  for (i=MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t)))
      continue;
    if (obj->getMaxStructPoints() < 0)
      continue;
    if (obj->getStructPoints() < 0)
      continue;
    if (material_nums[obj->getMaterial()].conductivity) {
      int dam = 1 + obj->getVolume() / 2000;
      if ((dam = getActualDamage(this, 0, dam, SPELL_LIGHTNING_BREATH)))
        act("Sparks fly from your $o, harming you!",TRUE,this,obj,0,TO_CHAR);
      else
        act("Sparks fly from your $o.",TRUE,this,obj,0,TO_CHAR);

      rc = applyDamage(this,dam,SPELL_LIGHTNING_BREATH);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }
  }
  return 1;
}

int TBeing::chlorineEngulfed()
{
  int rc;
  TThing *t;
  affectedData af;
  wearSlotT i;

  af.type = SPELL_POISON;
  af.level = 51;
  af.duration = (60) * UPDATES_PER_MUDHOUR;
  af.modifier = -20;
  af.location = APPLY_STR;
  af.bitvector = AFF_POISON;

  if (isImmune(IMMUNE_POISON)) {
  } else {
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
  }
  for (i=MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) && slotChance(i)) {
      int dam = ::number(2,11);
      if ((dam = getActualDamage(this, 0, dam, SPELL_CHLORINE_BREATH)))
        sendTo(fmt("The chlorine gas gives you a caustic burn on your %s.\n\r") %describeBodySlot(i));
      rc = applyDamage(this,dam,SPELL_CHLORINE_BREATH);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    } else if (t)
      t->poisonObject();
  }
  for (t = getStuff(); t; t = t->nextThing) 
    t->poisonObject();
  
  return TRUE;
}

void TBeing::flameRoom()
{
  TObj *obj = NULL;
  int rc;

  TThing *t, *t2;
  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    rc = obj->burnObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
  return;
}

void TBeing::freezeRoom()
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;

  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj || !obj->canWear(ITEM_TAKE))
      continue;

    rc = obj->freezeObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
  return;
}

void TBeing::acidRoom()
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;

  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (!obj->canWear(ITEM_TAKE))
      continue;
    rc = obj->meltObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
  return;
}

void TBeing::chlorineRoom()
{
  TThing *t;

  for (t = roomp->getStuff(); t; t = t->nextThing) {
    t->poisonObject();
  }
  return;
}
// if player has shield, destroy shield and "dodge"
int TBeing::shieldAbsorbDamage(int dam)
{
  TThing *left, *right;
  TBaseClothing *shield = NULL;
  wearSlotT slot = WEAR_NOWHERE;

  left = equipment[HOLD_LEFT];
  right = equipment[HOLD_RIGHT];

  TBaseClothing *tbc = NULL;
  if (left && 
      (tbc = dynamic_cast<TBaseClothing *>(left)) &&
      tbc->isShield()) {
    shield = tbc;
    slot = HOLD_LEFT;
  } else if (right &&
      (tbc = dynamic_cast<TBaseClothing *>(right)) &&
      tbc->isShield()) {
    shield = tbc;
    slot = HOLD_RIGHT;
  }
  if (!shield)
    return dam;

  // shield will absorb 10 to 20 percent of its structure
  int shielddam=(int)((shield->getMaxStructPoints()/100.0) * ::number(10,20));
  dam=max(0, dam-(shielddam*5)); // each structure point = 5 hp

  act("You hold your $o up to block the blast.",TRUE,this,shield,0,TO_CHAR);
  act("$n holds $s $o up to block the blast.",TRUE,this,shield,0,TO_ROOM);
  
  if (shielddam >= shield->getStructPoints()) {
    act("$p partially blocks the blast but is utterly destroyed at the same time.",TRUE,
         this,shield, 0,TO_CHAR);
    act("$p partially blocks the blast but is utterly destroyed at the same time.",TRUE,
         this,shield, 0,TO_ROOM);
    unequip(slot);
    delete shield;
    shield = NULL;
  } else {
    act("$p partially blocks the blast but is seriously damaged at the same time.",TRUE,
         this,shield, 0,TO_CHAR);
    act("$p partially blocks the blast but is seriously damaged at the same time.",TRUE,
         this,shield, 0,TO_ROOM);

    if (!roomp || !roomp->isRoomFlag(ROOM_ARENA))
      shield->addToStructPoints(-shielddam);
  }
  return dam;
}

bool TBeing::noHarmCheck(TBeing *vict)
{
  if (this == vict)
    return FALSE;

  if (desc && IS_SET(desc->autobits, AUTO_NOHARM)) {
    if (vict->isPc()) {
      sendTo("You have your AUTO NOHARM flag set.\n\r");
      sendTo("You must remove it before attacking another PC.\n\r");
      return TRUE;
    }
    if (vict->master == this) {
      if (vict->isPet(PETTYPE_PET)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your pets.\n\r");
        return TRUE;
      }
      if (vict->isPet(PETTYPE_CHARM)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your charms.\n\r");
        return TRUE;
      }
      if (vict->isPet(PETTYPE_THRALL)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your thralls.\n\r");
        return TRUE;
      }
    }
  }
  
  if(isPc() && vict->isPc() && !isImmortal() && !vict->isValidPkTarget(this)){
    sendTo("Your victim is not a valid PK target.\n\r");
    return TRUE;
  }
  
  if (desc && !isImmortal() && isPc() && 
          vict->desc && vict->isPc() &&
          (vict->GetMaxLevel() <= MAX_NEWBIE_LEVEL) && 
          !vict->isPlayerAction(PLR_KILLABLE) && 
      !toggleInfo[TOG_NEWBIEPK]->toggle) {
    sendTo("Your victim is a newbie and protected.\n\r");
    sendTo("If you have a problem with this newbie please see a god for action.\n\r");
    return TRUE;
  }
  return FALSE;
}

void TBeing::preKillCheck(bool rent)
{
  if (desc) {
    if (!roomp->isRoomFlag(ROOM_ARENA)) {
      if (!rent)
        saveChar(ROOM_NOWHERE);
    } else {
      saveChar(in_room);
    }
  }
}

// returns the number of attacks (roughly) the TBeing would have in rounds
// number of rounds.  Used as an input to cantHit or whatever.
int TBeing::loseRound(double rounds, bool randomize, bool check)
{
  double count;
  int num;

  if (isImmortal())
    return 0;

  float fx, fy;
  blowCount(check, fx, fy);
  count = ((fx + fy) * rounds);
  num = (int) count;

  if (randomize) {
    if (::number(0,999) < (int) ((count - (double) num) * 1000.0))
      num++;
  } else {
    // rounded up
    if (count - num)
      num++;
  }

  return num;
}

void TBeing::blowCount(bool check, float &fx, float &fy)
{
  float num;
  TThing *prim, *sec;
  TGun *gun;

  prim = heldInPrimHand();
  sec  = heldInSecHand();

  if (!isPc()) {
    // all mobs

    num = min(12.0, getMult());

    if (hasClass(CLASS_MONK) && (prim || sec))
      num = (float) 1.000;

    // cut attacks in half if using paired weapon
    // otherwise they will get double attacks
    TObj *tob = dynamic_cast<TObj *>(prim);
    if (tob && tob->isPaired())
      num /= 2.0;

    fx = 0.60 * num;
    fy = 0.40 * num;

    if((gun=dynamic_cast<TGun *>(prim))){
      fx = gun->getROF();
    }
    if((gun=dynamic_cast<TGun *>(sec)) && !gun->isPaired()){
      fy = gun->getROF();
    }
    

    // MOBS
  } else if (hasClass(CLASS_MONK)) {
    // Monk PCS

    num = getMult();
    fx=fy=0;

    if (!prim)
      fx += (0.60 * num);
    else {
      fx = prim->blowCountSplitter(this, true);
      if (getPosition() >= POSITION_STANDING) {
        prim->specializationCheck(this, &fx);
      }

      if((gun=dynamic_cast<TGun *>(prim))){
	fx = gun->getROF();
      }
    }



    if (!sec)
      fy += (0.40 * num);
    else if (sec != prim) {
      fy = sec->blowCountSplitter(this, false);
      if((gun=dynamic_cast<TGun *>(sec)) && !gun->isPaired()){
	fy = gun->getROF();
      }
    } else
      // don't give paired weapons extra hits
      fy = 0.00;

#if 0
    // kluge for barehand specialization check
    // can't use TWeapon::SpecializationCheck for obvious reasons
    if(!prim &&
       getPosition() >= POSITION_STANDING &&
       getDiscipline(DISC_BAREHAND)) {
      num = ::number(0,99);
      if (num < getSkillValue(SKILL_BAREHAND_SPEC) - 20 * drunkMinus())
	fx += (float) 1.0;
    }
    // specialization check
    if (prim && getPosition() >= POSITION_STANDING) {
      prim->specializationCheck(this, &fx);
    }
#endif

#if 0
// speed adjusted when we setMult
    // speed
    // from balance note, want high speed to hit 5/4 more, and low to hit 4/5
    // as much
    if (fx > 0.0)
      fx *= getSpeMod();
    //  this is the same - note we DIVIDE instead of multiply since we want a lower number as the bonus
    //  fx = fx * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);
    if (fy > 0.0)
      fy *= getSpeMod();
    //  ditto above - dash
    //  fy = fy * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);
#endif

    // Monk PCS
  } else {
    // NON-monk PCS

    TObj *tobj = dynamic_cast<TObj *>(prim);
    if (tobj && !tobj->isPaired() && !dynamic_cast<TBaseWeapon *>(tobj) && !check) 
      // no attacks for non-weapons
      fx = (float) 0.0;
    else {
      // generic 1 attack
      fx = (float) 1.0;
    }

    if((gun=dynamic_cast<TGun *>(prim))){
      fx = gun->getROF();
    }

    tobj = dynamic_cast<TObj *>(sec);
    if (tobj && !tobj->isPaired() && !dynamic_cast<TBaseWeapon *>(tobj) && !check) 
      fy = (float) 0.0;
    else if (sec && (sec == prim)) {
     // Don't give two handed weapons two hits - Russ 122797
      fy = (float) 0.0;
    } else {
      // generic 1 attack
      fy = (float) 1.0;
    }

    if((gun=dynamic_cast<TGun *>(sec)) && !gun->isPaired()){
      fy = gun->getROF();
    }

    // specialization check
    if (prim && getPosition() >= POSITION_STANDING) {
      prim->specializationCheck(this, &fx);
    }

    // berzerk
    if (isCombatMode(ATTACK_BERSERK) && 
        getPosition() >= POSITION_STANDING) {
      if (bSuccess(SKILL_BERSERK)) {
        if (fx > 0.0)
          fx += 0.5;
        if (fy > 0.0)
	  fy += 0.5;
      }
    }

    // speed
    // from balance note, want high speed to hit 5/4 more, and low to hit 4/5
    // as much
    if (fx > 0.0)
      fx *= getSpeMod();
    //  this is the same - note we DIVIDE instead of multiply since we want a lower number as the bonus
    //  fx = fx * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);
    if (fy > 0.0)
      fy *= getSpeMod();
    //  ditto above - dash
    //  fy = fy * plotStat(STAT_CURRENT, STAT_SPE, 0.8, 1.25, 1.0);

    // non-monk PCs
  }


  // haste
  if (affectedBySpell(SPELL_HASTE) && getPosition() >= POSITION_STANDING) {
    if (fx > 0.0)
      fx += 0.5;
    if (fy > 0.0)
      fy += 0.5;
  }
  if (affectedBySpell(SPELL_CELERITE) && getPosition() >= POSITION_STANDING) {
    if (fx > 0.0)
      fx += 0.5;
    if (fy > 0.0)
      fy += 0.5;
  }

  // a penalty for attacking while on horseback
  if (dynamic_cast<TBeing *>(riding)) {
    // don't factor in chiv here, the +tohit and AC bonuses are set to
    // counterbalance it already
    float factor = 0.67;  // lose 1/3 of attacks

    fx *= factor;
    fy *= factor;
  }

#if 0
  // artificial increase if only 1 handed
  if (isPc() && ((fx + fy) < 1.00)) {
    if (fx >= fy)
      fx = 1.00 - fy;
    else
      fy = 1.00 - fx;
  }
#endif
}
