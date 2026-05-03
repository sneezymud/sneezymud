//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "being.h"
#include "combat.h"
#include "obj_base_clothing.h"
#include "obj_general_weapon.h"

static int grapple(TBeing* c, TBeing* victim, spellNumT skill) {
  int percent;
  int level, i = 0;
  int rc;
  int bKnown;
  int grapple_move = 25 + ::number(1, 10);

  if (c->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (c->getCombatMode() == ATTACK_BERSERK) {
    c->sendTo(
      "You are berserking! You can't focus enough to grapple anyone!\n\r");
    return FALSE;
  }

  if (victim == c) {
    c->sendTo("Aren't we funny today?\n\r");
    return FALSE;
  }
  if (c->noHarmCheck(victim))
    return FALSE;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    c->sendTo("You can't grapple an immortal.\n\r");
    return FALSE;
  }

  if (c->riding) {
    c->sendTo("Grappling while mounted is impossible!\n\r");
    return FALSE;
  }
  if (victim->isFlying() && !c->isFlying()) {
    c->sendTo(
      "You can't grapple someone that is flying, unless you are also "
      "flying.\n\r");
    return FALSE;
  }

  if (c->getMove() < grapple_move) {
    c->sendTo("You lack the vitality to grapple.\n\r");
    return FALSE;
  }
  c->addToMove(-grapple_move);

  percent = 0;

  percent += c->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;

  if (victim->riding) {
    // difficult
    percent -= 35;
  }
  level = c->getSkillLevel(skill);

  bKnown = c->getSkillValue(skill);

  if ((c->bSuccess(bKnown + percent, skill) &&
        // insure they can hit this critter
        (i = c->specialAttack(victim, skill, 0, STAT_STR, STAT_DEX, STAT_BRA,
           STAT_AGI, false)) &&
        i != GUARANTEED_FAILURE &&
        // make sure they have reasonable training
        (percent < bKnown)) ||
      !victim->awake()) {
    if (victim->canCounterMove(bKnown / 2)) {
      SV(skill);
      act("$N blocks your grapple attempt and knocks you off balance.", true, c,
        0, victim, TO_CHAR, ANSI_RED);
      act("$N blocks $n's attempt to grapple, and knocks $m off balance.", true,
        c, 0, victim, TO_NOTVICT);
      act("You evade $n's attempt to grapple, and knock $m off balance.", true,
        c, 0, victim, TO_VICT);
      c->cantHit += c->loseRound(5 - (min(50, level) / 12));

      rc = c->stumble();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return rc;
    } else if (victim->canFocusedAvoidance(bKnown / 2)) {
      SV(skill);
      act("$N avoids your grapple attempt.", TRUE, c, 0, victim, TO_CHAR,
        ANSI_RED);
      act("$N avoids $n's attempt to grapple.", TRUE, c, 0, victim, TO_NOTVICT);
      act("You evade $n's attempt to grapple.", TRUE, c, 0, victim, TO_VICT);
    } else {
      if (victim->riding) {
        int kr = victim->knockOffMount(c->getSkillValue(skill) / 4);
        if (IS_SET_DELETE(kr, DELETE_THIS))
          return DELETE_VICT;
        if (victim->riding) {
          // Hung on — grapple can't wrestle a mounted target down.
          act("Your grapple slips as $N keeps $S seat.", true, c, 0, victim,
            TO_CHAR);
          act("$n's grapple slips as $N keeps $S seat.", true, c, 0, victim,
            TO_NOTVICT);
          act("$n grapples at you but slips as you keep your seat.", true, c, 0,
            victim, TO_VICT);
          c->cantHit += c->loseRound(2);
          return true;
        }
      }
      c->sendTo("You tie your opponent up, with an excellent maneuver.\n\r");
      act("$n wrestles $N to the $g with an excellent maneuver.", TRUE, c, 0,
        victim, TO_NOTVICT);
      act("$n wrestles you to the $g.", TRUE, c, 0, victim, TO_VICT);

      int dam = c->getSkillDam(victim, skill, c->getSkillLevel(skill),
        c->getAdvLearning(skill));
      // Use impactSpec to handle all impact effects (spikes, thorns, hardness)
      wearSlotT targetLimb = victim->getPartHit(c, TRUE);
      dam += impactSpec(c, victim, WEAR_BODY, targetLimb);
      if (c->reconcileDamage(victim, dam, SKILL_GRAPPLE) == -1)
        return DELETE_VICT;

      // Both end up pinned on the ground. Use setPosition + pin-specific
      // flavor instead of crashLanding so the messages match the takedown
      // context and the auto-springleap chain doesn't fire mid-pin.
      c->setPosition(POSITION_SITTING);
      victim->setPosition(POSITION_RESTING);
      act("<g>You kneel over $N, pinning $M to the $g.<1>", true, c, nullptr,
        victim, TO_CHAR);
      act("<r>$n kneels over you, pinning you to the $g.<1>", true, c, nullptr,
        victim, TO_VICT);
      act("$n kneels over $N, pinning $M to the $g.", true, c, nullptr, victim,
        TO_NOTVICT);

      if (!victim->fight()) {
        if (c->fight()) {
          if (c->fight() != victim) {
            act("You now turn your attention to $N!", TRUE, c, 0, victim,
              TO_CHAR);
            act("$n now turns $s attention to $N!", TRUE, c, 0, victim,
              TO_NOTVICT);
            act("$n has turned $s attention to you!", TRUE, c, 0, victim,
              TO_VICT);
          }
          c->stopFighting();
          c->setCharFighting(victim);
        } else {
          c->setCharFighting(victim);
        }
        c->setVictFighting(victim);
      } else if (::number(1, 5) < 4) {
        if (c->fight() && (c->fight() != victim)) {
          act("You now turn your attention to $N!", TRUE, c, 0, victim,
            TO_CHAR);
          act("$n now turns $s attention to $N!", TRUE, c, 0, victim,
            TO_NOTVICT);
          act("$n has turned $s attention to you!", TRUE, c, 0, victim,
            TO_VICT);
        }
        if (victim->fight() && (victim->fight() != c)) {
          act("$N now turns $S attention to you!", TRUE, c, 0, victim, TO_CHAR);
          act("$N now turns $S attention to $n!", TRUE, c, 0, victim,
            TO_NOTVICT);
          act("You now turn your attention to $n!", TRUE, c, 0, victim,
            TO_VICT);
        }
        if (c->fight())
          c->stopFighting();

        if (victim->fight())
          victim->stopFighting();
        c->setCharFighting(victim);
        c->setVictFighting(victim);
      }
      c->cantHit += c->loseRound(4);
      victim->cantHit += victim->loseRound(4);

      if (victim->spelltask) {
        victim->addToDistracted(1, FALSE);
      }
      victim->addToWait(combatRound(5));
    }
  } else {
    c->cantHit += c->loseRound(5 - (min(level, 50) / 12));
    c->addToWait(combatRound(1));

    act("You try to wrestle $N to the $g, but lose your hold.", true, c, 0,
      victim, TO_CHAR);
    act("$n tries to wrestle $N to the $g, but loses $s hold.", true, c, 0,
      victim, TO_NOTVICT);
    act("$n tries to wrestle you to the $g, but loses $s hold.", true, c, 0,
      victim, TO_VICT);

    rc = c->stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    if (!victim->fight()) {
      act("$N turns $S attention to $n", TRUE, c, 0, victim, TO_NOTVICT);
      c->setCharFighting(victim);
      c->setVictFighting(victim);
    }
  }
  c->reconcileHurt(victim, 0.01);
  return TRUE;
}

int TBeing::doGrapple(const char* argument, TBeing* vict) {
  int rc;
  char name_buf[30];
  TBeing* victim;

  spellNumT skill = getSkillNum(SKILL_GRAPPLE);

  if (checkBusy()) {
    return FALSE;
  }
  strcpy(name_buf, argument);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Grapple whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about grappling.\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = grapple(this, victim, skill);
  if (rc)
    addSkillLag(skill, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
