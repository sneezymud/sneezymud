//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#include <cstdio>

#include "extern.h"
#include "handler.h"
#include "being.h"
#include "monster.h"
#include "range.h"
#include "disease.h"
#include "combat.h"
#include "disc_dueling.h"
#include "obj_base_weapon.h"

int TBeing::doShove(const char* argument, TBeing* vict) {
  int rc;
  char name_buf[256], obje[100];
  TBeing* victim;

  spellNumT skill = getSkillNum(SKILL_SHOVE);

  half_chop(argument, name_buf, obje);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Shove whom?\n\r");
        return false;
      }
    }
  }
  if (victim == this) {
    sendTo("Aren't we funny today...\n\r");
    return false;
  }
  if (checkPeaceful("You can't shove from this room!\n\r"))
    return false;

  if (victim->isImmortal()) {
    sendTo("Oh no you don't!\n\r");
    return false;
  }
  if (noHarmCheck(victim))
    return false;

  if (riding) {
    act("You can't shove very well while on $p.", false, this, riding, 0,
      TO_CHAR);
    return false;
  }
  if (victim->riding) {
    // compare pusher's str to rider's dex
    if ((!victim->rideCheck(
          victim->plotStat(STAT_CURRENT, STAT_AGI, 3, 18, 12) -
          plotStat(STAT_CURRENT, STAT_STR, 3, 18, 10))) &&
        !isImmortal()) {
      act("You leap at $N, attempting to topple $M from $S $o, but fail.", true,
        this, victim->riding, victim, TO_CHAR);
      act("$n leaps at you, attempting to topple you from your $o, but fails.",
        true, this, victim->riding, victim, TO_VICT);
      act("$n leaps at $N, attempting to topple $M off $S $o, but fails.", true,
        this, victim->riding, victim, TO_NOTVICT);

      rc = crashLanding(POSITION_RESTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

      act("You land face-down on the $g.", false, this, 0, 0, TO_CHAR,
        ANSI_RED);
      addSkillLag(skill, rc);
      reconcileDamage(victim, 0, skill);

      if (!victim->isPc()) {
        TMonster* tmons = dynamic_cast<TMonster*>(victim);
        rc = tmons->takeFirstHit(*this);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          if (vict)
            return rc;
          delete tmons;
          tmons = nullptr;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
      return true;
    } else {
      act("You leap at $N, toppling $M from $S $o.", true, this, victim->riding,
        victim, TO_CHAR);
      act("$n leaps at you, toppling you from your $o.", true, this,
        victim->riding, victim, TO_VICT);
      act("$n leaps at $N, toppling $M off $S $o.", true, this, victim->riding,
        victim, TO_NOTVICT);
      rc = victim->fallOffMount(victim->riding, POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete victim;
        victim = nullptr;
        return true;
      }
      addSkillLag(skill, rc);
      reconcileDamage(victim, 0, skill);
      return true;
    }
  }
  if (!doesKnowSkill(skill)) {
    sendTo("You can't go pushing people around like that.\n\r");
    return false;
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }
  rc = shove(this, victim, obje, skill);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  if (rc)
    addSkillLag(skill, rc);
  return rc;
}

int shove(TBeing* caster, TBeing* victim, char* direction, spellNumT skill) {
  int percent, level;
  dirTypeT dr;
  int shove_move = ::number(5, 10);

  if (caster->getCombatMode() == ATTACK_BERSERK) {
    caster->sendTo(
      "You are berserking! You can't focus enough to shove anyone!\n\r");
    return false;
  }
  if (caster->fight()) {
    caster->sendTo("Not while fighting.\n\r");
    return false;
  }
  if (victim->fight()) {
    act("You can't shove $N while $E is fighting.", false, caster, 0, victim,
      TO_CHAR);
    return false;
  }
  if (caster->riding) {
    caster->sendTo("You'd need to dismount before doing that.\n\r");
    return false;
  }
  if (victim->rider) {
    caster->sendTo("Uhhh, I don't think you can shove the both of them...\n\r");
    return false;
  }
  if (caster->getMove() < shove_move) {
    act("You lack the vitality to shove $N.", false, caster, 0, victim,
      TO_CHAR);
    return false;
  }
  caster->addToMove(-shove_move);

  level = caster->getSkillLevel(skill);
  percent = 0;
  percent += caster->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;
  percent += (level - victim->GetMaxLevel());

  int bKnown = caster->getSkillValue(skill);
  dr = getDirFromChar(direction);
  if (dr == DIR_NONE) {
    caster->sendTo("You need to give a direction to shove.\n\r");
    return false;
  }
  if (caster->bSuccess(bKnown + percent, skill)) {
    if (victim->doesKnowSkill(SKILL_COUNTER_MOVE)) {
      if (min((int)victim->GetMaxLevel(), 100) > percent) {
        act("$N deftly resists your shove attempt.", false, caster, 0, victim,
          TO_CHAR);
        act("$N deftly resists $n's shove attempt.", false, caster, 0, victim,
          TO_NOTVICT);
        act("You deftly resist $n's attempt to shove you.", false, caster, 0,
          victim, TO_VICT);
        if (!victim->isPc())
          dynamic_cast<TMonster*>(victim)->aiShoveReact(caster, false, dr);
      } else
        caster->throwChar(victim, dr, false, SILENT_NO, false);

      if (!victim->isPc())
        dynamic_cast<TMonster*>(victim)->aiShoveReact(caster,
          (caster->exitDir(dr) ? true : false), dr);

    } else {
      caster->throwChar(victim, dr, false, SILENT_NO, false);
      if (!victim->isPc())
        dynamic_cast<TMonster*>(victim)->aiShoveReact(caster,
          (caster->exitDir(dr) ? true : false), dr);
    }
  } else {
    act("You try to shove $N to no avail!", true, caster, 0, victim, TO_CHAR);
    act("$n tries to shove $N but has no luck.", true, caster, 0, victim,
      TO_NOTVICT);
    caster->setCharFighting(victim);
    caster->setVictFighting(victim);
    caster->reconcileHurt(victim, 0.01);
    if (!victim->isPc())
      dynamic_cast<TMonster*>(victim)->aiShoveReact(caster, false, dr);
  }
  return true;
}

// DASH MARKER: this is my revamped copy of parryWarrior(), with the stuff for
// the trance of blades skill
#if 1

int TBeing::parryWarrior(TBeing* v, TThing* weapon, int* dam, int w_type,
  wearSlotT part_hit) {
  char buf[256], type[16], type2[16];
  bool trance = false;
  TObj* vweap = nullptr;

  // presumes warrior is in appropriate position for parry already

  if (!v->doesKnowSkill(SKILL_PARRY_WARRIOR))
    return false;
  if (v->doesKnowSkill(SKILL_TRANCE_OF_BLADES) && (v->task) &&
      (v->task->task == TASK_TRANCE_OF_BLADES) &&
      (vweap = dynamic_cast<TBaseWeapon*>(v->heldInPrimHand())))
    trance = true;

  w_type -= TYPE_HIT;

  // base amount, modified for difficulty
  // the higher amt is, the more things get blocked
  int amt = 0;
  if (trance) {
    // 50% base parry chance
    amt += 500;
    switch (::number(0, 2)) {
      case 0:
        strcpy(type, "parry");
        strcpy(type2, "parries");
        break;
      case 1:
        strcpy(type, "block");
        strcpy(type2, "blocks");
        break;
      case 2:
        strcpy(type, "deflect");
        strcpy(type2, "deflects");
        break;
    }
    // Give fast and agile trancers an improved chance
    // translates to rougly 66% max
    amt *= (getStatMod(STAT_SPE) * getStatMod(STAT_AGI));
  } else {
    // 4% base parry chance
    amt += 40;
    strcpy(type, "parry");
    strcpy(type2, "parries");
  }
  if (::number(0, 999) >= amt)
    return false;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (trance) {
    int val =
      (int)(((float)v->getSkillValue(SKILL_TRANCE_OF_BLADES) * 0.70) + 30);
    if (v->bSuccess(val, SKILL_TRANCE_OF_BLADES)) {
      *dam = 0;
      // base 30% chance combined with the base 50% chance gives us a 15%-50%
      // block rate not sure if this is the proper way to do this, but it works.
      // - dash
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "You %s $n's %s with your $o.", type,
          attack_hit_text_twink[w_type].singular);
      } else {
        sprintf(buf, "You %s $n's %s with your $o.", type,
          attack_hit_text[w_type].singular);
      }
      act(buf, false, this, vweap, v, TO_VICT, ANSI_CYAN);
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "$N %s your %s with $S $o.", type2,
          attack_hit_text_twink[w_type].singular);
      } else {
        sprintf(buf, "$N %s your %s with $S $o.", type2,
          attack_hit_text[w_type].singular);
      }
      act(buf, false, this, vweap, v, TO_CHAR, ANSI_CYAN);
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "$N %s $n's %s with $S $o.", type2,
          attack_hit_text_twink[w_type].singular);
      } else {
        sprintf(buf, "$N %s $n's %s with $S $o.", type2,
          attack_hit_text[w_type].singular);
      }
      act(buf, true, this, vweap, v, TO_NOTVICT);
      return true;
    }
  } else {
    if (v->bSuccess(SKILL_PARRY_WARRIOR)) {
      *dam = 0;
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "You %s $n's %s at your %s.", type,
          attack_hit_text_twink[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      } else {
        sprintf(buf, "You %s $n's %s at your %s.", type,
          attack_hit_text[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      }
      act(buf, false, this, 0, v, TO_VICT, ANSI_CYAN);
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "$N %s your %s at $S %s.", type2,
          attack_hit_text_twink[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      } else {
        sprintf(buf, "$N %s your %s at $S %s.", type2,
          attack_hit_text[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      }
      act(buf, false, this, 0, v, TO_CHAR, ANSI_CYAN);
      if (toggleInfo[TOG_TWINK]->toggle) {
        sprintf(buf, "$N %s $n's %s at $S %s.", type2,
          attack_hit_text_twink[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      } else {
        sprintf(buf, "$N %s $n's %s at $S %s.", type2,
          attack_hit_text[w_type].singular,
          v->describeBodySlot(part_hit).c_str());
      }
      act(buf, true, this, 0, v, TO_NOTVICT);
      return true;
    }
  }
  return false;
}

#else

int TBeing::parryWarrior(TBeing* v, TThing* weapon, int* dam, int w_type,
  wearSlotT part_hit) {
  char buf[256], type[16];

  // presumes warrior is in appropriate position for parry already

  if (!v->doesKnowSkill(SKILL_PARRY_WARRIOR))
    return false;

  w_type -= TYPE_HIT;

  // base amount, modified for difficulty
  // the higher amt is, the more things get blocked
  int amt = (int)(45 * 100 / getSkillDiffModifier(SKILL_PARRY_WARRIOR));

  if (::number(0, 999) >= amt)
    return false;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (bSuccess(v, SKILL_PARRY_WARRIOR)) {
    *dam = 0;

    strcpy(type, "parry");
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, false, this, 0, v, TO_VICT, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, false, this, 0, v, TO_CHAR, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, true, this, 0, v, TO_NOTVICT);

    return true;
  }
  return false;
}
#endif

int TBeing::doParry() {
  sendTo("Parry is not yet supported in this fashion.\n\r");
  return 0;
}

void TBeing::doTranceOfBlades(const char* newarg) {
  TThing* obj;

  if (!doesKnowSkill(SKILL_TRANCE_OF_BLADES)) {
    sendTo(
      "You do not have the knowledge to attempt the defensive trance.\n\r");
    return;
  }
  if (!(this->heldInPrimHand()) ||
      !(obj = dynamic_cast<TBaseWeapon*>(this->heldInPrimHand()))) {
    sendTo(
      "You'll need to be holding a suitable weapon to enter the defensive "
      "trance.\n\r");
    return;
  }

  if (!canUseArm(HAND_PRIMARY)) {
    sendTo("You can't enter the defensive trance with a useless arm.\n\r");
    return;
  }
  if (riding) {
    sendTo("You can't enter the defensive trance while mounted.\n\r");
    return;
  }
  if (getMove() < 30) {
    sendTo("You're too tired to enter the trance.\n\r");
    return;
  }
  if (getCombatMode() == ATTACK_BERSERK) {
    sendTo("You can't enter the defensive trance berserking. Like, duh.\n\r ");
    return;
  }
  if (checkPeaceful(
        "You cannot enter the defensive trance in a peaceful room.\n\r"))
    return;
  if (!(getPosition() > POSITION_SITTING)) {
    sendTo("You can not enter the defensive trance while sitting.\n\r");
    return;
  }
  act("You focus intensely upon your $o.", false, this, obj, nullptr, TO_CHAR);
  act("$n focuses intensely upon $s $o.", false, this, obj, nullptr, TO_ROOM);
  act(
    "Concentrating, you enter the trance, and you feel your defensive "
    "reactions quicken.",
    false, this, obj, nullptr, TO_CHAR);
  act("Concentrating, $n enters into a defensive trance.", false, this, obj,
    nullptr, TO_ROOM);

  start_task(this, (TObj*)obj, nullptr, TASK_TRANCE_OF_BLADES, nullptr, 0,
    (ushort)this->in_room, 0, 0, 0);
}
