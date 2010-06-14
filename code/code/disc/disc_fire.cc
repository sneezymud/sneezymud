//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "room.h"
#include "low.h"
#include "monster.h"
#include "disease.h"
#include "combat.h"
#include "spelltask.h"
#include "disc_fire.h"
#include "obj_light.h"
#include "obj_magic_item.h"

int handsOfFlame(TBeing *caster, TBeing *victim, int level, short bKnown, int *damage, int adv_learn)
{
  int ret = 0;

// First unusual Fails at cast time

  if (victim->getImmunity(IMMUNE_HEAT) >= 100) {
    act("Nothing seems to happen, $N seems totally immune to the flames.",
        TRUE, caster, 0, victim, TO_CHAR);
    act("Nothing seems to happen, you are totally immune to the flames.",
        TRUE, caster, 0, victim, TO_VICT);
    act("Nothing seems to happen, $N seems totally immune to the flames.", 
        TRUE, caster, 0, victim, TO_NOTVICT);
    return SPELL_FALSE;
  }
  if (!caster->hasHands()) {
    act("How do you expect to cast this spell without hands.", 
        TRUE, caster, 0, 0, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }
// Second Calculate Damage
  level = min(level, 10);
  *damage = caster->getSkillDam(victim, SPELL_HANDS_OF_FLAME, level, adv_learn);

//  ToBe coded
//  *damage = caster->modSpellDamage(*damage);
//  *damage = caster->modSectorDamage(*damage);

// Do success and Fail
  if (caster->bSuccess(bKnown, SPELL_HANDS_OF_FLAME)) {
    ret = SPELL_SUCCESS;
    switch (critSuccess(caster, SPELL_HANDS_OF_FLAME)) {
      case CRIT_S_DOUBLE:
        CS(SPELL_HANDS_OF_FLAME);
        *damage <<= 1;
        ret =  ret + SPELL_CRIT_SUCCESS + SPELL_CSUC_DOUBLE;
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_HANDS_OF_FLAME);
        *damage *= 3;
        ret = ret + SPELL_CRIT_SUCCESS + SPELL_CSUC_TRIPLE;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_HANDS_OF_FLAME))) {
          SV(SPELL_HANDS_OF_FLAME);
          ret = ret + SPELL_SAVE;
          *damage /= 2;
        }
        break;
    }
    caster->reconcileHurt(victim,discArray[SPELL_HANDS_OF_FLAME]->alignMod);
    return ret;
  } else {
    ret = SPELL_FAIL;
    switch (critFail(caster, SPELL_HANDS_OF_FLAME)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_HANDS_OF_FLAME);
        ret = ret + SPELL_CFAIL_DEFAULT + SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    return ret;
  }
  return ret;
}

int handsOfFlame(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;
  int rc = 0;
  int damage;

  ret=handsOfFlame(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), &damage, 0);

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p has embued your hands with a magic heat!",
        FALSE, caster, obj, victim, TO_CHAR, ANSI_RED);
    act("You reach out and touch $N!",
        FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
    act("$n reaches out and grabs $N!",
        FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
    act("$n reaches out and grabs you!",
        FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);

    if (IS_SET(ret, SPELL_SAVE)) {
      act("$N is only partially burned by your touch!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_ORANGE);
      act("$N is burned by $n's touch!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_ORANGE);
      act("You manage to avoid some of the fire in $n's touch!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_ORANGE);
    } else {
      act("$n screams in agony!", FALSE, victim, 0, 0, TO_ROOM, ANSI_RED);
      act("You scream in agony as the heat scalds you!",
          FALSE, victim, 0, 0, TO_CHAR, ANSI_RED);
    }
    if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
      if (IS_SET(ret, SPELL_CSUC_DEFAULT)) {
      }
      if (IS_SET(ret, SPELL_CSUC_DOUBLE)) {
        act("Second degree burns break out on $N!",
            FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
        act("$N suffers from second degree burns!",
            FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
        act("The painful fire has left a second degree burn!!",
            FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);
      }
      if (IS_SET(ret, SPELL_CSUC_TRIPLE)) {
        act("LARGE second degree burns break out on $N!",
            FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
        act("$N suffers from LARGE second degree burns!",
            FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
        act("The painful fire has left a LARGE second degree burn!!",
            FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);
      }
    }
    if (caster->reconcileDamage(victim, damage, SPELL_HANDS_OF_FLAME) == -1)
      ADD_DELETE(ret, VICTIM_DEAD);
  }

  if (IS_SET(ret, SPELL_FAIL) && !IS_SET(ret, SPELL_CRIT_FAIL)) {
     act("$n's hands glow for a brief second but then extinguish!",
         FALSE, caster, NULL, NULL, TO_ROOM, ANSI_ORANGE);
     act("You fail to add magic heat to your hands!",
         FALSE, caster, NULL, NULL, TO_CHAR, ANSI_ORANGE);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL) || IS_SET(ret, SPELL_CFAIL_DEFAULT)) {
    act("$n's hands become engulfed in flames!",
        FALSE, caster, NULL, NULL, TO_ROOM, ANSI_RED);
    act("Your hands become engulfed in flames!  Man, that hurts!",
        FALSE, caster, NULL, NULL, TO_CHAR, ANSI_RED);
    if (caster->reconcileDamage(caster, damage, SPELL_HANDS_OF_FLAME) == -1)
      ADD_DELETE(ret, CASTER_DEAD);
  } else {
    if (IS_SET(ret, SPELL_CFAIL_SELF)) {
    }
    if (IS_SET(ret, SPELL_CFAIL_OTHER)) {
    }
  }
  if (IS_SET(ret, SPELL_FAIL_SAVE)) {
  }
  if (IS_SET(ret, SPELL_ACTION)) {
  }

  if (IS_SET(ret, SPELL_FALSE)) {
  }

  if (IS_SET(ret, VICTIM_DEAD)) {
    ADD_DELETE(rc, DELETE_VICT);
  }
  if (IS_SET(ret, CASTER_DEAD)) {
    ADD_DELETE(rc, DELETE_THIS);
  }
  return rc;
}

int handsOfFlame(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;
// 1. First check for unusual fails
  if (!caster->hasHands()) {
    act("How do you expect to cast this spell without hands?",
        TRUE, caster, 0, 0, TO_CHAR);
    return FALSE;
  }

// 2. Second send to general mage check function 

  if (!bPassMageChecks(caster, SPELL_HANDS_OF_FLAME, victim))
    return FALSE;

// 3.  check for Component or use it
//     In this spell no component so don't need check

// 4.   Get lag and difficulty for adding to casting object
  lag_t rounds = discArray[SPELL_HANDS_OF_FLAME]->lag;
  diff = discArray[SPELL_HANDS_OF_FLAME]->task;

// 5.   Initialize the casting object with data
  start_cast(caster, victim, NULL, caster->roomp, SPELL_HANDS_OF_FLAME, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

// 6.   Start them fighting with 0 damage
  return TRUE;
}

int castHandsOfFlame(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;
  int damage;

  level = caster->getSkillLevel(SPELL_HANDS_OF_FLAME);

  ret=handsOfFlame(caster,victim,level,caster->getSkillValue(SPELL_HANDS_OF_FLAME), &damage, caster->getAdvLearning(SPELL_HANDS_OF_FLAME));

//  if (IS_SET(ret, SPELL_SUCCESS) && IS_SET(ret, SPELL_CRIT_SUCCESS)) {
//  } else if (IS_SET(ret, SPELL_SUCCESS)) {
//  }

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("You reach out and touch $N!", 
        FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
    act("$n reaches out and grabs $N!", 
        FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
    act("$n reaches out and grabs you!", 
        FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);

    if (IS_SET(ret, SPELL_SAVE)) {
      act("$N is only partially burned by your touch!",
          FALSE, caster, NULL, victim, TO_CHAR, ANSI_ORANGE);
      act("$N is burned by $n's touch!",
          FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_ORANGE);
      act("You manage to avoid some of the fire in $n's touch!",
          FALSE, caster, NULL, victim, TO_VICT, ANSI_ORANGE);
    } else {
      act("$n screams in agony!", FALSE, victim, 0, 0, TO_ROOM, ANSI_RED);
      act("You scream in agony as the heat scalds you!",
              FALSE, victim, 0, 0, TO_CHAR, ANSI_RED);
    }
    if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
      if (IS_SET(ret, SPELL_CSUC_DEFAULT)) {
      }
      if (IS_SET(ret, SPELL_CSUC_DOUBLE)) {
        act("Second degree burns break out on $N!",
            FALSE, caster, NULL, victim, TO_CHAR, ANSI_RED);
        act("$N suffers from second degree burns!",
            FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_RED);
        act("The painful fire has left a second degree burn!!",
            FALSE, caster, NULL, victim, TO_VICT, ANSI_RED);
      }
      if (IS_SET(ret, SPELL_CSUC_TRIPLE)) {
        act("LARGE second degree burns break out on $N!",
            FALSE, caster, NULL, victim, TO_CHAR, ANSI_ORANGE);
        act("$N suffers from LARGE second degree burns!",
            FALSE, caster, NULL, victim, TO_NOTVICT, ANSI_ORANGE);
        act("The painful fire has left a LARGE second degree burn!!",
            FALSE, caster, NULL, victim, TO_VICT, ANSI_ORANGE);
      }
    }
    if (caster->reconcileDamage(victim, damage, SPELL_HANDS_OF_FLAME) == -1)
      ADD_DELETE(ret, VICTIM_DEAD);
  }
  
//  if (IS_SET(ret, SPELL_SAVE)) {
//  }

  if (IS_SET(ret, SPELL_FAIL) && !IS_SET(ret, SPELL_CRIT_FAIL)) {
     act("$n's hands glow for a brief second but then extinguish!", 
         FALSE, caster, NULL, NULL, TO_ROOM, ANSI_ORANGE);
     act("You fail to add magic heat to your hands!", 
         FALSE, caster, NULL, NULL, TO_CHAR, ANSI_ORANGE);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL) || IS_SET(ret, SPELL_CFAIL_DEFAULT)) {
    act("$n's hands become engulfed in flames!", 
        FALSE, caster, NULL, NULL, TO_ROOM, ANSI_RED);
    act("Your hands become engulfed in flames!  Man, that hurts!", 
        FALSE, caster, NULL, NULL, TO_CHAR, ANSI_RED);
    if (caster->reconcileDamage(caster, damage, SPELL_HANDS_OF_FLAME) == -1)
      ADD_DELETE(ret, CASTER_DEAD);
  } else {
    if (IS_SET(ret, SPELL_CFAIL_SELF)) {
    }
    if (IS_SET(ret, SPELL_CFAIL_OTHER)) {
    }
  }
  if (IS_SET(ret, SPELL_FAIL_SAVE)) {
  }

  if (IS_SET(ret, SPELL_ACTION)) {
  }

  if (IS_SET(ret, SPELL_FALSE)) {
  }

  if (IS_SET(ret, VICTIM_DEAD)) {
    ADD_DELETE(rc, DELETE_VICT);
  }
  if (IS_SET(ret, CASTER_DEAD)) {
    ADD_DELETE(rc, DELETE_THIS);
  }
  return rc;
}

int faerieFire(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;
  int ret = 0;

  if (victim->affectedBySpell(SPELL_FAERIE_FIRE)) {
    act("You sense that $N is already affected by the spell!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to cast something on you!", 
        0, caster, NULL, victim, TO_VICT);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }

  caster->reconcileHurt(victim,discArray[SPELL_FAERIE_FIRE]->alignMod);

  aff.type = SPELL_FAERIE_FIRE;
  aff.level = level;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;

  // we'd like it to last about 5 minutes
  aff.duration = 5 * UPDATES_PER_MUDHOUR / 2;

  // let the affect be level dependant
  aff.modifier = 100 + (aff.level*4);

  if (caster->bSuccess(bKnown, SPELL_FAERIE_FIRE)) {
    ret = SPELL_SUCCESS;
    switch (critSuccess(caster, SPELL_FAERIE_FIRE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FAERIE_FIRE);
        aff.duration *=2;
        aff.modifier *=2;
        ret += SPELL_CRIT_SUCCESS + SPELL_CSUC_DOUBLE;
        break;
      case CRIT_S_NONE:
        if (victim->isLucky(caster->spellLuckModifier(SPELL_FAERIE_FIRE))) {
          SV(SPELL_FAERIE_FIRE);
          aff.duration /= 2;
          aff.modifier /= 2;
          ret += SPELL_SAVE;
        }
        break;
    }
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    // this spell is non-violent, cause it to piss off mobs though
    if (!victim->isPc()) {
      TMonster *tmons = dynamic_cast<TMonster *>(victim);
      tmons->UM(4);
      tmons->US(5);
      tmons->UA(7);
      tmons->aiTarget(caster);
    }
    return ret;
  } else {
    ret += SPELL_FAIL;
    switch (critFail(caster, SPELL_FAERIE_FIRE)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_FAERIE_FIRE);
        caster->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
        ret += SPELL_CFAIL_DEFAULT + SPELL_CRIT_FAIL;
        break;
      case CRIT_F_NONE:
        break;
    }
    return ret;
  }
}

void faerieFire(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;

  if (victim->affectedBySpell(SPELL_FAERIE_FIRE)) {
    act("You sense that $N is already affected by the spell!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to cast something on you!",
        0, caster, NULL, victim, TO_VICT);
    caster->nothingHappens(SILENT_YES);
    if (!victim->isPc()) {
      dynamic_cast<TMonster *>(victim)->addHated(caster);
    }
    return;
  }

  ret = faerieFire(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());


//  if (IS_SET(ret, SPELL_SUCCESS) && IS_SET(ret, SPELL_CRIT_SUCCESS)) {
//  } else if (IS_SET(ret, SPELL_SUCCESS)) {
//  }

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("A faint pink outline puffs out around $N!",
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("A faint pink outline puffs out around $N!",
        TRUE, caster, NULL, victim, TO_CHAR);
    act("A faint pink outline puffs out around you!",
        FALSE, caster, NULL, victim, TO_VICT);
    }
  if (IS_SET(ret, SPELL_SAVE)) {
    act("The outline appears to waver and grows a little fainter.",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("The outline appears to waver and grows a little fainter.",
        FALSE, caster, NULL, victim, TO_ROOM);
  } else {
  }

  if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (IS_SET(ret, SPELL_CSUC_DEFAULT)) {
    }
    if (IS_SET(ret, SPELL_CSUC_DOUBLE)) {
      act("The outline appears stronger than usual.",
          FALSE, caster, NULL, victim, TO_CHAR);
      act("The outline appears stronger than usual.",
          FALSE, caster, NULL, victim, TO_ROOM);
    }
    if (IS_SET(ret, SPELL_CSUC_TRIPLE)) {
    }
  }
//    if (IS_SET(ret, SPELL_SAVE)) {
//    }

  if (IS_SET(ret, SPELL_FAIL) && !IS_SET(ret, SPELL_CRIT_FAIL)) {
    caster->nothingHappens();
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL) || IS_SET(ret, SPELL_CFAIL_DEFAULT)) {
    act("A faint pink outline puffs out around $n!",
        TRUE, caster, NULL, victim, TO_ROOM);
    act("A faint pink outline puffs out around you!",
        TRUE, caster, NULL, victim, TO_CHAR);
  } else {
    if (IS_SET(ret, SPELL_CFAIL_SELF)) {
    }
    if (IS_SET(ret, SPELL_CFAIL_OTHER)) {
    }
  }

  if (IS_SET(ret, SPELL_FAIL_SAVE)) {
  }
  if (IS_SET(ret, SPELL_ACTION)) {
  }
  if (IS_SET(ret, SPELL_FALSE)) {
  }
  if (!victim->isPc()) {
    dynamic_cast<TMonster *>(victim)->addHated(caster);
  }
}

void faerieFire(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;
// 1. First check for unusual fails
  if (victim->affectedBySpell(SPELL_FAERIE_FIRE)) {
    act("You sense that $N is already affected by the spell!",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("$n just tried to cast something on you!",
        0, caster, NULL, victim, TO_VICT);
    caster->nothingHappens(SILENT_YES);
    return;
  }

// 2. Second send to general mage check function
  if (!bPassMageChecks(caster, SPELL_FAERIE_FIRE, victim))
    return;

// 4.   Get lag and difficulty for adding to casting object
  lag_t rounds = discArray[SPELL_FAERIE_FIRE]->lag;
  diff = discArray[SPELL_FAERIE_FIRE]->task;

// 5.   Initialize the casting object with data
  start_cast(caster, victim, NULL, caster->roomp, SPELL_FAERIE_FIRE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

// 6.   Start them fighting with 0 damage
// this spell is non-violent, it piss off mobs if it hits 

}

int castFaerieFire(TBeing *caster, TBeing *victim)
{
int ret,level;

  level = caster->getSkillLevel(SPELL_FAERIE_FIRE);

  ret=faerieFire(caster,victim,level,caster->getSkillValue(SPELL_FAERIE_FIRE));
//  if (IS_SET(ret, SPELL_SUCCESS) && IS_SET(ret, SPELL_CRIT_SUCCESS)) {
//  } else if (IS_SET(ret, SPELL_SUCCESS)) {
//  }

  if (!IS_SET(ret, SPELL_FALSE)) {
    act("You point at $N.", TRUE, caster, NULL, victim, TO_CHAR);
    act("$n points at $N.", TRUE, caster, NULL, victim, TO_NOTVICT);
    act("$n points at you.", TRUE, caster, NULL, victim, TO_VICT);
  }

  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("A faint pink outline puffs out around $N!", 
        TRUE, caster, NULL, victim, TO_NOTVICT);
    act("A faint pink outline puffs out around $N!", 
        TRUE, caster, NULL, victim, TO_CHAR);
    act("A faint pink outline puffs out around you!", 
        FALSE, caster, NULL, victim, TO_VICT);
  }
  if (IS_SET(ret, SPELL_SAVE)) {
    act("The outline appears to waver and grows a little fainter.",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("The outline appears to waver and grows a little fainter.",
        FALSE, caster, NULL, victim, TO_ROOM);
  } else {
  }
  if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (IS_SET(ret, SPELL_CSUC_DEFAULT)) {
    }
    if (IS_SET(ret, SPELL_CSUC_DOUBLE)) {
    act("The outline appears stronger than usual.",
        FALSE, caster, NULL, victim, TO_CHAR);
    act("The outline appears stronger than usual.",
        FALSE, caster, NULL, victim, TO_ROOM);
    }
    if (IS_SET(ret, SPELL_CSUC_TRIPLE)) {
    }
  }

//  if (IS_SET(ret, SPELL_SAVE)) {
//  }

  if (IS_SET(ret, SPELL_FAIL) && !IS_SET(ret, SPELL_CRIT_FAIL)) {
    caster->nothingHappens();
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL) || IS_SET(ret, SPELL_CFAIL_DEFAULT)) {
    act("A faint pink outline puffs out around $n!", 
        TRUE, caster, NULL, victim, TO_ROOM);
    act("A faint pink outline puffs out around you!", 
        TRUE, caster, NULL, victim, TO_CHAR);
  } else {

    if (IS_SET(ret, SPELL_CFAIL_SELF)) {
    }
    if (IS_SET(ret, SPELL_CFAIL_OTHER)) {
    }
  }
  if (IS_SET(ret, SPELL_FAIL_SAVE)) {
  }
  if (IS_SET(ret, SPELL_ACTION)) {
  }
  if (IS_SET(ret, SPELL_FALSE)) {
  }
// spell doesnt cause damage so return false
  return FALSE;

}

int flamingSword(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  int ret = 0;

  level = min(level, 25);

  int dam = caster->getSkillDam(victim, SPELL_FLAMING_SWORD, level, adv_learn);

  caster->reconcileHurt(victim,discArray[SPELL_FLAMING_SWORD]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_FLAMING_SWORD)) {
    switch(critSuccess(caster, SPELL_FLAMING_SWORD)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_FLAMING_SWORD);
        dam *= 2;
        act("You create an <R>intense flaming sword<z> and slash $N with it!",
            FALSE, caster, NULL,  victim, TO_CHAR);
        act("$n creates an <R>intense flaming sword<z> and slashes $N with it.",
            FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$n creates an <R>intense flaming sword<z> and slashes you with it.",
            FALSE, caster, NULL, victim, TO_VICT);
        ret = SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        act("You create a flaming sword and slash $N with it!",
            FALSE, caster, NULL,  victim, TO_CHAR);
        act("$n creates a flaming sword and slashes $N with it.",
            FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$n creates a flaming sword and slashes you with it.",
            FALSE, caster, NULL, victim, TO_VICT);
        ret = SPELL_SUCCESS;
        break;
    } 

    if (victim->isLucky(caster->spellLuckModifier(SPELL_FLAMING_SWORD))) {
      SV(SPELL_FLAMING_SWORD);
      ret |= SPELL_SAVE;
      dam /= 2;
      act("$N manages to avoid direct impact from the magic flaming sword!",
          FALSE, caster, NULL,  victim, TO_CHAR);
      act("$N manages to avoid direct impact from the magic flaming sword!",
          FALSE, caster, NULL, victim, TO_NOTVICT);
      act("You manage to avoid direct impact from the magic flaming sword.",
          FALSE, caster, NULL, victim, TO_VICT);
    }
    if (caster->reconcileDamage(victim,dam, SPELL_FLAMING_SWORD) == -1)
      ret |= VICTIM_DEAD;
    return ret;
  } else {
    switch (critFail(caster, SPELL_FLAMING_SWORD)) {
      case CRIT_F_HITOTHER:
      case CRIT_F_HITSELF:
        CF(SPELL_FLAMING_SWORD);
        act("You create a flaming sword and fumble it! You stab yourself in the foot!", FALSE, caster, NULL, victim, TO_CHAR);
        act("$n creates a flaming sword and fumbles it, stabbing $mself in the foot!", FALSE, caster, NULL, victim, TO_ROOM);

        if (caster->reconcileDamage(caster, dam, SPELL_FLAMING_SWORD) == -1)
          return SPELL_CRIT_FAIL | CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        act("You create a flaming sword but fail to slash $N with it!",
            FALSE, caster, NULL, victim, TO_CHAR);
        act("$n creates a flaming sword but fails to slash $N with it.",
            FALSE, caster, NULL, victim, TO_NOTVICT);
        act("$n creates a flaming sword and tried to slash you with it.",
            FALSE, caster, NULL, victim, TO_VICT);
        return SPELL_FAIL;
    }
    return SPELL_FAIL;
  }
}

int castFlamingSword(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_FLAMING_SWORD);

  ret=flamingSword(caster,victim,level,caster->getSkillValue(SPELL_FLAMING_SWORD), caster->getAdvLearning(SPELL_FLAMING_SWORD));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } 
  if (IS_SET(ret, SPELL_SAVE)) {
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
  } else {
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int flamingSword(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

// 1. First check for unusual fails

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }


// 2. Second send to general mage check function
  if (!bPassMageChecks(caster, SPELL_FLAMING_SWORD, victim))
    return FALSE;

// 4.   Get lag and difficulty for adding to casting object
  lag_t rounds = discArray[SPELL_FLAMING_SWORD]->lag; 
  diff = discArray[SPELL_FLAMING_SWORD]->task;

// 5.   Initialize the casting object with data
  start_cast(caster, victim, NULL, caster->roomp, SPELL_FLAMING_SWORD, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

// 6.   Start them fighting with 0 damage
// this spell is non-violent, it piss off mobs if it hits

  return FALSE;
}

int flamingSword(TBeing *caster, TBeing *victim, TMagicItem *obj)
{
  int ret = 0;
  int rc = 0;

  act("$p starts to glow and flames shoot out.",
      FALSE, caster, obj,  victim, TO_CHAR);
  act("$p starts to glow and flames shoot out.",
      FALSE, caster, obj,  victim, TO_ROOM);

  if (caster->roomp->isUnderwaterSector()) {
    act("The glow is quickly extinguished by the waters.",
      FALSE, caster, obj,  NULL, TO_CHAR);
    act("The glow is quickly extinguished by the waters.",
      FALSE, caster, obj,  NULL, TO_ROOM);
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  } else {
    ret=flamingSword(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  }
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_SAVE)) {
  } else {
    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    } else {
    }
  }
  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}



int inferno(TBeing *caster, TBeing *victim, int level, short bKnown, int adv_learn)
{
  int dam = caster->getSkillDam(victim, SPELL_INFERNO, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_INFERNO)) {
    caster->reconcileHurt(victim,discArray[SPELL_INFERNO]->alignMod);

    if (victim->isLucky(caster->spellLuckModifier(SPELL_INFERNO))) {
      SV(SPELL_INFERNO);
      dam /= 2;
    } else if (critSuccess(caster, SPELL_INFERNO) == CRIT_S_DOUBLE) {
      CS(SPELL_INFERNO);
      dam *= 2;
    }
    if (caster->reconcileDamage(victim, dam, SPELL_INFERNO) == -1)
      return SPELL_SUCCESS + VICTIM_DEAD;

    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_INFERNO) == CRIT_F_HITSELF) {
      CF(SPELL_INFERNO);
      dam = dam/2;
      dam = min(200,dam);
      if (caster->reconcileDamage(caster, dam, SPELL_INFERNO) == -1) 
        return SPELL_CRIT_FAIL + CASTER_DEAD;

      return SPELL_CRIT_FAIL;
    }
    return SPELL_FAIL;
  }
}

int inferno(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_INFERNO, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_INFERNO]->lag;
  diff = discArray[SPELL_INFERNO]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_INFERNO, diff, 1,
"", rounds, caster->in_room, 0, 0,TRUE, 0);

    return TRUE;
}

int castInferno(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;
  int rc2 = 0;

  level = caster->getSkillLevel(SPELL_INFERNO);

  ret=inferno(caster,victim,level,caster->getSkillValue(SPELL_INFERNO), caster->getAdvLearning(SPELL_INFERNO));
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("You cause $N to errupt in a pillar of fire!", 
                  FALSE, caster, NULL, victim, TO_CHAR);
    act("$n causes $N to errupt in a pillar of fire!", 
                  FALSE, caster, NULL, victim, TO_NOTVICT);
    act("$n causes you to errupt in a pillar of fire!", 
                  FALSE, caster, NULL, victim, TO_VICT);
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(rc2, DELETE_VICT);
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("Something goes terribly, terribly wrong!!", 
               FALSE, caster, 0, 0, TO_CHAR);
    act("You errupt in a pillar of fire!", 
               FALSE, caster, NULL, 0, TO_CHAR);
    act("$n causes $mself to errupt in a pillar of fire!", 
               FALSE, caster, NULL, 0, TO_ROOM);
    rc = caster->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(rc2, DELETE_THIS);
  } else {
    caster->nothingHappens();
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc2, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc2, DELETE_THIS);
  return rc2;
}

int inferno(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;
  int rc = 0;
  int rc2 = 0;

  if (caster->roomp->isUnderwaterSector()) { 
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }
 
  ret = inferno(caster, victim, obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, SPELL_SUCCESS)) {
    act("$p causes $n to errupt in a pillar of fire!", 
                  FALSE, victim, obj, NULL, TO_ROOM);
    act("$p causes you to errupt in a pillar of fire!", 
                  FALSE, victim, obj, NULL, TO_CHAR);
    rc = victim->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(rc2, DELETE_VICT);
  } else if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    act("Something goes terribly, terribly wrong!!", 
               FALSE, caster, 0, 0, TO_CHAR);
    act("You errupt in a pillar of fire!", 
               FALSE, caster, NULL, 0, TO_CHAR);
    act("$p causes $n to errupt in a pillar of fire!", 
               FALSE, caster, obj, 0, TO_ROOM);
    rc = caster->flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(rc2, DELETE_THIS);
  } else {
    caster->nothingHappens();
  }

  if (IS_SET(ret, VICTIM_DEAD))
    ADD_DELETE(rc2, DELETE_VICT);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc2, DELETE_THIS);
  return rc2;
}

int hellfire(TBeing *caster, int level, short bKnown, int adv_learn)
{
  int rc;
  int dam;
  TThing * t;
  TBeing *vict;

  level = min(level, 33);

  vict = NULL;
  dam = caster->getSkillDam(NULL, SPELL_HELLFIRE, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_HELLFIRE)) {

    caster->roomp->playsound(SOUND_SPELL_HELLFIRE, SOUND_TYPE_MAGIC);

    for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
      t=*(it++);
      vict = dynamic_cast<TBeing *>(t);
      if (!vict)
        continue;

      if (!caster->inGroup(*vict) && !vict->isImmortal()) {
        caster->reconcileHurt(vict, discArray[SPELL_HELLFIRE]->alignMod);
        act("$n is incinerated by the hellfire!", FALSE, vict, NULL, NULL, TO_ROOM);
        act("You are incinerated by the hellfire!", FALSE, vict, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(vict, dam, SPELL_HELLFIRE) == -1) {
          delete vict;
          vict = NULL;
          continue;
        }
        rc = vict->flameEngulfed();
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete vict;
          vict = NULL;
          continue;
        }
      }
    }
    act("$n loves the smell of brimstone in the morning!", FALSE, caster, NULL, NULL, TO_ROOM);
    act("You love the smell of brimstone in the morning!", FALSE, caster, NULL, NULL, TO_CHAR);
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_HELLFIRE)) {
      CF(SPELL_HELLFIRE);
      act("$n needs to play spin the bottle more, $e's pointing the wrong way!", FALSE, caster, NULL, NULL, TO_ROOM);
      act("You need to break out a compass, you're pointing the wrong way!", FALSE, caster, NULL, NULL, TO_CHAR);
      act("You hate the smell of brimstone in the morning!", FALSE, caster, NULL, NULL, TO_CHAR);
      for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
        t=*(it++);
        vict = dynamic_cast<TBeing *>(t);
        if (!vict)
          continue;

        if (caster->inGroup(*vict) && caster != vict && !vict->isImmortal()) {
          caster->reconcileHurt(vict, discArray[SPELL_HELLFIRE]->alignMod);
          act("$n is incinerated by the hellfire!", FALSE, vict, NULL, NULL, TO_ROOM);
          act("You are incinerated by the hellfire!", FALSE, vict, NULL, NULL, TO_CHAR);
          if (caster->reconcileDamage(vict, dam, SPELL_HELLFIRE) == -1) {
            delete vict;
            vict = NULL;
            continue;
          }
          rc = vict->flameEngulfed();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete vict;
            vict = NULL;
            continue;
          }
        }
      }
      act("$n hates the smell of brimstone in the morning!", FALSE, caster, NULL, NULL, TO_ROOM);
      return SPELL_CRIT_FAIL;
    } else {
    }
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int hellfire(TBeing *caster, TMagicItem * obj)
{
  int ret,level;
  int rc = 0;

  level = obj->getMagicLevel();

  act("A spray of HELLFIRE eradicates everything in its path!", FALSE, caster, NULL, NULL, TO_ROOM);

  ret=hellfire(caster,level,obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int hellfire(TBeing *caster)
{
  if (caster->roomp->isUnderwaterSector()) { 
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }
 
  if (!bPassMageChecks(caster, SPELL_HELLFIRE, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_HELLFIRE]->lag;
  taskDiffT diff = discArray[SPELL_HELLFIRE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_HELLFIRE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}
int castHellfire(TBeing *caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_HELLFIRE);   

  act("A spray of HELLFIRE eradicates everything in its path!", FALSE, caster, NULL, NULL, TO_ROOM);

  ret=hellfire(caster,level,caster->getSkillValue(SPELL_HELLFIRE), caster->getAdvLearning(SPELL_HELLFIRE));
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int fireball(TBeing *caster, int level, short bKnown, int adv_learn)
{
  int rc;
  int ret = 0;
  TBeing *tmp_victim, *temp;
  TRoom *rp;

  rp = caster->roomp;
  if (rp && rp->isUnderwaterSector()) {
    caster->sendTo("The water completely dissolves your fireball!\n\r");
    return SPELL_FAIL;
  } 

  level = min(level, 15);

  int damage = caster->getSkillDam(NULL, SPELL_FIREBALL, level, adv_learn);

  if (caster->bSuccess(bKnown, SPELL_FIREBALL)) {
    switch (critSuccess(caster, SPELL_FIREBALL)) {
      case CRIT_S_KILL:
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
        CS(SPELL_FIREBALL);
        act("A giant ball of fire screams from your hands and explodes! KERBLAAAAMM!", FALSE, caster, NULL, NULL, TO_CHAR);
        act("A giant ball of fire screams from $n's hands and explodes! KERBLAAAAMM!", FALSE, caster, NULL, NULL, TO_ROOM);
        damage *= 3;
        damage /= 2;
        ret = SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        ret = SPELL_SUCCESS;
        act("A ball of fire jets from your hands and explodes!", FALSE, caster, NULL, NULL, TO_CHAR);
        act("A ball of fire jets from $n's hands and explodes!", FALSE, caster, NULL, NULL, TO_ROOM);
        break;
    }

    caster->flameRoom();
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next;
      if (caster->sameRoom(*tmp_victim) && (caster != tmp_victim)) {
        if (!caster->inGroup(*tmp_victim) && !tmp_victim->isImmortal()) {
          caster->reconcileHurt(tmp_victim, discArray[SPELL_FIREBALL]->alignMod);
          if (tmp_victim->isLucky(caster->spellLuckModifier(SPELL_FIREBALL))) {
            act("$N is able to dodge part of the explosion!", FALSE, caster, NULL, tmp_victim, TO_CHAR);
            act("$N is able to dodge part of the explosion!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
            act("You are able to dodge part of the explosion!", FALSE, caster, NULL, tmp_victim, TO_VICT);
            damage >>= 1;
          } else {
            act("$N had no hope of dodging the lashing flames!", FALSE, caster, NULL, tmp_victim, TO_CHAR);
            act("$N had no hope of dodging the lashing flames!", FALSE, caster, NULL, tmp_victim, TO_NOTVICT);
            act("You had no hope of dodging the lashing flames!", FALSE, caster, NULL, tmp_victim, TO_VICT);
          }
          if (caster->reconcileDamage(tmp_victim, damage, SPELL_FIREBALL) == -1) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
          rc = tmp_victim->flameEngulfed();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tmp_victim;
            tmp_victim = NULL;
            continue;
          }
        } else
          act("You are able to avoid the flames!", FALSE, caster, NULL, tmp_victim, TO_VICT);
      } else if ((caster != tmp_victim) && (tmp_victim->in_room != Room::NOWHERE) &&
                 (rp->getZoneNum() == tmp_victim->roomp->getZoneNum())) {
        if (tmp_victim->awake())
          tmp_victim->sendTo("You hear a loud explosion and feel a gust of hot air.\n\r");
      }
    }
    return ret;
  } else {
    switch (critFail(caster, SPELL_FIREBALL)) {
      case CRIT_F_HITSELF:
      case CRIT_F_HITOTHER:
        CF(SPELL_FIREBALL);
        act("A large ball of fire swells up and explodes in $n's face!", 
            FALSE, caster, NULL, NULL, TO_ROOM);
        act("A large ball of fire swells up and explodes in your face!", 
            FALSE, caster, NULL, NULL, TO_CHAR);
        if (caster->reconcileDamage(caster, damage, SPELL_FIREBALL) == -1) 
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        
        rc = caster->flameEngulfed();
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return SPELL_CRIT_FAIL + CASTER_DEAD;
        return SPELL_CRIT_FAIL;
      case CRIT_F_NONE:
        break;
    }
    return SPELL_FAIL;
  }
}

int fireball(TBeing *caster, TMagicItem * obj)
{
  int ret;
  int rc = 0;
 
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot obtain that magic under these wet conditions!\n\r");
    return FALSE;
  }

  ret = fireball(caster,obj->getMagicLevel(),obj->getMagicLearnedness(), 0);
  if (IS_SET(ret, SPELL_FAIL)) {
    caster->nothingHappens();
  } else {
  }

  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int fireball(TBeing *caster)
{
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_FIREBALL, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_FIREBALL]->lag;
  taskDiffT diff = discArray[SPELL_FIREBALL]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_FIREBALL, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);

  return TRUE;
}

int castFireball(TBeing *caster)
{
  int ret,level;
  int rc = 0;

  level = caster->getSkillLevel(SPELL_FIREBALL);

  ret=fireball(caster,level,caster->getSkillValue(SPELL_FIREBALL), caster->getAdvLearning(SPELL_FIREBALL));
  if (IS_SET(ret, SPELL_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
  } else if (IS_SET(ret, SPELL_FAIL)) {
    caster->nothingHappens();
  } else {
  }
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int flamingFlesh(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff1;

  if (victim->affectedBySpell(SPELL_STONE_SKIN)) {
    act("$N's skin is already defended by elementals of earth.",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  caster->reconcileHelp(victim,discArray[SPELL_FLAMING_FLESH]->alignMod);

  if (caster->bSuccess(bKnown, SPELL_FLAMING_FLESH)) {

    // ARMOR APPLY
    aff1.type = SPELL_FLAMING_FLESH;
    aff1.level = level;
    aff1.duration = aff1.level * UPDATES_PER_MUDHOUR;
    aff1.location = APPLY_ARMOR;
    aff1.modifier = -75;

    switch (critSuccess(caster, SPELL_FLAMING_FLESH)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_FLAMING_FLESH);
        aff1.duration = aff1.duration * 3 / 2;
        aff1.modifier = aff1.modifier * 3 / 2;
        break;
      case CRIT_S_NONE:
        break;
    } 

    if (caster != victim)
      aff1.modifier /= 5;

    victim->affectJoin(caster, &aff1, AVG_DUR_NO, AVG_EFF_YES);
    act("A flaming ring of fire surrounds $n's skin.",
            FALSE, victim, NULL, 0, TO_ROOM);
    act("A flaming ring of fire surrounds your skin.",
            FALSE, victim, NULL, 0, TO_CHAR);

    return SPELL_SUCCESS;
  } else {
    act("$n's skin sparks then fades back.",
            FALSE, victim, NULL, 0, TO_ROOM);
    act("Your skin sparks then fades back.",
            FALSE, victim, NULL, 0, TO_CHAR);
    return SPELL_FAIL;
  }
}

void flamingFlesh(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  if (victim->affectedBySpell(SPELL_STONE_SKIN)) {  
    act("$N's skin is already defended by elementals of earth.",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return;
  }
 
  act("$p calls upon the powers of the elementals of fire.",
            FALSE, caster, obj, 0, TO_ROOM);
  act("$p calls upon the powers of the elementals of fire.",
            FALSE, caster, obj, 0, TO_CHAR);

  flamingFlesh(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int flamingFlesh(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }
  if (victim->affectedBySpell(SPELL_STONE_SKIN)) {  
    act("$N's skin is already defended by elementals of earth.",
        FALSE, caster, NULL, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_FLAMING_FLESH, victim))
    return FALSE;

  lag_t rounds = discArray[SPELL_FLAMING_FLESH]->lag;
  diff = discArray[SPELL_FLAMING_FLESH]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_FLAMING_FLESH, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castFlamingFlesh(TBeing *caster, TBeing *victim)
{
  int ret,level;
  level = caster->getSkillLevel(SPELL_FLAMING_FLESH);

  act("$n calls upon the powers of the elementals of fire.",
            FALSE, caster, NULL, 0, TO_ROOM);
  act("You call upon the powers of the elementals of fire.",
            FALSE, caster, NULL, 0, TO_CHAR);
  ret=flamingFlesh(caster,victim,level,caster->getSkillValue(SPELL_FLAMING_FLESH));
  return TRUE;
}

int conjureElemFire(TBeing *caster, int level, short bKnown)
{
  affectedData aff;
  TMonster *victim;

  if (!(victim = read_mobile(Mob::FIRE_ELEMENTAL, VIRTUAL))) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return SPELL_FAIL;
  }

  victim->elementalFix(caster, SPELL_CONJURE_FIRE, 0);

  if (caster->bSuccess(bKnown, SPELL_CONJURE_FIRE)) {
    act("You summon the powers of the inferno!", TRUE, caster, NULL, NULL, TO_CHAR);
    act("$n summons the powers of the inferno!", TRUE, caster, NULL, NULL, TO_ROOM);

    /* charm them for a while */
    if (victim->master)
      victim->stopFollower(TRUE);

    aff.type = SPELL_CONJURE_FIRE;
    aff.level = level;
    aff.duration  = caster->followTime();
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_CHARM;
    victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);

    aff.type = AFFECT_THRALL;
    aff.be = static_cast<TThing *>((void *) mud_str_dup(caster->getName()));
    victim->affectTo(&aff);

    // Add the restrict XP affect, so that you cannot twink newbies with this skill
    // this affect effectively 'marks' the mob as yours
    restrict_xp(caster, victim, PERMANENT_DURATION);

    /* Add hp for higher levels - Russ */
    victim->setMaxHit(victim->hitLimit() + number(1, level));
    victim->setHit(victim->hitLimit());

    *caster->roomp += *victim;

    switch (critSuccess(caster, SPELL_CONJURE_FIRE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_CONJURE_FIRE);
        act("$N flexes $S overly strong muscles.", TRUE, caster, 0, victim, TO_ROOM);
        caster->sendTo("You have conjured an unusually strong elemental!\n\r");
        victim->setMaxHit((int) (victim->hitLimit() * 1.5));
        victim->setHit((int) (victim->hitLimit() * 1.5));
        break;
      case CRIT_S_NONE:
        break;
    }
    if (caster->tooManyFollowers(victim, FOL_CHARM)) {
      act("$N refuses to enter a group the size of yours!",
             TRUE, caster, NULL, victim, TO_CHAR);
      act("$N refuses to enter a group the size of $n's!",
             TRUE, caster, NULL, victim, TO_ROOM);
      act("You've created a monster; $N hates you!",
             FALSE, caster, NULL, victim, TO_CHAR);
      victim->affectFrom(SPELL_CONJURE_FIRE);
      victim->affectFrom(AFFECT_THRALL);
      return SPELL_FAIL;
    }
    caster->addFollower(victim);
    return SPELL_SUCCESS;
  } else {
    act("Hmmm...that didn't feel quite right.", FALSE, caster, NULL, NULL, TO_CHAR);
    *caster->roomp += *victim;
    victim->developHatred(caster);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    act("When your eyes recover, you see $N standing before you!", TRUE, caster, NULL, victim, TO_ROOM);
    act("You've created a monster; $N hates you!", FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
  }
}

int conjureElemFire(TBeing *caster)
{
  TThing *t=NULL;
  int found=0;
  TLight *tl;
  TObj *o;

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }
  if (real_mobile(Mob::FIRE_ELEMENTAL) < 0) {
    caster->sendTo("There are no elementals of that type available.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_CONJURE_FIRE, NULL))
    return FALSE;

  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it){
    if((tl=dynamic_cast<TLight *>(t)) &&
       tl->isLit()){
      tl->putLightOut();
      found=1;
      break;
    }
    if((o=dynamic_cast<TObj *>(t)) && o->isObjStat(ITEM_BURNING)){
      o->remBurning(caster);
      found=1;
      break;
    }
  }
  if(!found){
    caster->sendTo("There doesn't seem to be enough fire around to conjure a fire elemental.\n\r");
    return FALSE;
  }
  


  lag_t rounds = discArray[SPELL_CONJURE_FIRE]->lag;
  taskDiffT diff = discArray[SPELL_CONJURE_FIRE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_CONJURE_FIRE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
}

int castConjureElemFire(TBeing *caster)
{
  int ret,level; 
  level = caster->getSkillLevel(SPELL_CONJURE_FIRE);

  if ((ret=conjureElemFire(caster,level,caster->getSkillValue(SPELL_CONJURE_FIRE))) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

int flare(TBeing *caster, int level, short bKnown)
{
  TBeing *tmp_victim = NULL;
  TObj *o = NULL;
  TThing *t=NULL;

  if (caster->roomp && caster->roomp->isUnderwaterSector()) {
    caster->sendTo("The water completely dissolves your flare!\n\r");
    return SPELL_FAIL;
  }

  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
    o = dynamic_cast<TObj *>(t);
    if (!o)
      continue;

    if (o->objVnum() == Obj::GENERIC_FLARE) {
      // lots of flares can overflow room-light buffer
      caster->sendTo("A magical force prevents more than 1 flare in the sky.\n\r");
      return SPELL_FAIL;
    }
  }

  if (!(o = read_object(Obj::GENERIC_FLARE, VIRTUAL))) {
    caster->sendTo("No flares are available.\n\r");
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown,SPELL_FLARE)) {
    if (caster->outside()) {
      *caster->roomp += *o;
      act("$n illuminates the sky.", FALSE, o, 0, 0, TO_ROOM);
      o->obj_flags.decay_time = level;
      o->affected[0].location = APPLY_LIGHT;
      o->affected[0].modifier = (level + 1)/2;
      o->addToLight(o->affected[0].modifier);
      caster->roomp->addToLight(o->affected[0].modifier);
      return SPELL_SUCCESS;
    } else {
      delete o;
      act("The flare rebounds off the ceiling into the wall...", TRUE, caster, 0, 0, TO_CHAR);
      act("   ...off the wall into the $g...", TRUE, caster, 0, 0, TO_CHAR);
      act("          ...off the $g, straight past your ear, singeing your hair...", TRUE, caster, 0, 0, TO_CHAR);
      act("   ...into another wall...", TRUE, caster, 0, 0, TO_CHAR);
      act("        ...then back toward the ceiling where it explodes violently!", TRUE, caster, 0, 0, TO_CHAR);
      act("The flare rebounds off the ceiling into the wall...",TRUE,caster,0,0,TO_ROOM);
      act("   ...off the wall into the $g...",TRUE,caster,0,0,TO_ROOM);
      act("          ...off the $g, straight past your ear, singeing your hair.",TRUE,caster,0,0,TO_ROOM);
      act("   ...into another wall...",TRUE,caster,0,0,TO_ROOM);
      act("        ...then back toward the ceiling where it explodes violently!",TRUE,caster,0,0,TO_ROOM);
      act("                ----<<<<<****  KA BOOM!!!  ****>>>>>----",TRUE,caster,0,0,TO_ROOM, ANSI_RED);
      act("                ----<<<<<****  KA BOOM!!!  ****>>>>>----",TRUE,caster,0,0,TO_CHAR, ANSI_RED);

      for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
        t=*(it++);
        tmp_victim = dynamic_cast<TBeing *>(t);
        if (!tmp_victim)
          continue;
        if (caster == tmp_victim)
          continue;
        if (!(tmp_victim->isImmortal() && tmp_victim->isPc())) { 
          int dam = dice(level, 2);
          dam = max(1,dam);
          caster->reconcileHurt(tmp_victim, discArray[SPELL_FLARE]->alignMod);
          if (tmp_victim->isLucky(caster->spellLuckModifier(SPELL_FLARE))) {
            act("$n is able to dodge part of the explosion!", FALSE, 
                 tmp_victim, NULL, NULL, TO_ROOM);
            act("You are able to dodge part of the explosion!", FALSE, 
                 tmp_victim, NULL, NULL, TO_CHAR);
              dam >>= 1;
          } else {
            act("You had no hope of dodging the lashing flames!", FALSE, 
                tmp_victim, NULL, NULL, TO_CHAR);
            act("$n had no hope of dodging the lashing flames!", FALSE, 
                tmp_victim, NULL, NULL, TO_ROOM);
          }
          // against PCs, consider safe, against mobs, consider aggro
          if (tmp_victim->isPc()) {
            if (tmp_victim->reconcileDamage(tmp_victim, dam, SPELL_FLARE) == -1) {
              delete tmp_victim;
              tmp_victim = NULL;
            }
          } else {
            if (caster->reconcileDamage(tmp_victim, dam, SPELL_FLARE) == -1) {
              delete tmp_victim;
              tmp_victim = NULL;
            }
          }
        } else
          act("You are able to avoid the flames!", FALSE, tmp_victim, NULL, 0, TO_CHAR);
      }
      int dam = dice(level, 2);
      dam = max(1,dam);
      if (caster->isLucky(caster->spellLuckModifier(SPELL_FLARE))) {
        act("$n is able to dodge part of the explosion!", FALSE,
             caster, NULL, NULL, TO_ROOM);
        act("You are able to dodge part of the explosion!", FALSE,
             caster, NULL, NULL, TO_CHAR);
        dam >>= 1;
      } else {
        act("You had no hope of dodging the lashing flames!", FALSE,
                caster, NULL, NULL, TO_CHAR);
        act("$n had no hope of dodging the lashing flames!", FALSE,
            caster, NULL, NULL, TO_ROOM);
      }
      if (caster->reconcileDamage(caster, dam, SPELL_FIREBALL) == -1) {
        return SPELL_SUCCESS + CASTER_DEAD;
      }      
    }
    return SPELL_SUCCESS;
  } else {
    *caster->roomp += *o;
    act("$n fizzles, looks like a dud.", FALSE, o, 0, 0, TO_ROOM);
    o->obj_flags.decay_time = 1;
    return SPELL_FAIL;
  }
}

int flare(TBeing *caster)
{
  taskDiffT diff;
  TBeing *tmp_victim = NULL;
  TObj *o = NULL; 
  TThing *t=NULL;

  // look to see if there is already a flare here
  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
    o = dynamic_cast<TObj *>(t);
    if (!o)
      continue;
    if (o->objVnum() == Obj::GENERIC_FLARE) {
      // lots of flares can overflow room-light buffer
      caster->sendTo("A magical force prevents more than 1 flare in the sky.\n\r");
      return FALSE;
    }
  }

  if (!real_object(Obj::GENERIC_FLARE)) {
    caster->sendTo("No flares are available.\n\r");
    return FALSE;
  }

  if (!bPassMageChecks(caster, SPELL_FLARE, NULL))
    return FALSE;

  if (caster->checkPeaceful("Flares are not needed in such places of tranquility.\n\r"))
    return FALSE;

  lag_t rounds = discArray[SPELL_FLARE]->lag;
  diff = discArray[SPELL_FLARE]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_FLARE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return FALSE;

  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end();){
    t=*(it++);
    tmp_victim = dynamic_cast<TBeing *>(t);
    if (!tmp_victim)
      continue;
    if (caster == tmp_victim)
      continue;
    if (!(tmp_victim->isImmortal() && tmp_victim->isPc())) {
    }          
    caster->reconcileHurt(tmp_victim, discArray[SPELL_FLARE]->alignMod);
  } 
  return TRUE;
}

int castFlare(TBeing *caster)
{
  int ret, level;
  int rc = 0;
  level = caster->getSkillLevel(SPELL_FLARE);

  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return FALSE;
  }

  act("You fire a magical flare skyward.", TRUE, caster, 0, 0, TO_CHAR);
  act("A brilliant flare leaps upward from $n's hands.", FALSE, caster, 0, 0, TO_ROOM);

  ret = flare(caster,level,caster->getSkillValue(SPELL_FLARE));
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int flare(TBeing *caster, TMagicItem * obj)
{
  TThing *t=NULL;
  TObj *o;

  if (caster->roomp->isUnderwaterSector()) {
    act("$p is too wet to use at the moment.", false, caster, obj, 0, TO_CHAR);
    return FALSE;
  }
  // look to see if there is already a flare here
  for(StuffIter it=caster->roomp->stuff.begin();it!=caster->roomp->stuff.end() && (t=*it);++it) {
    o = dynamic_cast<TObj *>(t);
    if (!o)
      continue;
    if (o->objVnum() == Obj::GENERIC_FLARE) {
      // lots of flares can overflow room-light buffer
      caster->sendTo("A magical force prevents more than 1 flare in the sky.\n\r");
      return FALSE;
    }
  }
  if (!real_object(Obj::GENERIC_FLARE)) {
    caster->sendTo("No flares are available.\n\r");
    return FALSE;
  }
  if (caster->checkPeaceful("Flares are not needed in such places of tranquility.\n\r"))
    return FALSE;


  int rc = 0;
  int ret;

  act("$p fires a magical flare skyward.", TRUE, caster, obj, 0, TO_CHAR);
  act("A brilliant flare leaps upward from $p.", FALSE, caster, obj, 0, TO_ROOM);

  ret = flare(caster,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, CASTER_DEAD))
    ADD_DELETE(rc, DELETE_THIS);
  return rc;
}

int infravision(TBeing *caster, TBeing *victim, int level, short bKnown)
{
  affectedData aff;
  int ret = 0;

  if (victim->isAffected(AFF_BLIND)) {
    act("Infravision can't work on the blind.", FALSE, caster, 0, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }
  if (victim->isAffected(AFF_INFRAVISION) &&
      !victim->affectedBySpell(SPELL_INFRAVISION)) {
    // having natural infra and spell-created infra causes natural infra to be
    // lost on spell decay
    if (victim != caster)
      act("$N already possesses infravision.", 
           FALSE, caster, 0, victim, TO_CHAR);
    else
      act("You already possess infravision.", 
           FALSE, caster, 0, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return SPELL_FALSE;
  }
  if (caster->bSuccess(bKnown, SPELL_INFRAVISION)) {
    ret = SPELL_SUCCESS;
    aff.type = SPELL_INFRAVISION;
    aff.duration = (level * UPDATES_PER_MUDHOUR)+level;
    aff.modifier = 0;
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_INFRAVISION;

    switch (critSuccess(caster, SPELL_INFRAVISION)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_INFRAVISION);
        aff.duration *= 2;
        ret += SPELL_CRIT_SUCCESS;
        break;
      case CRIT_S_NONE:
        break;
    }

    if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
      return SPELL_FAIL;
    }
    return ret;
  } else {
    ret += SPELL_FAIL;
    return ret; 
  }
}

void infravision(TBeing *caster, TBeing *victim, TMagicItem * obj)
{
  int ret;

  ret = infravision(caster,victim,obj->getMagicLevel(),obj->getMagicLearnedness());
  if (IS_SET(ret, SPELL_SUCCESS) && !IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster != victim) {
      act("Your $p causes $N's eyes to glow red.", 
          FALSE, caster, obj, victim, TO_CHAR);
      act("$n's $p cause your eyes to glow red.", 
          TRUE, caster, obj, victim, TO_VICT);
      act("$n uses $p to make $N's eyes glow red.", 
          TRUE, caster, obj, victim, TO_NOTVICT);
    } else {
      act("$p causes your eyes to glow red.", 
          FALSE, caster, obj, 0, TO_CHAR);
      act("$n's uses $p and $s eyes glow red.", 
          TRUE, caster, obj, 0, TO_ROOM);
    }
    act("Your eyesight feels enhanced.", FALSE, victim, obj, 0, TO_CHAR);
  }
  if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
    if (caster != victim) {
      act("Your $p causes $N's eyes to glow a deep red.", 
          FALSE, caster, obj, victim, TO_CHAR);
      act("$n's $p cause your eyes to glow a deep red.", 
          TRUE, caster, obj, victim, TO_VICT);
      act("$n uses $p to make $N's eyes glow a deep red.", 
          TRUE, caster, obj, victim, TO_NOTVICT);
    } else {
      act("$p causes your eyes to glow a deep red.", 
          FALSE, caster, obj, 0, TO_CHAR);
      act("$n's uses $p and $e eyes glow a deep red.", 
          TRUE, caster, obj, 0, TO_ROOM);
    }
    act("Your eyesight feels greatly enhanced.", 
        FALSE, victim, obj, 0, TO_CHAR);
  }
  if (IS_SET(ret, SPELL_SAVE)) {
  }
  if (IS_SET(ret, SPELL_FAIL)) {
    if (caster != victim) {
      act("$p fails to enhance $N's eyesight.", 
          FALSE, caster, obj, victim, TO_CHAR);
    } else {
      act("$p fails to enhance your eyesight.", 
          FALSE, caster, obj, 0, TO_CHAR);
    }
    caster->nothingHappens(SILENT_YES);
  }
  if (IS_SET(ret, SPELL_CRIT_FAIL)) {
  }
  if (IS_SET(ret, SPELL_FALSE)) {
  }
}
// RIX - get rid of the void function
void infravision(TBeing *caster, TBeing *victim)
{
  taskDiffT diff;
// COMMENTED OUT FOR DURATIONS
//  if (victim->isAffected(AFF_INFRAVISION) &&
//      !victim->affectedBySpell(SPELL_INFRAVISION)) {
//    act("$N's eyes already glow red.", FALSE, caster, 0, victim, TO_CHAR);
//    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
//    return;
//  }
  if (victim->isAffected(AFF_BLIND)) {
    act("Infravision can't work on the blind.", FALSE, caster, 0, victim, TO_CHAR);
    caster->nothingHappens(SILENT_YES);
    return;
  }
  if (caster->roomp->isUnderwaterSector()) {
    caster->sendTo("You cannot cast that under these wet conditions!\n\r");
    return;
  }

  if (!bPassMageChecks(caster, SPELL_INFRAVISION, victim))
    return;

  lag_t rounds = discArray[SPELL_INFRAVISION]->lag;
  diff = discArray[SPELL_INFRAVISION]->task;

  start_cast(caster, victim, NULL, caster->roomp, SPELL_INFRAVISION, diff, 1, 
        "", rounds, caster->in_room, 0, 0, TRUE, 0);

  return;
}

int castInfravision(TBeing *caster, TBeing *victim)
{
  int ret,level;
  int rc = 0;

  if (!caster || !victim)
    return TRUE;

  level = caster->getSkillLevel(SPELL_INFRAVISION);

  ret=infravision(caster,victim,level,caster->getSkillValue(SPELL_INFRAVISION));
    if (IS_SET(ret, SPELL_SUCCESS) && !IS_SET(ret, SPELL_CRIT_SUCCESS)) {
      if (caster != victim) {
        act("You cause $N's eyes to glow red.", 
            FALSE, caster, 0, victim, TO_CHAR);
        act("$n's spell causes your eyes to glow red.", 
            TRUE, caster, 0, victim, TO_VICT);
        act("$n's spell causes $N's eyes to glow red.", 
            TRUE, caster, 0, victim, TO_NOTVICT);
      } else {
        act("Your spell causes your eyes to glow red.", 
            FALSE, caster, 0, 0, TO_CHAR);
        act("$n's spell causes $s eyes to glow red.", 
            TRUE, caster, 0, 0, TO_ROOM);
      }
      act("Your eyesight feels enhanced.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if (IS_SET(ret, SPELL_CRIT_SUCCESS)) {
      if (caster != victim) {
        act("You cause $N's eyes to glow a deep red.", 
            FALSE, caster, 0, victim, TO_CHAR);
        act("$n's spell causes your eyes to glow a deep red.", 
            TRUE, caster, 0, victim, TO_VICT);
        act("$n's spell causes $N's eyes to glow a deep red.", 
            TRUE, caster, 0, victim, TO_NOTVICT);
      } else {
        act("Your spell causes your eyes to glow a deep red.", 
            FALSE, caster, 0, 0, TO_CHAR);
        act("$n's spell causes $s eyes to glow a deep red.", 
            TRUE, caster, 0, 0, TO_ROOM);
      }
      act("Your eyesight feels greatly enhanced.", FALSE, victim, 0, 0, TO_CHAR);
    }
    if (IS_SET(ret, SPELL_FAIL)) {
      if (caster != victim) {
        act("Your spell fails to enhance $N's eyesight.", 
            FALSE, caster, 0, victim, TO_CHAR);
      } else {
        act("Your attempt to enhance your eyesight fails.", 
            FALSE, caster, 0, 0, TO_CHAR);
      }
      caster->nothingHappens(SILENT_YES);
    }
    if (IS_SET(ret, SPELL_CRIT_FAIL)) {
    }
    if (IS_SET(ret, SPELL_FALSE)) {
    }
    if (IS_SET(ret, CASTER_DEAD))
      ADD_DELETE(rc, DELETE_THIS);
    return rc;
}

int protectionFromFire(TBeing *caster, TBeing *v,int level, short bKnown)
{
  affectedData aff;
 
  aff.type = SPELL_PROTECTION_FROM_FIRE;
  aff.level = level;
  aff.duration = (3 + (level / 2)) * UPDATES_PER_MUDHOUR;
  aff.location = APPLY_IMMUNITY;
  aff.modifier = IMMUNE_HEAT; 
  aff.modifier2 = ((level * 2)/3);
  aff.bitvector = 0;
 
  if (caster->bSuccess(bKnown,SPELL_PROTECTION_FROM_FIRE)) {
    act("$n glows with a faint red aura for a brief moment.", FALSE, v, NULL, NULL, TO_ROOM);
    act("You glow with a faint red aura for a brief moment.", FALSE, v, NULL, NULL, TO_CHAR);
    switch (critSuccess(caster, SPELL_PROTECTION_FROM_FIRE)) {
      case CRIT_S_DOUBLE:
      case CRIT_S_TRIPLE:
      case CRIT_S_KILL:
        CS(SPELL_PROTECTION_FROM_FIRE);
        aff.duration = (10 + (level / 2)) * UPDATES_PER_MUDHOUR;
        aff.modifier2 = (level * 2);
        break;
      case CRIT_S_NONE:
        break;
    }
 
    if (caster != v) 
      aff.modifier2 /= 2;
 
    v->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
    caster->reconcileHelp(v, discArray[SPELL_PROTECTION_FROM_FIRE]->alignMod);
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

void protectionFromFire(TBeing *caster, TBeing *v, TMagicItem * obj)
{
  protectionFromFire(caster,v,obj->getMagicLevel(),obj->getMagicLearnedness());
}

int protectionFromFire(TBeing *caster, TBeing *v)
{
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_PROTECTION_FROM_FIRE, v))
    return FALSE;

  lag_t rounds = discArray[SPELL_PROTECTION_FROM_FIRE]->lag;
  diff = discArray[SPELL_PROTECTION_FROM_FIRE]->task;

  start_cast(caster, v, NULL, caster->roomp, SPELL_PROTECTION_FROM_FIRE, diff, 1, "", rounds, caster->in_room, 0, 0,TRUE, 0);
  return TRUE;
} 

int castProtectionFromFire(TBeing *caster, TBeing *v)
{
int ret,level;
 
  level = caster->getSkillLevel(SPELL_PROTECTION_FROM_FIRE);
 
  if ((ret=protectionFromFire(caster,v,level,caster->getSkillValue(SPELL_PROTECTION_FROM_FIRE))) == SPELL_SUCCESS) {
  } else {
  }
  return TRUE;
}

