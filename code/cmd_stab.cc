//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_stab.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"

static int stab(TBeing *thief, TBeing * victim)
{
  const int STAB_MOVE = 2;
  int rc;
  int level;

  if (thief == victim) {
    thief->sendTo("Hey now, let's not be stupid.\n\r");
    return FALSE;
  }
  if (thief->checkPeaceful("Naughty, naughty.  None of that here.\n\r"))
    return FALSE;

  TGenWeapon * obj = dynamic_cast<TGenWeapon *>(thief->heldInPrimHand());
  if (!obj) {
    thief->sendTo("You need to wield a weapon, to make it a success.\n\r");
    return FALSE;
  }

  if (thief->riding) {
    thief->sendTo("Not while mounted!\n\r");
    return FALSE;
  }

  if (dynamic_cast<TBeing *> (victim->riding)) {
    thief->sendTo("Not while that person is mounted!\n\r");
    return FALSE;
  }

  if (thief->noHarmCheck(victim))
    return FALSE;

  if (!obj->canStab()) {
    act("You can't use $o to stab.",  false, thief, obj, NULL, TO_CHAR);
    return FALSE;
  }

  if (victim->attackers > 3) {
    thief->sendTo("There's not enough room for you to stab!\n\r");
    return FALSE;
  }

  if (thief->getMove() < STAB_MOVE) {
    thief->sendTo("You are too tired to stab.\n\r");
    return FALSE;
  }
  if (IS_SET(victim->specials.act, ACT_GHOST)) {
    // mostly because kill is "you slit the throat", etc.
    thief->sendTo("Ghosts can not be stabbed!\n\r");
    return FALSE;
  }

  thief->addToMove(-STAB_MOVE);

  thief->reconcileHurt(victim,0.06);

  level = thief->getSkillLevel(SKILL_STABBING);
  int bKnown = thief->getSkillValue(SKILL_STABBING);

  int dam = thief->getSkillDam(victim, SKILL_STABBING, level, thief->getAdvLearning(SKILL_STABBING));
  dam = thief->getActualDamage(victim, obj, dam, SKILL_STABBING);

  int i = thief->specialAttack(victim,SKILL_STABBING);
  if (!victim->awake() || 
      (i &&
       (i != GUARANTEED_FAILURE) &&
       bSuccess(thief, bKnown, SKILL_STABBING))) {

    switch (critSuccess(thief, SKILL_STABBING)) {
      case CRIT_S_KILL:
        if (!victim->getStuckIn(WEAR_BODY)) {
          CS(SKILL_STABBING);
          act("You thrust $p ***REALLY DEEP*** into $N and twist.", 
               FALSE, thief, obj, victim, TO_CHAR);
          act("$n thrusts $p ***REALLY DEEP*** into $N and twists.", 
               FALSE, thief, obj, victim, TO_NOTVICT);
          act("$n thrusts $p ***REALLY DEEP*** into you and twists it.", 
               FALSE, thief, obj, victim, TO_VICT);

          dam *= 4;
          act("You hit exceptionally well but lost your grasp on $p.", 
                 FALSE, thief, obj, victim, TO_CHAR, ANSI_RED);
          act("$n left $s $o stuck in you.", 
                 FALSE, thief, obj, victim, TO_VICT, ANSI_ORANGE);
          act("$n loses $s grasp on $p.", 
                 TRUE, thief, obj, victim, TO_NOTVICT);

          rc = victim->stickIn(thief->unequip(thief->getPrimaryHold()) ,WEAR_BODY);
          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_VICT;
          victim->rawBleed(WEAR_BODY, 150, SILENT_NO, CHECK_IMMUNITY_YES);
          break;
        }  // if already stuckIn, drop through to next
      case CRIT_S_TRIPLE:
        CS(SKILL_STABBING);
        act("You thrust $p REALLY DEEP into $N and twist.", 
               FALSE, thief, obj, victim, TO_CHAR);
        act("$n thrusts $p REALLY DEEP into $N and twists.", 
               FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n thrusts $p REALLY DEEP into you and twists it.", 
               FALSE, thief, obj, victim, TO_VICT);

        dam *= 3;
        break;
      case CRIT_S_DOUBLE:
        CS(SKILL_STABBING);
        dam *= 2;
        act("You thrust $p DEEP into $N and twist.", 
               FALSE, thief, obj, victim, TO_CHAR);
        act("$n thrusts $p DEEP into $N and twists.", 
               FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n thrusts $p DEEP into you and twists it.", 
               FALSE, thief, obj, victim, TO_VICT);

        break;
      case CRIT_S_NONE:
        act("You thrust $p into $N and twist.", 
               FALSE, thief, obj, victim, TO_CHAR);
        act("$n thrusts $p into $N and twists.", 
               FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n thrusts $p into you and twists it.", 
               FALSE, thief, obj, victim, TO_VICT);

        break;
    }
    if (thief->willKill(victim, dam, SKILL_STABBING, FALSE)) {
      act("Your hand is coated in ichor as you slit $N's guts!",
               FALSE, thief, obj, victim, TO_CHAR, ANSI_RED);
      act("Ichor spews from the gaping stab wound $n leaves in $N's lifeless body!",
               TRUE, thief, obj, victim, TO_NOTVICT);
    }

    if (thief->reconcileDamage(victim, dam, SKILL_STABBING) == -1)
      return DELETE_VICT;
  } else {
    switch (critFail(thief, SKILL_STABBING)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SKILL_STABBING);
        act("You miss your thrust into $N and stab yourself.", FALSE, thief, obj, victim, TO_CHAR);
        act("$n misses $s thrust into $N and stabs $mself.", FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n misses $s thrust into you and stabs $mself.", FALSE, thief, obj, victim, TO_VICT);
        if (thief->reconcileDamage(thief, dam/3, SKILL_STABBING) == -1)
          return DELETE_THIS;
        break;
      case CRIT_F_NONE:
        act("You miss your thrust into $N.", FALSE, thief, obj, victim, TO_CHAR);
        act("$n misses $s thrust into $N.",  FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n misses $s thrust into you.", FALSE, thief, obj, victim, TO_VICT);
        thief->reconcileDamage(victim, 0, SKILL_STABBING);
    }
  }
  return TRUE;
}

int TBeing::doStab(const char * argument, TBeing *vict)
{
  TBeing *victim;
  char namebuf[256];
  int rc;

  if (!doesKnowSkill(SKILL_STABBING)) {
    sendTo("You haven't learned how to stab yet.\n\r");
    return FALSE;
  }
  if (checkBusy(NULL)) {
    return FALSE;
  }
  only_argument(argument, namebuf);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      if (!(victim = fight())) {
        sendTo("Stab whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = stab(this, victim);
  if (rc)
    addSkillLag(SKILL_STABBING);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
