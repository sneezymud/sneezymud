//////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "equip.cc" - Procedures for equipping/removing items.
//
//////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_bandaid.h"
#include "obj_base_weapon.h"
#include "obj_armor.h"
#include "obj_base_clothing.h"
#include "obj_jewelry.h"
#include "obj_harness.h"
#include "obj_saddle.h"
#include "obj_saddlebag.h"

int GetItemClassRestrictions(const TObj *obj)
{
  int total = 0;

  if (obj->isObjStat(ITEM_ANTI_MAGE))
    total += CLASS_MAGE;
  if (obj->isObjStat(ITEM_ANTI_THIEF))
    total += CLASS_THIEF;
  if (obj->isObjStat(ITEM_ANTI_WARRIOR))
    total += CLASS_WARRIOR;
  if (obj->isObjStat(ITEM_ANTI_CLERIC))
    total += CLASS_CLERIC;
  if (obj->isObjStat(ITEM_ANTI_SHAMAN))
    total += CLASS_SHAMAN;
  if (obj->isObjStat(ITEM_ANTI_MONK))
    total += CLASS_MONK;
  if (obj->isObjStat(ITEM_ANTI_DEIKHAN))
    total += CLASS_DEIKHAN;
  if (obj->isObjStat(ITEM_ANTI_RANGER))
    total += CLASS_RANGER;

  return (total);
}

bool IsRestricted(ush_int mask, ush_int Class)
{
  ush_int i;

  for (i = CLASS_MAGE; i <= CLASS_THIEF; i *= 2) {
    if (IS_SET(i, mask) && !IS_SET(i, Class))
      mask -= i;
  }
  if (IS_SET(CLASS_DEIKHAN, mask) && (!IS_SET(CLASS_DEIKHAN, Class)))
    mask -= CLASS_DEIKHAN;
  if (IS_SET(CLASS_SHAMAN, mask) && (!IS_SET(CLASS_SHAMAN, Class)))
    mask -= CLASS_SHAMAN;
  if (IS_SET(CLASS_RANGER, mask) && (!IS_SET(CLASS_RANGER, Class)))
    mask -= CLASS_RANGER;
  if (IS_SET(CLASS_MONK, mask) && (!IS_SET(CLASS_MONK, Class)))
    mask -= CLASS_MONK;

  if (mask == Class)
    return TRUE;

  return FALSE;
}

int check_size_restrictions(const TBeing *ch, const TObj *o, wearSlotT slot, const TBeing *m)
{
  double perc;
  char buf[256];

  if (ch->isImmortal())
    return TRUE;
  const TBaseClothing *tbc = dynamic_cast<const TBaseClothing *>(o);
  if (!tbc || dynamic_cast<const TJewelry *>(tbc))
    return TRUE;
  const TBaseContainer *tbc2 = (dynamic_cast<const TBaseContainer *>(o)); 
  if (tbc2) 
    if (tbc2->isSaddle())
      return TRUE;
  if (tbc->isSaddle())
    return TRUE;
  if ((slot == WEAR_NECK) ||
      (slot == WEAR_FINGER_R) ||
      (slot == WEAR_FINGER_L))
    return TRUE;

  perc = (((double) ch->getHeight()) * (double) race_vol_constants[mapSlotToFile(slot)]);

  if (tbc->isPaired())
    perc *= 2;

  if (tbc->getVolume() > (int) (perc / 0.85)) {
    sprintf(buf, "Sorry, that item is too big to be worn by %s.",
            (ch == m ? "you" : "$N"));
    act(buf, FALSE, m, 0, ch, TO_CHAR);
    return FALSE;
  } else if (tbc->getVolume() < (int) (perc/ 1.15)) {
    sprintf(buf, "Sorry, that item is too small to be worn by %s.",
            (ch == m ? "you" : "$N"));
    act(buf, FALSE, m, 0, ch, TO_CHAR);
    return FALSE;
  }
  return TRUE;
}

bool TBeing::isWieldingWeapon()
{
  if (heldInPrimHand() && dynamic_cast<TBaseWeapon *>(heldInPrimHand())) 
    return TRUE;
  if (heldInSecHand() && dynamic_cast<TBaseWeapon *>(heldInSecHand())) 
    return TRUE;
  return FALSE;
}

bool TBeing::canUseEquipment(const TObj *o, silentTypeT silent, wearKeyT key) const
{
  bool held=false;

  if(key==WEAR_KEY_HOLD || key==WEAR_KEY_HOLD_R || key==WEAR_KEY_HOLD_L)
    held=true;

  if (!isImmortal()) {
    if(o->getStructPoints() <= 0){
      if(!silent)
	sendTo("That item is too damaged to use.\n\r");
      return FALSE;
    }
    if (IsRestricted(GetItemClassRestrictions(o), getClass())) {
      if (!silent)
        sendTo("You are forbidden to do that.\n\r");
      return FALSE;
    }
    if (o->monkRestrictedItem(this)) {
      if (!silent)
        sendTo("That item is too ostentatious for you to wear it.\n\r");
      return FALSE;
    }
    if (o->shamanRestrictedItem(this)) {
      if (!silent)
        sendTo("That item would interfere with communication to the ancestors.\n\r");
      return FALSE;
    }
    if (o->rangerRestrictedItem(this)) {
      if (!silent)
        sendTo("Rangers shun the use of metal armor.\n\r");
      return FALSE;
    }
    if (getRace() == RACE_HOBBIT && o->canWear(ITEM_WEAR_FEET) && !held) {
      if (!silent)
        sendTo("Cover up your beautiful, furry feet?!?  No way!\n\r");
      return FALSE;
    }

  }
  return TRUE;
}

// returns true if they can wield weapon
// adapts for !o->isWeapon()
int TBeing::checkWeaponWeight(TThing *o, handTypeT hand, bool text)
{
// kludge till this function is replaced.  Need to have this replaced soon.
// Cosmo- people couldnt wield at level 25- give them at least a pound a level

  if (isImmortal())
    return TRUE;

  // o weight > wield weight
  if (compareWeights(o->getWeight(), maxWieldWeight(o, hand)) == -1) {
    if (dynamic_cast<TBaseWeapon *>(o)) {
      if (text)
        act("$p is too heavy for you to use properly!", false, this, o, NULL, TO_CHAR);
    } else if (hand != HAND_TYPE_SEC) {
      // 2-handed and primary weapons
      if (text)
        act("$p is too heavy for you to use!", false, this, o, NULL, TO_CHAR);
    } else if (isRightHanded()) {
      if (text)
        act("$p is too heavy for you to use in your left hand!", false, this, o, NULL, TO_CHAR);
    } else {
      if (text)
        act("$p is too heavy for you to use in your right hand!", false, this, o, NULL, TO_CHAR);
    }
    return FALSE;
  } else {
    return TRUE;  // weight is less then max-wield-weight
  }
}

int TObj::personalizedCheck(TBeing *ch)
{
  char capbuf[256];
  char namebuf[256];
  TThing *t;

  if (action_description && strcmp(action_description, "")) {
    strcpy(capbuf, action_description);
    
    if ((sscanf(capbuf, "This is the personalized object of %s.", namebuf)) != 1) {
      vlogf(LOG_BUG, fmt("Bad personalized item (on %s) with bad action description...extracting from world.") %  ch->getName());
      return DELETE_THIS;
    } else if (strcmp(namebuf, ch->getName()) && (!ch->isPc() || dynamic_cast<TPerson *>(ch))) {
      // skips for polys
      act("You shouldn't have $p! It isn't yours!", 0, ch, this, NULL, TO_CHAR);
      sendTo(fmt("It's the personalized item of %s!\n\r") % namebuf);
      act("You are zapped by $p and drop it!", FALSE, ch, this, NULL, TO_CHAR);
      act("$n is zapped by $p!",TRUE,ch,this,0,TO_ROOM);

      vlogf(LOG_MISC, fmt("We got an illegal personalized item (%s) off of %s (was %s's item).") %  getName() % ch->getName() % namebuf);

      t = ch->roomp->getStuff();
      while (t) {
        TMonster *tmons = dynamic_cast<TMonster *>(t);
        if (tmons && tmons->canSee(ch) && (tmons != ch) && tmons->isHumanoid() && (tmons->getPosition() > POSITION_SLEEPING)) {
          tmons->US(1 + obj_flags.cost/5000);
          tmons->aiTarget(ch);
          if (!::number(0,2)) {
            act("$n glares at $N and says, \"Thief!\"",TRUE,tmons,0,ch,TO_NOTVICT);
            act("$n glares at you and says, \"Thief!\"",TRUE,tmons,0,ch,TO_VICT);
          }
        }
        t = t->nextThing;
      }

      if (eq_pos != WEAR_NOWHERE) 
	*ch->roomp += *ch->unequip(eq_pos);
      else if (parent) {
	--(*this);
	*ch->roomp += *this;
      }
    }
  }
  return FALSE;
}

int TBeing::loseRoundWear(double num, bool randomize, bool check)
{
  if (!fight())
    return FALSE;

  if (isAffected(AFF_ENGAGER) && !isTanking()) 
    return FALSE;

   int rc = loseRound(num, randomize, check);
   cantHit += rc;
   return rc;
}

// ch is who messages go to
// this is who is wearing the item
// DELETE_ITEM = nuke o
int TBeing::wear(TObj *o, wearKeyT keyword, TBeing *ch)
{       
  char buf[256];
  int primary = 0, rc = 0;
  spellNumT skill = TYPE_UNDEFINED;

  if (!canUseEquipment(o, SILENT_NO, keyword))
    return FALSE;

  if (!isImmortal() && desc && (isTanking() || 
                     (fight() && !isAffected(AFF_ENGAGER)))) {
    switch (keyword) {
      case WEAR_KEY_NONE:
      case WEAR_KEY_HOLD:
      case WEAR_KEY_HOLD_R:
      case WEAR_KEY_HOLD_L:
        break;
      default: 
        if (isTanking())
          sprintf(buf, "%s can't wear equipment like $p while tanking.",
             (ch == this ? "You" : "$N"));
        else
          sprintf(buf, "%s can't wear equipment like $p while fighting.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
        return FALSE; 
    }
  }

  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(o);
  if (tbc && tbc->isShield()) {
    if (keyword == WEAR_KEY_HOLD)
      keyword = (isRightHanded() ? WEAR_KEY_HOLD_L : WEAR_KEY_HOLD_R);
    else if ((keyword == WEAR_KEY_HOLD_R && isRightHanded()) ||
             (keyword == WEAR_KEY_HOLD_L && !isRightHanded())) {
      sprintf(buf, "%s can't equip $p in your %s %s.", 
         (ch == this ? "You" : "$N"),
         (ch == this ? "your" : "$S"),
         describeBodySlot(isRightHanded() ? HOLD_RIGHT : HOLD_LEFT).c_str());
      act(buf, FALSE, ch, o, this, TO_CHAR);

      return FALSE;
    }
  }

  switch (keyword) {
    case WEAR_KEY_FINGERS:
      if (validEquipSlot(WEAR_FINGER_L) && validEquipSlot(WEAR_FINGER_R)) {
        if (o->canWear(ITEM_WEAR_FINGERS)) {
          if (equipment[WEAR_FINGER_L] && equipment[WEAR_FINGER_R]) {
            sprintf(buf, "%s are already wearing something on %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_FINGER_R).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "%s are already wearing something on %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_FINGER_L).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);

            return FALSE;
          } else {
            if (equipment[WEAR_FINGER_L]) {
              sprintf(buf, "You slip $p around %s %s.",
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_FINGER_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n slips $p around %s %s.",
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_FINGER_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n slips $p around %s %s.",
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_FINGER_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_FINGER_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.05, TRUE, TRUE);
            } else {
              sprintf(buf, "You slip $p around %s %s.",
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_FINGER_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n slips $p around %s %s.",
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_FINGER_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n slips $p around %s %s.",
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_FINGER_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_FINGER_L);
              //  If fighting, make them lose a round or two. 
              loseRoundWear(0.05, TRUE, TRUE);
            }
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s fingers.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }
      break;
    case WEAR_KEY_NECK:
      if (validEquipSlot(WEAR_NECK)) {
        if (o->canWear(ITEM_WEAR_NECK)) {
          if (equipment[WEAR_NECK]) {
            sprintf(buf, "%s can't wear any more around %s %s.",
                 (ch == this ? "You" : "$N"),
                 (ch == this ? "your" : "$S"),
                 describeBodySlot(WEAR_NECK).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            sprintf(buf, "You put $p around %s %s.",
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(WEAR_NECK).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n puts $p around %s %s.",
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(WEAR_NECK).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n puts $p around %s %s.",
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(WEAR_NECK).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, WEAR_NECK);
            // If fighting, make them lose a round or two. 
            loseRoundWear(0.15, TRUE, TRUE);
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p around %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_NECK).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }
      break;
    case WEAR_KEY_BODY:
      if (validEquipSlot(WEAR_BODY)) {
        if (o->canWear(ITEM_WEAR_BODY)) {
          if (!check_size_restrictions(this, o, WEAR_BODY, ch))
            return FALSE;

          if (equipment[WEAR_BODY]) {
            sprintf(buf, "%s already %s something on %s %s.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"),
                   describeBodySlot(WEAR_BODY).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            sprintf(buf, "You %s $p around %s %s.",
                   (ch == this ? "wear" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(WEAR_BODY).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n %s $p around %s %s.",
                   (ch == this ? "wears" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(WEAR_BODY).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n %s $p around %s %s.",
                   (ch == this ? "wears" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(WEAR_BODY).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, WEAR_BODY);
            // If fighting, make them lose a round or two. 
            loseRoundWear(1.0, TRUE, TRUE);
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p around %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_BODY).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }
      break;
    case WEAR_KEY_HEAD:
      if (validEquipSlot(WEAR_HEAD)) {
        if (o->canWear(ITEM_WEAR_HEAD)) {
          if (!check_size_restrictions(this, o, WEAR_HEAD, ch))
            return FALSE;

          if (equipment[WEAR_HEAD]) {
            sprintf(buf, "%s already %s something on %s %s.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"),
                   describeBodySlot(WEAR_HEAD).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            sprintf(buf, "You %s $p on %s %s.",
                   (ch == this ? "wear" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(WEAR_HEAD).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n %s $p on %s %s.",
                   (ch == this ? "wears" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(WEAR_HEAD).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n %s $p on %s %s.",
                   (ch == this ? "wears" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(WEAR_HEAD).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, WEAR_HEAD);
            // If fighting, make them lose a round or two. 
            loseRoundWear(0.10, TRUE, TRUE);
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p around %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_HEAD).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_LEGS:
      if (validEquipSlot(WEAR_LEG_L) && validEquipSlot(WEAR_LEG_R)) {
        if (o->canWear(ITEM_WEAR_LEGS)) {
          if (equipment[WEAR_LEG_L] && equipment[WEAR_LEG_R]) {
            sprintf(buf, "%s already %s something on %s legs.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else if (o->isPaired()) {
            if (equipment[WEAR_LEG_L] || equipment[WEAR_LEG_R]) {
              sprintf(buf, "Both %slegs must be bare to wear $p.",
                     (ch == this ? "" : "of $N's "));
              act(buf, FALSE, ch, o, this, TO_CHAR);
            } else {
              if (!check_size_restrictions(this, o, WEAR_LEG_L, ch))
                return FALSE;
              sprintf(buf, "You %s $p on %s legs.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"));
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s legs.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"));
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s legs.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"));
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_LEG_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.60, TRUE, TRUE);
              aiWear(o);
            }
          } else {
            if (equipment[WEAR_LEG_L]) {
              if (!check_size_restrictions(this, o, WEAR_LEG_L, ch))
                return FALSE;
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_LEG_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_LEG_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_LEG_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_LEG_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.60, TRUE, TRUE);
            } else {
              if (!check_size_restrictions(this, o, WEAR_LEG_R, ch))
                return FALSE;
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_LEG_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_LEG_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_LEG_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_LEG_L);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.60, TRUE, TRUE);
            }
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p around %s legs.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_FEET:
      if (validEquipSlot(WEAR_FOOT_L) && validEquipSlot(WEAR_FOOT_R)) {
        if (o->canWear(ITEM_WEAR_FEET)) {
          if (!check_size_restrictions(this, o, WEAR_FOOT_L, ch))
            return FALSE;

          if ((equipment[WEAR_FOOT_L] || hasQuestBit(TOG_PEGLEG_L)) &&
	      (equipment[WEAR_FOOT_R] || hasQuestBit(TOG_PEGLEG_R))){
            sprintf(buf, "%s already %s something on %s feet.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            if ((equipment[WEAR_FOOT_L] || hasQuestBit(TOG_PEGLEG_L)) &&
		!hasQuestBit(TOG_PEGLEG_R)){
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_FOOT_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_FOOT_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_FOOT_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_FOOT_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.45, TRUE, TRUE);
	      aiWear(o);
            } else if(!hasQuestBit(TOG_PEGLEG_L)){
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_FOOT_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_FOOT_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_FOOT_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_FOOT_L);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.45, TRUE, TRUE);
	      aiWear(o);
            } else {
	      sprintf(buf, "%s can't wear $p on that body part.",
		      (ch == this ? "You" : "$N"));
	      act(buf, FALSE, ch, o, this, TO_CHAR);
	    }
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s feet.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_HANDS:
      // the logic in here is horrible, and I just made it much worse
      // with the hook hand toggles
      if (validEquipSlot(WEAR_HAND_L) && validEquipSlot(WEAR_HAND_R)) {
        if (o->canWear(ITEM_WEAR_HANDS)) {
          if (!check_size_restrictions(this, o, WEAR_HAND_L, ch))
            return FALSE;

          if ((equipment[WEAR_HAND_L] || hasQuestBit(TOG_HOOK_HAND_L)) && 
	      (equipment[WEAR_HAND_R] || hasQuestBit(TOG_HOOK_HAND_R))){
            sprintf(buf, "%s already %s something on %s hands.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            if ((equipment[WEAR_HAND_L] || hasQuestBit(TOG_HOOK_HAND_L)) &&
		!hasQuestBit(TOG_HOOK_HAND_R)){
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_HAND_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_HAND_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_HAND_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_HAND_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.25, TRUE, TRUE);
	      aiWear(o);
            } else if(!hasQuestBit(TOG_HOOK_HAND_L)){
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_HAND_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_HAND_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_HAND_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_HAND_L);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.25, TRUE, TRUE);
	      aiWear(o);
            } else {
	      sprintf(buf, "%s can't wear $p on that body part.",
		      (ch == this ? "You" : "$N"));
	      act(buf, FALSE, ch, o, this, TO_CHAR);
	    }
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s hands.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_ARMS:
      if (validEquipSlot(WEAR_ARM_L) && validEquipSlot(WEAR_ARM_R)) {
        if (o->canWear(ITEM_WEAR_ARMS)) {
          if (!check_size_restrictions(this, o, WEAR_ARM_R, ch))
            return FALSE;

          if (equipment[WEAR_ARM_L] && equipment[WEAR_ARM_R]) {
            sprintf(buf, "%s already %s something on %s arms.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            if (equipment[WEAR_ARM_L]) {
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_ARM_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_ARM_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_ARM_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_ARM_R);
              // If fighting, make them lose a round or two. 
             loseRoundWear(0.75, TRUE, TRUE);
            } else {
              sprintf(buf, "You %s $p on %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_ARM_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_ARM_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p on %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_ARM_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_ARM_L);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.75, TRUE, TRUE);
            }
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s arms.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_BACK:
      if (o->canWear(ITEM_WEAR_BACK)) {
        tbc = dynamic_cast<TBaseClothing *>(o);
	TBaseContainer *tbc2 = dynamic_cast<TBaseContainer *>(o);
	if (tbc2 && tbc2->isSaddle()){
	  sendTo("You have to be saddled to put that on.\n\r");
	  return FALSE;
	} else if (tbc && (tbc->isSaddle() || tbc->isHarness())) {
          sendTo("You have to be saddled to put that on.\n\r");
          return FALSE;
        } else if (validEquipSlot(WEAR_BACK)) {
          if (!check_size_restrictions(this, o, WEAR_BACK, ch))
            return FALSE;

          if (equipment[WEAR_BACK]) {
            sprintf(buf, "%s already %s something on %s %s.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"),
                   describeBodySlot(WEAR_BACK).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            sprintf(buf, "You strap $p on %s %s.",
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_BACK).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n straps $p on %s %s.",
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_BACK).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n straps $p on %s %s.",
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_BACK).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, WEAR_BACK);
            // If fighting, make them lose a round or two. 
            loseRoundWear(0.33, TRUE, TRUE);
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p on that body part.",
               (ch == this ? "You" : "$N"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_BODY).c_str());
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_WAIST:
      if (validEquipSlot(WEAR_WAIST)) {
        if (o->canWear(ITEM_WEAR_WAIST)) {
          if (!check_size_restrictions(this, o, WEAR_WAIST, ch))
            return FALSE;

          if (equipment[WEAR_WAIST]) {
            sprintf(buf, "%s already %s something on %s %s.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"),
                   describeBodySlot(WEAR_WAIST).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            sprintf(buf, "You %s $p around %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_WAIST).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_WAIST).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_WAIST).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, WEAR_WAIST);
            // If fighting, make them lose a round or two. 
            loseRoundWear(0.90, TRUE, TRUE);
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s %s.",
                 (ch == this ? "You" : "$N"),
                 (ch == this ? "your" : "$S"),
                 describeBodySlot(WEAR_WAIST).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_WRISTS:
      if (validEquipSlot(WEAR_WRIST_L) && validEquipSlot(WEAR_WRIST_R)) {
        if (o->canWear(ITEM_WEAR_WRISTS)) {
          if (!check_size_restrictions(this, o, WEAR_WRIST_L, ch))
            return FALSE;

          if (equipment[WEAR_WRIST_L] && equipment[WEAR_WRIST_R]) {
            sprintf(buf, "%s already %s something on %s wrists.",
                   (ch == this ? "You" : "$N"),
                   (ch == this ? "wear" : "wears"),
                   (ch == this ? "your" : "$S"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            if (equipment[WEAR_WRIST_L]) {
              sprintf(buf, "You %s $p around %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_WRIST_R).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_WRIST_R).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_WRIST_R).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_WRIST_R);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.25, TRUE, TRUE);
            } else {
              sprintf(buf, "You %s $p around %s %s.",
                     (ch == this ? "wear" : "outfit"),
                     (ch == this ? "your" : "$N's"),
                     describeBodySlot(WEAR_WRIST_L).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "your"),
                     describeBodySlot(WEAR_WRIST_L).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p around %s %s.",
                     (ch == this ? "wears" : "outfits"),
                     (ch == this ? "$s" : "$N's"),
                     describeBodySlot(WEAR_WRIST_L).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, WEAR_WRIST_L);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.25, TRUE, TRUE);
            }
            aiWear(o);
          }
        } else {
          sprintf(buf, "%s can't wear $p on %s wrists.",
                 (ch == this ? "You" : "$N"),
                 (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_HOLD:
      if (validEquipSlot(HOLD_RIGHT) && validEquipSlot(HOLD_LEFT)) {
        if (equipment[HOLD_RIGHT] && equipment[HOLD_LEFT]) {
          sprintf(buf, "%s already %s something in both %s hands.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "hold" : "holds"),
               (ch == this ? "your" : "$S"));
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else if (o->isPaired()) {
          if (equipment[HOLD_RIGHT] || equipment[HOLD_LEFT]) {
            sprintf(buf, "Both %shands must be free to hold $p.",
                 (ch == this ? "" : "of $N's "));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else if (!canUseLimb(HOLD_RIGHT) || !canUseLimb(HOLD_LEFT)) {
            sprintf(buf, "%s busted hand makes that tough to do.",
                 (ch == this ? "Your" : "$N's"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else if (!canUseLimb(WEAR_HAND_R) || !canUseLimb(WEAR_HAND_L)) {
            sprintf(buf, "%s damaged hand makes that tough to do.",
                 (ch == this ? "Your" : "$N's"));
            act(buf, FALSE, ch, o, this, TO_CHAR);
          } else {
            if (checkWeaponWeight(o, HAND_TYPE_BOTH, TRUE)) {
              sprintf(buf, "You %s $p in both %s hands.",
                     (ch == this ? "hold" : "outfit"),
                     (ch == this ? "your" : "$N's"));
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p in both %s hands.",
                     (ch == this ? "holds" : "outfits"),
                     (ch == this ? "$s" : "your"));
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p in both %s hands.",
                     (ch == this ? "holds" : "outfits"),
                     (ch == this ? "$s" : "$N's"));
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              --(*o);
              equipChar(o, HOLD_RIGHT);
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.05, TRUE, TRUE);
              aiWear(o);
            }
          }
        } else {
          if (!equipment[getPrimaryHold()] || isAmbidextrous())
            primary = 1;
          else
            primary = 2;
          if (checkWeaponWeight(o, (primary == 1) ? HAND_TYPE_PRIM : HAND_TYPE_SEC, TRUE)) {
            --(*o);
            if (!equipment[getPrimaryHold()]) {
              sprintf(buf, "You %s $p in %s %s.",
                   (ch == this ? "hold" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(isRightHanded() ? HOLD_RIGHT : HOLD_LEFT).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(isRightHanded() ? HOLD_RIGHT : HOLD_LEFT).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(isRightHanded() ? HOLD_RIGHT : HOLD_LEFT).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              equipChar(o, getPrimaryHold());
              // If fighting, make them lose a round or two. 
              loseRoundWear(0.05, TRUE, TRUE);
            } else {
              sprintf(buf, "You %s $p in %s %s.",
                   (ch == this ? "hold" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(isRightHanded() ? HOLD_LEFT : HOLD_RIGHT).c_str());
              act(buf, FALSE, ch, o, this, TO_CHAR);
              sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(isRightHanded() ? HOLD_LEFT : HOLD_RIGHT).c_str());
              act(buf, FALSE, ch, o, this, TO_VICT);
              sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(isRightHanded() ? HOLD_LEFT : HOLD_RIGHT).c_str());
              act(buf, FALSE, ch, o, this, TO_NOTVICT);

              equipChar(o, getSecondaryHold());
              // If fighting, make them lose a round or two. 
              skill = getSkillNum(SKILL_DUAL_WIELD);
              if (doesKnowSkill(skill) && dynamic_cast<TBaseWeapon *> (equipment[getPrimaryHold()])) {
                learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, skill, 100);
              }
              loseRoundWear(0.05, TRUE, TRUE);
            }
            aiWear(o);
          }
        }
      } else {
        sprintf(buf, "%s can't hold things.",
                   (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_HOLD_R:
     // hold right 
      if (validEquipSlot(HOLD_RIGHT)) {
        if (equipment[HOLD_RIGHT]) {
          sprintf(buf, "%s already %s something in %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "hold" : "holds"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(HOLD_RIGHT).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else if (!canUseLimb(WEAR_HAND_R)) {
          sprintf(buf, "%s can't seem to use %s %s!",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_HAND_R).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else if (!canUseLimb(WEAR_ARM_R)) {
          sprintf(buf, "%s can't seem to use %s %s!",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_ARM_R).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else {
          primary = 1 + (!isAmbidextrous() && !isRightHanded());

          if (checkWeaponWeight(o, (primary == 1) ? HAND_TYPE_PRIM : HAND_TYPE_SEC, TRUE)) {
            sprintf(buf, "You %s $p in %s %s.",
                   (ch == this ? "hold" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(HOLD_RIGHT).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(HOLD_RIGHT).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(HOLD_RIGHT).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, HOLD_RIGHT);
            // If fighting, make them lose a round or two. 
            skill = getSkillNum(SKILL_DUAL_WIELD);
            if (doesKnowSkill(skill) && dynamic_cast<TBaseWeapon *> (equipment[HOLD_LEFT])) {
              learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, skill, 100);
            }

            loseRoundWear(0.05, TRUE, TRUE);
            aiWear(o);
          } 
        } 
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_HOLD_L:
      // hold left 
      if (validEquipSlot(HOLD_LEFT)) {
        if (equipment[HOLD_LEFT]) {
          sprintf(buf, "%s already %s something in %s %s.",
               (ch == this ? "You" : "$N"),
               (ch == this ? "hold" : "holds"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(HOLD_LEFT).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else if (!canUseLimb(WEAR_HAND_L)) {
          sprintf(buf, "%s can't seem to use %s %s!",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_HAND_L).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else if (!canUseLimb(WEAR_ARM_L)) {
          sprintf(buf, "%s can't seem to use %s %s!",
               (ch == this ? "You" : "$N"),
               (ch == this ? "your" : "$S"),
               describeBodySlot(WEAR_ARM_L).c_str());
          act(buf, FALSE, ch, o, this, TO_CHAR);
        } else {
          primary = 1 + (!isAmbidextrous() && isRightHanded());

          if (checkWeaponWeight(o, (primary == 1) ? HAND_TYPE_PRIM : HAND_TYPE_SEC, TRUE)) {
            sprintf(buf, "You %s $p in %s %s.",
                   (ch == this ? "hold" : "outfit"),
                   (ch == this ? "your" : "$N's"),
                   describeBodySlot(HOLD_LEFT).c_str());
            act(buf, FALSE, ch, o, this, TO_CHAR);
            sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "your"),
                   describeBodySlot(HOLD_LEFT).c_str());
            act(buf, FALSE, ch, o, this, TO_VICT);
            sprintf(buf, "$n %s $p in %s %s.",
                   (ch == this ? "holds" : "outfits"),
                   (ch == this ? "$s" : "$N's"),
                   describeBodySlot(HOLD_LEFT).c_str());
            act(buf, FALSE, ch, o, this, TO_NOTVICT);

            --(*o);
            equipChar(o, HOLD_LEFT);
            // If fighting, make them lose a round or two. 
            skill = getSkillNum(SKILL_DUAL_WIELD);
            if (doesKnowSkill(skill) && dynamic_cast<TBaseWeapon *> (equipment[HOLD_RIGHT])) {
              learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, skill, 100);
            }

              loseRoundWear(0.05, TRUE, TRUE);
            aiWear(o);
          }
        }
      } else {
        sprintf(buf, "%s can't wear $p on that body part.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, o, this, TO_CHAR);
      }

      break;
    case WEAR_KEY_NONE:
      sprintf(buf, "%s can't wear $p.",
             (ch == this ? "You" : "$N"));
      act(buf, FALSE, ch, o, this, TO_CHAR);
      break;
  }
  rc = o->personalizedCheck(this);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_ITEM;

  return FALSE;
}


void TBeing::doWear(const char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[256];
  TThing *t, *temp;
  TObj *o;
  int i, rc;
  wearKeyT keyword;
  static const char *keywords[] = {
    "finger",
    "neck",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "back",
    "waist",
    "wrist",
    "hold",
    "\n"
  };
  if (!hasHands()) {
    sendTo("How do you expect to do that without any hands?!?\n\r");
    return;
  }
  argument_interpreter(argument, arg1, cElements(arg1), arg2, cElements(arg2));

  if (*arg1) {
    if (!strcmp(arg1, "all")) {
      for (t = getStuff(); t; t = temp) {
        temp = t->nextThing;
        TObj *tobj = dynamic_cast<TObj *>(t);
        if (!tobj)
          continue;

        keyword = tobj->getWearKey();
        if (keyword != WEAR_KEY_NONE) {
          strcpy(buf, tobj->getName());
          sendTo(COLOR_OBJECTS,fmt("%s: ") % sstring(buf).cap());
          rc = wear(tobj, keyword, this);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete tobj;
            tobj = NULL;
          }
        }
      }
    } else {
      TThing *tto = searchLinkedListVis(this, arg1, getStuff());
      o = dynamic_cast<TObj *>(tto);
      if (o) {
        if (*arg2) {
          int sbnum = search_block(arg2, keywords, FALSE);  // Partial Match 
          if (sbnum == -1) {
            sendTo(fmt("Unknown body location: %s\n\r") % arg2);
            sendTo("Valid body locations:\n\r");
            for (i = 0; *keywords[i] != '\n'; i++)
              sendTo(fmt("%s%s") % keywords[i] % (i%3 == 2 ? "\n\r" : "\t"));
            if (i%3)
              sendTo("\n\r");
          } else {
            strcpy(buf, o->shortDescr);
            sendTo(COLOR_OBJECTS,fmt("%s: ") % sstring(buf).cap());
            keyword = wearKeyT(sbnum+1);
            rc = wear(o, keyword, this);
            if (IS_SET_DELETE(rc, DELETE_ITEM)) {
              delete o;
              o = NULL;
            }
          }
        } else {
          keyword = o->getWearKey();

          strcpy(buf, o->shortDescr);
          sendTo(COLOR_OBJECTS,fmt("%s: ") % sstring(buf).cap());
          rc = wear(o, keyword, this);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete o;
            o = NULL;
          }
        }
      } else
        sendTo(fmt("You do not seem to have the '%s'.\n\r") % arg1);
    }
  } else
    sendTo("Wear what?\n\r");
}

int TThing::wieldMe(TBeing *ch, char *)
{
  ch->sendTo("Wielding is for weapons.\n\r");
  return 0;
}

void TBeing::doWield(const char *argument)
{
  char arg1[128];
  char arg2[128];
  TThing *o;

  if (!hasHands()) {
    sendTo("How do you expect to do that without any hands?!?\n\r");
    return;
  }
  half_chop(argument, arg1, arg2);

  if (*arg1) {
    if (!(o = searchLinkedList(arg1, getStuff()))) {
      sendTo(fmt("You do not seem to have the '%s'.\n\r") % arg1);
      return;
    }
    int rc = o->wieldMe(this, arg2);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete o;
      o = NULL;
    }
  } else
    sendTo("Wield what?\n\r");
}

void TBeing::doGrab(const char *argument)
{
  char arg1[128], arg2[128];
  int rc;

  if (!hasHands()) {
    sendTo("How do you expect to do that without any hands?!?\n\r");
    return;
  }
  half_chop(argument, arg1, arg2);

  if (*arg1) {
    TThing *tto = searchLinkedList(arg1, getStuff());
    TObj *o = dynamic_cast<TObj *>(tto);
    if (!o) {
      sendTo(fmt("You do not seem to have the '%s'.\n\r") % arg1);
      return;
    }
    if (!*arg2) {
      rc = wear(o, WEAR_KEY_HOLD, this);
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete o;
        o = NULL;
      }
    } else {
      if (!o->isPaired()) {
        if (is_abbrev(arg2, "right")) {
          rc = wear(o, WEAR_KEY_HOLD_R, this);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete o;
            o = NULL;
          }
        } else if (is_abbrev(arg2, "left")) {
          rc = wear(o, WEAR_KEY_HOLD_L, this);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete o;
            o = NULL;
          }
        } else {
          sendTo("Syntax : Hold <object> (right | left)\n\r");
          return;
        }
      } else {
        sendTo("That is a two handed item. It must be held in both hands.\n\r");
        sendTo("To hold it, clear both hands and type hold <item name>\n\r");
        return;
      }
    }
  } else
    sendTo("Hold what?\n\r");
}

// DELETE_THIS, DELETE_ITEM (buyer)
int TThing::removeMe(TBeing *buyer, wearSlotT)
{
  int rc;

  *buyer += *this;

  // If fighting, make them lose a round or two.
  buyer->loseRoundWear((double) getVolume()/4500, TRUE, TRUE);

  rc = buyer->genericItemCheck(this);
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    return DELETE_THIS;
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_ITEM;

  return FALSE;
}

int TBandaid::removeMe(TBeing *buyer, wearSlotT pos)
{
  if (!buyer->isLimbFlags(pos, PART_BANDAGED))
    return TObj::removeMe(buyer, pos);

  act("You remove the bandage and discard it.",
         0,buyer, this, 0, TO_CHAR);
  buyer->remLimbFlags(pos, PART_BANDAGED);
  return DELETE_THIS;
}

// returns DELETE_THIS for ch
// returns DELETE_ITEM for obj
int TBeing::remove(TThing *obj, TBeing *ch)
{
  wearSlotT pos = obj->eq_pos;
  int rc;
  char buf[256];


  if (!isImmortal() && desc && (isTanking() ||
                     (fight() && !isAffected(AFF_ENGAGER)))) {
    switch (pos) {
      case 18:
      case 19:
        break;
      default:
        if (isTanking())
          sprintf(buf, "%s can't remove equipment like $p while tanking.",
             (ch == this ? "You" : "$N"));
        else
          sprintf(buf, "%s can't remove equipment like $p while fighting.",
             (ch == this ? "You" : "$N"));
        act(buf, FALSE, ch, obj, this, TO_CHAR);
        return FALSE;
    }
  }


  if (this == ch) {
    act("You stop using $p.", FALSE, this, obj, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, this, obj, 0, TO_ROOM);
  } else {
    act("You take $p off $N.", FALSE, this, obj, ch, TO_CHAR);
    act("$n takes $p off you.", FALSE, this, obj, ch, TO_VICT);
    act("$n takes $p off $N.", TRUE, this, obj, ch, TO_NOTVICT);
  }

  if ((obj = ch->unequip(pos))) {
    rc = obj->removeMe(this, pos);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_ITEM;
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    return FALSE;
  } else {
    vlogf(LOG_BUG, "Bad call to remove(TObj *, TBeing *)");
    return FALSE;
  }
}

// DELETE_THIS
int TBeing::doRemove(const sstring &argument, TThing *obj)
{
  sstring arg1, buf;
  TThing *o = NULL;
  TBeing *ch;
  int res, count = 0;
  wearSlotT j;
  wearSlotT slot;
  int rc;

  if (!hasHands()) {
    sendTo("How do you expect to do that without any hands?!?\n\r");
    return FALSE;
  }
  if (unloadBow(argument.c_str()))
    return TRUE;

  arg1=argument.word(0);
  buf=argument.word(1);


  if (!arg1.empty() || obj) {
    if (arg1=="all"){
      if (getPosition() == POSITION_RESTING) {
        sendTo("You sit up in order to remove things more easily.\n\r");
        doSit("");
      }
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        if (getCarriedVolume() < carryVolumeLimit()) {
          if ((o = equipment[j])) {
            rc = remove(o, this);
            if (IS_SET_DELETE(rc, DELETE_ITEM)) {
              delete o;
              o = NULL;
            }
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              return DELETE_THIS;
            }
          }
        } else {
          sendTo("You can't carry any more stuff.\n\r");
          break;
        }
      }
      act("$n stops using $s equipment.", TRUE, this, o, 0, TO_ROOM);
      return TRUE;
    } else if (is_abbrev(arg1, "all.damaged")) {
      for (j = MIN_WEAR; j < MAX_WEAR; j++) {
        o = equipment[j];
        TObj *tobj = dynamic_cast<TObj *>(o);
        if (tobj && (tobj->getStructPoints() < tobj->getMaxStructPoints())) {
          if (carryVolumeLimit() > getCarriedVolume()) {
            rc = remove(tobj, this);
            if (IS_SET_DELETE(rc, DELETE_ITEM)) {
              delete tobj;
              tobj = NULL;
            }
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              return DELETE_THIS;
            }
          } else {
            sendTo("You can't carry any more stuff.\n\r");
            break;
          }
        }
      }
      act("$n stops using $s damaged equipment.", TRUE, this, o, 0, TO_ROOM);
      return TRUE;
    }
    int slot_i;
    if ((slot_i = old_search_block(arg1.c_str(), 0, arg1.length(), bodyParts, 0)) > 0) {
      slot = wearSlotT(--slot_i);
      if (!slotChance(slot)) {
        sendTo("Unfortunately, you don't have that limb.\n\r");
        return FALSE;
      }
      o = equipment[slot];
      if (o) {
        // Check for two handed held items
        if ((slot == HOLD_LEFT) && (equipment[slot] == equipment[HOLD_RIGHT]))
          slot = HOLD_RIGHT;

        if (getPosition() == POSITION_RESTING && slot != HOLD_RIGHT &&
              slot != HOLD_LEFT) {
          sendTo("You sit up in order to remove things more easily.\n\r");
          doSit("");
        }
        rc = remove(o, this);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete o;
          o = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        return TRUE;
      } else {
        sendTo(fmt("There is nothing %s your %s.\n\r") % 
               ((slot == HOLD_RIGHT || slot == HOLD_LEFT) ? "in" : "on") %
               describeBodySlot(slot));
        return FALSE;
      }
    }
    if(buf.empty()){
      if (!(o = obj)) {
        if (!(o = get_thing_in_equip(this, arg1.c_str(), equipment, &j, TRUE, &count))) {
          if (!(o = get_thing_stuck_in_vis(this, arg1.c_str(), &j, &count, this))) {
            sendTo("That item is nowhere on your body!\n\r");
            return FALSE;
          }
        }
      }
      if (obj && obj->stuckIn != this && obj->equippedBy != this) {
        sendTo("That item is nowhere on your body!\n\r");
        return FALSE;
      }
      TObj *tobj = dynamic_cast<TObj *>(o);
      if (tobj && tobj->isObjStat(ITEM_NODROP)) {
        act("You can't let go of $p, it must be CURSED!", FALSE, this, o, 0, TO_CHAR);
        return FALSE;
      }
      if (o->stuckIn) {
        if (carryVolumeLimit() > getCarriedVolume()) {
          buf = fmt("You rip $p out of your %s.") % 
	    describeBodySlot(j=o->eq_stuck);
          act(buf, FALSE, this, o, 0, TO_CHAR);
          buf = fmt("$n rips $p out of $s %s.") %
                     describeBodySlot(j);
          act(buf, FALSE, this, o, 0, TO_ROOM);
  
          // If fighting, make them lose a round or two.
          loseRoundWear((double) o->getVolume() / 2250, TRUE, TRUE);

          o = pulloutObj(j, FALSE, &res);
          if ((res == -1) && o) {
            *roomp += *o;
            return DELETE_THIS;
          } else if (o)
            *this += *o;
        } else {
          sendTo("You aren't agile enough to carry that much volume.\n\r");
          sendTo("Removing weapons, shields and other stuff in your hands may help some...\n\r");
          return FALSE;
        } 
      } else {
        if (getPosition() == POSITION_RESTING && o->eq_pos != HOLD_RIGHT &&
                o->eq_pos != HOLD_LEFT) {
          sendTo("You sit up in order to remove things more easily.\n\r");
          doSit("");
        }
  
        if (carryVolumeLimit() > getCarriedVolume()) {
          rc = remove(o, this);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            if (obj)
              return DELETE_ITEM;
            delete o;
            o = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            return DELETE_THIS;
          }
        } else {
          sendTo("You aren't able to juggle that much volume.\n\r");
          return FALSE;
        }
      }
    } else {
      if (!(ch = get_char_room_vis(this, buf))) {
        sendTo(fmt("You don't see '%s' here.\n\r") % buf);
        return FALSE;
      }
      if (! ((ch->master == this && ch->isAffected(AFF_CHARM)) ||
          (ch->master == this && ch == riding) ||
          (ch->getCaptiveOf() == this))) {
        act("You can't remove things of $N's; $E won't allow it.",
            FALSE, this, 0, ch, TO_CHAR);
        return FALSE;
      }
      if (!(o = obj)) {
        if (!(o = get_thing_in_equip(this, arg1.c_str(), ch->equipment, &j, TRUE, &count))) {
          if (!(o = get_thing_stuck_in_vis(this, arg1.c_str(), &j, &count, ch))) {
            act("That item is nowhere on $N's body,", 
                     FALSE, this, 0, ch, TO_CHAR);
            return FALSE;
          }
        }
      }
      if (obj && obj->stuckIn != ch && obj->equippedBy != ch) {
        act("That item is nowhere on $N's body,", 
                     FALSE, this, 0, ch, TO_CHAR);
        return FALSE;
      }
      TObj *tobj = dynamic_cast<TObj *>(o);
      if (tobj && tobj->isObjStat(ITEM_NODROP)) {
        act("$N can't let go of it, it must be CURSED!",
             FALSE, this, 0, ch, TO_CHAR);
        return FALSE;
      }
      if (o->stuckIn) {
        if ((o->getTotalVolume() + getCarriedVolume()) > carryVolumeLimit()) {
          sendTo("You aren't able to juggle that much volume.\n\r");
          sendTo("Removing weapons, shields and other stuff in your hands may help some...\n\r");
          return FALSE;
        }
        // o-weight > weight limit
        if (compareWeights(o->getTotalWeight(TRUE),
                   carryWeightLimit() - getCarriedWeight()) == -1) {
          sendTo("You aren't able to carry that much weight.\n\r");
          return FALSE;
        }
        buf = fmt("You rip $p out of $N's %s.") %
                   ch->describeBodySlot(j = o->eq_stuck);
        act(buf, FALSE, this, o, ch, TO_CHAR);
        buf = fmt("$n rips $p out of $N's %s.") %
                    ch->describeBodySlot(j);
        act(buf, FALSE, this, o, ch, TO_NOTVICT);
        buf = fmt("$n rips $p out of your %s.") %
                   ch->describeBodySlot(j);
        act(buf, FALSE, this, o, ch, TO_VICT);

        // If fighting, make them lose a round or two.
        loseRoundWear((double) o->getVolume() / 2250, TRUE, TRUE);
        o = ch->pulloutObj(j, FALSE, &res);
        if ((res == -1) && o) {
          *roomp += *o;
          return DELETE_THIS;
        } else if (o)
          *this += *o;
      } else {
        if (getPosition() == POSITION_RESTING) {
          sendTo("You sit up in order to remove things more easily.\n\r");
          doSit("");
        }
        if ((o->getTotalVolume() + getCarriedVolume()) > carryVolumeLimit()) {
          sendTo("You aren't able to juggle that much volume.\n\r");
          return FALSE;
        }
        // o-weight > weight limit
        if (compareWeights(o->getTotalWeight(TRUE),
                   carryWeightLimit() - getCarriedWeight()) == -1) {
          sendTo("You aren't able to carry that much weight.\n\r");
          return FALSE;
        }

        rc = remove(o, ch);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          if (obj)
            return DELETE_ITEM;
          delete o;
          o = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      }
    }
  } else {
    sendTo("Remove what?\n\r");
    return FALSE;
  }
  return TRUE;
}

void change_hands(TBeing *ch, const char *)
{
  TObj *tmp = NULL, *tmp2 = NULL;
  int rc;

  tmp = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT]);
  if (tmp && tmp->isPaired()) {
    ch->sendTo("The item in your hands is two handed!\n\r");
    return;
  }
  TThing *tt = ch->heldInSecHand();
  TBaseClothing *tbc;
  if (tt &&
      (tbc = dynamic_cast<TBaseClothing *>(tt)) &&
      tbc->isShield()) {
    ch->sendTo("You can't move your shield to that hand.\n\r");
    return;
  }

  ch->sendTo("You try to change the items in your hands!\n\r");

  tmp = NULL;
  tmp2 = NULL;

  if ((tmp = dynamic_cast<TObj *>(ch->equipment[HOLD_RIGHT]))) {
    *ch += *ch->unequip(HOLD_RIGHT);
  }
  if ((tmp2 = dynamic_cast<TObj *>(ch->equipment[HOLD_LEFT]))) {
    *ch += *ch->unequip(HOLD_LEFT);
  }
  if (tmp) {
    rc = ch->wear(tmp, WEAR_KEY_HOLD_L, ch);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      delete tmp;
      tmp = NULL;
    }
  }
  if (tmp2) {
    rc = ch->wear(tmp2, WEAR_KEY_HOLD_R, ch);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      delete tmp2;
      tmp2 = NULL;
    }
  }
}

void TBeing::wearNTear(void)
{
  int i = 0, j;

  if (!isPc())
    return;

  if (inRoom() == ROOM_NOWHERE)
    return;

  if (inLethargica())
    return;

  if (roomp && roomp->isRoomFlag(ROOM_ARENA))
    return;

  if (riding || isFlying())
    return;

  int chance = 70 - TerrainInfo[roomp->getSectorType()]->movement;
  chance = max(1,chance);

  for (j=1;j <= 8;j++) {
    if (j == 1)
       i = WEAR_LEG_R;
    else if (j == 2)
       i = WEAR_LEG_L;
    else if (j == 3)
       i = WEAR_FOOT_R;
    else if (j == 4)
       i = WEAR_FOOT_L;
    else if (j == 5)
       i = WEAR_EX_LEG_R;
    else if (j == 6)
       i = WEAR_EX_LEG_L;
    else if (j == 7)
       i = WEAR_EX_FOOT_R;
    else if (j == 8)
       i = WEAR_EX_FOOT_L;

    TThing *tt = equipment[i];
    TObj *obj = dynamic_cast<TObj *>(tt);
    if (obj) {
      if (obj->getMaxStructPoints() < 0)
        continue;
      if (obj->getStructPoints() < 0)
        continue;
      if (!::number(0,chance) && !::number(0,(100 * obj->getStructPoints()) / (max(1,(int)(obj->getMaxStructPoints()))))) {
        act("$p suffers damage from general wear and tear.",TRUE,this,obj,0,TO_CHAR);
	if(IS_SET_DELETE(obj->damageItem(1), DELETE_THIS)){
          delete obj;
          obj = NULL;
        }
      }
    } else if(hasPart((wearSlotT)i)) {
      if ((getRace()== RACE_HOBBIT))
	return; // hobbits have tough feet

      if(!::number(0,chance*2) && ::number(0,getCurLimbHealth((wearSlotT)i)) && GetMaxLevel() > 10) {
	int amount = 0;
	if(doesKnowSkill(SKILL_IRON_SKIN)){
	  amount = getSkillValue(SKILL_IRON_SKIN);
	  amount = max(amount, 0);
	  if (amount >= ::number(1,100)) {
	    return;	  
	  }
	}
	
	if (i == WEAR_FOOT_R || i == WEAR_FOOT_L || 
	    i == WEAR_EX_FOOT_R || i == WEAR_EX_FOOT_L) {
          if (isLevitating())
            return; // If they are levitating, they will not hurt their feet.
	  // animals and such don't have this problem
	  if(!isPeople())
	    return;

          act("<r>Your feet are getting sore from walking around barefoot.<1>",
	      TRUE,this,NULL,0,TO_CHAR);
        }
	
	hurtLimb(getMaxLimbHealth((wearSlotT)i)/10, (wearSlotT)i);
	return;
      }
    }
  }
  return;
}

bool TObj::monkRestrictedItem(const TBeing *ch) const
{
  if (ch && !ch->hasClass(CLASS_MONK))
    return FALSE;

  if (objVnum() == OBJ_SLEEPTAG_STAFF)
    return FALSE;

  if (objVnum() == DEITY_TOKEN)
    return FALSE;

  if (objVnum() == CRAPS_DICE)
    return FALSE;

  if (canWear(ITEM_WEAR_FINGERS))
    return FALSE;

#if 0
  // this includes all minerals (100-150) and metals (150-200)
  ubyte mat = getMaterial();
  if (mat >= 150 &&
      (mat != MAT_BONE))
    return TRUE;

  // this is here just for adding other ones (later) if needed
  switch (getMaterial()) {
    case MAT_IRON:
      return TRUE;
    default:
      break;
  }
#endif

  if (dynamic_cast<const TArmor *>(this))
    return TRUE;

  return FALSE;
}

bool TObj::shamanRestrictedItem(const TBeing *ch) const
{
  if (ch && !ch->hasClass(CLASS_SHAMAN))
    return FALSE;

  if (objVnum() == OBJ_SLEEPTAG_STAFF)
    return FALSE;

  if (objVnum() == DEITY_TOKEN)
    return FALSE;

  if (objVnum() == CRAPS_DICE)
    return FALSE;

  if (objVnum() == MASK1)
    return FALSE;

  if (objVnum() == MASK2)
    return FALSE;

  if (objVnum() == MASK3)
    return FALSE;

  if (objVnum() == MASK4)
    return FALSE;

  if (objVnum() == MASK5)
    return FALSE;

  if (objVnum() == MASK6)
    return FALSE;

  if (canWear(ITEM_WEAR_FINGERS))
    return FALSE;

#if 0
  // this includes all minerals (100-150) and metals (150-200)
  ubyte mat = getMaterial();
  if (mat >= 150 &&
      (mat != MAT_BONE))
    return TRUE;

  // this is here just for adding other ones (later) if needed
  switch (getMaterial()) {
    case MAT_IRON:
      return TRUE;
    default:
      break;
  }
#endif

  if (dynamic_cast<const TArmor *>(this))
    return TRUE;

  return FALSE;
}

bool TObj::rangerRestrictedItem(const TBeing *ch) const
{
  if (ch && !ch->hasClass(CLASS_RANGER))
    return FALSE;

  if (canWear(ITEM_WEAR_FINGERS))
    return FALSE;

  if (!dynamic_cast<const TArmor *>(this))
    return FALSE;

  if (!isMetal())
    return FALSE;

  return TRUE;
}

int TBeing::doUnsaddle(sstring arg)
{
  TThing *saddle;
  TBeing *horse;

  if (arg.empty()) {
    sendTo("Syntax: unsaddle <horse>\n\r");
    return FALSE;
  }
  if (!(horse = get_char_room_vis(this, arg))) {
    sendTo(fmt("You don't see '%s' here.\n\r") % arg);
    return FALSE;
  }
  if (!horse->isRideable()) {
    act("You can't unsaddle $N, $E isn't rideable.", 
          FALSE, this, 0, horse, TO_CHAR);
    return FALSE;
  }

  saddle = (horse->equipment[WEAR_BACK]?
	    horse->equipment[WEAR_BACK]:
	    horse->equipment[WEAR_NECK]);
  TSaddle *tbc = dynamic_cast<TSaddle *>(saddle);
  TSaddlebag *tbc2 = dynamic_cast<TSaddlebag *>(saddle);
  THarness *tbc3 = dynamic_cast<THarness *>(saddle);

  if(!tbc && !tbc2 && !tbc3){
    act("$N is not wearing a saddle or harness.",
          FALSE, this, 0, horse , TO_CHAR);
    return FALSE;
  }
  if (horse->rider) {
    act("$N can't be unsaddled while mounted.",
          FALSE, this, 0, horse, TO_CHAR);
    return FALSE;
  }
  if ((saddle->getTotalVolume() + getCarriedVolume()) > carryVolumeLimit()) {
    sendTo("You aren't able to juggle that much volume.\n\r");
    sendTo("Removing weapons, shields and other stuff in your hands may help some...\n\r");
    return FALSE;
  }
  if (compareWeights(saddle->getTotalWeight(TRUE),
             carryWeightLimit() - getCarriedWeight()) == -1) {
    sendTo("You aren't able to carry that much weight.\n\r");
    return FALSE;
  }

  act(fmt("You %s $p from $N.") % 
      (dynamic_cast<THarness *>(saddle)?"unharness":"unsaddle"),
      FALSE, this, saddle, horse , TO_CHAR);
  act(fmt("$n %s $p from $N.") %
      (dynamic_cast<THarness *>(saddle)?"unharnesses":"unsaddles"),
      FALSE, this, saddle, horse , TO_NOTVICT);
  act(fmt("$n %s your $p.") %
      (dynamic_cast<THarness *>(saddle)?"unharnesses":"unsaddles"),
      FALSE, this, saddle, horse , TO_VICT);

  if(dynamic_cast<THarness *>(saddle)){
    if(horse->tied_to){
      horse->tied_to->tied_to=NULL;
      horse->tied_to=NULL;
    }

    *this += *(horse->unequip(WEAR_NECK));
  } else
    *this += *(horse->unequip(WEAR_BACK));

  return TRUE;
}

void TBeing::doUntie(const sstring &arg)
{
  TBeing *horse;

  if(!(horse=generic_find_being(arg.word(0), FIND_CHAR_ROOM, this))){
    sendTo("Can't find that mount.\n\r");
    return;
  }

  if(!dynamic_cast<THarness *>(horse->equipment[WEAR_NECK])){
    sendTo("That mount isn't harnessed.\n\r");
    return;
  }
  
  if(!horse->tied_to){
    sendTo("That mount isn't tied to anything.\n\r");
    return;
  }

  horse->tied_to->tied_to=NULL;
  horse->tied_to=NULL;
  
  act("You untie $N.",
      FALSE, this, 0, horse, TO_CHAR);
  act("$n unties $N.",
      FALSE, this, 0, horse, TO_NOTVICT);
  act("$n unties you.",
      FALSE, this, 0, horse, TO_VICT);
}

// currently this is only for tying a harnessed horse to an object
void TBeing::doTie(const sstring &arg)
{
  TBeing *horse;
  TThing *obj;

  if(!(horse=generic_find_being(arg.word(0), FIND_CHAR_ROOM, this))){
    sendTo("Can't find that mount.\n\r");
    return;
  }

  if(!dynamic_cast<THarness *>(horse->equipment[WEAR_NECK])){
    sendTo("That mount isn't harnessed.\n\r");
    return;
  }
  
  if(horse->tied_to){
    sendTo("That mount is already tied to something.\n\r");
    return;
  }

  if(!(obj=generic_find_obj(arg.word(1), FIND_OBJ_ROOM, this)) &&
     !(isImmortal() && 
       (obj=generic_find_being(arg.word(1), FIND_CHAR_ROOM, this)))){
    sendTo("Can't find that object.\n\r");
    return;
  }

  if(obj->tied_to){
    sendTo("That object is already tied to something.\n\r");
    return;
  }
  
  act("You tie $N to $p.",
      FALSE, this, obj, horse, TO_CHAR);
  act("$n ties $N to $p.",
      FALSE, this, obj, horse, TO_NOTVICT);
  act("$n ties you to $p.",
      FALSE, this, obj, horse, TO_VICT);

  horse->tied_to=obj;
  obj->tied_to=horse;
}

int TBeing::doSaddle(sstring arg)
{
  TBeing *horse;
  TThing *t;
  TObj *saddle;
  sstring arg1, arg2;
  wearSlotT slot;
  unsigned int slot2;

  argument_interpreter(arg, arg1, arg2);

  if(arg1.empty() || arg2.empty()){
    sendTo("Syntax: saddle <horse> <saddle>\n\r");
    return FALSE;
  }
  if (!(horse = get_char_room_vis(this, arg1))) {
    sendTo(fmt("You don't see '%s' here.\n\r") % arg1);
    return FALSE;
  }

  if (!(t = searchLinkedListVis(this, arg2, getStuff()))) {
    sendTo(fmt("You don't seem to have the '%s'.\n\r") % arg2);
    return FALSE;
  }
  saddle = dynamic_cast<TObj *>(t);
  if (!saddle) {
    sendTo(fmt("You don't seem to have the '%s'.\n\r") % arg2);
    return FALSE;
  }
  if (this == horse) {
    sendTo("You can't saddle yourself, you dolt. Try wearing it.\n\r");
  }
  if (!horse->isRideable()) {
    act("You can't saddle $N, $E isn't rideable.", 
          FALSE, this, saddle, horse, TO_CHAR);
    return FALSE;
  }
  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(saddle);
  TBaseContainer *tbc2 = dynamic_cast<TBaseContainer *>(saddle);
  if((tbc && tbc->isSaddle()) ||
     (tbc2 && tbc2->isSaddle())){
    slot=WEAR_BACK;
    slot2=ITEM_WEAR_BACK;
  } else if((tbc && tbc->isHarness())){
    slot=WEAR_NECK;
    slot2=ITEM_WEAR_NECK;
  } else {
    act("$p is not a saddle or a harness.",
          FALSE, this, saddle, horse , TO_CHAR);
    return FALSE;
  }

  if (!saddle->canWear(slot2)) {
    act("$p is not wearable as a saddle or harness.",
          FALSE, this, saddle, horse , TO_CHAR);
    return FALSE;
  }
  if (!horse->hasPart(slot)) {
    act("$N seems to be lacking a place to put $p",
          FALSE, this, saddle, horse , TO_CHAR);
    return FALSE;
  }
  if (horse->equipment[slot]) {
    act("$N already wears something there.",
          FALSE, this, saddle, horse , TO_CHAR);
    return FALSE;
  }
  if (horse->rider) {
    act("$N can't be saddled while mounted.",
          FALSE, this, 0, horse, TO_CHAR);
    return FALSE;
  }

  if(dynamic_cast<THarness *>(saddle)){
    act("You harness $N with $p.",
	FALSE, this, saddle, horse, TO_CHAR);
    act("$n harnesses $N with $p.",
	FALSE, this, saddle, horse, TO_NOTVICT);
    act("$n harness you with $p.",
	FALSE, this, saddle, horse, TO_VICT);
  } else {
    act("You saddle $N with $p.",
	FALSE, this, saddle, horse, TO_CHAR);
    act("$n saddles $N with $p.",
	FALSE, this, saddle, horse, TO_NOTVICT);
    act("$n saddles you with $p.",
	FALSE, this, saddle, horse, TO_VICT);
  }

  --(*saddle);
  horse->equipChar(saddle, slot);
  return TRUE;
}

float TBeing::maxWieldWeight(const TThing *obj, handTypeT hands) const
{
  // once upon a time, we wanted to limit this so that weapon damage was
  // restrictive to best being warrior-only.
  // Weapon damage is less important nowadays with current combat setup
  // Additionally, this places limits on shield usage.  Balance arguments
  // make NO assumptions about AC limits from strength which is what that
  // would effectively create.  As a consequence, I have upped maxWieldWeight  
  // Bat 4-3-99
#if 0
  float maxval = plotStat(STAT_CURRENT, STAT_STR, (float) 8.0, (float) 25.0, (float) 12.0);
#else
  float maxval = plotStat(STAT_CURRENT, STAT_STR, (float) 16.0, (float) 50.0, (float) 24.0);
#endif

  if (hands == HAND_TYPE_SEC && isAmbidextrous())
    hands = HAND_TYPE_PRIM;

  const TBaseClothing *tbc;
  if (hands == HAND_TYPE_BOTH) {
    // result = sum of both
    maxval = maxWieldWeight(obj, HAND_TYPE_PRIM) + maxWieldWeight(obj, HAND_TYPE_SEC);
  } else if (obj && 
             (tbc = dynamic_cast<const TBaseClothing *>(obj)) &&
             tbc->isShield()) {
    maxval *= 150;
    maxval /= 100;
  } else if (obj && dynamic_cast<const TBaseWeapon *>(obj)) {
    if (hands == HAND_TYPE_PRIM) {
      maxval *= 1.0;
    } else if (hands == HAND_TYPE_SEC) {
      spellNumT skill = getSkillNum(SKILL_DUAL_WIELD);
      if (doesKnowSkill(skill)) {
        maxval *= 100 + getSkillValue(skill);
        maxval /= 200;
      } else
        maxval /= 2.0;
    }

    // boost for blunt weapons
    if (obj->isBluntWeapon())
      maxval += maxval/3.0;

    if (isPc()) {
      // we are going to insure everyone can wield newbie items
      // so this is where the "4" comes from (newbie weapon weight = 4)
      maxval -= min(maxval, (float) 4.0);

      // make wield weight a function of weapon proficiency
      maxval += 4.0;
    }

  } else {
    // non-weapons
    if (hands == HAND_TYPE_PRIM) {
      maxval *= 2.0;
    } else if (hands == HAND_TYPE_SEC) {
      maxval *= 1.0;
    }
  }

  return maxval;
}

void TBeing::doOutfit(const sstring &arg)
{
  sstring buf, buf2;
  TBeing *mob;
  TObj *obj;
  int rc;

  buf=arg.word(0);
  buf2=arg.word(1);

  if(buf.empty() || buf2.empty()){
    sendTo("Syntax: outfit <thing> <person>\n\r");
    return;
  }
  obj = get_obj_vis_accessible(this, buf.c_str());
  if (!obj || (obj && obj->equippedBy)) {
    sendTo(fmt("You can't seem to find '%s'.\n\r") % buf);
    sendTo("Syntax: outfit <thing> <person>\n\r");
    return;
  }
  if (!(mob = get_char_room_vis(this, buf2))) {
    sendTo(fmt("%s: No one by that name here.\n\r") % buf);
    sendTo("Syntax: outfit <thing> <person>\n\r");
    return;
  }

  wearKeyT keyword = obj->getWearKey();

  if (isImmortal() && hasWizPower(POWER_IMMORTAL_OUTFIT)) {
    // immortal equipping
    // use less restrictive rules for holding
    // if can't be worn, put in inventory
    if (obj->isObjStat(ITEM_PROTOTYPE) &&
        mob->isPc() &&
        !mob->isImmortal()) {
      act("You can't outfit prototype objects onto that being!", FALSE, this, obj, 0, TO_CHAR);
      return;
    }
    if (keyword == WEAR_KEY_HOLD) {
      TBaseClothing * tWorn = dynamic_cast<TBaseClothing *>(obj);
  
      handTypeT tHandLoc = (obj->isPaired() ? HAND_TYPE_BOTH :
                            ((tWorn && tWorn->isShield()) ? HAND_TYPE_SEC :
                             HAND_TYPE_PRIM));
  
      if (compareWeights(obj->getTotalWeight(true),
          mob->maxWieldWeight(obj, tHandLoc)) == -1) {
        act("$p is too heavy for $N to hold.", FALSE, this, obj, mob, TO_CHAR);
        return;
      }
    }
    if (keyword == WEAR_KEY_NONE) {
      act("$p: That item doesn't seem to be wearable anywhere.  Putting in inventory.",
           FALSE, this, obj, mob, TO_CHAR);
      
      --(*obj);
      *mob += *obj;
      doSave(SILENT_YES);
      return;
    }
  } else {
    // mortals trying to outfit their pets
    // additional checks for legitimacy
    // make more restrictive about hold slot
    if (mob->master != this) {
      act("You can't outfit $N; $E isn't following you.",
          FALSE, this, 0, mob, TO_CHAR);
      return;
    }
    if (obj->isObjStat(ITEM_NODROP)) {
      act("You can't let go of $p, it must be CURSED!", FALSE, this, obj, 0, TO_CHAR);
      return;
    }
    if (obj->isObjStat(ITEM_PROTOTYPE)) {
      act("You can't outfit prototype objects!", FALSE, this, obj, 0, TO_CHAR);
      return;
    }
  
    if (keyword == WEAR_KEY_HOLD) {
      if (obj->getTotalWeight(true) >= 1.0) {
        act("$p is too heavy for $N to hold.", FALSE, this, obj, mob, TO_CHAR);
        return;
      }
    }
    if (keyword == WEAR_KEY_NONE) {
      act("$p: That item doesn't seem to be wearable anywhere.",
           FALSE, this, obj, mob, TO_CHAR);
      return;
    }
  }
#if 0
  // keyword ain't the right stuff for this function
  if (!mob->canUseLimb(keyword)) {
    act("$N doesnt have the appropriate limb for $p.",
         FALSE, this, obj, mob, TO_CHAR);
    return;
  }
#endif
#if 0
  // text is done in wear()
  act("You outfit $N with $p.",
        FALSE, this, obj, mob, TO_CHAR);
  act("$n outfits you with $p.",
        FALSE, this, obj, mob, TO_VICT);
  act("$n outfits $N with $p.",
        FALSE, this, obj, mob, TO_NOTVICT);
#endif
     
  rc = mob->wear(obj, keyword, this);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    delete obj;
    obj = NULL;
  }

  doSave(SILENT_YES);
  mob->doSave(SILENT_YES);
  return;
}

wearKeyT TObj::getWearKey() const
{
  if (canWear(ITEM_WEAR_FINGERS))
    return WEAR_KEY_FINGERS;
  if (canWear(ITEM_WEAR_NECK))
    return WEAR_KEY_NECK;
  if (canWear(ITEM_WEAR_WRISTS))
    return WEAR_KEY_WRISTS;
  if (canWear(ITEM_WEAR_WAIST))
    return WEAR_KEY_WAIST;
  if (canWear(ITEM_WEAR_ARMS))
    return WEAR_KEY_ARMS;
  if (canWear(ITEM_WEAR_HANDS))
    return WEAR_KEY_HANDS;
  if (canWear(ITEM_WEAR_FEET))
    return WEAR_KEY_FEET;
  if (canWear(ITEM_WEAR_LEGS))
    return WEAR_KEY_LEGS;
  if (canWear(ITEM_WEAR_BACK))
    return WEAR_KEY_BACK;
  if (canWear(ITEM_WEAR_HEAD))
    return WEAR_KEY_HEAD;
  if (canWear(ITEM_WEAR_BODY))
    return WEAR_KEY_BODY;
  if (canWear(ITEM_HOLD))
    return WEAR_KEY_HOLD;
  return WEAR_KEY_NONE;
}
