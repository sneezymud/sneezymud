/*************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      "oldlimbs.c" - Procedures related to limbs

*************************************************************************/

#include "stdsneezy.h"
#include "combat.h"
#include "obj_corpse.h"

void TBeing::setCurLimbHealth(wearSlotT slot, ubyte num)
{
  body_parts[slot].setHealth(num);
}

ubyte TBeing::getCurLimbHealth(wearSlotT slot) const
{
  return body_parts[slot].getHealth();
}

void TBeing::addCurLimbHealth(wearSlotT slot, int num)
{
  int tmp = body_parts[slot].getHealth() + num;
  if (tmp > getMaxLimbHealth(slot))
    body_parts[slot].setHealth(getMaxLimbHealth(slot));
  else
    body_parts[slot].addHealth(num);
}

TThing * TBeing::getStuckIn(wearSlotT limb) const
{
  return body_parts[limb].getStuckIn();
}

void TBeing::setStuckIn(wearSlotT limb, TThing *item)
{
  body_parts[limb].setStuckIn(item);
}

unsigned short int TBeing::getLimbFlags(wearSlotT limb) const
{
  return body_parts[limb].getFlags();
}

void TBeing::setLimbFlags(wearSlotT limb, unsigned short int num)
{
  body_parts[limb].setFlags(num);
}

void TBeing::addToLimbFlags(wearSlotT limb, unsigned short int num)
{
  body_parts[limb].addFlags(num);
}

void TBeing::remLimbFlags(wearSlotT limb, unsigned short int num)
{
  body_parts[limb].remFlags(num);
}

bool TBeing::isLimbFlags(wearSlotT limb, int num) const
{
  return ((body_parts[limb].getFlags() & num) != 0);
}

ubyte TBeing::getMaxLimbHealth(wearSlotT limb) const
{
  int health;

  int sc = slotChance(limb);
  if (!sc)
    return 0;

  health = hitLimit();
  health *= 34;  // was 17 for v5.0
  health /= 10;
  health *= sc;
  health /= slotChance(WEAR_BODY);
  health += plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13); 

  if (getRace() == RACE_HOBBIT) {
    if (limb == WEAR_FOOT_R || limb == WEAR_FOOT_L)
      health *= 2;
  }

  health = min(max(1,health),255);
  return (ubyte) health;
}

bool TBeing::canUseLimb(wearSlotT slot) const
{
  // if it doesn't have the slot, pass true to avoid problems with things
  // like eitherLegHurt, etc
  if (slotChance(slot) <= 0)
    return TRUE;

  if (isLimbFlags(slot, PART_PARALYZED | PART_BROKEN | PART_MISSING | PART_USELESS))
    return FALSE;

  return TRUE;
}

bool TBeing::bothLegsHurt() const
{
  if (!isFourLegged())
    return (!canUseLeg(LEG_PRIMARY) && !canUseLeg(LEG_SECONDARY));
  else
    return (!canUseLeg(LEG_PRIMARY) && !canUseLeg(LEG_SECONDARY) &&
            !canUseLeg(LEG_PRIMARY_BACK) && !canUseLeg(LEG_SECONDARY_BACK));
}

bool TBeing::eitherLegHurt() const
{
  if (!isFourLegged())
    return (!canUseLeg(LEG_PRIMARY) || !canUseLeg(LEG_SECONDARY));
  else
    return (!canUseLeg(LEG_PRIMARY) || !canUseLeg(LEG_SECONDARY) || 
            !canUseLeg(LEG_PRIMARY_BACK) || !canUseLeg(LEG_SECONDARY_BACK));
}

bool TBeing::eitherArmHurt() const
{
  return (!canUseArm(HAND_PRIMARY) || !canUseArm(HAND_SECONDARY));
}

bool TBeing::eitherHandHurt() const
{
  return (!canUseHand(TRUE) || !canUseHand(FALSE));
}

bool TBeing::canUseLeg(primLegT primary) const
{
  switch (primary) {
    case LEG_SECONDARY:
      return (canUseLimb(getSecondaryLeg()) &&
              canUseLimb(getSecondaryFoot()) &&
              (canUseLimb(WEAR_WAISTE) || isFourLegged()));
    case LEG_PRIMARY:
      return (canUseLimb(getPrimaryLeg()) && 
              canUseLimb(getPrimaryFoot()) && 
              (canUseLimb(WEAR_WAISTE) || isFourLegged()));
    case LEG_SECONDARY_BACK:
      return (canUseLimb(WEAR_EX_LEG_R) &&
              canUseLimb(WEAR_EX_FOOT_R) &&
              (canUseLimb(WEAR_WAISTE) || isFourLegged()));
    case LEG_PRIMARY_BACK:
      return (canUseLimb(WEAR_EX_LEG_L) &&
              canUseLimb(WEAR_EX_FOOT_L) &&
              (canUseLimb(WEAR_WAISTE) || isFourLegged()));
  }
  return TRUE;
}

bool TBeing::canUseArm(primaryTypeT primary) const
{
  if (primary) 
    return (canUseLimb(getPrimaryArm()) &&
            canUseLimb(getPrimaryWrist()) &&
            canUseLimb(getPrimaryHand()) &&
            canUseLimb(getPrimaryHold()) &&
            canUseLimb(getPrimaryFinger()));
  else 
    return (canUseLimb(getSecondaryArm()) &&
            canUseLimb(getSecondaryWrist()) &&
            canUseLimb(getSecondaryHand()) &&
            canUseLimb(getSecondaryHold()) &&
            canUseLimb(getSecondaryFinger()));
}

bool TBeing::bothArmsHurt() const
{
  return (!canUseArm(HAND_PRIMARY) && !canUseArm(HAND_SECONDARY));
}

bool TBeing::canUseHand(bool primary) const
{
  if (primary) 
    return (canUseLimb(getPrimaryHand()) &&
            canUseLimb(getPrimaryHold()) &&
            canUseLimb(getPrimaryFinger()));
  else 
    return (canUseLimb(getSecondaryHand()) &&
            canUseLimb(getSecondaryHold()) &&
            canUseLimb(getSecondaryFinger()));
}

bool TBeing::bothHandsHurt() const
{
  return (!canUseHand(TRUE) && !canUseHand(FALSE));
}

void break_bone(TBeing *ch, wearSlotT slot)
{
  if (ch->hasPart(slot) && !ch->raceHasNoBones() &&
      !ch->isLimbFlags(slot, PART_BROKEN))
    ch->addToLimbFlags(slot, PART_BROKEN);
}

bool has_healthy_body(TBeing *ch)
{
  wearSlotT i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (ch->isLimbFlags(i, PART_BLEEDING | PART_INFECTED))
      return FALSE;
  }
  return TRUE;
}

bool TBeing::hasPart(wearSlotT part) const
{
  if (!slotChance(part))
    return FALSE;
  if (isLimbFlags(part, PART_MISSING))
    return FALSE;
  return TRUE;
}

int TBeing::getPosHeight() const
{
  int iHeight = getHeight();

  switch (getPosition()) {
    case POSITION_FIGHTING:
    case POSITION_STANDING:
      break;
    case POSITION_MOUNTED:
      if (riding) {
        iHeight = 2 * riding->getHeight()/ 3;
        iHeight += getHeight() /2;
      }
      break;
    case POSITION_CRAWLING:
    case POSITION_SITTING:
      if (riding) {
        // we'd like this to be based on height of chair, oh well
        iHeight *= 75;
        iHeight /= 100;
      } else {
        iHeight *= 4;
        iHeight /= 10;
      }
      break;
    case POSITION_RESTING:
    case POSITION_SLEEPING:
    default:  // this is all the dead/incap/etc
      if (riding) {
        // we'd like this to be based on height of chair, oh well
        iHeight *= 40;
        iHeight /= 100;
      } else {
        iHeight *= 15;
        iHeight /= 100;
      }
      break;
  }
  iHeight = max(1,iHeight);

  if (isLevitating())
    iHeight += 6;

  return iHeight;
}

void TBeing::makeBodyPart(wearSlotT pos, TBeing *opp)
{
  TCorpse *corpse;
  char buf[256];
  sstring sbuf;
  int v_vnum;
  TMonster *vmob;

  if ((vmob = dynamic_cast<TMonster *>(this)))
    v_vnum = vmob->number >= 0 ? mob_index[vmob->getMobIndex()].virt : -1;
  else
    v_vnum = -2;
    
  corpse = new TCorpse();
  sbuf = fmt("%s lost limb [%d]") % describeBodySlot(pos) % v_vnum;
  if (opp && opp->isPc())
    sbuf = fmt("%s lost limb [%s]") % sbuf % opp->getName();
  corpse->name = mud_str_dup(sbuf);
  
  if (getMaterial() > MAT_GEN_MINERAL) {
    // made of mineral or metal
    sprintf(buf, "the mangled %s of %s", 
          describeBodySlot(pos).c_str(), getName());
  } else {
    sprintf(buf, "the bloody, mangled %s of %s", 
          describeBodySlot(pos).c_str(), getName());
  }
  corpse->shortDescr = mud_str_dup(buf);

  if (getMaterial() > MAT_GEN_MINERAL) {
    // made of mineral or metal
    sprintf(buf, "The mangled, severed %s of %s is lying here.",
          describeBodySlot(pos).c_str(), getName());
  } else {
    sprintf(buf, "The bloody, mangled, severed %s of %s is lying here.",
          describeBodySlot(pos).c_str(), getName());
  }
  corpse->setDescr(mud_str_dup(buf));

  corpse->setStuff(NULL);
  corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
  corpse->addCorpseFlag(CORPSE_NO_REGEN);
  corpse->obj_flags.decay_time = 3 * (dynamic_cast<TMonster *>(this) ? MAX_NPC_CORPSE_TIME : MAX_PC_CORPSE_EMPTY_TIME);
  corpse->setWeight(getWeight() / 32.0);
  corpse->canBeSeen = canBeSeen;
  corpse->setVolume(getVolume() * slotChance(pos) / 100);
  corpse->setMaterial(getMaterial());

  act("$p goes flying through the air and bounces once before it rolls to a stop.",TRUE,this,corpse,0,TO_ROOM, ANSI_RED);
  *roomp += *corpse;
}

void TBeing::makeOtherPart(const char *single, const char *part, TBeing *opp)
{
  TCorpse *corpse;
  char buf[128];
  sstring sbuf;
  int v_vnum;
  TMonster *vmob;
  
  if ((vmob = dynamic_cast<TMonster *>(this)))
    v_vnum = vmob->number >= 0 ? mob_index[vmob->getMobIndex()].virt : -1;
  else
    v_vnum = -2;
    
  corpse = new TCorpse();
  sbuf = fmt("%s lost limb [%d]") % (single ? single : part) % v_vnum;
  if (opp && opp->isPc())
    sbuf = fmt("%s lost limb [%s]") % sbuf % opp->getName();
  corpse->name = mud_str_dup(sbuf);

  sprintf(buf, "%s's bloody %s", getName(),single ? single : part);
  corpse->shortDescr = mud_str_dup(buf);

  sprintf(buf, "%s's bloody and ichor coated %s %s here.",
          getName(), part ? part : single, part ? "lay" : "lies");
  corpse->setDescr(mud_str_dup(buf));
  corpse->setStuff(NULL);
  corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
  corpse->addCorpseFlag(CORPSE_NO_REGEN);
  corpse->setVolume(getHeight() * 122 / 100);
  corpse->setMaterial(getMaterial());

  corpse->obj_flags.decay_time = 3 * (dynamic_cast<TMonster *>(this) ? MAX_NPC_CORPSE_TIME : MAX_PC_CORPSE_EMPTY_TIME);
  corpse->setWeight(getWeight() / 64.0);
  *roomp += *corpse;
}

void TBeing::makeDiseasedPart(wearSlotT pos)
{
  TCorpse *corpse;
  char buf[256];
 
  corpse = new TCorpse();
  sprintf(buf, "%s lost limb", describeBodySlot(pos).c_str());
  corpse->name = mud_str_dup(buf);
  sprintf(buf, "the diseased %s of %s", describeBodySlot(pos).c_str(), getName())
;
  corpse->shortDescr = mud_str_dup(buf);

  sprintf(buf, "The diseased, puss-covered %s of %s is lying here.",
          describeBodySlot(pos).c_str(), getName());
  corpse->setDescr(mud_str_dup(buf));

  corpse->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD | ITEM_THROW;
  corpse->addCorpseFlag(CORPSE_NO_REGEN);
  corpse->obj_flags.decay_time = 15;
  corpse->setMaterial(getMaterial());
  corpse->setVolume(getHeight() * 122 * slotChance(pos) / 100);
  corpse->canBeSeen = canBeSeen;

  corpse->setWeight(getWeight() / 32.0);

  act("$p creaks once before falling to the $g and begins to decay.",
       TRUE,this,corpse,0,TO_ROOM);
  *roomp += *corpse;

  if (!isPc()) {
    // leprosy causing parts to go missing = cheap, easy kills
    double tExp = (getExp() / 2.0);

    // Make sure this mob is no longer deralict.
    if (!canUseArm(HAND_PRIMARY) && !canUseArm(HAND_SECONDARY))
      tExp = 0.0;

    setExp(tExp);
  }
}

bool TBeing::banished() const
{
  return isPlayerAction(PLR_BANISHED);
}

bool TBeing::isRightHanded() const
{
  return isPlayerAction(PLR_RT_HANDED);
}

bool TBeing::hasTransformedLimb() const
{
  wearSlotT slot;
  int found = FALSE;

  if (affectedBySpell(AFFECT_TRANSFORMED_ARMS) || 
      affectedBySpell(AFFECT_TRANSFORMED_HANDS) ||
      affectedBySpell(AFFECT_TRANSFORMED_LEGS) ||
      affectedBySpell(AFFECT_TRANSFORMED_HEAD) ||
      affectedBySpell(AFFECT_TRANSFORMED_NECK))
    found = TRUE;

  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    if (slot == HOLD_RIGHT || slot == HOLD_LEFT)
      continue;
    if (!slotChance(slot))
      continue;
    if (isLimbFlags(slot, PART_TRANSFORMED))
      found = TRUE;
    continue;
  }
  return found;
}

bool TBeing::isTransformableLimb(wearSlotT limb, int paired) 
{
  wearSlotT slot;

  if (paired) {
    for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
      if (slot == HOLD_RIGHT || slot == HOLD_LEFT)
        continue;
      if (!slotChance(slot))
        continue;
      if (isLimbFlags(slot, PART_MISSING | PART_PARALYZED | PART_BROKEN | PART_BLEEDING | PART_INFECTED | PART_USELESS | PART_LEPROSED | PART_TRANSFORMED)) { 
        switch (limb) {
          case WEAR_ARM_R:
          case WEAR_ARM_L:
            if ((slot == WEAR_ARM_R) || (slot == WEAR_ARM_L) || 
                (slot == WEAR_WRIST_R) ||(slot == WEAR_WRIST_L) || 
                (slot == WEAR_HAND_R) || (slot == WEAR_HAND_L) || 
                (slot == WEAR_FINGER_R) || (slot ==WEAR_FINGER_L)) {
              return FALSE;
            } else {
              break;
            }
          case WEAR_HAND_R:
          case WEAR_HAND_L:
            if ((slot ==  WEAR_HAND_R) || (slot == WEAR_HAND_L) || 
                (slot == WEAR_WRIST_R) || (slot == WEAR_WRIST_L) || 
                (slot == WEAR_FINGER_R) || (slot == WEAR_FINGER_L)) {
               return FALSE;
            } else {
              break;
            }
          case WEAR_LEGS_R:
          case WEAR_LEGS_L:
            if ((slot == WEAR_LEGS_R) || (slot == WEAR_LEGS_L) || 
                (slot == WEAR_FOOT_R) || (slot ==WEAR_FOOT_L)) {
              return FALSE;
            } else {
              break;
            }
          case WEAR_FOOT_R:
          case WEAR_FOOT_L:
            if ((slot == WEAR_FOOT_R) || (slot == WEAR_FOOT_L) || 
                (slot == WEAR_LEGS_R) || (slot == WEAR_LEGS_L)) {
              return FALSE;
            } else {
              break;
            }
          case WEAR_HEAD:
            if ((slot == WEAR_HEAD) || (slot == WEAR_NECK)) {
              return FALSE;
            } else {
              break;
            }
          case WEAR_NECK:
            if ((slot == WEAR_NECK)){
              return FALSE;
            } else {
              break;
            }
          case WEAR_WAISTE:
            if (slot == WEAR_WAISTE) {
              return FALSE;
            } else {
              break;
            }
          default:
            break;
          }
        continue;  
        }
    } 
    return TRUE; 
  } else {
    vlogf(LOG_BUG, "isTransformable called unpaired.");
    return FALSE;
  }
}

bool TBeing::hasLegs() const
{
  if (!slotChance(WEAR_LEGS_L) &&
       (!slotChance(WEAR_LEGS_R)) &&
       (!slotChance(WEAR_FOOT_L)) &&
       (!slotChance(WEAR_FOOT_R)) &&
       (!slotChance(WEAR_EX_LEG_R)) &&
       (!slotChance(WEAR_EX_LEG_L)) &&
       (!slotChance(WEAR_EX_FOOT_R)) &&
       (!slotChance(WEAR_EX_FOOT_R)))
    return FALSE;

  return TRUE;
}

bool TBeing::hasHands() const
{
  // transformed into bear-claws(hand) or eagles-wings(arm) 
  if (isLimbFlags(WEAR_HAND_R, PART_TRANSFORMED) ||
      isLimbFlags(WEAR_HAND_L, PART_TRANSFORMED))
    return false;

  body_t bod = getMyRace()->getBodyType();
  return (isHumanoid() ||
          isUndead() ||
          isLycanthrope() ||
          isDiabolic() ||
          bod == BODY_FROGMAN ||
          bod == BODY_CENTAUR ||
          bod == BODY_SIMAL) ;
}

const sstring TBeing::describeTransBodySlot(wearSlotT i) const
{
  mud_assert(i >= MIN_WEAR && i < MAX_WEAR, 
         "Bad limb slot, %s %d", getName(), i);

  if (!slotChance(i)) {
    vlogf(LOG_BUG, "There is a race problem in describeTransBodySlot");
    return "bogus body part";
  }
  switch (i) {
    case WEAR_FINGER_R:
      if (isLimbFlags(WEAR_HAND_R, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
          return "right wing";
        } else {
          return "right paw";
        }
      } else {
        vlogf(LOG_BUG, fmt("There is a bad case 1 in describeTransBodySlot, %s") %  getName());
        return "bogus transformed body part";
      } 
    case WEAR_FINGER_L:
      if (isLimbFlags(WEAR_HAND_L, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
          return "left wing";
        } else {
          return "left paw";
        }
      } else {
        vlogf(LOG_BUG, fmt("There is a bad case 2 in describeTransBodySlot, %s") %  getName());

        return "bogus transformed body part";
      }
    case WEAR_NECK:
      if (isLimbFlags(WEAR_HEAD, PART_TRANSFORMED)) {
        return "eagle's neck";
      } else {
        return "gills";
      }
    case WEAR_BODY:
        vlogf(LOG_BUG, fmt("There is a bad case 3 in describeTransBodySlot, %s") %  getName());

      return "bogus transformed body part";
    case WEAR_HEAD:
      return "eagle's head";
    case WEAR_LEGS_L:
    case WEAR_FOOT_L:
    case WEAR_LEGS_R:
    case WEAR_FOOT_R:
      return "dolphin's tail";
    case WEAR_HAND_R:
      if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
        return "right wing";
      } else {
        return "right paw";
      }
    case WEAR_HAND_L:
      if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
        return "left wing";
      } else {
        return "left paw";
      }
    case WEAR_ARM_R:
      return "right wing";
    case WEAR_ARM_L:
      return "left wing";
    case WEAR_BACK:
        vlogf(LOG_BUG, fmt("There is a bad case 4 in describeTransBodySlot, %s") %  getName());

      return "bogus transformed body part";
    case WEAR_WAISTE:
        vlogf(LOG_BUG, fmt("There is a bad case 5 in describeTransBodySlot, %s") %  getName());

      return "bogus transformed body part";
    case WEAR_WRIST_R:
      if (isLimbFlags(WEAR_HAND_R, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
          return "right wing";
        } else {
          return "right paw";
        }
      } else {
        vlogf(LOG_BUG, fmt("There is a bad case 6 in describeTransBodySlot, %s") %  getName());

        return "bogus transformed body part";
      }
    case WEAR_WRIST_L:
      if (isLimbFlags(WEAR_HAND_L, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
          return "left wing";
        } else {
          return "left paw";
        }
      } else {
        vlogf(LOG_BUG, fmt("There is a bad case 7 in describeTransBodySlot, %s") %  getName());

        return "bogus transformed body part";
      }
    case HOLD_RIGHT:
      if (isLimbFlags(WEAR_HAND_R, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
          return "right wing";
        } else {
          return "right paw";
        }
      } else {
        vlogf(LOG_BUG, "Something called HOLD_RIGHT in describeTransBodySlot");
        return "bogus transformed body part";
      }
    case HOLD_LEFT:
      if (isLimbFlags(WEAR_HAND_L, PART_TRANSFORMED)) {
        if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
          return "left wing";
        } else {
          return "left paw";
        }
      } else {
        vlogf(LOG_BUG, "Something called HOLD_LEFT in describeTransBodySlot");
        return "bogus transformed body part";
      }
    case WEAR_EX_LEG_R:
        vlogf(LOG_BUG, fmt("There is a bad case 10 in describeTransBodySlot, %s") %  getName());

      return "bogus transformed body part";
    case WEAR_EX_LEG_L:
        vlogf(LOG_BUG, fmt("There is a bad case 11 in describeTransBodySlot, %s") %  getName());
      return "bogus transformed body part";
    case WEAR_EX_FOOT_R:
        vlogf(LOG_BUG, fmt("There is a bad case 12 in describeTransBodySlot, %s") %  getName());

      return "bogus transformed body part";
    case WEAR_EX_FOOT_L:
        vlogf(LOG_BUG, fmt("There is a bad case 13 in describeTransBodySlot, %s") %  getName());
      return "bogus transformed body part";
    default:
        vlogf(LOG_BUG, fmt("There is a bad case 14 in describeTransBodySlot, %s") %  getName());
      return "bogus transformed body slot-part";
  }
}

int TBeing::shouldDescTransLimb(wearSlotT i) const
{
  switch (i) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_BODY:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
    case WEAR_LEGS_L:
    case WEAR_BACK:
    case WEAR_WAISTE:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case HOLD_RIGHT:
    case HOLD_LEFT:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return FALSE;
    case WEAR_NECK:
    case WEAR_LEGS_R:
    case WEAR_HEAD:
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return TRUE;
    case WEAR_HAND_R:
      if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
        return FALSE;
     }
     return TRUE;
    case WEAR_HAND_L:
      if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
        return FALSE;
     }
      return TRUE;
    default:
      vlogf(LOG_BUG, "There is a bad case in shouldDescTransLimb");
      return FALSE;
  }
}

const sstring TBeing::describeTransLimb(wearSlotT i) const
{
  if (!slotChance(i)) {
    vlogf(LOG_BUG, "There is a race problem in describeTransLimb");
    return "worn on bogus racial transformed body slot";
  }
  switch (i) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_BODY:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
    case WEAR_LEGS_L:
    case WEAR_BACK:
    case WEAR_WAISTE:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case HOLD_RIGHT:
    case HOLD_LEFT:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return "Bogus-Tranformed Slot-Bug Cosmo";
    case WEAR_NECK:
      if (isLimbFlags(WEAR_HEAD, PART_TRANSFORMED)) {
        return "In place of a neck, you see the base of an eagles's head.";
      }
      return "In place of a neck, you see a set of gills";
    case WEAR_HEAD:
      return "Sitting on the neck is an eagle's head";
    case WEAR_LEGS_R:
      return "In place of legs you see the tail fins of a dolphin";
    case WEAR_HAND_R:
      if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
        return "Bogus-Tranformed Slot-Bug Cosmo";
     }
     return "In place of a hand, you see a bear's paw";
    case WEAR_HAND_L:
      if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
        return "Bogus-Tranformed Slot-Bug Cosmo"; 
     }
      return "In place of a hand, you see a bear's paw";
    case WEAR_ARM_R:
      return "In place of an arm, you see a wing";
    case WEAR_ARM_L:
     return "In place of an arm, you see a wing";
    default:
      vlogf(LOG_BUG, "There is a bad case in describeTransLimb");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
  }
}

const sstring TBeing::describeTransEquipSlot(wearSlotT i) const
{
  if (!slotChance(i)) {
    vlogf(LOG_BUG, "There is a race problem in describeTransEquipSlot");
    return "worn on bogus racial transformed body slot";
  }
  switch (i) {
    case WEAR_FINGER_R:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_FINGER_L:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_NECK:
      if (isLimbFlags(WEAR_HEAD, PART_TRANSFORMED)) {
        return "In place of a neck, you see the base of an eagles's head.";
      }
      return "In place of a neck, you see a set of gills";
    case WEAR_HEAD:
        return "Sitting on the neck is an eagle's head.";
    case WEAR_BODY:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_LEGS_R:
        return "In place of a legs, you see the tail fins of a dolphin";
    case WEAR_LEGS_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_HAND_R:
     if (isLimbFlags(WEAR_ARM_R, PART_TRANSFORMED)) {
      vlogf(LOG_BUG, "There is a bad case in hand in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
     }
     return "In place of a hand, you see a bear's paw"; 
    case WEAR_HAND_L:
     if (isLimbFlags(WEAR_ARM_L, PART_TRANSFORMED)) {
      vlogf(LOG_BUG, "There is a bad case in hand in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
     }
     return "In place of a hand, you see a bear's paw";     
    case WEAR_ARM_R:
     return "In place of an arm, you see a wing";     
    case WEAR_ARM_L:
     return "In place of an arm, you see a wing"; 
    case WEAR_BACK:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_WAISTE:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_WRIST_R:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_WRIST_L:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case HOLD_RIGHT:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case HOLD_LEFT:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_EX_LEG_R:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_EX_LEG_L:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_EX_FOOT_R:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    case WEAR_EX_FOOT_L:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
    default:
      vlogf(LOG_BUG, "There is a bad case in describeTransEquipSlot");
      return "Worn on BOGUS Transformed slot -- bug Cosmo";
  }

}

// if hitter is NULL, we can hit anywhere on body  (ranged combat)
wearSlotT TBeing::getPartHit(TBeing *hitter, bool allowHold)
{
  int d, ct = 0;
  wearSlotT i;
  int tot = 0;
  ubyte real_slot_chance[MAX_WEAR];

  for (i = MIN_WEAR; i < MAX_WEAR; i++)
    real_slot_chance[i] = slotChance(i);

#if 0
  // this compar stuff disallows some slots based on height
  if (hitter) {
    double compar = (double) hitter->getPosHeight() / (double) getPosHeight();
    
    if (compar > 1.5) {
      real_slot_chance[WEAR_FOOT_L] = 0;
      real_slot_chance[WEAR_FOOT_R] = 0;
      real_slot_chance[WEAR_LEGS_R] = 0;
      real_slot_chance[WEAR_LEGS_L] = 0;
      real_slot_chance[WEAR_EX_LEG_R] = 0;
      real_slot_chance[WEAR_EX_LEG_L] = 0;
      real_slot_chance[WEAR_EX_FOOT_R] = 0;
      real_slot_chance[WEAR_EX_FOOT_L] = 0;
    } else if (compar > 1.2) {
      real_slot_chance[WEAR_FOOT_L] = 0;
      real_slot_chance[WEAR_FOOT_R] = 0;
      real_slot_chance[WEAR_EX_FOOT_R] = 0;
      real_slot_chance[WEAR_EX_FOOT_L] = 0;
    } else if (compar < 0.1) {
      // his hands are hitting me, but arms are too far away
      real_slot_chance[WEAR_HEAD] = 0;
      real_slot_chance[WEAR_NECK] = 0;
      real_slot_chance[WEAR_BODY] = 0;
      real_slot_chance[WEAR_BACK] = 0;
      real_slot_chance[WEAR_WAISTE] = 0;
      real_slot_chance[WEAR_LEGS_R] = 0;
      real_slot_chance[WEAR_LEGS_L] = 0;
      real_slot_chance[WEAR_ARM_R] = 0;
      real_slot_chance[WEAR_ARM_L] = 0;
    } else if (compar < 0.2) {
      real_slot_chance[WEAR_HEAD] = 0;
      real_slot_chance[WEAR_NECK] = 0;
      real_slot_chance[WEAR_BODY] = 0;
      real_slot_chance[WEAR_BACK] = 0;
      real_slot_chance[WEAR_WAISTE] = 0;
      real_slot_chance[WEAR_ARM_R] = 0;
      real_slot_chance[WEAR_ARM_L] = 0;
    } else if (compar < 0.4) {
      real_slot_chance[WEAR_HEAD] = 0;
      real_slot_chance[WEAR_NECK] = 0;
      real_slot_chance[WEAR_BODY] = 0;
      real_slot_chance[WEAR_BACK] = 0;
    } else if (compar < 0.7) {
      real_slot_chance[WEAR_HEAD] = 0;
      real_slot_chance[WEAR_NECK] = 0;
    }
  }
#endif
  tot = 0;
  for (i = MIN_WEAR;i < MAX_WEAR;i++) {
    if (!allowHold && (i == HOLD_RIGHT || i == HOLD_LEFT))
      continue;

    tot += real_slot_chance[i];
  }

  d = dice(1, tot);

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!allowHold && (i == HOLD_RIGHT || i == HOLD_LEFT))
      continue;

    if (d <= (ct += real_slot_chance[i])) 
      return i;
  }
  vlogf(LOG_BUG, fmt("Warning!  get_part_hit error on %s.") %  getName());
  return WEAR_BODY;
}

wearSlotT TBeing::getCritPartHit()
{
  wearSlotT part_hit = WEAR_BODY;
  switch (::number(0, 5)) {
    case 0:
      if (hasPart(WEAR_HEAD))
        part_hit = WEAR_HEAD;

      break;
    case 1:
      if (hasPart(WEAR_NECK))
        part_hit = WEAR_NECK;

      break;
    case 2:
      if (hasPart(WEAR_BACK))
        part_hit = WEAR_BACK;

      break;
    default:
      part_hit = WEAR_BODY;
      break;
  }
  return part_hit;
}

int TBeing::getPartMinHeight(int part) const
{
  int midline;

  // getPosHeight takes into account riding where appropriate
  // so what we want to do is take this height, adjust it up/down 
  // based on what they are sitting on
  // and then figure out where the limb is in relation
  int hgt = getPosHeight();

  if (riding) {
    // set midline to be where seat is
    midline = riding->getRiderHeight();

    switch (part) {
      case ITEM_WEAR_HEAD:
        return max(0, midline + (40 * hgt/100));
      case ITEM_WEAR_NECK:
        return max(0, midline + (35 * hgt/100));
      case ITEM_WEAR_ARMS:
        return max(0, midline + (27 * hgt/100));
      case ITEM_WEAR_BODY:
      case ITEM_WEAR_BACK:
        return max(0, midline + (5 * hgt/100));
      case ITEM_WEAR_WAISTE:
        return max(0, midline - (0 * hgt/100));
      case ITEM_WEAR_LEGS:
        return max(0, midline - (40 * hgt/100));
      case ITEM_WEAR_FEET:
        return max(0, midline - (50 * hgt/100));
      default:
        vlogf(LOG_BUG, fmt("Bogus part %d in getPartMinHeight()") %  part);
        return 0;
    }
  } else {
    switch (part) {
      case ITEM_WEAR_HEAD:
        return max(0, (90 * hgt/100));
      case ITEM_WEAR_NECK:
        return max(0, (85 * hgt/100));
      case ITEM_WEAR_ARMS:
        return max(0, (77 * hgt/100));
      case ITEM_WEAR_BODY:
      case ITEM_WEAR_BACK:
        return max(0, (55 * hgt/100));
      case ITEM_WEAR_WAISTE:
        return max(0, (50 * hgt/100));
      case ITEM_WEAR_LEGS:
        return max(0, (10 * hgt/100));
      case ITEM_WEAR_FEET:
        return 0;
      default:
        vlogf(LOG_BUG, fmt("Bogus part %d in getPartMinHeight()") %  part);
        return 0;
    }
  }
}

bool isCritPart(wearSlotT part_hit)
{
  switch (part_hit) {
    case WEAR_HEAD:
    case WEAR_NECK:
    case WEAR_BACK:
    case WEAR_BODY:
      return true;
    default:
      return false;
  }
}
