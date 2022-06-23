///////////////////////////////////////////////////////////////////////////
//
//     SneezyMUD++ - All rights reserved, SneezyMUD++ Coding Team
//
//     "offense.cc" - All offensive functions and routines
//
///////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "games.h"
#include "combat.h"
#include "obj_spellbag.h"
#include "materials.h"
#include "obj_open_container.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "obj_base_cup.h"
#include "obj_pool.h"
#include "obj_bag.h"
#include "liquids.h"
#include "weather.h"

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

    // trigger specials for starting a fight
    if ((rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, NULL, NULL)))
      return rc;
    if ((rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, NULL, NULL)))
      return rc;

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

            // trigger specials for starting a fight (switching counts as a new attack)
            if ((rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, NULL, NULL)))
              return rc;
            if ((rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, NULL, NULL)))
              return rc;

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
    vlogf(LOG_BUG,format("DoEngagedHit called without pc (%s) fighting") %  getName());
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

    // trigger specials for starting a fight
    int rc = 0;
    if (fight() != victim && (rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, NULL, NULL)))
      return rc;
    if (fight() != victim && (rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, NULL, NULL)))
      return rc;

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
      if (dynamic_cast<TMonster*>(v))
        dynamic_cast<TMonster*>(v)->checkResponses(this, 0, "", CMD_RESP_KILLED);
      v->rawKill(DAMAGE_NORMAL, this);
      if (vict)
        return DELETE_VICT;
      v->reformGroup();
      delete v;
      v = NULL;
    } else {
      sendTo(COLOR_MOBS, format("You can't kill %s!\n\r") % v->getName());
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
    vlogf(LOG_SILENT, format("%s ordering %s to '%s' at %d") % 
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
      is_abbrev(cmd_buf, "whirlwind") ||
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
  char buf[512];
  int found = FALSE;
  int org_room, i;
  char cmd_buf[40];
  int rc;

  if (applySoundproof())
    return FALSE;

  if(hasQuestBit(TOG_IS_MUTE)){
    sendTo("It's hard to order people around when you can't talk.");
    return FALSE;
  }

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
    if (!v->desc || !v->desc->ignored.isIgnored(desc))
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
    sstring garbled = garble(NULL, format("$n issues the order '%s'.") % message, Garble::SPEECH_SAY);

    // there is a possibility that the order would change our room
    // which might have drastic consequences, so we do this check
    org_room = in_room;

    // compare to specific-mob order above
    // horses are skipped (intentional) due to !AFF_CHARM
    // captives also skipped
    // we enumerate per room and message individually to avoid players issung 'orders' to subvert speech
    for(StuffIter it= roomp->stuff.begin();it!= roomp->stuff.end();) {
      TBeing *kfol = dynamic_cast<TBeing *>(*it++);
      if (!kfol || kfol == this || org_room != kfol->inRoom())
        continue;
      if (kfol->desc && kfol->desc->ignored.isIgnored(desc))
        continue;
      if (!kfol->isAffected(AFF_CHARM)) {
        act(garbled.c_str(), FALSE, this, 0, kfol, TO_VICT);
        continue;
      }

      found = TRUE;
      rc = applyOrder(this, kfol, message, SILENT_YES);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete kfol;
        kfol = NULL;
        break;
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

  if (ch->isAquatic() && !ch->isPc() &&
      !(rp2->isWaterSector() || rp2->isUnderwaterSector()))
    return false;

  if (rp2->getMoblim() && MobCountInRoom(rp2->stuff) >= rp2->getMoblim())
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

// returns DELETE_THIS
int TBeing::doFlee(const char *arg) {
  // automatic flee attempts should fail if lagging
  // remember that wait comes in as 1 naturaly i think
  if (getWait() > 1) {
    sendTo("You're too disoriented to flee.\n\r");
    return true;
  }

  // This is used to prevent 'insta-fleeing' on mobiles
  if (!isPc()) {
    if (isAffected(AFF_SHOCKED)) {
      sendTo("You are still recovering from something!\n\r");
      return false;
    }
    affectedData tAff;
    tAff.type = AFFECT_COMBAT;
    tAff.duration = 1;
    tAff.modifier = AFF_SHOCKED;
    tAff.location = APPLY_NONE;
    affectTo(&tAff, -1);
  }

  if (isAffected(AFF_PARALYSIS)) {
    sendTo("You can't flee while paralyzed.\n\r");
    return false;
  }

  if (isAffected(AFF_STUNNED)) {
    sendTo("You can't flee while stunned.\n\r");
    return false;
  }

  if (getPosition() <= POSITION_STUNNED)
    return false;

  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("Flee? FLEE!? Your rage is too overwhelming!!\n\r");
    return false;
  }

  if (!isFlying()) {
    if (bothLegsHurt()) {
      sendTo("You try to flee, but can't due to the condition of your legs!\n\r");
      act("$n tries to flee, but $s legs just won't move!", true, this, nullptr, nullptr, TO_ROOM);
      return false;
    }
    if ((eitherLegHurt() || (!isHumanoid() && eitherArmHurt())) && ::number(0, 2)) {
      sendTo("You try to flee, but can't due to the condition of your legs!\n\r");
      act("$n tries to flee, but $s leg just won't move!", true, this, nullptr, nullptr, TO_ROOM);
      return false;
    }
  }

  if (isAffected(AFF_CHARM) && master && sameRoom(*master)) {
    if (!::number(0, 5))
      act("$n bursts into tears at the thought of fleeing without $N!", true, this, nullptr, master,
          TO_ROOM);
    act("You can't bear the thought of leaving $N's side!", false, this, nullptr, master, TO_CHAR);
    return false;
  }

  if (roomp->isRoomFlag(ROOM_NO_FLEE)) {
    act("$n tries to flee but a strange power prevents $m from escaping.", false, this, nullptr,
        nullptr, TO_ROOM);
    act("A strange power prevents you from escaping.", false, this, nullptr, nullptr, TO_CHAR,
        ANSI_RED);
    return false;
  }

  
  // Do skill checks now and use results throughout so bSuccess only gets
  // called once per flee attempt. 
  bool wasRetreatSuccessful = bSuccess(getSkillNum(SKILL_RETREAT));

  bool panic = false;
  int rc = 0;
  auto *riderAsTBeing = dynamic_cast<TBeing *>(rider);

  // Could be either riding an actual mount, or be on a chair/bed of some sort.
  // Handle both possibilities. Failures here result in flee fail and return from function call.
  if (riding) {
    // If this resolves to nullptr we know 'riding' is a TObj (furniture)
    auto* ridingAsTBeing = dynamic_cast<TBeing*>(riding);

    // Condensed logic using ternaries. Determine if <this> is riding an actual mount or simply on
    // some furniture, then build some strings to send to act(). Avoids a bunch of nested ifs and
    // calling act repeatedly.
    bool shouldFallOffMount = ridingAsTBeing ? ((!bSuccess(SKILL_RIDE) || !wasRetreatSuccessful) &&
                                                !bSuccess(ridingAsTBeing->mountSkillType()))
                                             : !wasRetreatSuccessful;

    sstring toChar =
      ridingAsTBeing
        ? (shouldFallOffMount ? "You urge your $O to flee, causing it to panic and throw you off!"
                              : "")
        : (shouldFallOffMount ? "Lurching backwards, you attempt to escape!"
                              : "You smoothly stand up while looking for available exits.");

    sstring toRoom =
      ridingAsTBeing
        ? (shouldFallOffMount ? "$n urges $s $O to flee, causing it to panic and throw $n off!"
                              : "")
        : (shouldFallOffMount ? "$n lurches backwards, attempting to escape!"
                              : "$n stands up and conspicuously looks for an exit.");

    if (!toChar.empty())
      act(toChar, true, this, nullptr, riding, TO_CHAR);
    if (!toRoom.empty())
      act(toRoom, true, this, nullptr, riding, TO_ROOM);    

    if (shouldFallOffMount) {
      rc = fallOffMount(riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      addToWait(combatRound(1));
      return true;
    }

    // Have to use dismount here, as just forcing a standing position via setPosition actually sets
    // <this> to standing on top of whatever furniture they were on. Collapses chairs, etc.
    if (!ridingAsTBeing)
      dismount(POSITION_STANDING);

    // If <this> is a mount being ridden
  } else if (riderAsTBeing) {
    act("Your $O panics and attempts to flee!", true, riderAsTBeing, nullptr, this, TO_CHAR);
    act("$n's $O panics and attempts to flee!", true, riderAsTBeing, nullptr, this, TO_ROOM);

    // Give Deikhans a chance to prevent being bucked, with the right skills.
    if (riderAsTBeing->bSuccess(mountSkillType()) && riderAsTBeing->bSuccess(SKILL_CALM_MOUNT)) {
      act("You manage to calm $M down before $E dismounts you.", true, riderAsTBeing, nullptr, this,
          TO_CHAR);
      act("With soothing words $n manages to calm $N and remain mounted.", true,
          riderAsTBeing, nullptr, this, TO_ROOM);
      return true;
    }
    
    rc = riderAsTBeing->fallOffMount(this, POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      riderAsTBeing->reformGroup();
      delete riderAsTBeing;
    }
    // Don't want reference to rider anymore if they fell off
    riderAsTBeing = nullptr;
  }

  if (getPosition() <= POSITION_SITTING) {
    addToMove(-10);   

    // If retreat succeeded, simply being on the ground when fleeing doesn't
    // result in panic. And you get a different message.
    sstring toRoom = wasRetreatSuccessful
                       ? "$n rolls aside and regains $s footing!"
                       : "$n scrambles madly to $s feet!";
    sstring toChar = wasRetreatSuccessful
                       ? "You quickly roll aside and regain your footing!"
                       : "You scramble madly to your feet!";

    act(toRoom, true, this, nullptr, nullptr, TO_ROOM);
    act(toChar, true, this, nullptr, nullptr, TO_CHAR);

    setPosition(POSITION_STANDING);

    // If retreat didn't succeed or char doesn't have it, add some lag
    // and panic.
    if (!wasRetreatSuccessful) {
      addToWait(combatRound(1));
      panic = true;
    }
  }

  playsound(pickRandSound(SOUND_FLEE_01, SOUND_FLEE_03), SOUND_TYPE_COMBAT);

  dirTypeT chosenDir = getDirFromChar(arg);

  /*
    Whether retreating or fleeing, if they input a direction, check if it's a valid
    dir and return if not. If they've chosen a valid direction, actually do the
    checks to see if they succeed in fleeing that way further down.
  */
  if (chosenDir != DIR_NONE && !canFleeThisWay(this, chosenDir)) {
    act("You can't escape in that direction!", true, this, nullptr, nullptr, TO_CHAR);
    return true;
  }

  /*
    Create vector containing all currently valid flee directions, except for
    the chosen direction if one was given.
  */
  std::vector<dirTypeT> validDirections;
  for (dirTypeT direction = MIN_DIR; direction < MAX_DIR; direction++) {
    if (direction == chosenDir) continue;
    if (canFleeThisWay(this, direction))
      validDirections.push_back(direction);
  }

  /*
    If there are no valid flee directions (doors closed, etc), simply return with the panic
    message.
  */
  if (validDirections.empty() && chosenDir == DIR_NONE) {
    sendTo("PANIC! You couldn't escape!\n\r");
    return true;
  }

  /*
    Randomly select a valid flee direction for later use. If chosenDir is
    the only valid flee direction, just use it as randomDir. Otherwise choose
    any valid direction that's *not* chosenDir.
  */
  dirTypeT randomDir = validDirections.empty()
                           ? chosenDir
                           : validDirections[::number(0, validDirections.size() - 1)];

  // Save reference to who <this> was fighting for later
  TBeing *enemy = fight();

  /*
    Determine which direction to actually use. If panicked, no flee direction
    chosen, or fighting and both retreat and 50% chance fail then flee in random direction.
    Otherwise flee in chosen direction.
  */
  dirTypeT dirToUse =
    panic || chosenDir == DIR_NONE || (!wasRetreatSuccessful && enemy && ::number(0, 1))
      ? randomDir
      : chosenDir;

  // These messages don't make sense if not fighting
  if (enemy) {
    if (wasRetreatSuccessful && !panic) {
      act("$n is looking for an opening to escape!", true, this, nullptr, nullptr, TO_ROOM);
      act("You look for an opening to escape from the fight.", true, this, nullptr, nullptr,
          TO_CHAR);
    } else {
      act("$n panics and attempts to flee!", true, this, nullptr, nullptr, TO_ROOM);
      act("You turn tail and attempt to run away.", true, this, nullptr, nullptr, TO_CHAR);
      loseSneak();

      // Handle troglodyte racial
      if (panic && !::number(0, 1) && getMyRace()->hasTalent(TALENT_MUSK) && getCond(FULL) > 5) {
        act("Your fear causes you to release some musk scent to cover your tracks.", false, this, nullptr,
            nullptr, TO_CHAR);
        act("$n releases some musk into the room!", false, this, nullptr, nullptr, TO_ROOM);
        dropGas(::number(1, 3), GAS_MUSK);
        setCond(FULL, getCond(FULL) - 5);
      }
    }
  }

  // If <this> is a mount and still has a rider, have its rider execute the move
  int moveResult = riderAsTBeing 
      ? riderAsTBeing->moveOne(dirToUse) 
      : moveOne(dirToUse);

  if (IS_SET_DELETE(moveResult, DELETE_THIS)) {
    if (riderAsTBeing) {
      riderAsTBeing->reformGroup();
      delete riderAsTBeing;
      riderAsTBeing = nullptr;
      return false;
    }

    return DELETE_THIS;
  }

  if (!moveResult) {
    sendTo("Something prevents your escape!\n\r");
    act("Something prevents $s escape!", true, this, nullptr, nullptr, TO_ROOM);
    return true;
  }

  if (panic) 
    sendTo(format("Panic-stricken, you flee %s.\n\r") % dirs[dirToUse]);
  else if (wasRetreatSuccessful)
    sendTo(format("You skillfully retreat %s.\n\r") % dirs[dirToUse]);
  else
    sendTo(format("You nearly hurt yourself as you fled madly %swards.\n\r") % dirs[dirToUse]);

  // If <this> wasn't fighting when attempting the flee, just return here
  if (!enemy)
    return true;

  // Ensure <this> no longer fighting after successful flee
  // Removing AFF_ENGAGER is handled within stopFighting()
  if (fight())
    stopFighting();
  if (cantHit > 0)
    cantHit = 0;
  if (enemy->fight() == this)
    enemy->stopFighting();

  // Make mobs fear enemy who forced them to flee
  // Check IsPc() in case of disguised/polymorphed/otherwise transformed PC
  if (!isPc()) {
    auto* thisAsTMonster = dynamic_cast<TMonster*>(this);
    if (thisAsTMonster)
      thisAsTMonster->addFeared(enemy);
  }

  // Determine if enemy begins hunting <this>, then potentially look for
  // remaining opponents who are engaged and make enemy attack them if found
  if (!enemy->isPc()) {
    auto* enemyAsTMonster = dynamic_cast<TMonster*>(enemy);
    if (enemyAsTMonster) {
      int percent =
        100 * enemyAsTMonster->getHit() / enemyAsTMonster->hitLimit();

      if (::number(1, 100) < percent &&
          (enemyAsTMonster->Hates(this, nullptr) ||
           isOppositeFaction(enemyAsTMonster)))
        enemyAsTMonster->setHunting(this);

      if (IS_SET_DELETE(enemyAsTMonster->lookForEngaged(), DELETE_THIS)) {
        delete enemyAsTMonster;
        enemyAsTMonster = nullptr;
        enemy = nullptr;
      }
    }
  }

  return true;
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
    vlogf(LOG_BUG, format("%s not fighting after assist %s") %  getName() % v->getName());
#endif

  return rc;
}

// return DELETE_THIS
// return FALSE if item avoided flame, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::burnObject(TBeing *ch, int perc)
{
  int rc = 0;
  TThing *t;
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
  if (getLocked())
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
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    int perc2;
    t=*(it++);
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
     burndam >= getStructPoints()){
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
    for(StuffIter it=stuff.begin();it!=stuff.end();){
      t=*(it++);
      (*t)--;
      if (parent)
        *parent += *t;
      else if (roomp)
        *roomp += *t;
      else
        vlogf(LOG_BUG, format("Bad struct on burnObj %s") %  t->name);
      TObj * tot = dynamic_cast<TObj *>(t);
      if (tot) {
        rc = tot->burnObject(ch, 100);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tot;
          tot = NULL;
        }
      }
    }
    if (!makeScraps())
      return DELETE_THIS;
    return TRUE;
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
  TThing *t;
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
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
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

  if (affectedBySpell(AFFECT_WET))
  {
    if (0 == Weather::addWetness(this, -50))
      sendTo("You feel completely dried off now.\n\r");
    else
      sendTo(format("You dry out a bit from the flames!  You feel %s.\n\r") % Weather::describeWet(this));
  }

  if (hasQuestBit(TOG_HAS_PYROPHOBIA))
  {
    act("<R>FIRE!  There is FIRE everywhere!  Oh, the HORROR!!<1>",TRUE,this,0,0,TO_CHAR);
    if (::number(0, 1))
    {
      act("You faint from the shock!",TRUE,this,0,0,TO_CHAR);
      if (riding)
      {
        act("$n sways then crumples as $e faints.",FALSE,this,0,0,TO_ROOM);
        res = fallOffMount(riding, POSITION_RESTING);
        if (IS_SET_DELETE(res, DELETE_THIS))
          return DELETE_THIS;
      }
      else
        act("$n stumbles then crumples as $e faints.",FALSE,this,0,0,TO_ROOM);
      setPosition(POSITION_SLEEPING);
    }
  }

  return 1;
}

int TBeing::thawEngulfed()
{
  TThing *t;
  TObj *obj = NULL;
  int i;
  int res = 0;

  for (i = MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj *>(t))) 
      continue;
    
    res = obj->thawObject(this, 0);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    res = obj->thawObject(this, 0);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  return 1;
}

int TObj::thawObject(TBeing *ch, int perc)
{

  return FALSE;
}

int TBaseContainer::thawObject(TBeing *ch, int perc)
{
  if(roomp && roomp->isArcticSector())
    return FALSE;

  if(ch && ch->roomp && ch->roomp->isArcticSector())
    return FALSE;

  for(StuffIter it=stuff.begin();it!=stuff.end();it++){
    TObj * tot = dynamic_cast<TObj *>(*it);

    if (tot) {
      tot->thawObject(ch, perc);
    }
  }

  return FALSE;
}

int TBaseCup::thawObject(TBeing *ch, int perc)
{
  if(!isDrinkConFlag(DRINK_FROZEN))
    return FALSE;

  if(roomp && roomp->isArcticSector())
    return FALSE;
  
  if(ch && ch->roomp && ch->roomp->isArcticSector())
    return FALSE;

  // do recursive stuff first
  int rc = TObj::thawObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;

  if (ch) {
    act(format("The warmth causes the %s in your $o to thaw.") %
	liquidInfo[getDrinkType()]->name,
	TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
  } else {
    act(format("The warmth causes the %s in $n to thaw.") %
	liquidInfo[getDrinkType()]->name,
	TRUE,this,0,0,TO_ROOM);
  }
  remDrinkConFlags(DRINK_FROZEN);

  updateDesc();

  return FALSE;
}

int TPool::thawObject(TBeing *ch, int perc)
{
  if(!isDrinkConFlag(DRINK_FROZEN))
    return FALSE;

  if(roomp && roomp->isArcticSector())
    return FALSE;
  
  if(ch && ch->roomp && ch->roomp->isArcticSector())
    return FALSE;

  // do recursive stuff first
  int rc = TBaseCup::thawObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;


  updateDesc();
  if(ch){
    if (equippedBy)
      *ch += *ch->unequip(eq_pos);
    // if it's in a bag or something, put it in inventory so they can drop it
    --(*this);
    *ch += *this;
    ch->doDrop("", this, true);
  }

  return FALSE;
}



// return DELETE_THIS
// return FALSE if item avoided frost, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::freezeObject(TBeing *ch, int perc)
{
  //roll in global modifier to damage chance
  perc *= tweakInfo[TWEAK_FREEZEDAMRATE]->current;
  
  int rc = 0;
  TThing *t = NULL;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  if (getMaxStructPoints() < 0)
      return FALSE;
  if (getStructPoints() < 0)
    return FALSE;
  if (::number(1,100) > perc)
    return FALSE;
  if (getLocked())
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
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    int perc2;
    t=*(it++);
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
      if (!makeScraps())
        return DELETE_THIS;
      return TRUE;
    case MAT_MICA:
    case MAT_AMETHYST:
    case MAT_QUARTZ:
    case MAT_CRYSTAL:
      if (ch) {
        act("The chill causes your $o to freeze and crack severely.",TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
      } else {
        act("The chill causes $n to freeze and crack severely.",TRUE,this,0,0,TO_ROOM);
      }
      if(IS_SET_DELETE(damageItem(::number(1,12)), DELETE_THIS)){
        if (ch) {
          act("..The mineral splits and $p $q destroyed.",TRUE,ch,this,0,TO_CHAR, ANSI_GREEN);
        } else {
          act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
        }
      }
      return TRUE;
    default:
      break;
  }
  return FALSE;
}

int TPool::freezeObject(TBeing *ch, int perc)
{
  if(isDrinkConFlag(DRINK_FROZEN))
    return FALSE;
  addDrinkConFlags(DRINK_FROZEN);

  if (ch) {
    act(format("The chill causes the %s in your $o to freeze.") %
	liquidInfo[getDrinkType()]->name,
	TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
  } else {
    act(format("The chill causes the %s in $n to freeze.") %
	liquidInfo[getDrinkType()]->name,
	TRUE,this,0,0,TO_ROOM);
  }

  updateDesc();

  return FALSE;
}

int TBaseCup::freezeObject(TBeing *ch, int perc)
{
  // do recursive stuff first
  int rc = TObj::freezeObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;
  
  char buf [256];
  bool damage=false;

  if(isDrinkConFlag(DRINK_FROZEN))
    return FALSE;
  
  // figure anything that can compact can also expand a bit
  // a glass bottle can't compact/expand, a leather waterskin can
  // also, room to expand if it's less than half full
  // and, only take damage if less than or as hard as glass, so things like
  // a steel thermos wouldn't crack (neither would plastic, as it can stretch)
  if((material_nums[getMaterial()].vol_mult <= 1) &&
     (material_nums[getMaterial()].hardness <= 
      material_nums[MAT_GLASS].hardness) &&
     (2 * getDrinkUnits() >= getMaxDrinkUnits()))
    damage=true;

  if (::number(1,100) > perc)
    damage=false;


  addDrinkConFlags(DRINK_FROZEN);
  updateDesc();
  
  if (ch) {
    sprintf(buf, "The chill causes the %s in your $o to freeze.",
	    liquidInfo[getDrinkType()]->name);
    act(buf,TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
    
    if(damage)
      act("$p is damaged severely by the expanding ice!",
	  TRUE,ch, this,0,TO_CHAR, ANSI_BLUE);
  } else {
    sprintf(buf, "The chill causes the %s in $n to freeze.",
	    liquidInfo[getDrinkType()]->name);
    act(buf,TRUE,this,0,0,TO_ROOM);
    
    if(damage)
      act("$n is damaged severely by the expanding ice!",
	  TRUE,this,0,0,TO_ROOM);
  }
  
  if(damage){
    if(IS_SET_DELETE(damageItem(::number(1,12)), DELETE_THIS)){
      if (ch) {
	act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
      } else {
	act("   $n $r destroyed.",TRUE,this,0,0,TO_ROOM); 
      }
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
  TThing *t = NULL;
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
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
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
  TThing *t;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;


  if (getMaxStructPoints() < 0)
    return FALSE;
  if (getStructPoints() < 0)
    return FALSE;
  if (material_nums[getMaterial()].acid_susc <= 0)
    return FALSE;
  if (!isMetal())
    return FALSE;
  if (::number(1,100) > perc)
    return FALSE;

  TOpenContainer *trc = dynamic_cast<TOpenContainer *>(this);
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    int perc2;
    t=*(it++);
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
    if(IS_SET_DELETE(damageItem(::number(0,2)), DELETE_THIS)){
      if (ch) {
        act("Your $o $q destroyed by the corrosive acid.",
                TRUE,ch,this,0,TO_CHAR, ANSI_GREEN);
      } else {
        act("$n $r destroyed by acid.",TRUE,this,0,0,TO_ROOM);
      }
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
  TThing *t;
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
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
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
  af.duration = (60) * Pulse::UPDATES_PER_MUDHOUR;
  af.modifier = -20;
  af.location = APPLY_STR;
  af.bitvector = AFF_POISON;

  if (isImmune(IMMUNE_POISON, WEAR_BODY)) {
  } else {
    affectJoin(NULL, &af, AVG_DUR_NO, AVG_EFF_NO);
  }
  for (i=MIN_WEAR;i < MAX_WEAR;i++) {
    if (!(t = equipment[i]) && slotChance(i)) {
      int dam = ::number(2,11);

      if(isImmune(IMMUNE_POISON, i))
	continue;

      if ((dam = getActualDamage(this, 0, dam, SPELL_CHLORINE_BREATH)))
        sendTo(format("The chlorine gas gives you a caustic burn on your %s.\n\r") %describeBodySlot(i));
      rc = applyDamage(this,dam,SPELL_CHLORINE_BREATH);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    } else if (t)
      t->poisonObject();
  }
  for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) 
    t->poisonObject();
  
  return TRUE;
}

void TBeing::flameRoom()
{
  roomp->flameRoom();
}

void TRoom::flameRoom()
{
  TObj *obj = NULL;
  int rc;

  TThing *t;
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    rc = obj->burnObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
}


void TBeing::freezeRoom()
{
  TThing *t;
  TObj *obj = NULL;
  int rc;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    obj = dynamic_cast<TObj *>(t);
    if (!obj || !obj->canWear(ITEM_WEAR_TAKE))
      continue;

    rc = obj->freezeObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
}

void TBeing::acidRoom()
{
  TThing *t;
  TObj *obj = NULL;
  int rc;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (!obj->canWear(ITEM_WEAR_TAKE))
      continue;
    rc = obj->meltObject(NULL, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = NULL;
    }
  }
}

void TBeing::chlorineRoom()
{

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it) {
    (*it)->poisonObject();
  }
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
        saveChar(Room::NOWHERE);
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
  } else {
    // PCS

    // Get getMult returns the number of monk barehand attacks
    num = getMult();
    fx=fy=0;

    // Primary hand
    // If nothing in the hand and a monk
    if (!prim && hasClass(CLASS_MONK))
      fx += (0.60 * num);
    // else we are holding something
    else if (prim) {
      // blowCountSplitter will return 0 if it is not a weapon and 1 if it is a weapon
      fx = prim->blowCountSplitter(this, true);
      
      // Guns
      if((gun=dynamic_cast<TGun *>(prim))){
        fx = gun->getROF();
      }

      // Check specialization after guns
      if (getPosition() >= POSITION_STANDING) {
        prim->specializationCheck(this, &fx);
      }

      // Now we can do the speed mod in here it won't affect monks barehand
      if (fx > 0.0)
        fx *= getStatMod(STAT_SPE);
    // finally if we are holding nothing and not a monk
    } else {
      fx = 1.0;
    }

    // Now do Secondary hand
    if (!sec && hasClass(CLASS_MONK))
      fy += (0.40 * num);
    // if holding something and it's not paired
    else if (sec && sec != prim) {
      fy = sec->blowCountSplitter(this, false);

      // Guns
      if((gun=dynamic_cast<TGun *>(sec)) && !gun->isPaired()){
        fy = gun->getROF();
      }

      // Speed mod
      if (fy > 0.0)
        fy *= getStatMod(STAT_SPE);
    // finally if barehand secondary and not a monk
    } else if (!sec) {
      fy = 1.0;
    }


    // berzerk affects any combo
    if (isCombatMode(ATTACK_BERSERK) && getPosition() >= POSITION_STANDING) {
      if (bSuccess(SKILL_BERSERK)) {
        if (fx > 0.0)
          fx += 0.5;
        if (fy > 0.0)
	        fy += 0.5;
      }
      if (bSuccess(SKILL_ADVANCED_BERSERKING)) {
        if (fx > 0.0)
          fx += 0.5;
        if (fy > 0.0)
	  fy += 0.5;
      }
    }
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
