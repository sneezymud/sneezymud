//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "crit_combat.cc" - All functions and routines related to combat
//
//////////////////////////////////////////////////////////////////////////
#include <cmath>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "statistics.h"
#include "obj_corpse.h"
#include "obj_gun.h"
#include "obj_trash.h"
#include "obj_drinkcon.h"
#include "database.h"
#include "materials.h"
#include "person.h"

// adjust the crit table for fighting barehand.  All crit types are not equal,
// but for barehand we want them to appear so.  This means nerfing slash barehand crits.
// The crit table for pierce is slightly worse than blunt, so shouldn't need any changing
int adjustCritRollForBarehand(int roll, TBeing *me, spellNumT w_type)
{
  static bool inited = false;
  static int slash_adjust[101 -67];

  if (bluntType(w_type) || pierceType(w_type) || roll <= 66 || roll > 100)
    return roll;

  // adjust to the roll table base (special crits start at 67)
  int index = roll - 67;

  if (!inited)
  {
    for(unsigned int i=0;i < cElements(slash_adjust);i++)
      slash_adjust[i] = i + 67;

    slash_adjust[69 -67] = 0;
    slash_adjust[70 -67] = 0;
    slash_adjust[72 -67] = 0;
    slash_adjust[73 -67] = 0;
    slash_adjust[76 -67] = 0;
    slash_adjust[78 -67] = 0;
    slash_adjust[80 -67] = 0;
    slash_adjust[82 -67] = 0;
    slash_adjust[85 -67] = 0;
    slash_adjust[86 -67] = 0;
    slash_adjust[87 -67] = 0;
    slash_adjust[88 -67] = 0;
    slash_adjust[89 -67] = 0;
    slash_adjust[90 -67] = 0;
    inited = true;
  }

  if (index < 0 || index >= (int)cElements(slash_adjust))
  {
    vlogf(LOG_BUG, "Error in adjustCritRollForBarehand, bad index!");
    return 0;
  }

  if (slashType(w_type))
    return slash_adjust[index];

  // wierd case
  return roll;
}


// returns DELETE_THIS
// return ONEHIT_MESS_CRIT_S if oneHit should abort and no further oneHits should occur
int TBeing::critFailureChance(TBeing *v, TThing *weap, spellNumT w_type)
{
  char buf[256];
  int rc;
  int num, num2;
  TBeing *tbt = NULL;

  if (isAffected(AFF_ENGAGER))
    return FALSE;

  TObj *weapon = dynamic_cast<TObj *>(weap);
    
  stats.combat_crit_fail++;
  if (::number(1, 300) <= 
      (plotStat(STAT_CURRENT, STAT_KAR, 50, 3, 10) + 10*drunkMinus())) {
    if (isLucky(levelLuckModifier(v->GetMaxLevel())))
      return FALSE;

    stats.combat_crit_fail_pass++;
    if (desc)
      desc->career.crit_misses++;

    // OK, kind of silly, but if you trace the code, you'll realize
    // that a critfail blocks the actual hit, thus if you crit fail
    // on the first hit, no fight starts...
    // do this, to force the fight.  Bat 11/9/98
    reconcileDamage(v, 0, DAMAGE_NORMAL);

    num = dice(1, 20);

    // keep chance of sword in foot low
    while ((num == 11 || num == 12) && ::number(0, 2))
      num = dice(1,20);

    // don't want them "stumbling"
    if ((getPosition() != POSITION_STANDING) || isFlying())
      num = 4;

    switch (num) {
      case 1:
        // Slip, dex check or fall and be stunned for a few rounds.
        if (::number(0, plotStat(STAT_NATURAL, STAT_AGI, -30, 60, 0)) <
            (30 + getCond(DRUNK))) {
          act("In your attempt to swing at $N, you trip and fall!", FALSE, this, NULL, v, TO_CHAR);
          act("In an attempt to swing at $N, $n trips and falls!", TRUE, this, NULL, v, TO_NOTVICT);
          act("In an attempt to swing at you, $n trips and falls!", TRUE, this, NULL, v, TO_VICT);
          addToWait(combatRound(1));
          cantHit += loseRound(::number(1,2));
          setPosition(POSITION_SITTING);
          if (riding)
            dismount(POSITION_SITTING);
        } else {
          act("You start to stumble, but catch yourself before you fall!", FALSE, this, NULL, NULL, TO_CHAR);
          act("$n starts to stumble but catches $mself before $e falls!", TRUE, this, NULL, NULL, TO_ROOM);
        }
        return (ONEHIT_MESS_CRIT_S);
      case 2:
        // Slip, dex check or fall and be stunned for some rounds. 
	// Speef changed to Agility but probably use Speed for reaction mod.
        if (::number(0, plotStat(STAT_NATURAL, STAT_AGI, -30, 60, 0)) <
            (30 + getCond(DRUNK))) {
          act("In your attempt to swing at $N, you trip and fall!", FALSE, this, NULL, v, TO_CHAR);
          act("In an attempt to swing at $N, $n trips and falls!", TRUE, this, NULL, v, TO_NOTVICT);
          act("In an attempt to swing at you, $n trips and falls!", TRUE, this, NULL, v, TO_VICT);
          addToWait(combatRound(1));
          cantHit += loseRound(::number(1,3));
          setPosition(POSITION_SITTING);
          if (riding)
            dismount(POSITION_SITTING);
        } else {
          act("You start to stumble, but catch yourself before you fall!", FALSE, this, NULL, NULL, TO_CHAR);
          act("$n starts to stumble but catches $mself before $e falls!", TRUE, this, NULL, NULL, TO_ROOM);
        }
        return (ONEHIT_MESS_CRIT_S);
      case 3:
        // Trip and fall. No dex check. Stunned for many rounds.   
        act("In your attempt to swing at $N, you trip and fall!", TRUE, this, NULL, v, TO_CHAR);
        act("In an attempt to swing at $N, $n trips and falls!", TRUE, this, NULL, v, TO_NOTVICT);
        act("In an attempt to swing at you, $n trips and falls!", TRUE, this, NULL, v, TO_VICT);
        addToWait(combatRound(1));
        cantHit += loseRound(::number(1,4));
        setPosition(POSITION_SITTING);
        if (riding)
          dismount(POSITION_SITTING);

        return (ONEHIT_MESS_CRIT_S);
      case 4:
        // Lose grip. Dex check or drop weapon!
        if (weapon) {
	  int agi=plotStat(STAT_NATURAL, STAT_AGI, -30, 60, 0);

	  if(doesKnowSkill(SKILL_WEAPON_RETENTION) &&
	     bSuccess(SKILL_WEAPON_RETENTION)){
            act("Your grasp on $p loosens, but you shift position for a firmer grip.",
		FALSE, this, weapon, NULL, TO_CHAR);
	  } else if ((::number(0, agi) < (30 + getCond(DRUNK))) &&
		     weapon->canDrop()) {
            sprintf(buf, "You %slose%s your grip on $p and it %sfalls out of your grasp%s!",
                         red(), norm(), red(), norm());
            act(buf, FALSE, this, weapon, NULL, TO_CHAR);
            act("$n seems to lose $s grip on $p and it falls out of $s grasp!", TRUE, this, weapon, NULL, TO_ROOM);
            *roomp += *unequip(weapon->eq_pos);
          } else {
            act("You start to fumble $p, but miraculously retrieve it before you drop it.", FALSE, this, weapon, NULL, TO_CHAR);
            act("$n starts to fumble $p, but miraculously retrieves it before $e drops it.", TRUE, this, weapon, NULL, TO_ROOM);
          }
          return (ONEHIT_MESS_CRIT_S);
        }
      case 5:
        // Lose grip and drop weapon with no dex check. 
        if (weapon && weapon->canDrop()) {
	  if(doesKnowSkill(SKILL_WEAPON_RETENTION) &&
	     bSuccess(SKILL_WEAPON_RETENTION)){
            act("Your grasp on $p loosens, but you shift position for a firmer grip.",
		FALSE, this, weapon, NULL, TO_CHAR);
	  } else {
	    sprintf(buf, "You %slose%s your grip on $p and it %sfalls out of your grasp%s!",
		    red(), norm(), red(), norm());
	    act(buf, FALSE, this, weapon, NULL, TO_CHAR);
	    act("$n seems to lose $s grip on $p and it falls out of $s grasp!", TRUE, this, weapon, NULL, TO_ROOM);
	    *roomp += *unequip(weapon->eq_pos);
	  }

	  return (ONEHIT_MESS_CRIT_S);
        }
      case 6:
      case 7:
        // Hit self (half damage) 
        if (weapon) {
          act("$n gets clumsy, and nails $mself with $p.", TRUE, this, weapon, v, TO_ROOM);
          act("You become careless, and nail yourself with $p.", FALSE, this, weapon, v, TO_CHAR, ANSI_ORANGE);
          int dam = getWeaponDam(this, weapon,(weapon == heldInPrimHand() ? HAND_PRIMARY : HAND_SECONDARY));
          dam = getActualDamage(this, weapon, dam, w_type);
          dam /= 4;
          rc = applyDamage(this, dam,w_type);
          if (IS_SET_DELETE(rc, DELETE_VICT))
            return DELETE_THIS;
        } else {
          act("With stunning grace, $n has just struck $mself.", TRUE, this, NULL, v, TO_ROOM);
          act("You strike yourself in your carelessness.", FALSE, this, NULL, v, TO_CHAR, ANSI_ORANGE);
          int dam = getWeaponDam(this, NULL, HAND_SECONDARY);
          dam = getActualDamage(this, NULL, dam, w_type);
          dam /= 4;
          rc = applyDamage(this, dam,w_type);
          if (IS_SET_DELETE(rc, DELETE_VICT))
            return DELETE_THIS;
        }
        return (ONEHIT_MESS_CRIT_S);
      case 8:
        // Hit self (full damage) 
        if (weapon) {
          act("$n slips, and smacks $mself with $p.", TRUE, this, weapon, v, TO_ROOM);
          act("You become careless, and smack yourself with your $o.", FALSE, this, weapon, v, TO_CHAR, ANSI_ORANGE);
          int dam = getWeaponDam(this, weapon,(weapon == heldInPrimHand() ? HAND_PRIMARY : HAND_SECONDARY));
          dam = getActualDamage(this, weapon, dam, w_type);
          dam /= 2;
          rc = applyDamage(this, dam,w_type);
          if (IS_SET_DELETE(rc, DELETE_VICT))
            return DELETE_THIS;
        } else {
          act("With stunning grace, $n has just struck $mself.", TRUE, this, NULL, v, TO_ROOM);
          act("You strike yourself in your carelessness.", FALSE, this, NULL, v, TO_CHAR, ANSI_ORANGE);
          int dam = getWeaponDam(this, NULL, HAND_SECONDARY);
          dam = getActualDamage(this, NULL, dam, w_type);
          dam /= 2;
          rc = applyDamage(this, dam,w_type);
          if (IS_SET_DELETE(rc, DELETE_VICT))
             return DELETE_THIS;
        }
        return (ONEHIT_MESS_CRIT_S);
      case 9:
        // Hit friend (half damage) (if no friend hit self) 

	for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it){
          tbt = dynamic_cast<TBeing *>(*it);
          if (!tbt)
            continue;

          if (!tbt->isAffected(AFF_CHARM) && 
               inGroup(*tbt) && (tbt != this))
            break;
        }
        if (!tbt) 
          tbt = this;
        
        if (weapon) {
          if (tbt != this) {
            act("$n slips, and smacks $N with $p.", TRUE, this, weapon, tbt, TO_NOTVICT);
            act("You become careless, and smack $N with your $o.", FALSE, this, weapon, tbt, TO_CHAR, ANSI_ORANGE);
            act("$n carelessly smacks you with $p.",TRUE,this,weapon,tbt,TO_VICT, ANSI_ORANGE);
          } else {
            act("$n slips, and smacks $mself with $p.", TRUE, this, weapon, this, TO_ROOM);
            act("You become careless, and smack yourself with your $o.", FALSE, this, weapon, this, TO_CHAR, ANSI_ORANGE);
          }
          int dam = getWeaponDam(tbt, weapon,(weapon == heldInPrimHand() ? HAND_PRIMARY : HAND_SECONDARY));
          dam = getActualDamage(tbt, weapon, dam, w_type);
          dam /=4;
          rc = applyDamage(tbt, dam,w_type);
          if (tbt != this) {
            // this check added since otherwise a fight between "friends"
            // could break out.
            act("In the confusion, you and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_CHAR);
            act("In the confusion, $n and you stop fighting.",
                  FALSE, this, 0, tbt, TO_VICT);
            act("In the confusion, $n and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_NOTVICT);
            stopFighting();
            tbt->stopFighting();
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            if (tbt != this) {
              delete tbt;
              tbt = NULL;
            } else
              return DELETE_THIS;
          }
        } else {
          if (tbt != this) {
            act("$n slips, and smacks $N.", TRUE, this, NULL, tbt,TO_NOTVICT);
            act("You become careless, and smack $N.", FALSE, this,NULL, tbt, TO_CHAR, ANSI_ORANGE);
            act("$n carelessly smacks you.",TRUE,this,NULL,tbt,TO_VICT, ANSI_ORANGE);
          } else {
            act("$n slips, and smacks $mself.", TRUE, this, NULL, this, TO_ROOM);
            act("You become careless, and smack yourself.", FALSE,this, NULL, this, TO_CHAR, ANSI_ORANGE);
          }
          int dam = getWeaponDam(tbt, NULL,  HAND_SECONDARY);
          dam = getActualDamage(tbt, NULL, dam, w_type);
          dam /=4;
          rc = applyDamage(tbt, dam,w_type);
          if (tbt != this) {
            act("In the confusion, you and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_CHAR);
            act("In the confusion, $n and you stop fighting.",
                  FALSE, this, 0, tbt, TO_VICT);
            act("In the confusion, $n and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_NOTVICT);
            stopFighting();
            tbt->stopFighting();
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            if (tbt != this) {
              delete tbt;
              tbt = NULL;
            } else
              return DELETE_THIS;
          }
        }
        return ONEHIT_MESS_CRIT_S;
      case 10:
        // Hit friend (full damage) (if no friend hit self) 
	for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();++it){
          tbt = dynamic_cast<TBeing *>(*it);
          if (!tbt)
            continue;
          if (!tbt->isAffected(AFF_CHARM) &&
               inGroup(*tbt) && (tbt != this))
            break;
        }
        if (!tbt)
          tbt = this;
        if (weapon) {
          if (tbt != this) {
            act("$n slips, and smacks $N with $p.", TRUE, this, weapon, tbt,TO_NOTVICT);
            act("You become careless, and smack $N with your $o.", FALSE, this,weapon, tbt, TO_CHAR, ANSI_ORANGE);
            act("$n carelessly smacks you with $p.",TRUE,this,weapon,tbt,TO_VICT, ANSI_ORANGE);
          } else {
            act("$n slips, and smacks $mself with $p.", TRUE, this, weapon, this, TO_ROOM);
            act("You become careless, and smack yourself with your $o.", FALSE,this, weapon, this, TO_CHAR, ANSI_ORANGE);
          }
          int dam = getWeaponDam(tbt, weapon,(weapon == heldInPrimHand() ? HAND_PRIMARY : HAND_SECONDARY));
          dam = getActualDamage(tbt, weapon, dam, w_type);
          dam /= 2;
          rc = applyDamage(tbt, dam,w_type);
          if (tbt != this) {
            act("In the confusion, you and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_CHAR);
            act("In the confusion, $n and you stop fighting.",
                  FALSE, this, 0, tbt, TO_VICT);
            act("In the confusion, $n and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_NOTVICT);
            stopFighting();
            tbt->stopFighting();
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            if (tbt != this) {
              delete tbt;
              tbt = NULL;
            } else
              return DELETE_THIS;
          }
        } else {
          if (tbt != this) {
            act("$n slips, and smacks $N.", TRUE, this, NULL, tbt,TO_NOTVICT);
            act("You become careless, and smack $N.", FALSE, this,NULL, tbt,TO_CHAR, ANSI_ORANGE);
            act("$n carelessly smacks you.",TRUE,this,NULL,tbt,TO_VICT, ANSI_ORANGE);
          } else {
            act("$n slips, and smacks $mself.", TRUE, this, NULL, this, TO_ROOM);
            act("You become careless, and smack yourself.", FALSE,this, NULL, this, TO_CHAR, ANSI_ORANGE);
          }
          int dam = getWeaponDam(tbt, NULL, HAND_SECONDARY);
          dam = getActualDamage(tbt, NULL, dam, w_type);
          dam /= 2;
          rc = applyDamage(tbt, dam,w_type);
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            if (tbt != this) {
              delete tbt;
              tbt = NULL;
            } else
              return DELETE_THIS;
          }

          // if nobody died from the hit, stop the fight
          if (tbt && tbt != this) {
            act("In the confusion, you and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_CHAR);
            act("In the confusion, $n and you stop fighting.",
                  FALSE, this, 0, tbt, TO_VICT);
            act("In the confusion, $n and $N stop fighting.",
                  FALSE, this, 0, tbt, TO_NOTVICT);
            stopFighting();
            if (tbt->fight())
              tbt->stopFighting();
          }
        }
        return ONEHIT_MESS_CRIT_S;
      case 11:
        // Miss miserably and stick weapon in foot (with dex check). 
        if (::number(0, plotStat(STAT_NATURAL, STAT_AGI, -30, 60, 0)) >=
            (30 + getCond(DRUNK)))
          return 0;
      case 12:
        // Miss miserably and stick weapon in foot (no dex check). 
        if (weapon && hasPart(WEAR_FOOT_R) && !weapon->isBluntWeapon() &&
            !getStuckIn(WEAR_FOOT_R)) {
          sprintf(buf, "You miserably miss $N, %sand stick $p in your foot%s!",
                  red(),norm());
          act(buf, FALSE, this, weapon, v, TO_CHAR);
          sprintf(buf, "$n miserably misses $N, and sticks $p in $s foot!");
          act(buf, TRUE, this, weapon, v, TO_NOTVICT);
          sprintf(buf, "$n miserably misses you, %sand sticks $p in $s foot%s!",
                  v->cyan(),v->norm());
          act(buf, TRUE, this, weapon, v, TO_VICT);
          num2 = getWeaponDam(this,weapon,(weapon == heldInPrimHand() ? HAND_PRIMARY : HAND_SECONDARY));
          num2 /= 8;
	  num2 += 1;
          rc = stickIn(unequip(weapon->eq_pos), WEAR_FOOT_R);
          if (desc)
            desc->career.stuck_in_foot++;

          if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
	  num2 *= 4;
          rc = damageLimb(this,WEAR_FOOT_R,weapon,&num2);
          if (IS_SET_DELETE(rc, DELETE_VICT))
            return DELETE_THIS;

          return (ONEHIT_MESS_CRIT_S);
        }
        // no weapon, blunt weapn, etc = fall through into next case

      case 13:
       if (!isHumanoid())
         return 0;

        // Fall and twist ankle, affect with new affect that halfs max moves 
        act("You stumble and twist your ankle as you swing at $N!", 
               FALSE, this, 0, v, TO_CHAR);
        act("$n stumbles and twists $s ankle as $e tries to swing at $N!", 
               TRUE, this, 0, v, TO_NOTVICT);
        act("$n stumbles and twists $s ankle as $e tries to swing at you!", 
               TRUE, this, 0, v, TO_VICT);
        setMove(getMove()/2);

        return (ONEHIT_MESS_CRIT_S);
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
        break;
      default:
        act("$n stumbles as $e tries pathetically to attack $N.", TRUE, this, 0, v, TO_NOTVICT);
        act("You stumble pathetically as you try to attack $N.", FALSE, this, 0, v, TO_CHAR);
        act("$n stumbles as $e tries pathetically to attack you.", TRUE, this, 0, v, TO_VICT);
        return (ONEHIT_MESS_CRIT_S);
    }
  }
  return FALSE;
}

void TBeing::critHitEqDamage(TBeing *v, TThing *obj, int eqdam)
{
  TObj *damaged_item = dynamic_cast<TObj *>(obj);

  act("$N's $p is greatly damaged by the force of your hit.",
    FALSE, this, obj, v, TO_CHAR, ANSI_ORANGE);

  act("Your $p suffers massive damage from $n's mighty hit.",
    FALSE, this, obj, v, TO_VICT, ANSI_RED);

  act("$N's $p suffers massive damage from $n's powerful hit.",
    FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);

  if(IS_SET_DELETE(damaged_item->damageItem(-eqdam), DELETE_THIS)){
    delete damaged_item;
    damaged_item = NULL;
  }
}


// This will just be a big ass case statement based on random diceroll 
// returns DELETE_VICT if v dead
// mod is -1 from generic combat, mod == crit desired from immortal command.
int TBeing::critSuccessChance(TBeing *v, TThing *weapon, wearSlotT *part_hit, spellNumT wtype, int *dam, int mod)
{
  int crit_num = 0, dicenum, crit_chance, new_wtype;
  affectedData *adp;

  if (isAffected(AFF_ENGAGER))
    return FALSE;

  if (!isImmortal() && 
      (v->isImmortal() || IS_SET(v->specials.act, ACT_IMMORTAL)))
    return FALSE;

  if (dynamic_cast<TGun *>(weapon))
    return FALSE;

  if ((mod == -1) && v->isImmune(getTypeImmunity(wtype), *part_hit))
    return FALSE;

  if(mod>100){
    vlogf(LOG_BUG, format("critSuccessChance called with mod>100 (%i)") %  mod);
    return FALSE;
  }

  if (mod == -1 && v->affectedBySpell(AFFECT_DUMMY)) {
    for (affectedData *an = v->affected; an; an = an->next) {
      if (an->type == AFFECT_DUMMY && an->level == 60) {
        mod = an->modifier2;
      }
    }
    v->affectFrom(AFFECT_DUMMY);
  }

  // get wtype back so it fits in array  
  new_wtype = wtype - TYPE_HIT;

  stats.combat_crit_suc++;

  // determine dice roll for crit, modified by skill(s)
  if(doesKnowSkill(SKILL_CRIT_HIT) && isPc()){
    dicenum = dice(1, (int)(100000-(getSkillValue(SKILL_CRIT_HIT)*850)));
  } else if(doesKnowSkill(SKILL_POWERMOVE) && isPc()){
    dicenum = dice(1, (int)(100000-(getSkillValue(SKILL_POWERMOVE)*800)));
  } else if(dynamic_cast<TMonster *>(this)){
    // less crits for mobs
    dicenum = dice(1, 1000000);
  } else {
    dicenum = dice(1, 100000);    // This was 10k under 3.x - Bat
  }

  // adjust for immorts being awesome
  if (isImmortal())
    dicenum /= 10;

  // begin with dexterity
  crit_chance = plotStat(STAT_CURRENT, STAT_DEX, 10, 100, 63);

  // drunkeness reduces chance
  crit_chance -= 2 * getCond(DRUNK);

  // factor in karma
  crit_chance *= plotStat(STAT_CURRENT, STAT_KAR, 80, 125, 100);
  crit_chance /= 100;

  // factor in relative levels
  int diff = GetMaxLevel() - v->GetMaxLevel();
  if (diff == 0) {
    crit_chance *= 50;
  } else {
    double absdiff = abs(diff);
    double level_mod = 50.0 + ((double)diff * log(absdiff/20.0+1) / absdiff) * 75.0;
    if (level_mod <= 0) {
      crit_chance = 1;
    } else {
      crit_chance = (int)((double)crit_chance * level_mod);
    }
  }

  // if affected by APPLY_CRIT_FREQUENCY then multiply out by modifier
  for (adp = affected; adp; adp = adp->next) {
    if (adp->location == APPLY_CRIT_FREQUENCY) {
      crit_chance *= adp->modifier;
    }
  }

  if(mod == -1){
    // check the roll versus the chance
    if(dicenum > crit_chance)
      return FALSE;
    
    // if there is greater than 10 levels of different in either direction
    // then either make the crits better, or worse
    int level_mod=(GetMaxLevel() - v->GetMaxLevel());
    if(level_mod > -10 && level_mod < 10)
      level_mod = 0;

    // determine which crit to do, higher number = better crits
    crit_num = ::number(1, 100) + ::number(0, level_mod);
    crit_num = max(1, crit_num);
    crit_num = min(100, crit_num);

    // if we are fighting barehand, adjust the crit based on type
    if (isPc() && !weapon) {
      crit_num = adjustCritRollForBarehand(crit_num, this, wtype);
      if (crit_num == 0)
        return 0;
    }

    // update crit stats
    stats.combat_crit_suc_pass++;
    if (desc)
      desc->career.crit_hits++;
    if (v->desc)
      v->desc->career.crit_hits_suff++;

  } else {
    // specified crit
    crit_num = mod;
  }

  // critical hitting gets damage boost as well
  if(doesKnowSkill(SKILL_CRIT_HIT))
    *dam = (int)(*dam * ((getSkillValue(SKILL_CRIT_HIT)/100.0)+1.0));

  // play the crit-hit sound
  // boost the priority so that this sound will trump normal combat sounds
  soundNumT snd = pickRandSound(SOUND_CRIT_01, SOUND_CRIT_43);
  playsound(snd, SOUND_TYPE_COMBAT, 100, 45, 1);

  if (pierceType(wtype)) {
    return critPierce(v, weapon, part_hit, wtype, dam, crit_num);
  } else if (slashType(wtype)) {
    return critSlash(v, weapon, part_hit, wtype, dam, crit_num);
  } else if (bluntType(wtype)) {
    return critBlunt(v, weapon, part_hit, wtype, dam, crit_num);
  } else {
    vlogf(LOG_BUG, format("unknown weapon type in critSuccessChance (%i)") %  wtype);
  }
  return FALSE;
}

/* ------------------------------------------------------------

====Blunt crit table====

0-33: double damage
34-66: triple damage
67,68: crush finger
69,70: break hand right
71,72: break hand left
73,74: break arm (primary)
75,76: crush nerves arm (useless)
77,78: break bones left leg
79,80: crush muscles left leg (useless)
81,82: head blow (stun 10 rounds)
83,84: head blow (stun 5 rounds)
85,86: shatter rib
87,88: shatter rib + internal bleeding
96,97: break tooth
98,99,100: crush skull/rip heart

total:
  double damage: 33%
  triple damage: 33%
  minor break/ailment: 12%
  major break/ailment: 12%
  death: 3%

------------------------------------------------------------ */
int TBeing::critBlunt(TBeing *v, TThing *weapon, wearSlotT *part_hit,
		       spellNumT wtype, int *dam, int crit_num)
{
  sstring buf, limbStr;
  TThing *obj=NULL;
  int rc, i;
  affectedData af;
  wearSlotT new_slot;
  int new_wtype = wtype - TYPE_HIT;
  int v_vnum;
  TMonster * vmob;

  if ((vmob = dynamic_cast<TMonster *>(v)))
    v_vnum = vmob->number >= 0 ? mob_index[vmob->getMobIndex()].virt : -1;
  else
    v_vnum = -2;

  if(crit_num>100){
    vlogf(LOG_BUG, format("critBlunt called with crit_num>100 (%i)") %  crit_num);
    crit_num=0;
  }

  // Do crush crit 
  limbStr = (weapon ? fname(weapon->name) : getMyRace()->getBodyLimbBlunt());

  if (crit_num <= 33) {
    // double damage 
    *dam *= 2;

    buf=format("You strike $N exceptionally well, %s $S %s with your %s!") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) %
      limbStr;
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n strikes you exceptionally well, %s your %s with $s %s.") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) %
      limbStr;
    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);

    buf = format("$n strikes $N exceptionally well, %s $S %s with $s %s.") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) % 
      limbStr;
    act(buf, TRUE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else if (crit_num <= 66) {
    // triple damage 
    *dam *= 3;
    
    buf = format("You critically strike $N, %s $S %s with your %s!") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) %
      limbStr;
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n critically strikes you, %s your %s with $s %s.") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) %
      limbStr;
    act(buf, TRUE, this, 0, v, TO_VICT, ANSI_RED);

    buf = format("$n critically strikes $N, %s $S %s with $s %s.") %
      attack_hit_text[new_wtype].hitting %
      v->describeBodySlot(*part_hit) %
      limbStr;
    act(buf, TRUE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else {
    // better stuff 
    switch (crit_num) {
      case 67:
      case 68:
	// crush finger
	if (!v->hasPart(WEAR_FINGER_R))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_FINGER_R))
	  return 0;
        if (v->isLimbFlags(WEAR_FINGER_R, PART_BROKEN))
          return 0;
	buf = format("With your %s, you crush $N's %s!") %
	  limbStr %
	  v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	buf=format("$n's %s crushes your %s!") %
		limbStr %
		v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	buf = format("$n's %s crushes $N's %s!") %
	  limbStr %
	  v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(WEAR_FINGER_R, PART_BROKEN);
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
	    (obj = v->equipment[WEAR_FINGER_R]))
	  critHitEqDamage(v, obj, (::number(-21,-15)));

	*part_hit = WEAR_FINGER_R;
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 69:
      case 70:
	//shatter bones in 1 hand
	if (!v->hasPart(WEAR_HAND_R))
	  return 0;
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_HAND_R))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
        if (v->isLimbFlags(WEAR_HAND_R, PART_BROKEN))
          return 0;
	buf=format("With your %s, you shatter the bones in $N's %s!") %
	  limbStr %
	  v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	buf=format("$n's %s shatters the bones in your %s!") %
	  limbStr %
	  v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	buf = format("$n's %s shatters the bones in $N's %s!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(WEAR_HAND_R, PART_BROKEN);
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_HAND_R])
	    v->damageItem(this,WEAR_HAND_R,wtype,weapon,*dam);
	v->woundedHand(v->isRightHanded());
	*part_hit = WEAR_HAND_R;
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 71:
      case 72:
	// shatter bones other hand
	if (!v->hasPart(WEAR_HAND_L))
	  return 0;
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_HAND_L))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
        if (v->isLimbFlags(WEAR_HAND_L, PART_BROKEN))
          return 0;
	*part_hit = WEAR_HAND_L;
	buf=format("With your %s, you shatter the bones in $N's %s!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	buf=format("$n's %s shatters the bones in your %s!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	buf=format("$n's %s shatters the bones in $N's %s!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(WEAR_HAND_L, PART_BROKEN);
	v->woundedHand(!v->isRightHanded());
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_HAND_L])
	    v->damageItem(this,WEAR_HAND_L,wtype,weapon,*dam);
	*part_hit = WEAR_HAND_L;
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 73:
      case 74:
	// break bones arm - broken
	if (!v->hasPart(v->getPrimaryArm()))
	  return 0;
	if (v->isImmune(IMMUNE_BONE_COND, v->getPrimaryArm()))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
        if (v->isLimbFlags(v->getPrimaryArm(), PART_BROKEN))
          return 0;
	*part_hit = v->getPrimaryArm();
	buf=format("You shatter the bones in $N's forearm with your %s!") %
	        limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	buf=format("$n's %s shatters the bones in your forearm!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	buf=format("$n's %s shatters the bones in $N's forearm!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(v->getPrimaryArm(), PART_BROKEN);
	for (i=1;i<5;i++)
	  if (v->equipment[v->getPrimaryArm()])
	    v->damageItem(this,v->getPrimaryArm(),wtype,weapon,*dam);
	*part_hit = v->getPrimaryArm();
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 75:
      case 76:
	// crush nerves arm - useless
	new_slot = v->getPrimaryArm();
	if (!v->slotChance(new_slot))
	  return 0;

        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;

        if (v->isLimbFlags(new_slot, PART_USELESS))
          return 0;

        buf=format("With your %s, you crush the nerves in $N's shoulder!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
        buf=format("$n's %s crushes the nerves in your shoulder!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
        buf=format("$n's %s crushes the nerves in $N's shoulder!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->damageArm(TRUE,PART_USELESS);
	*part_hit = new_slot;
	if (desc)
	  desc->career.crit_crushed_nerve++;
	if (v->desc)
	  v->desc->career.crit_crushed_nerve_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 77:
      case 78:
	// break bones leg
	if (!v->hasPart(WEAR_LEG_L))
	  return 0;
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_LEG_L))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
        if (v->isLimbFlags(WEAR_LEG_L, PART_BROKEN))
          return 0;

	*part_hit = WEAR_LEG_L;
buf=format("You shatter $N's femur with your %s!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s shatters your femur!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s shatters $N's femur!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	*part_hit = WEAR_LEG_L;
	v->addToLimbFlags(WEAR_LEG_L, PART_BROKEN);
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_LEG_L])
	    v->damageItem(this,WEAR_LEG_L,wtype,weapon,*dam);
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 79:
      case 80:
	// crush muscles in leg
	if (!v->hasPart(WEAR_LEG_L))
	  return 0;

        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;

        if (v->isLimbFlags(WEAR_LEG_L, PART_USELESS))
          return 0;

	*part_hit = WEAR_LEG_L;
        buf=format("With your %s, you crush the muscles in $N's leg!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
        buf=format("$n's %s crushes the muscles in your leg!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
        buf=format("$n's %s crushes the muscles in $N's leg!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	*part_hit = WEAR_LEG_L;
	v->addToLimbFlags(WEAR_LEG_L, PART_USELESS);
	v->addToLimbFlags(WEAR_FOOT_L, PART_USELESS);
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_LEG_L])
	    v->damageItem(this,WEAR_LEG_L,wtype,weapon,*dam);
	rc = damageLimb(v,*part_hit,weapon,dam);

	if (desc)
	  desc->career.crit_crushed_nerve++;
	if (v->desc)
	  v->desc->career.crit_crushed_nerve_suff++;

	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 81:
      case 82:
	// head blow - stuns 10 rounds
	// this is half, goes into next case for other half
	if (!v->hasPart(WEAR_HEAD))
	  return 0;
	*part_hit = WEAR_HEAD;
	v->cantHit += v->loseRound(10);
      case 83:
      case 84:
	// head blow - stuns 5 rounds
	if (!v->hasPart(WEAR_HEAD))
	  return 0;
	*part_hit = WEAR_HEAD;
buf=format("You slam $N's head massively with your %s!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s slams into your head and .. What?  who?  Where am I????") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s slams into $N's head, stunning $M completely!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->cantHit += v->loseRound(5);
	*part_hit = WEAR_HEAD;
	for (i = 1;i < 5; i++) {
	  if (v->equipment[WEAR_HEAD]) {
	    v->damageItem(this,WEAR_HEAD,wtype,weapon,*dam);
	    if (v->cantHit > 0)
	      v->cantHit -= v->loseRound(2);
	  }
	}
	rc = damageLimb(v,*part_hit,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 85:
      case 86:
	//  shatter rib
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_BODY))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
        if (v->isLimbFlags(WEAR_BODY, PART_BROKEN))
          return 0;

	*part_hit = WEAR_BODY;
buf=format("With your %s, you slam $N's chest, breaking a rib!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s crushes your chest and shatters a rib!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s shatters one of $N's ribs!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(WEAR_BODY, PART_BROKEN);
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	for (i=1;i<7;i++)
	  if (v->equipment[WEAR_BODY])
	    v->damageItem(this,WEAR_BODY,wtype,weapon,*dam);
	return ONEHIT_MESS_CRIT_S;
      case 87:
      case 88:
	//  shatter rib - internal damage, death if not healed
	if (v->isImmune(IMMUNE_BONE_COND, WEAR_BODY))
	  return 0;
	if (v->race->hasNoBones())
	  return 0;
	if (v->hasDisease(DISEASE_HEMORRAGE))
	  return 0;
        if (v->isLimbFlags(WEAR_BODY, PART_BROKEN))
          return 0;

	*part_hit = WEAR_BODY;
buf=format("With your %s, you slam $N's chest, breaking a rib and causing internal damage!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s crushes your chest, shatters a rib and causes internal bleeding!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s shatters one of $N's ribs!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->addToLimbFlags(WEAR_BODY, PART_BROKEN);
	for (i=1;i<9;i++)
	  if (v->equipment[WEAR_BODY])
	    v->damageItem(this,WEAR_BODY,wtype,weapon,*dam);
	af.type = AFFECT_DISEASE;
	af.level = 0;   // has to be 0 for doctor to treat
	af.duration = PERMANENT_DURATION;
	af.modifier = DISEASE_HEMORRAGE;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	v->affectTo(&af);
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 89:
      case 90:
      case 91:
      case 92:
      case 93:
      case 94:
      case 95:
	return FALSE;
      case 96:
      case 97:
        if (v->race->hasNoBones())
          return 0;

	buf = format("You swing your %s right into $N's face, sending a tooth flying.") % limbStr;
	act(buf, FALSE, this, obj, v, TO_CHAR, ANSI_ORANGE);
	buf = format("$n's %s connects with your face, sending a tooth flying.") % limbStr;
	act(buf, FALSE, this, obj, v, TO_VICT, ANSI_ORANGE);
	buf = format("$n's %s connects with $N's face, sending a tooth flying.") % limbStr;
	act(buf, FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);
	      	      
	TObj *corpse;
	TCorpse *tooth;
	
	corpse = read_object(Obj::GENERIC_TOOTH, VIRTUAL);
	corpse->swapToStrung();

	if((tooth=dynamic_cast<TCorpse *>(corpse))){
	  tooth->setCorpseRace(v->getRace());
	  tooth->setCorpseLevel(v->GetMaxLevel());
	  tooth->setCorpseVnum(v->mobVnum());
	}

  if (dynamic_cast<TPerson *>(this)) {
    // check to see if this should be a limb quest tooth
    int limb_quest = -1;
    TDatabase db(DB_SNEEZY);
    db.query("select team from quest_limbs_team where player = '%s'", getName());
    if (db.fetchRow())
      limb_quest = 0;
    buf = format("tooth lost limb %s [q] [tooth] [%d] [%d] [%s]") % v->name % limb_quest % v_vnum % getName();
  } else{
    buf = format("tooth lost limb %s") % v->name;
  }
  
	delete corpse->name;
	corpse->name = mud_str_dup(buf);
	
	buf = format("<W>a <1><r>bloody<1><W> tooth of %s<1>") % v->getName();
	delete corpse->shortDescr;
	corpse->shortDescr = mud_str_dup(buf);
	
	buf = format("<W>A <1><r>bloody<1><W> tooth lies here, having been knocked out of %s's mouth.<1>") % v->getName();
	delete corpse->descr;
	corpse->setDescr(mud_str_dup(buf));
	      
	corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
	//	corpse->addCorpseFlag(CORPSE_NO_REGEN);
	//	corpse->obj_flags.decay_time = 3 * (dynamic_cast<TMonster *>(this) ? MAX_NPC_CORPSE_TIME : MAX_PC_CORPSE_EMPTY_TIME);
	corpse->obj_flags.decay_time=-1;
	corpse->setWeight(0.1);
	corpse->canBeSeen = v->canBeSeen;
	corpse->setVolume(1);
	corpse->setMaterial(MAT_BONE);
	corpse->obj_flags.cost=50;
	      
	act("$p goes flying through the air and bounces once before it rolls to a stop.",TRUE,v,corpse,0,TO_ROOM, ANSI_RED);
	*v->roomp += *corpse;

	if (desc)
	  desc->career.crit_tooth++;
	      
	if (v->desc)
	  v->desc->career.crit_tooth_suff++;
      
	return ONEHIT_MESS_CRIT_S;
      case 98:
      case 99:
      case 100:
	if (doesKnowSkill(SKILL_CRIT_HIT) && !v->equipment[WEAR_BODY] &&
	    v->hasPart(WEAR_BODY) && !weapon && bSuccess(SKILL_CRIT_HIT) && 
	    !::number(0,4) && !IS_SET(v->specials.act, ACT_SKELETON)) {
  
    // Rip out the heart instead of head crush whee fancy.
    // ...But make it a bit fancier on ghosts, zombies and skeletons.
  
    if (IS_SET(v->specials.act, ACT_GHOST)) {
      buf = format("With your %s, you reach into $N's chest and rip out what you think is $S heart!") % limbStr;
      act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
      act("You hold the ghostly organ above your head in triumph!", FALSE, this, 0, v, TO_CHAR, ANSI_RED);
      act("$n reaches into your chest and rips out your heart!", FALSE, this, 0, v, TO_VICT, ANSI_RED);
      act("$n reaches into $N's chest and rips out what appears to be $S heart!", FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	  } else {
	    buf = format("With your %s, you reach into $N's chest and rip out $S heart!") % limbStr;
	    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	    act("You hold the still beating heart above your head in triumph, as blood runs down your arm!", FALSE, this, 0, v, TO_CHAR, ANSI_RED);
	    act("$n reaches into your chest and rips out your heart!", FALSE, this, 0, v, TO_VICT, ANSI_RED);
	    act("$n reaches into $N's chest and rips out $S heart!", FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
    }

    if (IS_SET(v->specials.act, ACT_GHOST))
      act("The ghostly heart of $N fades as quickly as you removed it.", FALSE, this, 0, v, TO_CHAR, ANSI_RED);
    else {
      TDrinkCon *corpse;
      corpse = new TDrinkCon();
      if (dynamic_cast<TPerson *>(this))
        buf = format("heart lost limb %s [q] [heart] [0] [%d] [%s]") % v->name % v_vnum % getName();
      else
        buf = format("heart lost limb %s") % v->name;

      corpse->name = mud_str_dup(buf);

      if (IS_SET(v->specials.act, ACT_ZOMBIE)) {
        buf = format("the putrid <r>heart<1> of %s") % v->getName();
        corpse->shortDescr = mud_str_dup(buf);

        buf = format("The putrid <r>heart<1> of %s lies here.") % v->getName();
        corpse->setDescr(mud_str_dup(buf));
      } else {
        buf = format("the lifeless <r>heart<1> of %s") % v->getName();
        corpse->shortDescr = mud_str_dup(buf);

        buf = format("The lifeless <r>heart<1> of %s lies here.") % v->getName();
        corpse->setDescr(mud_str_dup(buf));
      }

	    corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
	    //	    corpse->addCorpseFlag(CORPSE_NO_REGEN);
	    corpse->obj_flags.decay_time = 3 * (dynamic_cast<TMonster *>(this) ? MAX_NPC_CORPSE_TIME : MAX_PC_CORPSE_EMPTY_TIME);
	    corpse->setWeight(0.1);
	    corpse->canBeSeen = v->canBeSeen;
	    corpse->setVolume(25);
	    corpse->setMaterial(v->getMaterial(WEAR_BODY));

	    corpse->setDrinkConFlags(0);
	    corpse->setMaxDrinkUnits(5);
	    corpse->setDrinkUnits(5);

      if (IS_SET(v->specials.act, ACT_ZOMBIE)) {
        corpse->setDrinkType(LIQ_POISON_VIOLET_FUNGUS);
        dropPool(9, LIQ_POISON_VIOLET_FUNGUS);
      } else {
        corpse->setDrinkType(LIQ_BLOOD);
        dropPool(9, LIQ_BLOOD);
      }

	    if(!heldInPrimHand())
	      equipChar(corpse, getPrimaryHold(), SILENT_YES);
	    else
	      equipChar(corpse, getSecondaryHold(), SILENT_YES);

	  }

	  if (desc)
	    desc->career.crit_ripped_out_heart++;
	    
	  if (v->desc)
	    v->desc->career.crit_ripped_out_heart_suff++;

	  applyDamage(v, (20 * v->hitLimit()),DAMAGE_RIPPED_OUT_HEART);
	  return DELETE_VICT;
	} else {
	  // crush skull unless helmet
	  if (!v->hasPart(WEAR_HEAD))
	    return 0;
	  if ((obj = v->equipment[WEAR_HEAD])) {
buf=format("With a mighty blow, you crush $N's head with your %s. Unfortunately, $S $o saves $M.") %
		     limbStr;
	    act(buf, FALSE, this, obj, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s strikes a mighty blow to your head crushing your $o!") %
		     limbStr;
	    act(buf, FALSE, this, obj, v, TO_VICT, ANSI_RED);
buf=format("$n's %s strikes a mighty blow to $N's head, crushing $S $o!") %
		     limbStr;
	    act(buf, FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);
	    if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA))
	      critHitEqDamage(v, obj, (::number(-55,-40)));
	    
	    *part_hit = WEAR_HEAD;
	    rc = damageLimb(v,*part_hit,weapon,dam);
	    if (IS_SET_DELETE(rc, DELETE_VICT))
	      return DELETE_VICT;
	    return ONEHIT_MESS_CRIT_S;
	  } else { // no head gear
	    // crush skull
	    if (!v->hasPart(WEAR_HEAD))
	      return 0;
buf=format("With your %s, you crush $N's skull, and $S brains ooze out!") %
		     limbStr;
	    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s crushes your skull and The World goes dark!") %
		     limbStr;
	    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s crushes $N's skull.  Brains ooze out as $E crumples!") %
		     limbStr;
	    act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	    if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
		(obj = v->equipment[WEAR_HEAD]))
	      critHitEqDamage(v, obj, (::number(-55,-40)));
	    
	    if (desc)
	      desc->career.crit_crushed_skull++;
	    
	    if (v->desc)
	      v->desc->career.crit_crushed_skull_suff++;
	    
	    applyDamage(v, (20 * v->hitLimit()),DAMAGE_CAVED_SKULL);
	    return DELETE_VICT;
	  }
	}
      default:
	vlogf(LOG_BUG, format("crit_num=%i in critBlunt switch, shouldn't happen") % 
	      crit_num);
	break;
    }
  }
  return 0;
}


/* ------------------------------------------------------------

====Slash crit table====

0-33: double damage
34-66: triple damage
67: sever finger left
68: sever finger right
69,70: sever hand right
71: sever wrist right
72,73: sever hand left
74: sever wrist left
75,76: sever arm right
77,78: sever arm left
79,80: sever foot right + right leg useless
81,82: sever foot left + left leg useless
83,84: impale (not if cleaving)
85,86: cleave in two if ! waist gear(kill)
87,88: cleave in two if and destroy waist (kill)
89,90: gullet to groin slice
91,92: sever genitalia
98,99,100 - decap if no neck armor

total:
  double damage: 33%
  triple damage: 33%
  impale: 2%
  minor sever: 4%
  major sever: 14%
  death: 9%

------------------------------------------------------------ */
int TBeing::critSlash(TBeing *v, TThing *weapon, wearSlotT *part_hit,
		       spellNumT wtype, int *dam, int crit_num)
{
  sstring buf, limbStr;
  TThing *obj=NULL;
  int rc, i, v_vnum;
  affectedData af;
  TMonster *vmob;

  if(crit_num>100){
    vlogf(LOG_BUG, format("critSlash called with crit_num>100 (%i)") %  crit_num);
    crit_num=0;
  }
  if ((vmob = dynamic_cast<TMonster *>(v)))
    v_vnum = vmob->number >= 0 ? mob_index[vmob->getMobIndex()].virt : -1;
  else
    v_vnum = -2;

  // Do slash crit
  limbStr = (weapon ? fname(weapon->name) : getMyRace()->getBodyLimbSlash());

  if (crit_num <= 33) {
    // double damage 
    *dam *= 2;

    buf = format("You strike $N's %s exceptionally well, sinking your %s deep into $S flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n strikes your %s exceptionally well, sinking $s %s deep into your flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);

    buf = format("$n strikes $N's %s exceptionally well, sinking $s %s deep into $N's flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else if (crit_num <= 66) {
    // triple damage
    *dam *= 3;

    buf = format("You critically strike $N's %s, sinking your %s deep into $S flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n critically strikes your %s, sinking $s %s deep into your flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);

    buf = format("$n critically strikes $N's %s, sinking $s %s deep into $N's flesh!") % v->describeBodySlot(*part_hit) % limbStr;
    act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else {
    // better stuff 
    if ((crit_num == 83 || crit_num == 84) && wtype == TYPE_CLEAVE)
      crit_num = 85;  // axes don't impale

    switch (crit_num) {
      case 67:
	// sever finger-r
	if (!v->hasPart(WEAR_FINGER_R))
	  return 0;

        buf=format("Your %s severs $N's %s and sends it flying!") % limbStr % v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);

        buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") % limbStr % v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);

        buf=format("$n's %s severs $N's %s and sends it flying!") % limbStr % v->describeBodySlot(WEAR_FINGER_R);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

	v->makePartMissing(WEAR_FINGER_R, FALSE, this);
	v->rawBleed(WEAR_HAND_R, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);

	*part_hit = WEAR_FINGER_R;

	if (desc)
	  desc->career.crit_sev_limbs++;

	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;

	return ONEHIT_MESS_CRIT_S;
      case 68:
	// sever finger-l
	if (!v->hasPart(WEAR_FINGER_L))
	  return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_FINGER_L);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") %
		limbStr %
		v->describeBodySlot(WEAR_FINGER_L);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_FINGER_L);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(WEAR_FINGER_L, FALSE, this);
	v->rawBleed(WEAR_HAND_L, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	*part_hit = WEAR_FINGER_L;
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 69:
      case 70:
	if (!v->hasPart(WEAR_HAND_R))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
	if ((obj = v->equipment[WEAR_WRIST_R])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from losing your %s!\n\r") %
		    fname(obj->name) % v->describeBodySlot(WEAR_HAND_R));
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_WRIST_R])
	      v->damageItem(this,WEAR_WRIST_R,wtype,weapon,*dam);
	  *part_hit = WEAR_HAND_R;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 71:
	// sever hand-r at wrist
	if (!v->hasPart(WEAR_HAND_R))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_R);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(WEAR_HAND_R, FALSE, this);
	v->rawBleed(WEAR_WRIST_R, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	v->woundedHand(v->isRightHanded());
	*part_hit = WEAR_WRIST_R;
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 72:
      case 73:
	if (!v->hasPart(WEAR_WRIST_L))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
	if ((obj = v->equipment[WEAR_WRIST_L])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from losing a hand!\n\r") %
		    fname(obj->name));
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_WRIST_L])
	      v->damageItem(this,WEAR_WRIST_L,wtype,weapon,*dam);
	  *part_hit = WEAR_WRIST_L;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 74:
	// sever hand-l at wrist
	if (!v->hasPart(WEAR_HAND_L))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_HAND_L);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(WEAR_HAND_L, FALSE, this);
	v->rawBleed(WEAR_WRIST_L, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	v->woundedHand(!v->isRightHanded());
	*part_hit = WEAR_WRIST_L;
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 75:
      case 76:
	// cleave arm at shoulder
	if (!v->hasPart(WEAR_ARM_R))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_R);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_R);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_R);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(WEAR_ARM_R, FALSE, this);
	v->rawBleed(WEAR_BODY, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	v->woundedHand(v->isRightHanded());
	*part_hit = WEAR_ARM_R;
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 77:
      case 78:
	// cleave l-arm at shoulder
	if (!v->hasPart(WEAR_ARM_L))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_L);
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s and sends it flying!!  OH THE PAIN!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_L);
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		limbStr %
		v->describeBodySlot(WEAR_ARM_L);
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(WEAR_ARM_L, FALSE, this);
	v->rawBleed(WEAR_BODY, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	v->woundedHand(!v->isRightHanded());
	*part_hit = WEAR_ARM_L;
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 79:
      case 80:
	// sever leg: foot missing, leg useless
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA)) {
	  if (!v->hasPart(WEAR_FOOT_R))
	    return 0;
buf=format("Your %s severs $N's %s and sends it flying!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_R);
	  act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s!!  OH THE PAIN!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_R);
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_R);
	  act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	  v->makePartMissing(WEAR_FOOT_R, FALSE, this);
	  v->addToLimbFlags(WEAR_LEG_R, PART_USELESS);
	  if ((obj = v->equipment[WEAR_LEG_R]))
	    critHitEqDamage(v, obj, (::number(-35,-22)));

	  v->rawBleed(WEAR_LEG_R, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	  *part_hit = WEAR_LEG_R;
	  if (desc)
	    desc->career.crit_sev_limbs++;
	  if (v->desc)
	    v->desc->career.crit_sev_limbs_suff++;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 81:
      case 82:
	// sever other leg: foot missing, leg useless
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
	    v->hasPart(WEAR_FOOT_L)) {
buf=format("Your %s severs $N's %s and sends it flying!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_L);
	  act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your %s!!  OH THE PAIN!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_L);
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's %s and sends it flying!") %
		  limbStr %
		  v->describeBodySlot(WEAR_FOOT_L);
	  act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	  v->makePartMissing(WEAR_FOOT_L, FALSE, this);
	  v->addToLimbFlags(WEAR_LEG_L, PART_USELESS);
	  if ((obj = v->equipment[WEAR_LEG_L]))
	    critHitEqDamage(v, obj, (::number(-35,-22)));

	  v->rawBleed(WEAR_LEG_L, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	  *part_hit = WEAR_LEG_L;
	  if (desc)
	    desc->career.crit_sev_limbs++;
	  if (v->desc)
	    v->desc->career.crit_sev_limbs_suff++;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 83:
      case 84:
	// impale with weapon
	if (!v->hasPart(WEAR_BODY))
	  return 0;
buf=format("You stick your %s through $N's body, impaling $M!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s is thrust through your torso, impaling you!!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n thrusts $s %s deep into $N's torso, impaling $M!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	if (desc)
	  desc->career.crit_impale++;
	if (v->desc)
	  v->desc->career.crit_impale_suff++;
	rc = dislodgeWeapon(v, weapon, WEAR_BODY);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	rc = applyDamage(v, v->hitLimit()/2,DAMAGE_IMPALE);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	*part_hit = WEAR_BODY;
	return ONEHIT_MESS_CRIT_S;
      case 85:
      case 86:
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
	    v->hasPart(WEAR_WAIST) &&
	    (obj = v->equipment[WEAR_WAIST])) {
buf=format("You attempt to cleave $N in two with your %s, but $p saves $M from a hideous fate.") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n tries to cleave you in two with $s %s, but $p saves you thankfully!") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_VICT, ANSI_RED);
buf=format("$n attempts to cleave $N in two with $s %s! Thankfully $p saves $M!") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);

	  critHitEqDamage(v, obj, (::number(-55,-40)));

	  *part_hit = WEAR_WAIST;
	  rc = damageLimb(v,WEAR_WAIST,weapon,dam);
	  if (IS_SET_DELETE(rc, DELETE_VICT))
	    return DELETE_VICT;
	  return ONEHIT_MESS_CRIT_S;
	}
	// if no girth, its going into next critSuccess...
      case 87:
      case 88:
	// cleave in two
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA)) {
	  if (!weapon && ((getHeight()*3) < v->getHeight()) &&
	      !isDiabolic() && !isLycanthrope()) {
buf=format("With a mighy warcry, you almost cleave $N in two with your %s.") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n unleashes a mighty warcry and slashes you HARD down the center with $s %s!") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n gives a mighty warcry and slashes $N down the center with $s %s!") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	    if ((obj = v->equipment[WEAR_WAIST]))
	      critHitEqDamage(v, obj, (::number(-45,-30)));

	    *part_hit = WEAR_WAIST;
	    if (desc)
	      desc->career.crit_cleave_two++;
	    if (v->desc)
	      v->desc->career.crit_cleave_two_suff++;
	    return applyDamage(v, GetMaxLevel()*3, DAMAGE_HACKED);
	  } else {
buf=format("With a mighty warcry, you cleave $N in two with your %s.") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n unleashes a mighty warcry before cleaving you in two with $s %s!") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n gives a mighty warcry and cleaves $N in two with $s %s!") %
		    limbStr;
	    act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	    if ((obj = v->equipment[WEAR_WAIST]))
	      critHitEqDamage(v, obj, (::number(-45,-30)));

	    applyDamage(v, 20 * v->hitLimit(),DAMAGE_HACKED);
	    *part_hit = WEAR_WAIST;
	    if (desc)
	      desc->career.crit_cleave_two++;
	    if (v->desc)
	      v->desc->career.crit_cleave_two_suff++;
	    return DELETE_VICT;
	  }
	}
      case 89:
      case 90:
	// slice torso from gullet to groin
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) && !IS_SET(v->specials.act, ACT_SKELETON)) {
buf=format("With your %s, you slice $N from gullet to groin disembowling $M!") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s slices you from gullet to groin, disembowling you!") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s slices into $N from gullet to groin, disembowling $M!") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	  if ((obj = v->equipment[WEAR_BODY]))
	    critHitEqDamage(v, obj, (::number(-105,-80)));

	  applyDamage(v, 20 * v->hitLimit(),DAMAGE_DISEMBOWLED_HR);
	  *part_hit = WEAR_BODY;
	  if (desc)
	    desc->career.crit_disembowel++;
	  if (v->desc)
	    v->desc->career.crit_disembowel_suff++;
	  return DELETE_VICT;
	}
      case 91:
      case 92:
	if (v->getSex()==SEX_MALE && v->hasPart(WEAR_WAIST) && (!(obj = v->equipment[WEAR_WAIST]) || !obj->isMetal()) && !IS_SET(v->specials.act, ACT_SKELETON) && !IS_SET(v->specials.act, ACT_GHOST)){
	  buf = format("With a deft swing of your %s, you sever $N's genitals.") % limbStr;
	  act(buf,FALSE,this,obj,v,TO_CHAR,ANSI_ORANGE);
	  buf = format("$n deftly severs your genitals with $s %s!  OWWWWW!") % limbStr;
	  act(buf,FALSE,this,obj,v,TO_VICT,ANSI_ORANGE);

	  if(obj)
	    critHitEqDamage(v, obj, (::number(-45,-30)));

	  TCorpse *corpse;
		
	  corpse = new TCorpse();
    if (dynamic_cast<TPerson *>(this)) 
      buf = format("genitalia lost limb %s [q] [jumblies] [0] [%d] [%s]") % v->name % v_vnum % getName();
    else
      buf = format("genitalia lost limb %s") % v->name;
    corpse->name = mud_str_dup(buf);
		
	  if (v->getMaterial(WEAR_WAIST) > MAT_GEN_MINERAL) {
	    // made of mineral or metal
	    buf = format("the mangled genitalia of %s") % v->getName();
	  } else {
	    buf = format("the bloody, mangled genitalia of %s") % v->getName();
	  }
	  corpse->shortDescr = mud_str_dup(buf);
		
	  if (v->getMaterial(WEAR_WAIST) > MAT_GEN_MINERAL) {
	    // made of mineral or metal
	    buf = format("The mangled, severed genitalia of %s is lying here.") % v->getName();
	  } else {
	    buf = format("The bloody, mangled, severed genitalia of %s is lying here.") % v->getName();
	  }
	  corpse->setDescr(mud_str_dup(buf));
		
	  corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
	  corpse->addCorpseFlag(CORPSE_NO_REGEN);
	  corpse->obj_flags.decay_time = 3 * (dynamic_cast<TMonster *>(this) ? MAX_NPC_CORPSE_TIME : MAX_PC_CORPSE_EMPTY_TIME);
	  corpse->setWeight(v->getWeight() / 32.0);
	  corpse->canBeSeen = v->canBeSeen;
	  corpse->setVolume(v->getVolume() * 2/100);
	  corpse->setMaterial(v->getMaterial(WEAR_WAIST));
		
	  act("$p goes flying through the air and bounces once before it rolls to a stop.",TRUE,v,corpse,0,TO_ROOM, ANSI_RED);
	  *v->roomp += *corpse;

	  v->setSex(SEX_NEUTER);
	  v->rawBleed(WEAR_WAIST, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);

	  if (desc)
	    desc->career.crit_genitalia++;
		
	  if (v->desc)
	    v->desc->career.crit_genitalia_suff++;
		
	  return ONEHIT_MESS_CRIT_S;
	}
	break;
      case 93:
      case 94:
      case 95:
      case 96:
      case 97:
	return FALSE;
	break;
      case 98:
      case 99:
      case 100:
	// decapitate if no neck armor
	if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
	    (obj = v->equipment[WEAR_NECK])) {
buf=format("You attempt to decapitate $N with your %s, but $p saves $M from a hideous fate.") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n tries to decapitate you with $s %s, but $p saves you!") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_VICT, ANSI_RED);
buf=format("$n attempts to decapitate $N with $s %s!  Luckily, $p saves $M!") %
		  limbStr;
	  act(buf, FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);

	  critHitEqDamage(v, obj, (::number(-40,-25)));
	  *part_hit = WEAR_NECK;
	  rc = damageLimb(v,*part_hit,weapon,dam);
	  if (IS_SET_DELETE(rc, DELETE_VICT))
	    return DELETE_VICT;
	  return ONEHIT_MESS_CRIT_S;
	} else { // no collar
	  // if no collar, its going into next critSuccess...
	  // POW! He was deCAPITATED! 
	  act("$n strikes a fatal blow and cuts off $N's head!", FALSE, this, 0, v, TO_NOTVICT, ANSI_CYAN);
	  act("You strike a fatal blow and completely behead $N!", FALSE, this, 0, v, TO_CHAR, ANSI_RED);
	  act("$n strikes a fatal blow and completely beheads you!", FALSE, this, 0, v, TO_VICT, ANSI_RED);
	  if (v->roomp && !v->roomp->isRoomFlag(ROOM_ARENA) &&
	      (obj = v->equipment[WEAR_NECK]))
	    critHitEqDamage(v, obj, (::number(-40,-25)));

	  v->makeBodyPart(WEAR_HEAD, this);
	  applyDamage(v, (20 * v->hitLimit()),DAMAGE_BEHEADED);
	  *part_hit = WEAR_NECK;
	  if (desc)
	    desc->career.crit_beheads++;
		
	  if (v->desc)
	    v->desc->career.crit_beheads_suff++;
		
	  return DELETE_VICT;
	}
      default:
	vlogf(LOG_BUG, format("crit_num=%i in critSlash switch, shouldn't happen") % 
	      crit_num);
	break;
    }
  }
  return 0;
}

/* ------------------------------------------------------------

====Pierce crit table====

0-33: double damage
34-66: triple damage
67,68: pierced larynx
69,70: gouged out eye
71,72: sever tendon
73,74: stab back (impale)
75,76: pierce cranium
77,78: shatter elbow
79,80: sever hand
81,82: punctured lung (impale)
83,84,85: punctured kindey infect (impale)
86,87: punctured stomach
100: crit kill (death)

total:
  double damage: 33%
  triple damage: 33%
  impale: 7%
  minor sever/ailment: 11%
  major sever/ailment: 8%
  death: 3%

------------------------------------------------------------ */
int TBeing::critPierce(TBeing *v, TThing *weapon, wearSlotT *part_hit,
		       spellNumT wtype, int *dam, int crit_num)
{
  sstring buf, weaponStr, limbStr;
  TThing *obj=NULL;
  int rc, i;
  affectedData af;
  wearSlotT new_slot;

  weaponStr=(weapon ? "$o" : getMyRace()->getBodyLimbPierce(this));
  
  if(crit_num>100){
    vlogf(LOG_BUG, format("critPierce called with crit_num>100 (%i)") %  crit_num);
    crit_num=0;
  }

  // Do pierce crit 
  if (crit_num <= 33) {
    // double damage 
    *dam *= 2;

    buf = format("You strike $N's %s exceptionally well, sinking your %s deep into $S flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n strikes your %s exceptionally well, sinking $s %s deep into your flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_VICT, ANSI_RED);

    buf = format("$n strikes $N's %s exceptionally well, sinking $s %s deep into $N's flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else if (crit_num <= 66) {
    // triple damage 
    *dam *= 3;

    buf = format("You critically strike $N's %s, sinking your %s deep into $S flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_CHAR, ANSI_ORANGE);

    buf = format("$n critically strikes your %s, sinking $s %s deep into your flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_VICT, ANSI_RED);

    buf = format("$n critically strikes $N's %s, sinking $s %s deep into $N's flesh!") % v->describeBodySlot(*part_hit) % weaponStr;
    act(buf, FALSE, this, weapon, v, TO_NOTVICT, ANSI_BLUE);

    return (ONEHIT_MESS_CRIT_S);
  } else {
    // better stuff 
    limbStr=(weapon ? fname(weapon->name) : getMyRace()->getBodyLimbPierce(this));

    switch (crit_num) {
      case 67:
	if (!v->hasPart(WEAR_NECK) || IS_SET(v->specials.act, ACT_GHOST) || IS_SET(v->specials.act, ACT_SKELETON))
	  return 0;
	*part_hit = WEAR_NECK;
	if ((obj = v->equipment[WEAR_NECK])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from a punctured larynx!\n\r") %
		    fname(obj->name));
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_NECK])
	      v->damageItem(this,WEAR_NECK,wtype,weapon,*dam);
	  return ONEHIT_MESS_CRIT_S;
	}
	// intentional drop through
      case 68:
	// Punctured Larnyx, can't speak 
	if (!v->hasPart(WEAR_NECK) || IS_SET(v->specials.act, ACT_GHOST) || IS_SET(v->specials.act, ACT_SKELETON))
	  return 0;
	if (v->hasDisease(DISEASE_VOICEBOX))
	  return 0;
buf=format("You pop your %s into $N's throat, puncturing $S voice box!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n pops $s %s into your throat, puncturing your voice box!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n pops $s %s into $N's throat, puncturing $S voice box!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_NECK])
	    v->damageItem(this,WEAR_NECK,wtype,weapon,*dam);
	af.type = AFFECT_DISEASE;
	af.level = 0;   // has to be 0 for doctor to treat
	af.duration = PERMANENT_DURATION;
	af.modifier = DISEASE_VOICEBOX;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SILENT;
	v->affectTo(&af);
	*part_hit = WEAR_NECK;
	if (desc)
	  desc->career.crit_voice++;
	if (v->desc)
	  v->desc->career.crit_voice_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 69:
      case 70:
	// Struct in eye, blinded with new blind type
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (v->hasDisease(DISEASE_EYEBALL))
	  return 0;
	if (!v->hasPart(WEAR_HEAD))
	  return 0;
buf=format("You pop your %s into $N's eyes, gouging them out and blinding $M!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n pops $s %s into your eyes and The World goes DARK!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n pops $s %s into $N's eyes, gouging them out and blinding $M!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	act("$n's eyeballs fall from $s sockets!",TRUE,v,0,0,TO_ROOM);
	v->makeOtherPart(NULL,"eyeballs",this);
	af.type = AFFECT_DISEASE;
	af.level = 0;   // has to be 0 for doctor to treat
	af.duration = PERMANENT_DURATION;
	af.modifier = DISEASE_EYEBALL;
	af.location = APPLY_NONE;
	af.bitvector = AFF_BLIND;
	v->affectTo(&af);
	v->rawBlind(GetMaxLevel(), af.duration, SAVE_NO);
	*part_hit = WEAR_HEAD;
	if (desc)
	  desc->career.crit_eye_pop++;
	if (v->desc)
	  v->desc->career.crit_eye_pop_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 71:
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_LEG_R))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
	if ((obj = v->equipment[WEAR_LEG_R])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from losing a tendon!\n\r") %
		    fname(obj->name));
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_LEG_R])
	      v->damageItem(this,WEAR_LEG_R,wtype,weapon,*dam);
	  *part_hit = WEAR_LEG_R;
	  return ONEHIT_MESS_CRIT_S;
	}
	// an intentional drop through
      case 72:
	// strike lower leg, rip tendons, vict at -25% move. 
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_LEG_R))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s rips through $N's tendon on $S lower leg!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s rips through the tendon in your lower leg.") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s rips into $N, tearing the tendon in $S lower leg.") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->setMove(v->getMove()/4);
	for (i = 1; i < 5; i++) {
	  if (v->equipment[WEAR_LEG_R])
	    v->damageItem(this,WEAR_LEG_R,wtype,weapon,*dam);
	}
	*part_hit = WEAR_LEG_R;
	rc = damageLimb(v,WEAR_LEG_R,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 73:
	if (!v->hasPart(WEAR_BACK))
	  return 0;
	if ((obj = v->equipment[WEAR_BACK])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from a gory wound!\n\r") %
		    fname(obj->name));
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_BACK])
	      v->damageItem(this,WEAR_BACK,wtype,weapon,*dam);
	  *part_hit = WEAR_BACK;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 74:
	// Side wound, vict stunned 6 rounds. 
	if (!v->hasPart(WEAR_BACK))
	  return 0;
buf=format("You plunge your %s deep into $N's side, stunning $M!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n plunges $s %s deep into your side.  The agony makes you forget about the fight.") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n plunges $s %s deep into $N's side, stunning $M.") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	for (i=1;i<5;i++)
	  if (v->equipment[WEAR_BACK])
	    v->damageItem(this,WEAR_BACK,wtype,weapon,*dam); 
	v->cantHit += v->loseRound(6);
	rc = dislodgeWeapon(v,weapon,WEAR_BACK);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	*part_hit = WEAR_BACK;
	return ONEHIT_MESS_CRIT_S;
      case 75:
      case 76:
	// Strike in back of head. If no helm, vict dies. 
	if (!v->hasPart(WEAR_HEAD))
	  return 0;
	if ((obj = v->equipment[WEAR_HEAD])) {
	  buf=format("You try to thrust your %s into the back of $N's head.") %
	    limbStr;
	  act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	  buf = "Unfortunately, $p saves $M from a hideous death!";
	  act(buf, FALSE, this, obj, v, TO_CHAR);
	  buf=format("$n tries to thrust $s %s into the back of your head.") %
	    limbStr;
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	  buf="But $p saves you from a hideous death!";
	  act(buf, FALSE, this, obj, v, TO_VICT, ANSI_RED);
	  buf=format("$n tries plunging $s %s into the back of $N's head, but $p saves $M.") %
	    limbStr;
	  act(buf, FALSE, this, obj, v, TO_NOTVICT, ANSI_BLUE);
	  for (i=1;i<5;i++)
	    if (v->equipment[WEAR_HEAD])
	      v->damageItem(this,WEAR_HEAD,wtype,weapon,*dam);
	  rc = dislodgeWeapon(v,weapon,WEAR_HEAD);
	  if (IS_SET_DELETE(rc, DELETE_VICT))
	    return DELETE_VICT;
	  *part_hit = WEAR_HEAD;
	  rc = damageLimb(v,WEAR_HEAD,weapon,dam);
	  if (IS_SET_DELETE(rc, DELETE_VICT))
	    return DELETE_VICT;
	  return ONEHIT_MESS_CRIT_S;
	} else {
buf=format("You thrust your %s into the back of $N's head causing an immediate death.") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s tears into the back of your unprotected head.") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	  buf = format("The world goes black and dark...");
	  act(buf, FALSE, this, 0, v, TO_VICT, ANSI_BLACK);
buf=format("$n thrusts $s %s deep into the back of $N's unprotected head, causing an immediate death.") %
		  limbStr;
	  act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	  rc = dislodgeWeapon(v,weapon,WEAR_HEAD);
	  if (IS_SET_DELETE(rc, DELETE_VICT))
	    return DELETE_VICT;
	  applyDamage(v, (20 * v->hitLimit()),wtype);
	  *part_hit = WEAR_HEAD;
	  if (desc)
	    desc->career.crit_cranial_pierce++;
	  if (v->desc)
	    v->desc->career.crit_cranial_pierce_suff++;
	  return DELETE_VICT;
	}
	return FALSE;   // not possible, but just in case
      case 77:
      case 78:
	// Strike shatters elbow in weapon arm. Arm broken 
	new_slot = v->getSecondaryArm();
	if (!v->hasPart(new_slot))
	  return 0;
	if (!v->isHumanoid())
	  return FALSE;
buf=format("$N blocks your %s with $S arm.  However the force shatters $S elbow!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s is blocked by your arm.  Unfortunately your elbow is shattered!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s shatters $N's elbow!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->damageArm(FALSE,PART_BROKEN);
	if (desc)
	  desc->career.crit_broken_bones++;
	if (v->desc)
	  v->desc->career.crit_broken_bones_suff++;
	*part_hit = new_slot;
	rc = damageLimb(v,new_slot,weapon,dam);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	return ONEHIT_MESS_CRIT_S;
      case 79:
	new_slot = v->getPrimaryHand();
	if (!v->hasPart(new_slot))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
	if ((obj = v->equipment[v->getPrimaryWrist()])) {
	  act("Your $o just saved you from losing your hand!",TRUE,v,obj,0,TO_CHAR, ANSI_PURPLE);
	  act("You nearly sever $N's hand, but $S $o saved $M!",TRUE,this,obj,v,TO_CHAR);
	  *part_hit = v->getPrimaryWrist();
	  return ONEHIT_MESS_CRIT_S;
	}
      case 80:
	// Sever weapon arm at hand 
	if (!v->hasPart(v->getPrimaryHand()))
	  return 0;
	if (!v->isHumanoid())
	  return 0;
buf=format("Your %s severs $N's hand at $S wrist!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s severs your arm below the wrist!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s severs $N's hand at the wrist!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->makePartMissing(v->getPrimaryHand(), FALSE, this);
	v->rawBleed(v->getPrimaryWrist(), PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	v->woundedHand(TRUE);
	*part_hit = v->getPrimaryHand();
	if (desc)
	  desc->career.crit_sev_limbs++;
	if (v->desc)
	  v->desc->career.crit_sev_limbs_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 81:
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_BODY))
	  return 0;
	if ((obj = v->equipment[WEAR_BODY])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from a punctured lung!\n\r") %
		    fname(obj->name));
	  for (i=1;i<9;i++)
	    if (v->equipment[WEAR_BODY])
	      v->damageItem(this,WEAR_BODY,wtype,weapon,*dam);
	  *part_hit = WEAR_BODY;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 82:
	// Punctured lungs. Can't breathe. Dies if not healed quickly 
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (v->hasDisease(DISEASE_LUNG))
	  return 0;
buf=format("Your %s plunges into $N's chest puncturing a lung!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s plunges into your chest and punctures a lung!!!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s plunges into $N's chest.\n\rA hiss of air escapes $S punctured lung!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	af.type = AFFECT_DISEASE;
	af.level = 0;   // has to be 0 for doctor to treat
	af.duration = PERMANENT_DURATION;
	af.modifier = DISEASE_LUNG;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SILENT;
	v->affectTo(&af);
	rc = dislodgeWeapon(v,weapon,WEAR_BODY);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	v->sendTo("You won't be able to speak or breathe until you get that punctured lung fixed!!!\n\r");
	*part_hit = WEAR_BODY;
	if (desc)
	  desc->career.crit_lung_punct++;
	if (v->desc)
	  v->desc->career.crit_lung_punct_suff++;
	return ONEHIT_MESS_CRIT_S;
      case 83:
      case 84:
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_BODY))
	  return 0;
	if ((obj = v->equipment[WEAR_BODY])) {
	  v->sendTo(COLOR_OBJECTS, format("Your %s saves you from a kidney wound!\n\r") %
		    fname(obj->name));
	  for (i=1;i<7;i++)
	    if (v->equipment[WEAR_BODY])
	      v->damageItem(this,WEAR_BODY,wtype,weapon,*dam);
	  *part_hit = WEAR_BODY;
	  return ONEHIT_MESS_CRIT_S;
	}
      case 85:
	// punctured kidney causes infection
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_BODY))
	  return 0;
buf=format("You puncture $N's kidney with your %s and cause an infection!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s tears into your kidney; the pain is AGONIZING and an infection has started!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s punctures $N's kidney!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);

	if (desc)
	  desc->career.crit_kidney++;
	if (v->desc)
	  v->desc->career.crit_kidney_suff++;

	rc = dislodgeWeapon(v,weapon,WEAR_BODY);
	if (IS_SET_DELETE(rc, DELETE_VICT))
	  return DELETE_VICT;
	v->rawInfect(WEAR_BODY, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	*part_hit = WEAR_BODY;
	return ONEHIT_MESS_CRIT_S;
      case 86:
      case 87:
	// stomach wound.  causes death 5 mins later if not healed.
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	if (!v->hasPart(WEAR_BODY))
	  return 0;
	if (v->hasDisease(DISEASE_STOMACH))
	  return 0;
buf=format("You plunge your %s into $N's stomach, opening up $S gullet!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
buf=format("$n's %s tears into your stomach and exposes your intestines!!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
buf=format("$n's %s tears into $N's stomach exposing intestines!") %
		limbStr;
	act(buf, FALSE, this, 0, v, TO_NOTVICT, ANSI_BLUE);
	v->rawInfect(WEAR_BODY, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	if (v->hasPart(WEAR_WAIST))
	  v->rawInfect(WEAR_WAIST, PERMANENT_DURATION, SILENT_NO, CHECK_IMMUNITY_YES);
	af.type = AFFECT_DISEASE;
	af.level = 0;   // for doctor to heal
	af.duration = PERMANENT_DURATION;
	af.modifier = DISEASE_STOMACH;
	v->affectTo(&af);
	*part_hit = WEAR_WAIST;
	if (desc)
	  desc->career.crit_eviscerate++;
	if (v->desc)
	  v->desc->career.crit_eviscerate_suff++;

	return ONEHIT_MESS_CRIT_S;
      case 88:
      case 89:
	// abdominal wound
	// You plunge your %s into $N's abdoman and tear out causing a shower of blood

      case 90:
      case 91:
      case 92:
      case 93:
      case 94:
      case 95:
      case 96:
      case 97:
      case 98:
      case 99:
	return FALSE;
	break;
      case 100:
        if (IS_SET(v->specials.act, ACT_SKELETON) || IS_SET(v->specials.act, ACT_GHOST))
          return 0;
	buf=format("You sink your %s between $N's eyes, causing an immediate death!") %
	  limbStr;
	act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_ORANGE);
	buf=format("$n sinks $s %s right between your eyes, causing an immediate death!") %
	  limbStr;
	act(buf, FALSE, this, 0, v, TO_VICT, ANSI_RED);
	buf=format("$n sinks $s %s smack between $N's eyes, causing an immediate death!") %
	  limbStr;
	act(buf, FALSE, this, 0, v, TO_ROOM);
	applyDamage(v, (20 * v->hitLimit()),wtype);
	*part_hit = WEAR_HEAD;
	if (desc)
	  desc->career.crit_cranial_pierce++;
	if (v->desc)
	  v->desc->career.crit_cranial_pierce_suff++;
	return DELETE_VICT;
      default:
	vlogf(LOG_BUG, format("crit_num=%i in critPierce switch, shouldn't happen") % 
	      crit_num);
	break;
    }
  }
  return 0;
}
