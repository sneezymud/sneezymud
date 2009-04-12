//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1996, SneezyMUD Coding Team
// All rights reserved.
//
// "materials.cc" - Various functions related to materials.
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "materials.h"
#include "room.h"
#include "statistics.h"
#include "obj_base_weapon.h"

static bool genericDamCheck(int susc, int sharp)
{
#if 0
  // gold_repair is taken into account in the cost to repair stuff formula
  // this is probably obsolete - 9/30/99
  int chance = 1000 - (int) (1000 * gold_modifier[GOLD_REPAIR].getVal());
  if (::number(0,999) < chance)
    return false;
#endif

  if (::number(0,999) >= 300)
    return false;

  // num is the hardness of what i a hitting
  // susc is the hardness of the hitter
  // for the NPC vs PC situation, sharp comes from sharpness[] which
  // we can roughly figure is "30"
  // we should shoot for the randomness being roughly what the susc would
  // be : implying (num + susc) > number(30,130)
  // but make the chance fairly high and let the global modifier above
  // suck up the slack to make economy come out right
  // the min(30, sharp) case is to keep PC v NPC from shredding equipment
  if (sharp && susc && ((min(30, sharp) + susc) > ::number(20, 120)))
    return TRUE;

  return false;
}

bool TObj::willDent(int num)
{
  int susc = material_nums[getMaterial()].smash_susc;
  return genericDamCheck(susc, num);
}

bool TObj::willTear(int num)
{
  int susc = material_nums[getMaterial()].cut_susc;
  return genericDamCheck(susc, num);
}

bool TObj::willPuncture(int num)
{
  int susc = material_nums[getMaterial()].pierced_susc;
  return genericDamCheck(susc, num);
}

// item should be the item being dented (victim's)
// slot should be the bodyslot that is doing the denting (caster's)
int TBeing::dentItem(TBeing *victim, TObj *item, int amt, int slot)
{
  char buf[256];
  int hardness;

  // Don't damage equipment in arena fights.
  if (victim->roomp && victim->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  // use hardness of eq on slot, or use skintype for hardness
  hardness = equipment[slot] ? 
     material_nums[(equipment[slot])->getMaterial()].hardness :
     material_nums[getMaterial((wearSlotT)slot)].hardness;

  if (item->willDent(hardness)) {
    if (item->isMineral()) {
      if (item->thingHolding() == this) {
        act("You chip your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s chipped.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You chip $S $o.", TRUE, this, item, victim, TO_CHAR);
        sprintf(buf, "Your $o %s chipped.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s chipped.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    } else {
      if (item->thingHolding() == this) {
        act("You dent your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s dented.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You dent $S $o.", TRUE, this, item, victim, TO_CHAR);

        sprintf(buf, "Your $o %s dented.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s dented.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    }
    if(IS_SET_DELETE(item->damageItem(amt), DELETE_THIS))
      return DELETE_ITEM;
    return TRUE;
  }
  return FALSE;
}

// item should be the item being dented (victim's)
// slot should be the bodyslot that is doing the denting (caster's)
int TBeing::tearItem(TBeing *victim, TObj *item, int amt, int slot)
{
  char buf[256];
  int sharp = 0;

  // Don't damage equipment in arena fights.
  if (victim->roomp && victim->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  // use hardness of eq on slot, or use skintype for hardness
  sharp = (equipment[slot] && !dynamic_cast<TBaseWeapon *>(equipment[slot])) ? 
     material_nums[(equipment[slot])->getMaterial()].hardness :
     material_nums[getMaterial((wearSlotT)slot)].hardness;

  if (item->willTear(sharp)) {
    if (item->isMineral()) {
      if (item->thingHolding() == this) {
        act("You scratch your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s scratched.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You scratch $S $o.", TRUE, this, item, victim, TO_CHAR);
   
        sprintf(buf, "Your $o %s scratched.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s scratched.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    } else {
      if (item->thingHolding() == this) {
        act("You tear your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s torn.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You tear $S $o.", TRUE, this, item, victim, TO_CHAR);
 
        sprintf(buf, "Your $o %s torn.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s torn.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    }
    if(IS_SET_DELETE(item->damageItem(amt), DELETE_THIS))
      return DELETE_ITEM;
    return TRUE;
  }
  return FALSE;
}

// item should be the item being dented (victim's)
// slot should be the bodyslot that is doing the denting (caster's)
int TBeing::pierceItem(TBeing *victim, TObj *item, int amt, int slot)
{
  char buf[256];
  int sharp;

  // Don't damage equipment in arena fights.
  if (victim->roomp && victim->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  // use hardness of eq on slot, or use skintype for hardness
  sharp = (equipment[slot] && !dynamic_cast<TBaseWeapon *>(equipment[slot])) ? 
     material_nums[(equipment[slot])->getMaterial()].hardness :
     material_nums[getMaterial((wearSlotT)slot)].hardness;

  if (item->willPuncture(sharp)) {
    if (item->isMineral()) {
      if (item->thingHolding() == this) {
        act("You crack your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s cracked.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You crack $S $o.", TRUE, this, item, victim, TO_CHAR);
 
        sprintf(buf, "Your $o %s cracked.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s cracked.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    } else {
      if (item->thingHolding() == this) {
        act("You puncture your $o.", TRUE, this, item, 0, TO_CHAR);
        sprintf(buf, "$n's $o %s punctured.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, 0, TO_ROOM);
      } else {
        act("You puncture $S $o.", TRUE, this, item, victim, TO_CHAR);
 
        sprintf(buf, "Your $o %s punctured.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_VICT, ANSI_RED);
        sprintf(buf, "$N's $o %s punctured.", item->isPaired() ? "are" : "is");
        act(buf, TRUE, this, item, victim, TO_NOTVICT);
      }
    }
    if(IS_SET_DELETE(item->damageItem(amt), DELETE_THIS))
      return DELETE_ITEM;
    return TRUE;
  }
  return FALSE;
}

bool TThing::isMetal() const
{
  ubyte mat = convertV9MaterialToV10(getMaterial());

  switch (mat) {
    case MAT_COPPER:
    case MAT_BRONZE:
    case MAT_BRASS:
    case MAT_IRON:
    case MAT_STEEL:
    case MAT_SILVER:
    case MAT_GOLD:
    case MAT_PLATINUM:
    case MAT_TITANIUM:
    case MAT_MITHRIL:
    case MAT_ALUMINUM:
    case MAT_ELECTRUM:
    case MAT_ATHANOR:
    case MAT_TIN:
    case MAT_TERBIUM:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TThing::isMineral() const
{
  ubyte mat = convertV9MaterialToV10(getMaterial());

  switch (mat) {
    case MAT_GLASS:
    case MAT_PORCELAIN:
    case MAT_GEN_MINERAL:
    case MAT_RUNED:
    case MAT_CRYSTAL:
    case MAT_DIAMOND:
    case MAT_EBONY:
    case MAT_EMERALD:
    case MAT_IVORY:
    case MAT_OBSIDIAN:
    case MAT_ONYX:
    case MAT_OPAL:
    case MAT_RUBY:
    case MAT_SAPPHIRE:
    case MAT_MARBLE:
    case MAT_STONE:
    case MAT_BONE:
    case MAT_JADE:
    case MAT_AMBER:
    case MAT_TURQUOISE:
    case MAT_AMETHYST:
    case MAT_MICA:
    case MAT_DRAGONBONE:
    case MAT_MALACHITE:
    case MAT_GRANITE:
    case MAT_ADAMANTITE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TThing::isOrganic() const
{
  ubyte mat = convertV9MaterialToV10(getMaterial());

  switch (mat) {
    case MAT_PAPER:
    case MAT_CLOTH:
    case MAT_WAX:
    case MAT_WOOD:
    case MAT_SILK:
    case MAT_FOODSTUFF:
    case MAT_RUBBER:
    case MAT_CARDBOARD:
    case MAT_STRING:
    case MAT_TOUGH_CLOTH:
    case MAT_CORAL:
    case MAT_HORSEHAIR:
    case MAT_HAIR:
    case MAT_GEN_ORG:
    case MAT_LEATHER:
    case MAT_TOUGH_LEATHER:
    case MAT_DRAGON_SCALE:
    case MAT_WOOL:
    case MAT_FUR:
    case MAT_FEATHERED:
    case MAT_STRAW:
    case MAT_PEARL:
    case MAT_HUMAN_FLESH:
    case MAT_FUR_CAT:
    case MAT_FUR_DOG:
    case MAT_FUR_RABBIT:
    case MAT_GHOSTLY:
    case MAT_DWARF_LEATHER:
    case MAT_SOFT_LEATHER:
    case MAT_FISHSCALE:
    case MAT_OGRE_HIDE:
    case MAT_HEMP:
    case MAT_BONE:
    case MAT_IVORY:
    case MAT_DRAGONBONE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TObj::canRust()
{
  ubyte mat = convertV9MaterialToV10(getMaterial());

  if (material_nums[mat].acid_susc <= 0)
    return FALSE;
  if (!isMetal())
    return FALSE;
  
  switch (mat) {
    case MAT_IRON:
    case MAT_STEEL:
      return TRUE;
    default:
      return FALSE;
  }
 
  return FALSE;
}

ubyte convertV9MaterialToV10(ubyte oldMat)
{
  switch (oldMat)
  {
  case 101: // MAT_JEWELED
    return MAT_GEN_MINERAL;

  case 150: // MAT_GEN_METAL
  case 152: // MAT_SCALE_MAIL
  case 153: // MAT_BANDED_MAIL
  case 154: // MAT_CHAIN_MAIL
  case 155: // MAT_PLATE
  case 167: // MAT_RINGMAIL
  case 168: // MAT_GNOMEMAIL
    return MAT_STEEL;

  case 175: // MAT_ELVENMAIL
  case 176: // MAT_ELVENSTEEL
    return MAT_MITHRIL;

  default:
    break;
  }
  return oldMat;
}


