///////////////////////////////////////////////////////////////////////////
//
//     SneezyMUD++ - All rights reserved, SneezyMUD++ Coding Team
//
//     "offense.cc" - All offensive functions and routines
//
///////////////////////////////////////////////////////////////////////////

#include <boost/format.hpp>
#include <string.h>
#include <algorithm>
#include <cstdio>
#include <list>
#include <vector>

#include "ansi.h"
#include "being.h"
#include "cmd_message.h"
#include "combat.h"
#include "comm.h"
#include "connect.h"
#include "defs.h"
#include "enum.h"
#include "extern.h"
#include "games.h"
#include "garble.h"
#include "handler.h"
#include "immunity.h"
#include "limbs.h"
#include "liquids.h"
#include "log.h"
#include "low.h"
#include "materials.h"
#include "monster.h"
#include "obj.h"
#include "obj_bag.h"
#include "obj_base_container.h"
#include "obj_base_cup.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "obj_open_container.h"
#include "obj_pool.h"
#include "obj_spellbag.h"
#include "parse.h"
#include "race.h"
#include "room.h"
#include "sound.h"
#include "spells.h"
#include "sstring.h"
#include "stats.h"
#include "structs.h"
#include "thing.h"
#include "toggle.h"
#include "tweaks.h"
#include "weather.h"
#include "wiz_powers.h"

// DELETE_THIS implies this needs to be deleted
int TBeing::doHit(const sstring& argument, TBeing* vict) {
  sstring arg;
  TBeing *victim, *tmp;
  int rc = false;
  bool shouldHit = false;

  spellNumT skill = getSkillNum(SKILL_SWITCH_OPP);

  if (checkBlackjack()) {
    gBj.Hit(this);
    return false;
  }
  if (checkBaccarat()) {
    gBaccarat.Hit(this);
    return false;
  }
  if (getPosition() <= POSITION_SITTING) {
    sendTo("Try standing up first.\n\r");
    return false;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  arg = argument.word(0);

  if (vict) {
    victim = vict;
    if (!canSee(victim) && (victim != fight())) {
      sendTo("Just whom do you want to hit?\n\r");
      return false;
    }
  } else {
    if ((tmp = get_best_char_room(this, arg.c_str()))) {
      victim = tmp;
    } else if (fight() && isAffected(AFF_ENGAGER)) {
      victim = fight();
      if (!canSee(victim)) {
        act("You can't see $N to start hitting $M.", false, this, 0, victim,
          TO_CHAR);
        return false;
      }
    } else {
      sendTo("Just whom do you want to hit?\n\r");
      return false;
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
        victim = nullptr;
      }
      if (!rc)
        setVictFighting(victim);
      return rc;
    } else {
      return false;
    }
  }

  if (victim == this) {
    sendTo("You hit yourself..OUCH!\n\r");
    act("$n hits $mself, and says OUCH!", false, this, 0, victim, TO_ROOM);
    return false;
  } else if (isAffected(AFF_CHARM) && (master == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", false, this,
      0, victim, TO_CHAR);
    return false;
  }
  if (noHarmCheck(victim))
    return false;

  if ((getPosition() >= POSITION_CRAWLING) && !fight()) {
    TThing* tThing;
    TBaseWeapon* tWeapon;

    if (isPc())
      if ((tThing = equipment[getPrimaryHold()]) &&
          !(tWeapon = dynamic_cast<TBaseWeapon*>(tThing)) &&
          (tThing = equipment[getSecondaryHold()]) &&
          !(tWeapon = dynamic_cast<TBaseWeapon*>(tThing)) &&
          !IS_SET(desc->autobits, AUTO_ENGAGE) &&
          !IS_SET(desc->autobits, AUTO_ENGAGE_ALWAYS)) {
        sendTo("You are wielding no weapons and intend to attack?\n\r");
        sendTo("I'm afraid that's rather hard, use engage instead.\n\r");
        return false;
      }

    // trigger specials for starting a fight
    if ((rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, nullptr, nullptr)))
      return rc;
    if ((rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, nullptr, nullptr)))
      return rc;

    // put if statement back in if doesnt work right
    if (!isAffected(AFF_AGGRESSOR))
      SET_BIT(specials.affectedBy, AFF_AGGRESSOR);

    // this line is only if they have been sent back engaged by doEngagedhit
    // this will stopthemfighting and return it o here
    if (isAffected(AFF_ENGAGER)) {
      act("You engage $N in battle.", true, this, 0, victim, TO_CHAR,
        ANSI_YELLOW);
      act("$n engages you in battle.", true, this, 0, victim, TO_VICT,
        ANSI_YELLOW);
      act("$n engages $N in battle.", true, this, 0, victim, TO_NOTVICT,
        ANSI_YELLOW);
    } else {
      act("You attack $N.", true, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act("$n attacks you.", true, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("$n attacks $N.", true, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      shouldHit = true;

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
      victim = nullptr;
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
            // trigger specials for starting a fight (switching counts as a new
            // attack)
            if ((rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, nullptr, nullptr)))
              return rc;
            if ((rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, nullptr,
                   nullptr)))
              return rc;

            if (isAffected(AFF_ENGAGER)) {
              doEngage("", victim);
              sendTo("You switch opponents.\n\r");
              act("$n switches targets.", false, this, 0, 0, TO_ROOM);
              addToWait(combatRound(1));
            } else {
              setCharFighting(victim);
              sendTo("You switch opponents.\n\r");
              act("$n switches targets.", false, this, 0, 0, TO_ROOM);
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

  return false;
}

int TBeing::doEngagedHit(const char* argument, TBeing* vict) {
  char arg[80];
  TBeing* victim;

  // used when the person is both engaged and tries to hit
  // will start them hitting

  spellNumT skill = getSkillNum(SKILL_SWITCH_OPP);

  if (!fight()) {
    vlogf(LOG_BUG,
      format("DoEngagedHit called without pc (%s) fighting") % getName());
    return false;
  }

  strcpy(arg, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, arg, nullptr, EXACT_NO, INFRA_YES))) {
      sendTo("Hit whom?\n\r");
      return false;
    }
  }

  if (victim == this) {
    sendTo("You hit yourself..OUCH!.\n\r");
    act("$n hits $mself, and says OUCH!", false, this, 0, victim, TO_ROOM);
    return false;
  } else if (isAffected(AFF_CHARM) && (master == victim)) {
    act("$N is just such a good friend, you simply can't hit $M.", false, this,
      0, victim, TO_CHAR);
    return false;
  }
  if (noHarmCheck(victim))
    return false;

  if (fight() == victim) {
    if (victim->attackers < MAX_COMBAT_ATTACKERS) {
      act("You start to physically attack $N.", true, this, 0, victim, TO_CHAR);
      act("$n starts to physically attack you.", true, this, 0, victim,
        TO_VICT);
      act("$n attacks $N.", true, this, 0, victim, TO_NOTVICT);
      stopFighting();
      // remove engager after stopFight so attackers kept up with OK
      REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
      return true;
    } else {
      act("There is no room to begin physically attacking $N.", false, this, 0,
        victim, TO_CHAR);
      return false;
    }
  } else {
    if (doesKnowSkill(getSkillNum(skill))) {
      if (bSuccess(skill)) {
        if (victim->attackers < MAX_COMBAT_ATTACKERS) {
          stopFighting();
          // remove engager after stopFight so attackers kept up with OK
          REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
          sendTo("You switch opponents.\n\r");
          act("$n switches targets.", false, this, 0, 0, TO_ROOM);
          addToWait(combatRound(1));
          return true;
        } else {
          act("There is no room to switch and begin attacking $N.", false, this,
            0, victim, TO_CHAR);
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
  return false;
}

// DELETE_THIS return implies this needs to be deleted
int TBeing::doEngage(const char* argument, TBeing* vict) {
  char arg[MAX_INPUT_LENGTH];
  TBeing *victim, *tmp;

  if (checkPeaceful("You feel too peaceful to contemplate violence!\n\r"))
    return false;

  //  if (fight() && isAffected(AFF_ENGAGER)) {
  //    sendTo("You are already engaged in combat.\n\r");
  //    return false;
  //  }

  // target setting

  strcpy(arg, argument);

  if (vict) {
    victim = vict;
    if (!canSee(victim)) {
      sendTo("Just whom do you want to engage?\n\r");
      return false;
    }
  } else {
    if ((tmp = get_char_room_vis(this, arg, nullptr, EXACT_NO, INFRA_YES))) {
      victim = tmp;
    } else if (fight() && !isAffected(AFF_ENGAGER)) {
      victim = fight();
    } else {
      sendTo("Just whom do you want to engage?\n\r");
      return false;
    }
  }

  if (this == victim) {
    sendTo("How are you going to engage yourself in combat.\n\r");
    return false;
  }

  // result setting

  if (fight()) {
    if (fight() == victim) {
      if (isAffected(AFF_ENGAGER)) {
        act("You are already engaged in combat with $N.", true, this, 0, victim,
          TO_CHAR);
        return false;
      } else {
        SET_BIT(specials.affectedBy, AFF_ENGAGER);
        act("You stop physically attacking $N.", true, this, 0, victim,
          TO_CHAR);
        act("$n stops physically attacking you.", true, this, 0, victim,
          TO_VICT);
        act("$n stops physically attacking $N.", true, this, 0, victim,
          TO_NOTVICT);

        // setFighting added to attackers, so allow for this
        fight()->attackers--;
        return true;
      }
    } else {
      if (isTanking()) {
        act("You can not simply engage another while you are tanking.", true,
          this, 0, 0, TO_CHAR);
        return false;
      }
      if (isAffected(AFF_ENGAGER)) {
        addToWait(combatRound(2));
      } else {
        addToWait(combatRound(3));
      }
      tmp = fight();
      act("You turn your attention from $N.", true, this, 0, tmp, TO_CHAR);
      act("$n disengages $s attention from the battle against you.", true, this,
        0, tmp, TO_VICT);
      act("$n disengages from the attack on $N.", true, this, 0, tmp,
        TO_NOTVICT);
      stopFighting();
      SET_BIT(specials.affectedBy, AFF_ENGAGER);
      return doHit(argument, victim);
    }
  } else {
    if (!victim)
      return false;

    if (getPosition() <= POSITION_SITTING) {
      sendTo("Try standing up first.\n\r");
      return false;
    }
    if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
      return false;

    if (noHarmCheck(victim) || victim->isImmortal()) {
      sendTo("You can't engage that target in combat.\n\r");
      return false;
    }
    if (victim == this) {
      sendTo("You dance around and engage yourself..OUCH!.\n\r");
      act("$n engages $mself, and stares $eself down!", false, this, 0, victim,
        TO_ROOM);
      return false;
    } else if (isAffected(AFF_CHARM) && (master == victim)) {
      act("$N is just such a good friend, you simply can't engage $M.", false,
        this, 0, victim, TO_CHAR);
      return false;
    }

    // trigger specials for starting a fight
    int rc = 0;
    if (fight() != victim &&
        (rc = checkSpec(victim, CMD_MOB_COMBAT_ONATTACK, nullptr, nullptr)))
      return rc;
    if (fight() != victim &&
        (rc = victim->checkSpec(this, CMD_MOB_COMBAT_ONATTACKED, nullptr, nullptr)))
      return rc;

    SET_BIT(specials.affectedBy, AFF_ENGAGER);
    SET_BIT(specials.affectedBy, AFF_AGGRESSOR);

    setCharFighting(victim);
    act("You engage $N in battle.", true, this, 0, victim, TO_CHAR);
    act("$n engages you in battle.", true, this, 0, victim, TO_VICT);
    act("$n engages $N in battle.", true, this, 0, victim, TO_NOTVICT);
    addToWait(combatRound(1));
    setVictFighting(victim);
    return false;
  }
}

int TBeing::doDisengage() {
  if (!fight()) {
    act("Why do you need to disengage. You are not even fighting.", false, this,
      0, 0, TO_CHAR);
    return false;
  }

  if (isTanking() && isAffected(AFF_ENGAGER) &&
      (fight()->getPosition() > POSITION_SLEEPING)) {
    act("You can not fully disengage while you are tanking.", false, this, 0, 0,
      TO_CHAR);
    return false;
  }

  if (isAffected(AFF_ENGAGER)) {
    addToWait(combatRound(2));
    act("You disengage totally from the fight.", false, this, 0, 0, TO_CHAR);
    stopFighting();
    REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
  } else {
    SET_BIT(specials.affectedBy, AFF_ENGAGER);
    addToWait(combatRound(1));
    fight()->attackers--;
    act("You disengage from physical participation in the fight.", false, this,
      0, 0, TO_CHAR);
  }
  if (!isTanking() && (cantHit > 0)) {
    if (cantHit <= 2) {
      cantHit = 0;
      act("You finish orienting yourself.", false, this, 0, 0, TO_CHAR);
      addToWait(combatRound(2));
    } else if (!fight()) {
      addToWait(combatRound(2 + (cantHit / 5)));
      act("You finish orienting yourself.", false, this, 0, 0, TO_CHAR);
      cantHit = 0;
    } else {
      act("By disengaging, you are able focus on orienting yourself.", false,
        this, 0, 0, TO_CHAR);
    }
  }
  return true;
}

// DELETE_THIS return implies this needs to be deleted
int TBeing::doKill(const char* argument, TBeing* vict) {
  char arg[MAX_INPUT_LENGTH];
  TBeing* v;

  if (checkPeaceful("You feel too peaceful to contemplate violence!\n\r"))
    return false;

  if (!hasWizPower(POWER_SLAY)) {
    return doHit(argument, vict);
  }

  strcpy(arg, argument);

  if (!(v = vict)) {
    if (!(v = get_char_room_vis(this, arg, nullptr, EXACT_NO, INFRA_YES))) {
      sendTo("They aren't here.\n\r");
      return false;
    }
  }

  if (this == v)
    sendTo("Your mother would be so sad.. :(\n\r");
  else if (!noHarmCheck(v)) {
    if ((v->GetMaxLevel() < GetMaxLevel()) || !v->isPc()) {
      act("You chop $M to pieces! Ah! The blood!", 0, this, 0, v, TO_CHAR);
      act(msgVariables(MSG_SLAY_TARG, (TThing*)nullptr), false, v, 0, this,
        TO_CHAR);
      act(msgVariables(MSG_SLAY, v), false, this, 0, v, TO_NOTVICT);
      if (dynamic_cast<TMonster*>(v))
        dynamic_cast<TMonster*>(v)->checkResponses(this, 0, "",
          CMD_RESP_KILLED);
      v->rawKill(DAMAGE_NORMAL, this);
      if (vict)
        return DELETE_VICT;
      v->reformGroup();
      delete v;
      v = nullptr;
    } else {
      sendTo(COLOR_MOBS, format("You can't kill %s!\n\r") % v->getName());
    }
  }
  return false;
}

bool TBeing::isOrderAllowed(const char* argument) { return true; }

// returns DELETE_VICT (vict)
static int applyOrder(TBeing* ch, TBeing* vict, const char* message,
  silentTypeT silent) {
  int rc;

  if (!silent)
    ch->sendTo("OK.\n\r");

  if (vict->getWait() <= 1) {
    vlogf(LOG_SILENT, format("%s ordering %s to '%s' at %d") % ch->getName() %
                        vict->getName() % message % ch->inRoom());
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
  return false;
}

static bool orderDenyCheck(const char* cmd_buf) {
  return (is_abbrev(cmd_buf, "kill") || is_abbrev(cmd_buf, "hit") ||
          is_abbrev(cmd_buf, "engage") || is_abbrev(cmd_buf, "assist") ||
          is_abbrev(cmd_buf, "attack") || is_abbrev(cmd_buf, "kick") ||
          is_abbrev(cmd_buf, "bash") || is_abbrev(cmd_buf, "deathstroke") ||
          is_abbrev(cmd_buf, "whirlwind") || is_abbrev(cmd_buf, "grapple") ||
          is_abbrev(cmd_buf, "shove") || is_abbrev(cmd_buf, "bodyslam") ||
          is_abbrev(cmd_buf, "rescue") || is_abbrev(cmd_buf, "kneestrike") ||
          is_abbrev(cmd_buf, "sleep") ||  // order sleep, backstab....
          is_abbrev(cmd_buf, "emote") || is_abbrev(cmd_buf, "mount") ||
          is_abbrev(cmd_buf, "headbutt") || is_abbrev(cmd_buf, "open") ||
          is_abbrev(cmd_buf, "lower") || is_abbrev(cmd_buf, "lift") ||
          is_abbrev(cmd_buf, "raise"));
}

// returns DELETE_THIS
int TBeing::doOrder(const char* argument) {
  char caName[100], message[256];
  char buf[512];
  int found = false;
  int org_room, i;
  char cmd_buf[40];
  int rc;

  if (applySoundproof())
    return false;

  if (hasQuestBit(TOG_IS_MUTE)) {
    sendTo("It's hard to order people around when you can't talk.");
    return false;
  }

  if (roomp && roomp->isRoomFlag(ROOM_NO_ORDER)) {
    sendTo("A mystical force prevents your order from being uttered here.\n\r");
    return false;
  }
  half_chop(argument, caName, message);

  if (!*caName || !*message) {
    sendTo("Order whom to do what?\n\r");
    return false;
  }
  /*  TBeing *v = get_char_room_vis(this, caName, nullptr, EXACT_NO, INFRA_YES);*/
  TBeing* v = get_best_char_room(this, caName, VISIBLE_YES, INFRA_NO);
  if (!v)
    v = get_best_char_room(this, caName, VISIBLE_YES, INFRA_YES);

  if (!v && !is_abbrev(caName, "followers") && strcmp("all", caName)) {
    sendTo("That person isn't here.\n\r");
    return false;
  }
  if (this == v) {
    sendTo("You obviously suffer from Multiple Personality Disorder.\n\r");
    return false;
  }
  if (isAffected(AFF_CHARM)) {
    sendTo("Your superior would not approve of you giving orders.\n\r");
    return false;
  }
  if (v) {
    sprintf(buf, "$N orders you to '%s'", message);
    if (!v->desc || !v->desc->ignored.isIgnored(desc))
      act(buf, false, v, 0, this, TO_CHAR);
    act("$n gives $N an order.", false, this, 0, v, TO_NOTVICT);

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
    if (!legitimate && v->getCaptiveOf() == this)
      legitimate = true;

    // I am a horse, taking orders from my master
    if (!legitimate && v->horseMaster() == this) {
      int check = MountEgoCheck(this, v);

      // mounts are not gung-ho supporters of their riders
      if (orderDenyCheck(cmd_buf))
        check = 3;  // force indifference

      // if mount has no ego (check <=0) do command
      // if mount is VERY egotistical, and rider not skilled, buck off
      if (check > 5) {
        if (!rideCheck(-5)) {
          act("$n gets pissed and bucks $N off!", false, v, 0, this,
            TO_NOTVICT);
          act("$n gets pissed and bucks you off!", false, v, 0, this, TO_VICT);
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
        v = nullptr;
      }
    } else if (!messSent) {
      act("$n has an indifferent look.", false, v, 0, 0, TO_ROOM);
    }
  } else {
    // This is order "followers"
    sstring garbled = garble(nullptr,
      format("$n issues the order '%s'.") % message, Garble::SPEECH_SAY);

    // there is a possibility that the order would change our room
    // which might have drastic consequences, so we do this check
    org_room = in_room;

    // compare to specific-mob order above
    // horses are skipped (intentional) due to !AFF_CHARM
    // captives also skipped
    // we enumerate per room and message individually to avoid players issung
    // 'orders' to subvert speech
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TBeing* kfol = dynamic_cast<TBeing*>(*it++);
      if (!kfol || kfol == this || org_room != kfol->inRoom())
        continue;
      if (kfol->desc && kfol->desc->ignored.isIgnored(desc))
        continue;
      if (!kfol->isAffected(AFF_CHARM)) {
        act(garbled.c_str(), false, this, 0, kfol, TO_VICT);
        continue;
      }

      found = true;
      rc = applyOrder(this, kfol, message, SILENT_YES);
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete kfol;
        kfol = nullptr;
        break;
      }
    }
    if (found)
      sendTo("Ok.\n\r");
    else {
      sendTo("Nobody here is a loyal subject of yours!\n\r");
      return false;
    }
  }
  doSave(SILENT_YES);
  return true;
}

static bool canFleeThisWay(TBeing* ch, dirTypeT dir) {
  if (!ch->canGo(dir))
    return false;

  TRoom* rp2 = real_roomp(ch->exitDir(dir)->to_room);
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
int TBeing::doFlee(const char* arg) {
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
      sendTo(
        "You try to flee, but can't due to the condition of your legs!\n\r");
      act("$n tries to flee, but $s legs just won't move!", true, this, nullptr,
        nullptr, TO_ROOM);
      return false;
    }
    if ((eitherLegHurt() || (!isHumanoid() && eitherArmHurt())) &&
        ::number(0, 2)) {
      sendTo(
        "You try to flee, but can't due to the condition of your legs!\n\r");
      act("$n tries to flee, but $s leg just won't move!", true, this, nullptr,
        nullptr, TO_ROOM);
      return false;
    }
  }

  if (isAffected(AFF_CHARM) && master && sameRoom(*master)) {
    if (!::number(0, 5))
      act("$n bursts into tears at the thought of fleeing without $N!", true,
        this, nullptr, master, TO_ROOM);
    act("You can't bear the thought of leaving $N's side!", false, this,
      nullptr, master, TO_CHAR);
    return false;
  }

  if (roomp->isRoomFlag(ROOM_NO_FLEE)) {
    act("$n tries to flee but a strange power prevents $m from escaping.",
      false, this, nullptr, nullptr, TO_ROOM);
    act("A strange power prevents you from escaping.", false, this, nullptr,
      nullptr, TO_CHAR, ANSI_RED);
    return false;
  }

  // Do skill checks now and use results throughout so bSuccess only gets
  // called once per flee attempt.
  bool wasRetreatSuccessful = bSuccess(getSkillNum(SKILL_RETREAT));

  bool panic = false;
  int rc = 0;
  auto* riderAsTBeing = dynamic_cast<TBeing*>(rider);

  // Could be either riding an actual mount, or be on a chair/bed of some sort.
  // Handle both possibilities. Failures here result in flee fail and return
  // from function call.
  if (riding) {
    // If this resolves to nullptr we know 'riding' is a TObj (furniture)
    auto* ridingAsTBeing = dynamic_cast<TBeing*>(riding);

    // Condensed logic using ternaries. Determine if <this> is riding an actual
    // mount or simply on some furniture, then build some strings to send to
    // act(). Avoids a bunch of nested ifs and calling act repeatedly.
    bool shouldFallOffMount =
      ridingAsTBeing ? ((!bSuccess(SKILL_RIDE) || !wasRetreatSuccessful) &&
                         !bSuccess(ridingAsTBeing->mountSkillType()))
                     : !wasRetreatSuccessful;

    sstring toChar =
      ridingAsTBeing
        ? (shouldFallOffMount ? "You urge your $O to flee, causing it to panic "
                                "and throw you off!"
                              : "")
        : (shouldFallOffMount
              ? "Lurching backwards, you attempt to escape!"
              : "You smoothly stand up while looking for available exits.");

    sstring toRoom =
      ridingAsTBeing
        ? (shouldFallOffMount
              ? "$n urges $s $O to flee, causing it to panic and throw $n off!"
              : "")
        : (shouldFallOffMount
              ? "$n lurches backwards, attempting to escape!"
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

    // Have to use dismount here, as just forcing a standing position via
    // setPosition actually sets <this> to standing on top of whatever furniture
    // they were on. Collapses chairs, etc.
    if (!ridingAsTBeing)
      dismount(POSITION_STANDING);

    // If <this> is a mount being ridden
  } else if (riderAsTBeing) {
    act("Your $O panics and attempts to flee!", true, riderAsTBeing, nullptr,
      this, TO_CHAR);
    act("$n's $O panics and attempts to flee!", true, riderAsTBeing, nullptr,
      this, TO_ROOM);

    // Give Deikhans a chance to prevent being bucked, with the right skills.
    if (riderAsTBeing->bSuccess(mountSkillType()) &&
        riderAsTBeing->bSuccess(SKILL_CALM_MOUNT)) {
      act("You manage to calm $M down before $E dismounts you.", true,
        riderAsTBeing, nullptr, this, TO_CHAR);
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
    Whether retreating or fleeing, if they input a direction, check if it's a
    valid dir and return if not. If they've chosen a valid direction, actually
    do the checks to see if they succeed in fleeing that way further down.
  */
  if (chosenDir != DIR_NONE && !canFleeThisWay(this, chosenDir)) {
    act("You can't escape in that direction!", true, this, nullptr, nullptr,
      TO_CHAR);
    return true;
  }

  /*
    Create vector containing all currently valid flee directions, except for
    the chosen direction if one was given.
  */
  std::vector<dirTypeT> validDirections;
  for (dirTypeT direction = MIN_DIR; direction < MAX_DIR; direction++) {
    if (direction == chosenDir)
      continue;
    if (canFleeThisWay(this, direction))
      validDirections.push_back(direction);
  }

  /*
    If there are no valid flee directions (doors closed, etc), simply return
    with the panic message.
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
  dirTypeT randomDir =
    validDirections.empty()
      ? chosenDir
      : validDirections[::number(0, validDirections.size() - 1)];

  // Save reference to who <this> was fighting for later
  TBeing* enemy = fight();

  /*
    Determine which direction to actually use. If panicked, no flee direction
    chosen, or fighting and both retreat and 50% chance fail then flee in random
    direction. Otherwise flee in chosen direction.
  */
  dirTypeT dirToUse = panic || chosenDir == DIR_NONE ||
                          (!wasRetreatSuccessful && enemy && ::number(0, 1))
                        ? randomDir
                        : chosenDir;

  // These messages don't make sense if not fighting
  if (enemy) {
    if (wasRetreatSuccessful && !panic) {
      act("$n is looking for an opening to escape!", true, this, nullptr,
        nullptr, TO_ROOM);
      act("You look for an opening to escape from the fight.", true, this,
        nullptr, nullptr, TO_CHAR);
    } else {
      act("$n panics and attempts to flee!", true, this, nullptr, nullptr,
        TO_ROOM);
      act("You turn tail and attempt to run away.", true, this, nullptr,
        nullptr, TO_CHAR);
      loseSneak();

      // Handle troglodyte racial
      if (panic && !::number(0, 1) && getMyRace()->hasTalent(TALENT_MUSK) &&
          getCond(FULL) > 5) {
        act(
          "Your fear causes you to release some musk scent to cover your "
          "tracks.",
          false, this, nullptr, nullptr, TO_CHAR);
        act("$n releases some musk into the room!", false, this, nullptr,
          nullptr, TO_ROOM);
        dropGas(::number(1, 3), GAS_MUSK);
        setCond(FULL, getCond(FULL) - 5);
      }
    }
  }

  // If <this> is a mount and still has a rider, have its rider execute the move
  int moveResult =
    riderAsTBeing ? riderAsTBeing->moveOne(dirToUse) : moveOne(dirToUse);

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
    sendTo(format("You nearly hurt yourself as you fled madly %swards.\n\r") %
           dirs[dirToUse]);

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
int TBeing::doAssist(const char* argument, TBeing* vict, bool flags) {
  TBeing *v = nullptr, *tmp_ch = nullptr;
  char v_name[240];
  int rc = false;

  if (checkPeaceful("No one should need assistance here.\n\r"))
    return false;

  strcpy(v_name, argument);

  if (!(v = vict)) {
    // Default to the first PC group member who is currently tanking, or the
    // first mobile group member if no PC is.
    if (isAffected(AFF_GROUP) && (!argument || !*argument) &&
        (followers || (master && master->followers))) {
      followData* tFData = (master ? master->followers : followers);
      TBeing *tAssistMe = nullptr, *tMaybeMe = nullptr;

      if (master && master->isAffected(AFF_GROUP) && sameRoom(*master) &&
          canSee(master) && master->fight() &&
          (master->fight()->fight() == master))
        tAssistMe = master;
      else
        for (; tFData; tFData = tFData->next)
          if (tFData->follower && tFData->follower->isAffected(AFF_GROUP) &&
              canSee(tFData->follower) && tFData->follower->fight() &&
              (tFData->follower->fight()->fight() == tFData->follower) &&
              (tFData->follower != this) && sameRoom(*tFData->follower)) {
            if (tFData->follower->isPc()) {
              tAssistMe = tFData->follower;
              break;
            } else if (!tMaybeMe)
              tMaybeMe = tFData->follower;
          }

      if (!tAssistMe && tMaybeMe)
        tAssistMe = tMaybeMe;

      v = tAssistMe;  // Since this is nullptr'ed above we are safe doing this
                      // without a check.
    }

    if (!v)
      if (!(v = get_char_room_vis(this, v_name, nullptr, EXACT_NO, INFRA_YES))) {
        sendTo("Whom do you want to assist?\n\r");
        return false;
      }
  }
  if (v == this) {
    sendTo("Oh, by all means, help yourself...\n\r");
    return false;
  }
  if (dynamic_cast<TMonster*>(v) && (v->master != this) && !flags) {
    sendTo("You can't assist non-charmed monsters sorry.\n\r");
    return false;
  }
  if (fight() == v) {
    sendTo("That would be counterproductive?\n\r");
    return false;
  }
  if (fight()) {
    sendTo("You have your hands full right now.\n\r");
    return false;
  }
  if ((v->attackers >= MAX_COMBAT_ATTACKERS) && !flags) {
    sendTo("You can't get close enough to them to assist!\n\r");
    return false;
  }
  if (!(tmp_ch = v->fight())) {
    act("But $E's not fighting anyone.", false, this, 0, v, TO_CHAR);
    return false;
  }
  if (isAffected(AFF_CHARM) && (master == v->fight())) {
    act("You can't assist $N against your beloved master.", false, this, 0, v,
      TO_CHAR);
    return false;
  }
  // See if they have chosen to be an engager or if they are currently engaged
  if (checkEngagementStatus()) {
    act("You turn to face $N's enemy.", true, this, 0, v, TO_CHAR, ANSI_ORANGE);
    act("$n assists you.", true, this, 0, v, TO_VICT, ANSI_GREEN);
    act("$n turns to face you in battle.", true, this, 0, tmp_ch, TO_VICT,
      ANSI_RED);
    act("$n turns to face $N's enemy in battle.", true, this, 0, v, TO_NOTVICT,
      ANSI_BLUE);
    doEngage("", v->fight());
    return true;
  }

  // this checks to verify an attack could actually happen.
  // i.e. they need to be wielding a weapon or be barehanded
  if ((equipment[HOLD_LEFT] && equipment[HOLD_RIGHT] &&
        !dynamic_cast<TBaseWeapon*>(equipment[HOLD_LEFT]) &&
        !dynamic_cast<TBaseWeapon*>(equipment[HOLD_RIGHT])) ||
      !equipment[HOLD_LEFT] || !equipment[HOLD_RIGHT]) {
  } else {
    reconcileHelp(v, 0.01);
  }

  // This should be difficult, considering what they are trying to do.  But even
  // blind mobs should have a chance to assist in due situations, but not a
  // guaranteed assist.  This also applies to any situation in which a mobile
  // wanting to assist can not see the person in trouble and/or the aggressor.
  // -Lapsos
  if ((!canSee(v) || !canSee(tmp_ch)) &&
      ((::number(-100, getStat(STAT_CURRENT, STAT_FOC)) < 0) ||
        (::number(-100, getStat(STAT_CURRENT, STAT_PER)) < 0))) {
    act(
      "You try to assist $N but you can not see well enough to join the fray.",
      true, this, 0, v, TO_CHAR, ANSI_ORANGE);
    act("$n fails to assist you, look at them stumble around.", true, this, 0,
      v, TO_VICT, ANSI_GREEN);
    act("$n tried to attack you but their inability to see clearly stops them.",
      true, this, 0, tmp_ch, TO_VICT, ANSI_RED);
    act("$n tries to assist against $N, but fails.", true, this, 0, tmp_ch,
      TO_NOTVICT, ANSI_BLUE);
    return false;
  }

  switch (::number(1, 6)) {
    case 1:
      act("You jump into the battle attacking $N's enemy.", true, this, 0, v,
        TO_CHAR, ANSI_ORANGE);
      act("$n jumps into the battle, assisting you.", true, this, 0, v, TO_VICT,
        ANSI_GREEN);
      act("$n jumps into the battle, attacking you.", true, this, 0, tmp_ch,
        TO_VICT, ANSI_RED);
      act("$n jumps into the battle, attacking $N.", true, this, 0, tmp_ch,
        TO_NOTVICT, ANSI_BLUE);
      break;
    case 2:
      act("You leap to the aid of $N.", true, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("$n leaps into the fray, assisting you.", true, this, 0, v, TO_VICT,
        ANSI_GREEN);
      act("$n leaps into the fray, attacking you.", true, this, 0, tmp_ch,
        TO_VICT, ANSI_RED);
      act("$n leaps into the fray, attacking $N.", true, this, 0, tmp_ch,
        TO_NOTVICT, ANSI_BLUE);
      break;
    case 3:
      act("You add the strength of your arms to $N's cause.", true, this, 0, v,
        TO_CHAR, ANSI_ORANGE);
      act("$n joins the fray, assisting you.", true, this, 0, v, TO_VICT,
        ANSI_GREEN);
      act("$n joins the fray, attacking you.", true, this, 0, tmp_ch, TO_VICT,
        ANSI_RED);
      act("$n joins the battle attacking $N.", true, this, 0, tmp_ch,
        TO_NOTVICT, ANSI_BLUE);
      break;
    case 4:
      act("You charge ahead in order to assist $N in battle.", true, this, 0, v,
        TO_CHAR, ANSI_ORANGE);
      act("$n charges into the fray, assisting you.", true, this, 0, v, TO_VICT,
        ANSI_GREEN);
      act("$n charges into the fray, attacking you.", true, this, 0, tmp_ch,
        TO_VICT, ANSI_RED);
      act("$n charges into the battle and begins to attack $N.", true, this, 0,
        tmp_ch, TO_NOTVICT, ANSI_BLUE);
      break;
    case 5:
      act("You join forces with $N in battle.", true, this, 0, v, TO_CHAR,
        ANSI_ORANGE);
      act("$n joins forces with you against your enemy.", true, this, 0, v,
        TO_VICT, ANSI_GREEN);
      act("$n joins forces with $N in battle coming to $S aid.", true, this, 0,
        v, TO_NOTVICT, ANSI_BLUE);
      break;
    default:
      act("You come to the assistance of $N in $S time of need.", true, this, 0,
        v, TO_CHAR, ANSI_ORANGE);
      act("$n assists you against your enemy.", true, this, 0, v, TO_VICT,
        ANSI_GREEN);
      act("$n enters the battle coming to the assistance of $N.", true, this, 0,
        v, TO_NOTVICT, ANSI_BLUE);
      break;
  }

  rc = hit(tmp_ch);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    tmp_ch->reformGroup();
    delete tmp_ch;
    tmp_ch = nullptr;
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
// return false if item avoided flame, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::burnObject(TBeing* ch, int perc) {
  int rc = 0;
  TThing* t;
  char buf[256];

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return false;

  if (getMaxStructPoints() < 0)
    return false;
  if (getStructPoints() < 0)
    return false;
  if (::number(1, 100) > perc)
    return false;
  if (dynamic_cast<TSpellBag*>(this))
    return false;
  if (dynamic_cast<TBag*>(this))
    return false;
  if (getLocked())
    return false;

  if (ch && (ch == equippedBy) && material_nums[getMaterial()].conductivity) {
    int dam = 1 + getVolume() / 2000;
    if ((dam = ch->getActualDamage(ch, 0, dam, DAMAGE_FIRE)))
      act("Your $o turns red hot and burns you!", true, ch, this, 0, TO_CHAR);
    else
      act("Your $o turns red hot.", true, ch, this, 0, TO_CHAR);
    rc = ch->applyDamage(ch, dam, DAMAGE_FIRE);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_VICT;
    }
  }

  if (material_nums[getMaterial()].flammability <= 0)
    return false;

  TOpenContainer* trc = dynamic_cast<TOpenContainer*>(this);
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    int perc2;
    t = *(it++);
    if (trc) {
      if (trc->isClosed())
        perc2 = 5 * perc / 100;  // 5% chance if closed
      else if (trc->isCloseable())
        perc2 = 6 * perc / 10;  // 60% chance if open and closable
      else
        perc2 = 5 * perc / 10;  // 50% chance for unclosable containers
    } else
      perc2 = 5 * perc / 10;  // 50% chance for unclosable containers
    TObj* tot = dynamic_cast<TObj*>(t);

    if (tot) {
      rc = tot->burnObject(ch, perc2);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = nullptr;
      }
    }
  }

  int orig = getMaxStructPoints();
  int burndam = ::number(0,
    (int)((double)orig * 1.00 *
          ((double)(material_nums[getMaterial()].flammability)) / 1000.0) +
      1);
  burndam /= 5;
  if (burndam <= 0)
    burndam = 1;

  if (material_nums[getMaterial()].flammability &&
      burndam >= getStructPoints()) {
    // destroyed
    setStructPoints(0);
    if (ch) {
      sprintf(buf, "Your $o $q consumed in <r>flame<1>.");
      act(buf, true, ch, this, 0, TO_CHAR, ANSI_YELLOW);
      sprintf(buf, "$n's $o $q consumed in the flames.");
      act(buf, true, ch, this, 0, TO_ROOM);
    } else {
      act("The flame consumes $n.", true, this, 0, 0, TO_ROOM);
    }
    // since bag went poof, lets (re)check contents
    // with an even higher percentage, and empty bag into owner
    for (StuffIter it = stuff.begin(); it != stuff.end();) {
      t = *(it++);
      (*t)--;
      if (parent)
        *parent += *t;
      else if (roomp)
        *roomp += *t;
      else
        vlogf(LOG_BUG, format("Bad struct on burnObj %s") % t->name);
      TObj* tot = dynamic_cast<TObj*>(t);
      if (tot) {
        rc = tot->burnObject(ch, 100);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tot;
          tot = nullptr;
        }
      }
    }
    if (!makeScraps())
      return DELETE_THIS;
    return true;
  } else {
    setStructPoints(getStructPoints() - burndam);

    if (ch) {
      sprintf(buf, "Your $o $q singed slightly, but look$Q intact.");
      act(buf, true, ch, this, 0, TO_CHAR);
    } else {
      act("The flame slightly singes $n, but it looks intact.", true, this, 0,
        0, TO_ROOM);
    }

    // i want to give objs only a CHANCE to burn
    //  flamability is usually 1-10 so if we use this as a
    //  modifier
    //  and give flammable objects only a 0-50% chance of burning, it should be
    //  good

    if (::number(0, 100) <
          (int)(0.050 * (double)material_nums[getMaterial()].flammability) &&
        !isObjStat(ITEM_BURNING) && !isObjStat(ITEM_PAIRED)) {
      setBurning(ch);
      if (ch) {
        sprintf(buf, "Your $o start$Q to burn!\a");
        act(buf, true, ch, this, 0, TO_CHAR);
      }
    }

    return true;
  }

  return false;
}

int TBeing::flameEngulfed() {
  TThing* t;
  TObj* obj = nullptr;
  int i;
  int res = 0;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj*>(t)))
      continue;

    res = obj->burnObject(this, getImmunity(IMMUNE_HEAT) ? 1 : 50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;

    res = obj->burnObject(this, getImmunity(IMMUNE_HEAT) ? 1 : 50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }

  if (affectedBySpell(AFFECT_WET)) {
    if (0 == Weather::addWetness(this, -50))
      sendTo("You feel completely dried off now.\n\r");
    else
      sendTo(format("You dry out a bit from the flames!  You feel %s.\n\r") %
             Weather::describeWet(this));
  }

  if (hasQuestBit(TOG_HAS_PYROPHOBIA)) {
    act("<R>FIRE!  There is FIRE everywhere!  Oh, the HORROR!!<1>", true, this,
      0, 0, TO_CHAR);
    if (::number(0, 1)) {
      act("You faint from the shock!", true, this, 0, 0, TO_CHAR);
      if (riding) {
        act("$n sways then crumples as $e faints.", false, this, 0, 0, TO_ROOM);
        res = fallOffMount(riding, POSITION_RESTING);
        if (IS_SET_DELETE(res, DELETE_THIS))
          return DELETE_THIS;
      } else
        act("$n stumbles then crumples as $e faints.", false, this, 0, 0,
          TO_ROOM);
      setPosition(POSITION_SLEEPING);
    }
  }

  return 1;
}

int TBeing::thawEngulfed() {
  TThing* t;
  TObj* obj = nullptr;
  int i;
  int res = 0;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj*>(t)))
      continue;

    res = obj->thawObject(this, 0);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;

    res = obj->thawObject(this, 0);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  return 1;
}

int TObj::thawObject(TBeing* ch, int perc) { return false; }

int TBaseContainer::thawObject(TBeing* ch, int perc) {
  if (roomp && roomp->isArcticSector())
    return false;

  if (ch && ch->roomp && ch->roomp->isArcticSector())
    return false;

  for (StuffIter it = stuff.begin(); it != stuff.end(); it++) {
    TObj* tot = dynamic_cast<TObj*>(*it);

    if (tot) {
      tot->thawObject(ch, perc);
    }
  }

  return false;
}

int TBaseCup::thawObject(TBeing* ch, int perc) {
  if (!isDrinkConFlag(DRINK_FROZEN))
    return false;

  if (roomp && roomp->isArcticSector())
    return false;

  if (ch && ch->roomp && ch->roomp->isArcticSector())
    return false;

  // do recursive stuff first
  int rc = TObj::thawObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;

  if (ch) {
    act(format("The warmth causes the %s in your $o to thaw.") %
          liquidInfo[getDrinkType()]->name,
      true, ch, this, 0, TO_CHAR, ANSI_BLUE);
  } else {
    act(format("The warmth causes the %s in $n to thaw.") %
          liquidInfo[getDrinkType()]->name,
      true, this, 0, 0, TO_ROOM);
  }
  remDrinkConFlags(DRINK_FROZEN);

  updateDesc();

  return false;
}

int TPool::thawObject(TBeing* ch, int perc) {
  if (!isDrinkConFlag(DRINK_FROZEN))
    return false;

  if (roomp && roomp->isArcticSector())
    return false;

  if (ch && ch->roomp && ch->roomp->isArcticSector())
    return false;

  // do recursive stuff first
  int rc = TBaseCup::thawObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;

  updateDesc();
  if (ch) {
    if (equippedBy)
      *ch += *ch->unequip(eq_pos);
    // if it's in a bag or something, put it in inventory so they can drop it
    --(*this);
    *ch += *this;
    ch->doDrop("", this, true);
  }

  return false;
}

// return DELETE_THIS
// return false if item avoided frost, true otherwise
// return DELETE_VICT if ch should be deleted
int TObj::freezeObject(TBeing* ch, int perc) {
  // roll in global modifier to damage chance
  perc *= tweakInfo[TWEAK_FREEZEDAMRATE]->current;

  int rc = 0;
  TThing* t = nullptr;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return false;

  if (getMaxStructPoints() < 0)
    return false;
  if (getStructPoints() < 0)
    return false;
  if (::number(1, 100) > perc)
    return false;
  if (getLocked())
    return false;

  if (ch && (ch == equippedBy) && material_nums[getMaterial()].conductivity) {
    int dam = 1 + getVolume() / 2000;
    if ((dam = ch->getActualDamage(ch, 0, dam, SPELL_FROST_BREATH)))
      act("Your $o turns bitterly cold and freeze-burns you!", true, ch, this,
        0, TO_CHAR);
    else
      act("Your $o turns bitterly cold.", true, ch, this, 0, TO_CHAR);

    rc = ch->applyDamage(ch, dam, SPELL_FROST_BREATH);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  }
  TOpenContainer* trc = dynamic_cast<TOpenContainer*>(this);
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    int perc2;
    t = *(it++);
    if (trc) {
      if (trc->isClosed())
        perc2 = 10 * perc / 100;  // 10% chance if closed
      else if (trc->isCloseable())
        perc2 = 8 * perc / 10;  // 80% chance if open and closable
      else
        perc2 = 6 * perc / 10;  // 60% chance for unclosable containers
    } else
      perc2 = 6 * perc / 10;  // 60% chance for unclosable containers

    TObj* tot = dynamic_cast<TObj*>(t);
    if (tot) {
      rc = tot->freezeObject(ch, perc2);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = nullptr;
      }
    }
  }
  switch (getMaterial()) {
    case MAT_GLASS:
    case MAT_WATER:
      if (ch) {
        act("The chill freezes your $o and it shatters.", true, ch, this, 0,
          TO_CHAR, ANSI_BLUE);
      } else {
        act("The chill freezes $n and it shatters.", true, this, 0, 0, TO_ROOM);
      }
      if (!makeScraps())
        return DELETE_THIS;
      return true;
    case MAT_MICA:
    case MAT_AMETHYST:
    case MAT_QUARTZ:
    case MAT_CRYSTAL:
      if (ch) {
        act("The chill causes your $o to freeze and crack severely.", true, ch,
          this, 0, TO_CHAR, ANSI_BLUE);
      } else {
        act("The chill causes $n to freeze and crack severely.", true, this, 0,
          0, TO_ROOM);
      }
      if (IS_SET_DELETE(damageItem(::number(1, 12)), DELETE_THIS)) {
        if (ch) {
          act("..The mineral splits and $p $q destroyed.", true, ch, this, 0,
            TO_CHAR, ANSI_GREEN);
        } else {
          act("   $n $r destroyed.", true, this, 0, 0, TO_ROOM);
        }
      }
      return true;
    default:
      break;
  }
  return false;
}

int TPool::freezeObject(TBeing* ch, int perc) {
  if (isDrinkConFlag(DRINK_FROZEN))
    return false;
  addDrinkConFlags(DRINK_FROZEN);

  if (ch) {
    act(format("The chill causes the %s in your $o to freeze.") %
          liquidInfo[getDrinkType()]->name,
      true, ch, this, 0, TO_CHAR, ANSI_BLUE);
  } else {
    act(format("The chill causes the %s in $n to freeze.") %
          liquidInfo[getDrinkType()]->name,
      true, this, 0, 0, TO_ROOM);
  }

  updateDesc();

  return false;
}

int TBaseCup::freezeObject(TBeing* ch, int perc) {
  // do recursive stuff first
  int rc = TObj::freezeObject(ch, perc);
  if (IS_SET_DELETE(rc, DELETE_THIS | DELETE_VICT))
    return rc;

  char buf[256];
  bool damage = false;

  if (isDrinkConFlag(DRINK_FROZEN))
    return false;

  // figure anything that can compact can also expand a bit
  // a glass bottle can't compact/expand, a leather waterskin can
  // also, room to expand if it's less than half full
  // and, only take damage if less than or as hard as glass, so things like
  // a steel thermos wouldn't crack (neither would plastic, as it can stretch)
  if ((material_nums[getMaterial()].vol_mult <= 1) &&
      (material_nums[getMaterial()].hardness <=
        material_nums[MAT_GLASS].hardness) &&
      (2 * getDrinkUnits() >= getMaxDrinkUnits()))
    damage = true;

  if (::number(1, 100) > perc)
    damage = false;

  addDrinkConFlags(DRINK_FROZEN);
  updateDesc();

  if (ch) {
    sprintf(buf, "The chill causes the %s in your $o to freeze.",
      liquidInfo[getDrinkType()]->name);
    act(buf, true, ch, this, 0, TO_CHAR, ANSI_BLUE);

    if (damage)
      act("$p is damaged severely by the expanding ice!", true, ch, this, 0,
        TO_CHAR, ANSI_BLUE);
  } else {
    sprintf(buf, "The chill causes the %s in $n to freeze.",
      liquidInfo[getDrinkType()]->name);
    act(buf, true, this, 0, 0, TO_ROOM);

    if (damage)
      act("$n is damaged severely by the expanding ice!", true, this, 0, 0,
        TO_ROOM);
  }

  if (damage) {
    if (IS_SET_DELETE(damageItem(::number(1, 12)), DELETE_THIS)) {
      if (ch) {
        act("   $n $r destroyed.", true, this, 0, 0, TO_ROOM);
      } else {
        act("   $n $r destroyed.", true, this, 0, 0, TO_ROOM);
      }
      return DELETE_THIS;
    }
  }

  return false;
}

// returns DELETE_THIS
int TBeing::frostEngulfed() {
  TObj* obj = nullptr;
  int i;
  int res;
  TThing* t = nullptr;
  // Need to account for worn containers
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj*>(t)))
      continue;

    res = obj->freezeObject(this, getImmunity(IMMUNE_COLD) ? 1 : 50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;
    res = obj->freezeObject(this, getImmunity(IMMUNE_COLD) ? 1 : 50);
    if (IS_SET_DELETE(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
    if (IS_SET_DELETE(res, DELETE_VICT))
      return DELETE_THIS;
  }
  return 1;
}

// return DELETE_VICT if ch should be deleted
int TObj::meltObject(TBeing* ch, int perc) {
  int rc = 0;
  int orig = getStructPoints();
  TThing* t;

  if (ch && ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA))
    return false;

  if (getMaxStructPoints() < 0)
    return false;
  if (getStructPoints() < 0)
    return false;
  if (material_nums[getMaterial()].acid_susc <= 0)
    return false;
  if (!isMetal())
    return false;
  if (::number(1, 100) > perc)
    return false;

  TOpenContainer* trc = dynamic_cast<TOpenContainer*>(this);
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    int perc2;
    t = *(it++);
    if (trc) {
      if (trc->isClosed())
        perc2 = 10 * perc / 100;  // 10% chance if closed
      else if (trc->isCloseable())
        perc2 = 8 * perc / 10;  // 80% chance if open and closable
      else
        perc2 = 6 * perc / 10;  // 60% chance for unclosable containers
    } else
      perc2 = 6 * perc / 10;  // 60% chance for unclosable containers
    TObj* tot = dynamic_cast<TObj*>(t);
    if (tot) {
      rc = tot->meltObject(ch,
        max(5, perc2 - (ch ? ch->getImmunity(IMMUNE_ACID) : 0)));
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tot;
        tot = nullptr;
      }
    }
  }
  while (::number(0, 101) <= material_nums[getMaterial()].acid_susc) {
    if (IS_SET_DELETE(damageItem(::number(0, 2)), DELETE_THIS)) {
      if (ch) {
        act("Your $o $q destroyed by the corrosive acid.", true, ch, this, 0,
          TO_CHAR, ANSI_GREEN);
      } else {
        act("$n $r destroyed by acid.", true, this, 0, 0, TO_ROOM);
      }
      return DELETE_THIS;
    }
  }
  if (orig != getStructPoints()) {
    if (ch) {
      act("Your $o $q corroded, but look$Q intact.", true, ch, this, 0,
        TO_CHAR);
    } else {
      act("$n decays slightly in the corrosive acid.", true, this, nullptr, 0,
        TO_ROOM);
    }
    return true;
  }
  return false;
}

int TBeing::acidEngulfed() {
  TThing* t;
  TObj* obj = nullptr;
  int i, res;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj*>(t)))
      continue;
    res = obj->meltObject(this, getImmunity(IMMUNE_ACID) ? 1 : 50);
    if (IS_SET_ONLY(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
  }
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;
    res = obj->meltObject(this, getImmunity(IMMUNE_ACID) ? 1 : 50);
    if (IS_SET_ONLY(res, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
  }
  return false;
}

// returns DELETE_THIS
int TBeing::lightningEngulfed() {
  int rc;
  TObj* obj = nullptr;
  TThing* t;
  int i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) || !(obj = dynamic_cast<TObj*>(t)))
      continue;
    if (obj->getMaxStructPoints() < 0)
      continue;
    if (obj->getStructPoints() < 0)
      continue;
    if (material_nums[obj->getMaterial()].conductivity) {
      int dam = 1 + obj->getVolume() / 2000;
      if ((dam = getActualDamage(this, 0, dam, SPELL_LIGHTNING_BREATH)))
        act("Sparks fly from your $o, harming you!", true, this, obj, 0,
          TO_CHAR);
      else
        act("Sparks fly from your $o.", true, this, obj, 0, TO_CHAR);

      rc = applyDamage(this, dam, SPELL_LIGHTNING_BREATH);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }
  }
  return 1;
}

int TBeing::chlorineEngulfed() {
  int rc;
  TThing* t;
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
    affectJoin(nullptr, &af, AVG_DUR_NO, AVG_EFF_NO);
  }
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(t = equipment[i]) && slotChance(i)) {
      int dam = ::number(2, 11);

      if (isImmune(IMMUNE_POISON, i))
        continue;

      if ((dam = getActualDamage(this, 0, dam, SPELL_CHLORINE_BREATH)))
        sendTo(
          format("The chlorine gas gives you a caustic burn on your %s.\n\r") %
          describeBodySlot(i));
      rc = applyDamage(this, dam, SPELL_CHLORINE_BREATH);
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    } else if (t)
      t->poisonObject();
  }
  for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it)
    t->poisonObject();

  return true;
}

void TBeing::flameRoom() { roomp->flameRoom(); }

void TRoom::flameRoom() {
  TObj* obj = nullptr;
  int rc;

  TThing* t;
  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;

    rc = obj->burnObject(nullptr, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
  }
}

void TBeing::freezeRoom() {
  TThing* t;
  TObj* obj = nullptr;
  int rc;

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj || !obj->canWear(ITEM_WEAR_TAKE))
      continue;

    rc = obj->freezeObject(nullptr, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
  }
}

void TBeing::acidRoom() {
  TThing* t;
  TObj* obj = nullptr;
  int rc;

  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    obj = dynamic_cast<TObj*>(t);
    if (!obj)
      continue;
    if (!obj->canWear(ITEM_WEAR_TAKE))
      continue;
    rc = obj->meltObject(nullptr, 100);
    if (IS_SET_ONLY(rc, DELETE_THIS)) {
      delete obj;
      obj = nullptr;
    }
  }
}

void TBeing::chlorineRoom() {
  for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end(); ++it) {
    (*it)->poisonObject();
  }
}

bool TBeing::noHarmCheck(const TBeing* vict) const {
  if (this == vict)
    return false;

  if (desc && IS_SET(desc->autobits, AUTO_NOHARM)) {
    if (vict->isPc()) {
      sendTo("You have your AUTO NOHARM flag set.\n\r");
      sendTo("You must remove it before attacking another PC.\n\r");
      return true;
    }
    if (vict->master == this) {
      if (vict->isPet(PETTYPE_PET)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your pets.\n\r");
        return true;
      }
      if (vict->isPet(PETTYPE_CHARM)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your charms.\n\r");
        return true;
      }
      if (vict->isPet(PETTYPE_THRALL)) {
        sendTo("You have your AUTO NOHARM flag set.\n\r");
        sendTo("You must remove it before attacking one of your thralls.\n\r");
        return true;
      }
    }
  }

  if (isPc() && vict->isPc() && !isImmortal() && !vict->isValidPkTarget(this)) {
    sendTo("Your victim is not a valid PK target.\n\r");
    return true;
  }

  if (desc && !isImmortal() && isPc() && vict->desc && vict->isPc() &&
      (vict->GetMaxLevel() <= MAX_NEWBIE_LEVEL) &&
      !vict->isPlayerAction(PLR_KILLABLE) &&
      !toggleInfo[TOG_NEWBIEPK]->toggle) {
    sendTo("Your victim is a newbie and protected.\n\r");
    sendTo(
      "If you have a problem with this newbie please see a god for "
      "action.\n\r");
    return true;
  }

  return false;
}

void TBeing::preKillCheck(bool rent) {
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
int TBeing::loseRound(double rounds, bool randomize, bool check) {
  double count;
  int num;

  if (isImmortal())
    return 0;

  float fx, fy;
  blowCount(check, fx, fy);
  count = ((fx + fy) * rounds);
  num = (int)count;

  if (randomize) {
    if (::number(0, 999) < (int)((count - (double)num) * 1000.0))
      num++;
  } else {
    // rounded up
    if (count - num)
      num++;
  }

  return num;
}

void TBeing::blowCount(bool check, float& fx, float& fy) const {
  float num;
  TThing *prim, *sec;
  TGun* gun;

  prim = heldInPrimHand();
  sec = heldInSecHand();

  if (!isPc()) {
    // all mobs

    num = min(12.0, getMult());

    if (hasClass(CLASS_MONK) && (prim || sec))
      num = (float)1.000;

    // cut attacks in half if using paired weapon
    // otherwise they will get double attacks
    TObj* tob = dynamic_cast<TObj*>(prim);
    if (tob && tob->isPaired())
      num /= 2.0;

    fx = 0.60 * num;
    fy = 0.40 * num;

    if ((gun = dynamic_cast<TGun*>(prim))) {
      fx = gun->getROF();
    }
    if ((gun = dynamic_cast<TGun*>(sec)) && !gun->isPaired()) {
      fy = gun->getROF();
    }

    // MOBS
  } else {
    // PCS

    // Get getMult returns the number of monk barehand attacks
    num = getMult();
    fx = fy = 0;

    // Primary hand
    // If nothing in the hand and a monk
    if (!prim && hasClass(CLASS_MONK))
      fx += (0.60 * num);
    // else we are holding something
    else if (prim) {
      // blowCountSplitter will return 0 if it is not a weapon and 1 if it is a
      // weapon
      fx = prim->blowCountSplitter(this, true);

      // Guns
      if ((gun = dynamic_cast<TGun*>(prim))) {
        fx = gun->getROF();
      }

      // Check specialization after guns
      if (getPosition() >= POSITION_STANDING) {
        fx += prim->specializationCheck(this);
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
      if ((gun = dynamic_cast<TGun*>(sec)) && !gun->isPaired()) {
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
      if (fx > 0.0)
        fx += 0.5;
      if (fy > 0.0)
        fy += 0.5;
      if (doesKnowSkill(SKILL_ADVANCED_BERSERKING)) {
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
  if (dynamic_cast<TBeing*>(riding)) {
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
