#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_general_weapon.h"
#include "materials.h"
#include "skills.h"

//#define ALLOW_STAB_SEVER
#define USE_NEW_STAB

spellNumT doStabMsg(TBeing *tThief, TBeing *tSucker, TGenWeapon *tWeapon, wearSlotT tLimb, int & tDamage, int tSever)
{
  const float tDamageValues[MAX_WEAR] =
  { 0.00, // Nowhere
    1.50, // Head
    1.10, // Neck
    1.05, // Body
    1.10, // Back
    0.90, // Arm-R
    0.90, // Arm-L
    0.80, // Wrist-R
    0.80, // Wrist-L
    0.40, // Hand-R
    0.40, // Hand-L
    0.20, // Finger-R
    0.20, // Finger-L
    1.20, // Waist
    1.10, // Leg-R
    1.10, // Leg-L
    0.30, // Foot-R
    0.30, // Foot-L
    0.00, // Hold-R
    0.00, // Hold-L
    1.10, // Ex-Leg-R
    1.10, // Ex-Leg-L
    0.30, // Ex-Foot-R
    0.30, // Ex-Foot-L
  };

  spellNumT tDamageType = DAMAGE_NORMAL;

  switch (tLimb) {
    case WEAR_HEAD:
      tDamageType = (!(rand() % 5) ? DAMAGE_CAVED_SKULL : SKILL_STABBING);
      break;
    case WEAR_NECK:
      tDamageType = (!(rand() % 5) ? DAMAGE_BEHEADED : SKILL_STABBING);
      break;
    case WEAR_BODY:
      tDamageType = DAMAGE_IMPALE;
      break;
    case WEAR_WAIST:
      tDamageType = DAMAGE_DISEMBOWLED_VR;
      break;
    case WEAR_BACK:
      tDamageType = SKILL_STABBING;
      break;
    default:
      tDamageType = TYPE_STAB;
      break;
  }

  // Basically damage is varied based on the limb here.
  tDamage = (int) ((float) tDamage * tDamageValues[tLimb]);

  bool tKill = tThief->willKill(tSucker, tDamage, tDamageType, FALSE);

  sstring tStLimb(tSucker->describeBodySlot(tLimb));
  sstring tStringChar, tStringVict, tStringOthr, tStringMess;

  if (tLimb == WEAR_HEAD && !(rand() % 99) && !tSucker->hasDisease(DISEASE_EYEBALL)) {
    affectedData tAff;

    tStringChar="You glance $N's eyes with your $o, slicing them wide open!";
    tStringVict="$n glances your eyes with $s $o, slicing them wide open!  YOUR BLIND!";
    tStringOthr="$n glances $N's eyes with $s $o, slicing them wide open!";

    tAff.type      = AFFECT_DISEASE;
    tAff.level     = 0;
    tAff.duration  = PERMANENT_DURATION;
    tAff.modifier  = DISEASE_EYEBALL;
    tAff.location  = APPLY_NONE;
    tAff.bitvector = AFF_BLIND;
    tSucker->affectTo(&tAff);
    tSucker->rawBlind(tThief->GetMaxLevel(), tAff.duration, SAVE_NO);
  } else switch (::number(0, 5)) {
    case 0:
      tStringChar=format("You thrust your $o into $N's %s!") % tStLimb;
      tStringVict=format("$n thrusts $s $o into your %s!") % tStLimb;
      tStringOthr=format("$n thrusts $s $o into $N's %s!") % tStLimb;
      break;

    case 1:
      tStringChar=format("You stab $N in $S %s!") % tStLimb;
      tStringVict=format("$n stabs you in your %s!") % tStLimb;
      tStringOthr=format("$n stabs $N in $S %s!") % tStLimb;
      break;

    case 2:
      tStringChar=format("You gouge $N in $S %s!") % tStLimb;
      tStringVict=format("$n gouges you in your %s!") % tStLimb;
      tStringOthr=format("$n gouges $N in $S %s!") % tStLimb;

    default:
      tStringChar=format("You puncture $N's %s with your $o!") % tStLimb;
      tStringVict=format("$n punctures your %s with $s $o!") % tStLimb;
      tStringOthr=format("$n punctures $N's %s with $s $o!") % tStLimb;
      break;
  }

  act(tStringChar, FALSE, tThief, tWeapon, tSucker, TO_CHAR);
  act(tStringVict, FALSE, tThief, tWeapon, tSucker, TO_VICT);
  act(tStringOthr, FALSE, tThief, tWeapon, tSucker, TO_NOTVICT);

  if (tKill) {
    switch (tLimb) {
      case WEAR_HEAD:
        if (tDamageType == SKILL_STABBING) {
          tStringChar="Your weapon sinks DEEP into $N's skull!";
          tStringVict="$n's weapon sinks DEEP into your skull!";
	  tStringOthr="$n's weapon sinks DEEP into $N's skull!";
        } else {
          tStringChar="You cause $N's skull to cave in!";
          tStringVict="$n caused your skull to cave in!";
          tStringOthr="$n caused $N's skull to cave in!";
          tStringMess="You get a good look at $N's grey matter...";
        }

        break;

      case WEAR_NECK:
        if (tDamageType == SKILL_STABBING) {
          tStringChar="You slice $N's neck wide open!";
          tStringVict="$n slices your neck wide open!";
          tStringOthr="$n slices $N's neck wide open!";
          tStringMess="Blood spews forth from $N's severed jugular!";
        } else {
          tStringChar="You sever $N at the neck!";
          tStringVict="$n severs you at the neck!";
          tStringOthr="$n severs $N at the neck!";
          tStringMess="Blood gushes from $N's neck!";
        }

        break;

      case WEAR_BODY:
        if (!(rand() % 9)) {
          tStringChar="You thrust $p DEEP into $N's gut!";
          tStringVict="$n thusts $p DEEP into your gut, good thing you are dead now!";
          tStringOthr="$n thrusts $p DEEP into $N's gut!";
        } else {
          tStringChar="You plunge $p DEEP into $N's chest, you can feel $S heart slow then stop through your weapon";
          tStringVict="$n plunges $p DEEP into your chest, piercing your heart!";
          tStringOthr="$n plunges $p DEEP into $N's chest, looks like he nailed them in the heart!";
          tStringMess="Blood spews forth from the stab wound!";
        }

        break;

      case WEAR_WAIST:
        tStringChar="You slice $N from love handle to love handle!";
        tStringVict="$n slices you from love handle to love handle!";
	tStringOthr="$n slices $N from love handle to love handle!";
        break;

      case WEAR_BACK:
        if (!(rand() % 9) && !tSucker->raceHasNoBones()) {
          tStringChar="You rake $p down $N's back, slicing the spine!";
          tStringVict="$n rakes $p down your back, slicing your spine!";
          tStringOthr="$n rakes $p down the back, slicing their spine!";
        } else {
          tStringChar="You carve a rather nice hole in $N's back!";
          tStringVict="$n carves you a good sized hole in your back!";
          tStringOthr="$n carves $N a good sized hole in the back!";
        }

        break;

      default:
        tStringChar=format("Your stab to $N's %s ceases their existence!") %
	  tStLimb;
        tStringVict=format("$n stabs you in your %s, ceasing your existence!") %
	  tStLimb;
        tStringOthr=format("$n stabs $N in $S %s, ceasing their existence!") %
	  tStLimb;
        break;
    }

    act(tStringChar, FALSE, tThief, tWeapon, tSucker, TO_CHAR);
    act(tStringVict, FALSE, tThief, tWeapon, tSucker, TO_VICT);
    act(tStringOthr, FALSE, tThief, tWeapon, tSucker, TO_NOTVICT);

    if(!tStringMess.empty()){
      act(tStringMess, FALSE, tThief, tWeapon, tSucker, TO_CHAR);
      act(tStringMess, FALSE, tThief, tWeapon, tSucker, TO_ROOM);
    }

    if (tLimb == WEAR_NECK)
      if (tDamageType == DAMAGE_BEHEADED)
        tSucker->makeBodyPart(WEAR_HEAD, tThief);
      else
        tSucker->dropPool(50, LIQ_BLOOD);
  } else {
    // Apply some limb damage and have a *Very* remote chance of whacking the limb off.

    int tHardness = material_nums[tSucker->getMaterial(tLimb)].hardness *
                    tSucker->getCurLimbHealth(tLimb) /
                    tSucker->getMaxLimbHealth(tLimb);
    int tRc;

    // This was mostly pulled from combat.cc:damageLimb()
    if ((::number(1, (tSucker->getMaxLimbHealth(tLimb) / 4)) < tDamage) &&
        (::number(1, 101) >= (tSucker->getConShock() / 3)) &&
        (!tHardness || (::number(1, 101) >= (tHardness / 2)))) {
      float tNewDamage = (tDamage * (25 + tThief->GetMaxLevel()));

      tNewDamage /= 100; // Make the damage *REAL* light
      tNewDamage = std::max((float) 0.0, tNewDamage);

      // These limbs should be VERY difficult to damage with this.
      if (tLimb == WEAR_HEAD ||
          tLimb == WEAR_NECK ||
          tLimb == WEAR_BODY ||
          tLimb == WEAR_BACK ||
          tLimb == WEAR_WAIST)
        tNewDamage /= 10;

      tRc = tSucker->hurtLimb((unsigned int)tNewDamage, tLimb);

      if (IS_SET_DELETE(tRc, DELETE_THIS)) {
        tDamage = -1;
        return tDamageType;
      }

      // If weapon is poisoned, then toss that sucker in there!
      // not bleeding  == poison victim
      // limb bleeding == infect limb
      for (int tSwingIndex = 0; tSwingIndex < MAX_SWING_AFFECT; tSwingIndex++) {
        int tDuration = (tThief->GetMaxLevel() * UPDATES_PER_MUDHOUR);

  
	if(tWeapon->isPoisoned()){
          if (!tSucker->isLimbFlags(tLimb, PART_BLEEDING) &&
              !tSucker->isAffected(AFF_POISON)) {
            if (!tSucker->isImmune(IMMUNE_POISON, tLimb) && !::number(0, 9)) {
	      tWeapon->applyPoison(tSucker);
	      

              act("You poison $N with your stab!",
                  FALSE, tThief, NULL, tSucker, TO_CHAR);
              act("You begin to feel strange, that bastard $n just poisoned you!",
                  FALSE, tThief, NULL, tSucker, TO_VICT);
              act("$N gets a strange look on their face, apparently they are now poisoned!",
                  FALSE, tThief, NULL, tSucker, TO_NOTVICT);
            }
          } else if (!tSucker->isLimbFlags(tLimb, PART_INFECTED))
            if (!::number(0, 9) &&
                tSucker->rawInfect(tLimb, tDuration, SILENT_YES, CHECK_IMMUNITY_YES)) {
              tStringChar=format("Your stab to $N's %s infects it!") %
		tStLimb;
              tStringVict=format("Your %s gets infected from $n's stab!") %
		tStLimb;
              tStringOthr=format("$N's %s gets infected from $n's stab!") %
		tStLimb;
	      
              act(tStringChar, FALSE, tThief, NULL, tSucker, TO_CHAR);
              act(tStringVict, FALSE, tThief, NULL, tSucker, TO_VICT);
              act(tStringOthr, FALSE, tThief, NULL, tSucker, TO_NOTVICT);
            }
	}
      }
    } else {
#if ALLOW_STAB_SEVER
      // Here is where we check the tSever bit.
      // Crit 3: 30 : 31 in 531
      // Crit 2: 15 : 16 in 516
      // Crit 1:  5 :  6 in 506
      // Normal:  0 :  1 in 501

      int tNewSever = (int) (((float)tSever * -tDamageValues[tLimb]) - 4.0);

      // We further modify it based on the limb for a reason:
      // Chance for whacking:
      // Head   :  2 (can not be whacked)
      // Neck   :  1 (can not be whacked)
      // Body   :  1 (can not be whacked)
      // Back   :  1 (can not be whacked)
      // Arms   :  0 (can     be whacked)
      // Waist :  2 (can not be whacked)
      // Legs   :  1 (can not be whacked)
      // Fingers: -4 (can     be whacked)
      if (::number(tNewSever, 500) <= 0 && !tSucker->isLucky(tSucker->GetMaxLevel())) {
        tStringChar=format("You slice $N's %s right off!") % tStLimb;
        tStringVict=format("$n slices your %s right off!") % tStLimb;
        tStringOthr="$n slices $N's %s right off!") % tStLimb;

        tSucker->makePartMissing(tLimb, false, tThief);

        act(tStringChar, FALSE, tThief, NULL, tSucker, TO_CHAR);
        act(tStringVict, FALSE, tThief, NULL, tSucker, TO_VICT);
        act(tStringOthr, FALSE, tThief, NULL, tSucker, TO_NOTVICT);
      }
#endif
    }
  }

  return tDamageType;
}

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

  if (!obj->canStab()){
    act("You can't use $o to stab.",  false, thief, obj, NULL, TO_CHAR);
    return FALSE;
  }

  if (thief->getMove() < STAB_MOVE) {
    thief->sendTo("You are too tired to stab.\n\r");
    return FALSE;
  }
  /*
  if (IS_SET(victim->specials.act, ACT_GHOST)) {
    // mostly because kill is "you slit the throat", etc.
    thief->sendTo("Ghosts can not be stabbed!\n\r");
    return FALSE;
  }
  */

  thief->addToMove(-STAB_MOVE);

  thief->reconcileHurt(victim,0.06);

  level = thief->getSkillLevel(SKILL_STABBING);
  int bKnown = thief->getSkillValue(SKILL_STABBING);

  int dam = thief->getSkillDam(victim, SKILL_STABBING, level, thief->getAdvLearning(SKILL_STABBING));
  dam = thief->getActualDamage(victim, obj, dam, SKILL_STABBING);

  int i = thief->specialAttack(victim,SKILL_STABBING);


#ifdef USE_NEW_STAB
  // Start of test stab-limb code.


  wearSlotT tLimb  = victim->getPartHit(thief, FALSE);
  int       tSever = 0;

  if (!victim->hasPart(tLimb))
    tLimb = victim->getCritPartHit();

  if (!victim->awake() ||
      (i && (i != GUARANTEED_FAILURE) && 
      thief->bSuccess(bKnown, SKILL_STABBING))) {
    switch (critSuccess(thief, SKILL_STABBING)) {
      case CRIT_S_KILL:
        CS(SKILL_STABBING);
        dam   *=  4;
        tSever = 30;
        break;
      case CRIT_S_TRIPLE:
        CS(SKILL_STABBING);
        dam   *=  3;
        tSever = 15;
        break;
      case CRIT_S_DOUBLE:
        CS(SKILL_STABBING);
        dam   *= 2;
        tSever = 5;
        break;
      case CRIT_S_NONE:
        tSever = 0;
        break;
    }

    spellNumT tDamageType = doStabMsg(thief, victim, obj, tLimb, dam, tSever);

    // If they got nuked in doStabMsg then work with that.
    if (dam == -1 || thief->reconcileDamage(victim, dam, tDamageType) == -1)
      return DELETE_VICT;

    if (obj->checkSpec(victim, CMD_STAB, "-special-", thief) == DELETE_VICT)
      return DELETE_VICT;

    return TRUE;
  } else {
    switch (critFail(thief, SKILL_STABBING)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        act("You over thrust and fall flat on your face!",
            FALSE, thief, obj, victim, TO_CHAR);
        act("$n misses $s thrust into $N and falls flat on their face!",
            FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n misses $s thrust into you and falls flat on their face!",
            FALSE, thief, obj, victim, TO_VICT);
        rc = thief->crashLanding(POSITION_SITTING);

        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;

        break;
      case CRIT_F_NONE:
      default:
        act("You miss your thrust into $N.",
            FALSE, thief, obj, victim, TO_CHAR);
        act("$n misses $s thrust into $N.",
            FALSE, thief, obj, victim, TO_NOTVICT);
        act("$n misses $s thrust into you.",
            FALSE, thief, obj, victim, TO_VICT);
        break;
    }

    thief->reconcileDamage(victim, 0, SKILL_STABBING);
  }

  return TRUE;

  // End of test stab-limb code.
#else
  // Start of old stab code.

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

  // End of old stab code
#endif

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
  if (checkBusy()) {
    return FALSE;
  }
  strcpy(namebuf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, namebuf))) {
      if (!(victim = fight())) {
        sendTo("Stab whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (IS_SET(victim->specials.act, ACT_IMMORTAL) || victim->isImmortal()) {
    sendTo("Your stab attempt has no effect on your immortal target.\n\r");
    return FALSE;
  }
  rc = stab(this, victim);
  if (rc)
    addSkillLag(SKILL_STABBING, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
