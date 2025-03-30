#include <stdio.h>

#include "ansi.h"
#include "being.h"
#include "comm.h"
#include "extern.h"
#include "immunity.h"
#include "obj_base_weapon.h"
#include "spells.h"

int ColdHit(TBeing* ch, TBeing* vict, TObj* obj, int level) {
  TObj* icicle = read_object(31349, VIRTUAL);
  /*
  Damage is slightly higher than a standard proc and based on wielder level
  If damage is beyond a threshold, the proc gives an affect that reduces
  victim's immunity to cold. This is meant to be the basis of a compound tactic
  that capitalizes on a single high-risk/high-reward Cold damage spell-like
  attack Given the level dependence of the damage spread of the onHit proc, it
  will apply 11% of the time at level 10, and roughly 55% of the time at
  level 50. The second part of the onHit
  */
  int dam = ::number(2 + level / 10, 10 + level / 10);
  if (dam <= 11) {
    act("$p becomes covered with ice and freezes $n.", false, vict, obj, ch, TO_ROOM, ANSI_CYAN);
    act("$p becomes covered with ice and freezes you.", false, vict, obj, ch, TO_CHAR, ANSI_CYAN);
  } else {
    act("$p becomes covered with ice and sends a violent chill through $n.", false, vict, obj, ch, TO_ROOM, ANSI_BLUE);
    act("$p becomes covered with ice and sends a violent chill through you.", false, vict, obj, ch, TO_CHAR, ANSI_BLUE);

    affectedData aff;
    aff.type = SPELL_FROST_BREATH;
    aff.level = level;
    aff.duration = level / 2 * Pulse::UPDATES_PER_MUDHOUR;
    aff.modifier = -(level / 5);
    aff.location = APPLY_IMMUNITY;
    aff.bitvector = IMMUNE_COLD;
    vict->affectTo(&aff);

    if (dam >= 15) {
      wearSlotT limb = vict->getRandomPart(PART_MISSING);
      act(
        "<C>The air around $p becomes an<1> <W>arctic blizzard,<1> <C>which "
        "freezes $n to $s core!<1>",
        false, vict, obj, ch, TO_ROOM, ANSI_BLUE);
      act(
        "<C>The air around $p becomes an<1> <W>arctic blizzard,<1> <C>which "
        "freezes you to your core!<1>",
        false, vict, obj, ch, TO_CHAR, ANSI_BLUE);
      if (!vict->isImmune(IMMUNE_COLD, WEAR_BODY)) {
        vict->stickIn(icicle, limb);
        act(
          "A chunk of $N's blood is frozen solid, leaving behind a jagged "
          "<W>icicle<1>.",
          false, ch, obj, vict, TO_NOTVICT);
        act(
          "A chunk of your blood is frozen solid, leaving behind a jagged "
          "<W>icicle<1>.",
          false, ch, obj, vict, TO_VICT);
      }
    }
  }
  int rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
  if (rc == -1)
    return DELETE_VICT;
  return TRUE;}

int ColdShroud(TBeing* ch, TObj* obj, int level) {
  affectedData aff1;
  affectedData aff2;

  ch = dynamic_cast<TBeing*>(obj->equippedBy);
  // This applies a level based immunity to cold, which tops out at 35%
  aff1.type = SPELL_ICY_GRIP;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR * 3;
  aff1.location = APPLY_IMMUNITY;
  aff1.modifier = IMMUNE_COLD;
  aff1.modifier2 = ((level * 7) / 10);
  aff1.bitvector = IMMUNE_COLD;
  aff1.level = 40;

  ch->affectTo(&aff1);

  act("A frigid wind emerges from $p and envelopes $N.", false, ch, obj, ch, TO_ROOM, ANSI_BLUE);
  act("A frigid wind emerges from $p and envelopes you.", false, ch, obj, ch, TO_CHAR, ANSI_BLUE);
  
  if (ch->getImmunity(IMMUNE_COLD) >= 100) {
    // ARMOR APPLY
    // checks for the wielder's immunity to cold and if it is 100% it applies
    // a level based AC bonus
    aff2.type = SPELL_ICY_GRIP;
    aff2.level = 30;
    aff2.duration = 3 * Pulse::UPDATES_PER_MUDHOUR;
    aff2.location = APPLY_ARMOR;
    aff2.modifier = -(25 + (level / 2));
  
    ch->affectTo(&aff2);

    act("The air around $N coallesces into a layer of protective frost.", false, ch, obj, ch, TO_ROOM, ANSI_CYAN);
    act("The air around you coallesces into a layer of protective frost.", false, ch, obj, ch, TO_CHAR, ANSI_CYAN);
  }
    return true;
}

int coldBlast(TBeing* ch, TBeing* vict, TObj* obj, int level, int coldimmunity) {
  if (ch->checkObjUsed(obj)) {
    act("You cannot use $p's powers again this soon.", TRUE, ch, obj, NULL, TO_CHAR, NULL);
    return FALSE;
  }

  if (!(vict = ch->fight())) {
    act("You cannot use $p's powers unless you are fighting.", TRUE, ch, obj, NULL, TO_CHAR, NULL);
    return FALSE;
  }

  if (vict->isImmune(IMMUNE_COLD, WEAR_BODY)) {
    act("You cannot use $p's powers on someone who is immune to cold.", TRUE, ch, obj, NULL, TO_CHAR, NULL);
    return FALSE;
  }

  ch->addObjUsed(obj, 3 * Pulse::UPDATES_PER_MUDHOUR);
  int manaSpent = ch->getMana();
  ch->addToMana(-manaSpent);
  
  // Fix damage calculation
  int dam = manaSpent;  // Base damage is mana spent
  dam = dam * (1 + coldimmunity / 100);  // Increase damage based on cold immunity
  dam = dam * (level / 50);  // Scale with level
 
  act("A savage winter storm erupts from $p!",
      false, ch, obj, vict, TO_CHAR, ANSI_WHITE_BOLD);
  act("A savage winter storm erupts from $p!",
      false, ch, obj, vict, TO_ROOM, ANSI_WHITE_BOLD);

  act("Howling winds pummel into $N's body with intense fury!",
      false, ch, obj, vict, TO_ROOM, ANSI_BLUE_BOLD);
  act("Howling winds pummel into your body with intense fury!",
      false, ch, obj, vict, TO_VICT, ANSI_BLUE_BOLD);  
  act("Howling winds pummel into $N's body with intense fury!",
      false, ch, obj, vict, TO_CHAR, ANSI_BLUE_BOLD);
  
  int rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
  if (rc == -1)
    return DELETE_VICT;
    
  if (manaSpent >= 300) {
    affectedData aff;
    aff.type = SKILL_DOORBASH;
    aff.duration = Pulse::TICK * 4;
    aff.bitvector = AFF_STUNNED;
    vict->affectTo(&aff, -1);
  
    act("Wind slams into $N, knocking them to the ground!", 
      false, ch, obj, vict, TO_ROOM, ANSI_CYAN);
    act("Wind slams into you, knocking you to the ground!", 
      false, ch, obj, vict, TO_VICT, ANSI_CYAN);
    act("Wind slams into $N, knocking them to the ground!", 
      false, ch, obj, vict, TO_CHAR, ANSI_CYAN);
  
    if (vict->riding) {
      vict->dismount(POSITION_STUNNED);
    } else {
      vict->setPosition(POSITION_STUNNED);
    }

    if (manaSpent >= 400) {
      vict->cantHit += vict->loseRound(3 + (coldimmunity / 50));
      act("The storm rages around $N, freezing $M to the core!", 
          false, ch, obj, vict, TO_ROOM, ANSI_CYAN_BOLD);
      act("The storm rages around you, freezing you to the core!", 
          false, ch, obj, vict, TO_VICT, ANSI_CYAN_BOLD);
      act("The storm rages around $N, freezing $M to the core!", 
          false, ch, obj, vict, TO_CHAR, ANSI_CYAN_BOLD);
    }

    if (manaSpent >= 500) {
      int numLimbs = ::number(manaSpent / 100 - 2, manaSpent / 100 + 2);
      std::vector<wearSlotT> validLimbs;

      for (wearSlotT limb = MIN_WEAR; limb < MAX_WEAR; limb = static_cast<wearSlotT>(limb + 1)) {
        if (vict->hasPart(limb) && !vict->equipment[limb] && !vict->getStuckIn(limb)) {
          validLimbs.push_back(limb);
        }
      }

      while (numLimbs > 0 && !validLimbs.empty()) {
        unsigned int index = ::number(0, validLimbs.size() - 1);
        wearSlotT limb = validLimbs[index];
        
        sstring buf = format("The icy winds tear into $N's %s with shards of ice!") % vict->describeBodySlot(limb);
        act(buf, false, ch, obj, vict, TO_ROOM, ANSI_BLUE_BOLD);
        
        buf = format("The icy winds tear into your %s with shards of ice!") % vict->describeBodySlot(limb);
        act(buf, false, ch, obj, vict, TO_VICT, ANSI_BLUE_BOLD);

        buf = format("The icy winds tear into $N's %s with shards of ice!") % vict->describeBodySlot(limb);
        act(buf, false, ch, obj, vict, TO_CHAR, ANSI_BLUE_BOLD);

        vict->hurtLimb(::number(manaSpent / 100, manaSpent / 100 + 10), limb);

        if (manaSpent >= 600) {
          TObj* icicle = read_object(31349, VIRTUAL);
          if (icicle) {
            vict->stickIn(icicle, limb, SILENT_YES);
            buf = format("An icicle embeds itself into %s's %s!")
                 % vict->getName() % vict->describeBodySlot(limb);
            act(buf, false, ch, obj, vict, TO_ROOM, ANSI_WHITE_BOLD);
            
            buf = format("An icicle embeds itself into your %s!")
                  % vict->describeBodySlot(limb);
            act(buf, false, ch, obj, vict, TO_VICT, ANSI_WHITE_BOLD);

            buf = format("An icicle embeds itself into $N's %s!")
                  % vict->describeBodySlot(limb);
            act(buf, false, ch, obj, vict, TO_CHAR, ANSI_WHITE_BOLD);
          }
        }

        --numLimbs;
        validLimbs.erase(validLimbs.begin() + index);
      }
    }
  }
  
  if (!ch || !percentChance(manaSpent / 60)) {
    return false;
  }

  int backfire_rc = ch->reconcileDamage(ch, dam, DAMAGE_TRAP_PIERCE);
  act("Deadly shards of ice tear through your flesh, impaling you from within.",
      FALSE, ch, obj, vict, TO_CHAR, ANSI_BLUE_BOLD);

  act("Jagged shards of ice explode from within $n's body, impaling them in a bloody spray.",
      FALSE, ch, obj, vict, TO_ROOM, ANSI_BLUE_BOLD);

  if (IS_SET_DELETE(backfire_rc, DELETE_VICT)) {
    vict->reformGroup();
    delete vict;
    return DELETE_VICT;
  }

  return true;
}


int coldThief(TBeing* ch, TBeing* vict, TObj* obj, int level) {
  if (!ch || !vict)
    return FALSE;

  if (vict->isImmune(IMMUNE_COLD, WEAR_BODY) && ::number(0, 1))
    return FALSE;

  wearSlotT limb = WEAR_BACK;
  
  act("<W>$p turns bright white and freezes down $N's spine!<z>", false,
    ch, obj, vict, TO_CHAR);
  act("<W>$p turns bright white and freezes down $N's spine!<z>", false,
    ch, obj, vict, TO_ROOM);
  act("<W>$p turns bright white and freezes down your spine!<z>", false,
    ch, obj, vict, TO_VICT);

  if (vict->slotChance(limb) && !vict->equipment[limb] &&
      !vict->getStuckIn(limb)) {
    TObj* icicle = read_object(31349, VIRTUAL);
    if (!icicle)
      return TRUE;  // Still return success even if icicle creation fails

    sstring buf = format("$p freezes as it pierces $N's %s, leaving behind a jagged icicle!<z>")
                 % vict->describeBodySlot(limb);
    act(buf, false, ch, obj, vict, TO_ROOM, ANSI_BLUE_BOLD);
    
    buf = format("$p freezes as it pierces your %s, leaving behind a jagged icicle!")
         % vict->describeBodySlot(limb);
    act(buf, false, ch, obj, vict, TO_VICT, ANSI_BLUE_BOLD);
    
    buf = format("Your $p freezes as it pierces $N's %s, leaving behind a jagged icicle!")
         % vict->describeBodySlot(limb);
    act(buf, false, ch, obj, vict, TO_CHAR, ANSI_BLUE_BOLD);
    
    vict->stickIn(icicle, limb);
  }

  int dam = level * 3;
  int rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;

  return TRUE;
}
int icyDeath(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* obj, TObj*) {
  TBeing* ch = dynamic_cast<TBeing*>(obj->equippedBy);
  if (!ch)
    return FALSE;  
  int rc;
  int level = ch->GetMaxLevel();
  int coldimmunity = ch->getImmunity(IMMUNE_COLD);
  rc = false;
  int dam = 0;
  if (cmd == CMD_OBJ_HIT && vict && percentChance(20)) {
    ColdHit(ch, vict, obj, level);
    rc = ch->reconcileDamage(vict, dam, DAMAGE_FROST);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      vict->reformGroup();
      delete vict;
      vict = NULL;
    }
  }

  if (cmd == CMD_GENERIC_PULSE && percentChance(5)) {
    ColdShroud(ch, obj, level);
    return true;
  }

  if (cmd == (CMD_SAY) || cmd == (CMD_SAY2)) {
    sstring buf, buf2;
    TBeing* vict = ch->fight();
    buf = sstring(arg).word(0);
    buf2 = sstring(arg).word(1);

    if (buf == "winter" && buf2 == "cometh") {
      if (!vict) {
        act("You cannot use $p's powers unless you are fighting.", TRUE, ch, obj, NULL, TO_CHAR, NULL);
        return FALSE;
      }
      coldBlast(ch, vict, obj, level, coldimmunity);
      return true;
    }
  }

  if (cmd == CMD_BACKSTAB) {
    // The arg will be WEAR_BACK if backstab was successful
    wearSlotT limb = static_cast<wearSlotT>(reinterpret_cast<uintptr_t>(arg));
    if (limb == WEAR_BACK) {  // Only trigger on successful backstab
      coldThief(ch, vict, obj, level);
    }
  }
  return false;
}
