#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

static int deathstroke(TBeing *caster, TBeing *victim)
{
  int i = 0, percent;
  bool wasSuccess = FALSE; 
  const int DEATHSTROKE_MOVE = 15;
  TBaseWeapon *tw;
  spellNumT sktype = SKILL_DEATHSTROKE;
  affectedData aff1, aff2;
  
  if (!caster->doesKnowSkill(SKILL_DEATHSTROKE)) {
    caster->sendTo("You know nothing about deathblows.\n\r");
    return FALSE;
  }

  if (caster->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  // Adding a lockout 
  if (caster->affectedBySpell(SKILL_DEATHSTROKE)) {
    caster->sendTo("You are still recovering from your last deathstroke and cannot use this ability again at this time.\n\r");
    return FALSE;
  }

  if (caster->getMove() < DEATHSTROKE_MOVE) {
    caster->sendTo("You don't have the vitality to make the move!\n\r");
    return FALSE;
  }
  if (victim == caster) {
    caster->sendTo("Do you REALLY want to kill yourself?...\n\r");
    return FALSE;
  }
  if (caster->noHarmCheck(victim))
    return FALSE;
  /* to prevent from misuse in groups ... */
  if (caster->riding) {
    caster->sendTo("You can't deathstroke while mounted!\n\r");
    return FALSE;
  }
  if (!caster->heldInPrimHand() || 
      !(tw=dynamic_cast<TBaseWeapon *>(caster->heldInPrimHand()))) {
    caster->sendTo("You need to hold a weapon in your primary hand to make this a success.\n\r");
    return FALSE;
  }
  
  if (!caster->isImmortal())
    caster->addToMove(-DEATHSTROKE_MOVE);

  int lev = caster->getSkillLevel(SKILL_DEATHSTROKE);
  int bKnown= caster->getSkillValue(SKILL_DEATHSTROKE);
  int successfulHit = caster->specialAttack(victim, SKILL_DEATHSTROKE);
  int successfulSkill = caster->bSuccess(bKnown, SKILL_DEATHSTROKE);
  int dam = caster->getSkillDam(victim, SKILL_DEATHSTROKE, lev, caster->getAdvLearning(SKILL_DEATHSTROKE));

  // Assessing an armor penalty regardless of success - this penalty greatly scales with level
  // to ensure that players are not overly penalized early on
  aff1.type = SKILL_DEATHSTROKE;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = (caster->GetMaxLevel()*caster->GetMaxLevel())/5 - caster->getSkillLevel(SKILL_DEATHSTROKE);
  aff1.bitvector = 0;

  if (!victim->awake() || 
    (successfulHit && successfulSkill && successfulHit != GUARANTEED_FAILURE)) {
    wasSuccess = TRUE;

    if (!dam) {
      act("$n's attempt at $N's vital area is ineffective.",
                FALSE, caster, 0, victim, TO_NOTVICT);
      act("You hit to $N's vital area is ineffective.", FALSE, caster, 0, victim, 
                TO_CHAR);
      act("$n attempts to hit your vital area, but the blow is ineffective.", 
                FALSE, caster, 0, victim, TO_VICT);

    } else {
      act("$n hits $N in $S vital organs!", FALSE, caster, 0, victim, 
                TO_NOTVICT);
      act("You hit $N in $S vital organs!", FALSE, caster, 0, victim, 
                TO_CHAR);
      act("$n hits you in your vital organs!", FALSE, caster, 0, 
                victim, TO_VICT);

      // You successfully landed your deathstroke - assessing a bonus
      aff2.type = SKILL_DEATHSTROKE;
      aff2.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
      aff2.location = APPLY_HITROLL;
      aff2.modifier = 3;
      aff2.bitvector = 0;
    }

    TThing *ob = caster->heldInPrimHand();
    if (ob && ob->isBluntWeapon())
      sktype = DAMAGE_CAVED_SKULL;
    if (caster->reconcileDamage(victim, dam,sktype) == -1) 
      return DELETE_VICT;
  }
  // Unsuccessful skill check
  else {
    if (victim->getPosition() > POSITION_DEAD) {
      act("$n's attempt at $N's vital area falls far short of hitting.", 
                  FALSE, caster, 0, victim, TO_NOTVICT);
      act("You fail to hit $N's vital area.", FALSE, caster, 0, victim, 
                  TO_CHAR);
      act("$n attempts to hit your vital area, but fails miserably.", 
                  FALSE, caster, 0, victim, TO_VICT);
    }

    if (caster->reconcileDamage(victim, 0,sktype) == -1) 
      return DELETE_VICT;
  }
  // Applying the debuff and, on success, the buff
  if (wasSuccess) {
    caster->affectJoin(caster, &aff1, AVG_DUR_YES, AVG_EFF_YES, FALSE);
    caster->affectJoin(caster, &aff2, AVG_DUR_YES, AVG_EFF_YES, FALSE);
  }
  else {
    caster->affectTo(&aff1, -1);
  }

    // what happens here is that the mob gets a shot at the players vitals
    // ... fair is fair right? 
    percent = ((10 - (caster->getArmor() / 20)) << 1);
    percent += victim->getDexReaction() * 5;
    percent -= caster->getAgiReaction() * 5;

    int bKnown2 = victim->getSkillValue(SKILL_DEATHSTROKE);

    if (bKnown2 > 0 &&
        victim->bSuccess(bKnown2 + percent, SKILL_DEATHSTROKE) &&
         (i = victim->specialAttack(caster,SKILL_DEATHSTROKE)) &&
         i != GUARANTEED_FAILURE) {
      /* victim hits attacker vitals while attacker is exposed */
      CF(SKILL_DEATHSTROKE);
      if (victim->getPosition() > POSITION_STUNNED) {

        dam = victim->GetMaxLevel()*2;
        dam += victim->plotStat(STAT_CURRENT, STAT_STR, 0, 6, 3);
        dam = ::number(victim->GetMaxLevel()/3, dam);
        dam = victim->getActualDamage(victim, NULL, dam, SKILL_DEATHSTROKE);
        dam /= 3;

        act("$N exploits $n's vulnerable state with a quick hit to the heart.",
              FALSE, caster, 0, victim, TO_NOTVICT);
        act("While you are vulnerable, $N strikes you in the center of your torso.",
              FALSE, caster, 0, victim, TO_CHAR);
        act("You take advantage of $n's vulnerability for a cheap shot!", 
             FALSE, caster, 0, victim, TO_VICT);

        TThing *ob = victim->heldInPrimHand();
        if (ob && ob->isBluntWeapon())
          sktype = DAMAGE_CAVED_SKULL;
        if (victim->reconcileDamage(caster, dam,sktype) == -1)
          return DELETE_THIS;
      }
    }

  /* success make monster attack caster player as player is  now */
  /* perceived as a greatest threat to the monster's livelyhood */
  TBeing *cfight = caster->fight();
  TBeing *vfight = victim->fight();
  if (!wasSuccess){
    // on failure, only set fighting if not already fighting
     if (!cfight)
       caster->setCharFighting(victim);
     if (!vfight)
       caster->setVictFighting(victim);
  } else if (!vfight && caster->sameRoom(*victim)) {
    // check sameRoom since may have fled from the damage
    if (victim->getPosition() > POSITION_STUNNED) {
      act("You begin fighting $n.", TRUE, caster, 0, victim, TO_VICT);
      act("$N begins fighting $n.", TRUE, caster, 0, victim, TO_NOTVICT);
      act("$N begins fighting you.", TRUE, caster, 0, victim, TO_CHAR);
    }
    if (!cfight) {
      act("You begin fighting $N.", TRUE, caster, 0, victim, TO_CHAR);
      act("$n begins fighting $N.", TRUE, caster, 0, victim, TO_NOTVICT);
      act("$n begins fighting you.", TRUE, caster, 0, victim, TO_VICT);
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
    } else if (cfight && (cfight == vfight)) {
      caster->setVictFighting(victim);
      vlogf(LOG_BUG, format("Should never have gotten here in deathstroke (%s)") %  caster->getName());
    } else if (cfight && (cfight != victim) && (::number(0,4) < 2)) {
      act("You turn your attention to $N.", TRUE, caster, 0, victim, TO_CHAR);
      act("$n turns $s attention to $N.", TRUE, caster, 0, victim, TO_NOTVICT);
      act("$n turns $s attention to you.", TRUE, caster, 0, victim, TO_VICT);
      caster->stopFighting();
      caster->setCharFighting(victim);
      caster->setVictFighting(victim);
    } else {
      caster->setVictFighting(victim);
      // do nothing here cept set victim fighting
    }
  } else if (caster->sameRoom(*victim)) { 
    // again, check sameRoom in case victim fled when he was hit
    if (!cfight) {
      if (caster->getPosition() > POSITION_STUNNED) {
        act("You begin fighting $N.", TRUE, caster, 0, victim, TO_CHAR);
        act("$n begins fighting $N.", TRUE, caster, 0, victim, TO_NOTVICT);
        act("$n begins fighting you.", TRUE, caster, 0, victim, TO_VICT);
      }
      caster->setCharFighting(victim);
      if (::number(0,4) < 2) {  // chance of victim switching to you
        if (vfight) {
          act("You turn your attention to $n.", 
              TRUE, caster, 0, victim, TO_VICT);
          act("$N turns $S attention to $n.", 
              TRUE, caster, 0, victim, TO_NOTVICT);
          act("$N turns $S attention to you.", 
              TRUE, caster, 0, victim, TO_CHAR);
          victim->stopFighting();
          caster->setVictFighting(victim);
        }
      }
    } else if (cfight != victim) {    // victim and you fighting
      if (::number(0,4) < 2) {  // chance of victim switching to you
        if (victim->getPosition() > POSITION_STUNNED) {
          act("You turn your attention to $n and you engage each other.",
              TRUE, caster, 0, victim, TO_VICT);
          act("$N turns $S attention to $n and they begin to fight.",
              TRUE, caster, 0, victim, TO_NOTVICT);
          act("$N turns $S attention to you and you begin to fight.",
              TRUE, caster, 0, victim, TO_CHAR);
        } else if (caster->getPosition() > POSITION_STUNNED) {
          act("You turn your attention to $N.", 
              TRUE, caster, 0, victim, TO_CHAR);
          act("$n turns $s attention to $N.", 
              TRUE, caster, 0, victim, TO_NOTVICT);
          act("$n turns $s attention to you.", 
              TRUE, caster, 0, victim, TO_VICT);
        }
        caster->stopFighting();
        victim->stopFighting();
        caster->setCharFighting(victim);
        caster->setVictFighting(victim);
      } else {
       // do nothing
      }
    }
  } else {
    // victim fled and is in other room
    // do nothing
  }
  return TRUE;
}

int TBeing::doDeathstroke(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char v_name[MAX_INPUT_LENGTH];
  
  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_DEATHSTROKE)) {
    sendTo("You know nothing about making deathstrokes.\n\r");
    return FALSE;
  }
  strcpy(v_name, argument);
  
  if (!(victim = vict)) {

    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        sendTo("Deathstroke whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = deathstroke(this,victim);

  if (rc)
    addSkillLag(SKILL_DEATHSTROKE, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  return rc;
}

