#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "shop.h"
#include "shopowned.h"
#include "liquids.h"
#include "spells.h"
#include "toggle.h"
#include "extern.h"
#include "being.h"
#include "materials.h"

TGenWeapon::TGenWeapon() : TBaseWeapon() {
  for (int i = 0; i < 3; ++i) {
    weapon_type[i] = WEAPON_TYPE_NONE;
    wtype_frequency[i] = 0;
  }
}

TGenWeapon::TGenWeapon(const TGenWeapon& a) : TBaseWeapon(a) {
  for (int i = 0; i < 3; ++i) {
    weapon_type[i] = a.weapon_type[i];
    wtype_frequency[i] = a.wtype_frequency[i];
  }
}

TGenWeapon& TGenWeapon::operator=(const TGenWeapon& a) {
  if (this == &a)
    return *this;
  TBaseWeapon::operator=(a);
  for (int i = 0; i < 3; ++i) {
    weapon_type[i] = a.weapon_type[i];
    wtype_frequency[i] = a.wtype_frequency[i];
  }
  return *this;
}

TGenWeapon::~TGenWeapon() {}

void TGenWeapon::assignFourValues(int x1, int x2, int x3, int x4) {
  TBaseWeapon::assignFourValues(x1, x2, x3, x4);

  setWeaponType((weaponT)GET_BITS(x3, 7, 8), 0);
  setWeaponFreq(GET_BITS(x3, 15, 8), 0);

  setWeaponType((weaponT)GET_BITS(x3, 23, 8), 1);
  setWeaponFreq(GET_BITS(x3, 31, 8), 1);

  setWeaponType((weaponT)GET_BITS(x4, 7, 8), 2);
  setWeaponFreq(GET_BITS(x4, 15, 8), 2);
}

void TGenWeapon::getFourValues(int* x1, int* x2, int* x3, int* x4) const {
  int x = 0;

  TBaseWeapon::getFourValues(x1, x2, x3, x4);

  SET_BITS(x, 7, 8, getWeaponType(0));
  SET_BITS(x, 15, 8, getWeaponFreq(0));
  SET_BITS(x, 23, 8, getWeaponType(1));
  SET_BITS(x, 31, 8, getWeaponFreq(1));
  *x3 = x;
  SET_BITS(x, 7, 8, getWeaponType(2));
  SET_BITS(x, 15, 8, getWeaponFreq(2));
  *x4 = x;
}

weaponT TGenWeapon::getWeaponType(int which) const {
  if (which >= 0 && which < static_cast<int>(cElements(weapon_type))) {
    return weapon_type[which];
  } else {  // return a random type
    int c = ::number(0, getWeaponFreq(0) + getWeaponFreq(1) + getWeaponFreq(2));

    for (int i = 0; i < 3; ++i) {
      c -= getWeaponFreq(i);
      if (c <= 0)
        return weapon_type[i];
    }
  }

  return WEAPON_TYPE_NONE;
}

int TGenWeapon::getWeaponFreq(int which) const {
  return wtype_frequency[which];
}

void TGenWeapon::setWeaponType(weaponT n, int which) { weapon_type[which] = n; }

void TGenWeapon::setWeaponFreq(int n, int which) { wtype_frequency[which] = n; }

sstring TGenWeapon::statObjInfo() const {
  sstring a = "";

  a += format("Current %-11s %-7d  Damage Level:     %d\n\r") %
       ((isBluntWeapon() ? "bluntness:"
                         : (isPierceWeapon() ? "pointiness:" : "sharpness:"))) %
       getCurSharp() % (int)(getWeapDamLvl() / 4.0);
  a += format("Maximum %-11s %-7d  Damage Deviation: %d\n\r") %
       ((isBluntWeapon() ? "bluntness:"
                         : (isPierceWeapon() ? "pointiness:" : "sharpness:"))) %
       getMaxSharp() % getWeapDamDev();

  double base = baseDamage();
  double flux = base * getWeapDamDev() / 10;
  sstring buf =
    format("%d-%d") % (int)(base - (int)flux) % (int)(base + (int)flux);
  a += format("Damage When Swung:  %-7s  Average Damage:   %d\n\r") % buf %
       (int)baseDamage();

  a += format("Damage When Thrown: %d\n\r") % (int)damageLevel();

  for (int wt = 0; wt < 3; ++wt) {
    if (!getWeaponType(wt))
      continue;
    if (toggleInfo[TOG_TWINK]->toggle) {
      a += format("Attack Type:        %-8s") %
           attack_hit_text_twink[getWtype(wt) - TYPE_MIN_HIT].singular;
    } else {
      a += format("Attack Type:        %-8s") %
           attack_hit_text[getWtype(wt) - TYPE_MIN_HIT].singular;
    }
    if (getWeaponFreq(wt))
      a += format(" Attack Frequency:  %d%%") % getWeaponFreq(wt);
    a += "\n\r";
  }
  if (isPoisoned()) {
    a += format("Poisoned with:      %s\n\r") % liquidInfo[getPoison()]->name;
  }
  return a;
}

float TGenWeapon::blowCountSplitter(const TBeing*, bool) const { return 1.0; }

float TThing::blowCountSplitter(const TBeing*, bool) const { return 0.0; }

void TGenWeapon::lowCheck() {
  if ((int)getWeight() < 1)
    vlogf(LOG_LOW, format("weapon %s has a bad weight set.") % getName());

  if ((getVolume() <= 800) && (getWeight() < 3))
    if (!canWear(ITEM_WEAR_THROW) && !dynamic_cast<TGun*>(this))
      vlogf(LOG_LOW,
        format("weapon %s probably needs to be set throwable.") % getName());

  if (getWeaponType() == WEAPON_TYPE_NONE)
    vlogf(LOG_LOW, format("weapon %s needs a weapon_type defined") % getName());

  if (!isBluntWeapon() && !isSlashWeapon() && !isPierceWeapon())
    vlogf(LOG_LOW, format("weapon %s has bogus type apparently.") % getName());

  TBaseWeapon::lowCheck();
}

bool TGenWeapon::sellMeCheck(TBeing* ch, TMonster* keeper, int, int) const {
  return TBaseWeapon::sellMeCheck(ch, keeper, 1, 10);
}

bool TGenWeapon::canCudgel() const {
  return isBluntWeapon() && getVolume() <= 1500;
}

bool TGenWeapon::canBackstab() const {
  return isPierceWeapon() && getVolume() <= 1500;
}

bool TGenWeapon::canStab() const {
  return isPierceWeapon() && getVolume() <= 2000;
}

bool TGenWeapon::hasSpikes() const { return isObjStat(ITEM_SPIKED); }

int thornsHit(TBeing* victim, TBeing* ch, wearSlotT chLimb, wearSlotT vicLimb) {
  // Check for valid parameters
  if (!victim || vicLimb == WEAR_NOWHERE || !ch->affectedBySpell(SPELL_THORNFLESH)) {
    return false;
  }

  if (victim->isLimbFlags(vicLimb, PART_MISSING)) {
    return false;
  }

  if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED, vicLimb)) {
    // No bleeding for undead or immune creatures
    return false;
  }
  // Check for victim's limb hardness against attacker's limb hardness
  int vicLimbHardness = material_nums[victim->getMaterial(vicLimb)].hardness;
  
  // Access wood hardness directly from the material_nums array
  // This matches how describeMaterial accesses hardness
  int chLimbHardness = material_nums[MAT_WOOD].hardness;
  
  // If character has Iron Flesh skill, it can enhance the thorns' effectiveness
  if (ch->doesKnowSkill(SKILL_IRON_FLESH)) {
    int ironFleshHardness = (ch->getSkillValue(SKILL_IRON_FLESH) * 85/100);
    if (ironFleshHardness > chLimbHardness) {
      chLimbHardness = ironFleshHardness;
    }
  }
  
  // Calculate chance based on hardness difference
  int hardnessDiff = chLimbHardness - vicLimbHardness;
  
  // Only proceed if attacker's hardness is higher AND random check passes
  if (hardnessDiff <= 0) {
    return false;
  }
  
  // Use hardness difference as percentage chance
  if (!::percentChance(hardnessDiff)) {
    return false;
  }
  
  if (victim->isLimbFlags(vicLimb, PART_BLEEDING)) {
    act(format("Blood spatters as the thorns on your %s sink into $N's bleeding %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_CHAR);
    act(format("Blood spatters as the thorns on $n's %s sink into $N's bleeding %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_NOTVICT);
    act(format("Blood spatters as the thorns on $n's %s sink into your bleeding %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_VICT);
    
    // Increment the bleed stack
    victim->incrementBleedStack(vicLimb, 250);
  } else {   
    act(format("The thorns on your %s tear open a <R>bloody wound<1> in $N's %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_CHAR);
    act(format("The thorns on $n's %s tear open a <R>bloody wound<1> in $N's %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_NOTVICT);
    act(format("The thorns on $n's %s tear open a <R>bloody wound<1> in your %s!") % 
        ch->describeBodySlot(chLimb) % victim->describeBodySlot(vicLimb), 
        FALSE, ch, nullptr, victim, TO_VICT);
     
    victim->rawBleed(vicLimb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  }
  
  return true;
}

int hardHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT vicLimb, wearSlotT chLimb) {
  // Get the attacker's weapon/limb and victim's equipment/limb
  TObj *weap = obj;
  TObj *vicEq = dynamic_cast<TObj*>(victim->equipment[vicLimb]);
  
  // Initialize hardness values
  int vicHard = 0;
  int weapHard = 0;
  
  // Calculate victim's hardness
  if (vicEq) {
    vicHard = material_nums[vicEq->getMaterial()].hardness;
  } else {
    vicHard = material_nums[victim->getMaterial(vicLimb)].hardness;
  }
  
  // Calculate attacker's hardness
  if (weap) {
    weapHard = material_nums[weap->getMaterial()].hardness;
  } else {
    // No weapon - use body part hardness
    weapHard = material_nums[ch->getMaterial(chLimb)].hardness;
    
    // Apply Iron Flesh skill if character has it
    if (ch->doesKnowSkill(SKILL_IRON_FLESH)) {
      int ironFleshHardness = (ch->getSkillValue(SKILL_IRON_FLESH) * 85/100);
      if (ironFleshHardness > weapHard) {
        weapHard = ironFleshHardness;
      }
    }
  }
  
  // Calculate chance based on hardness difference
  int hardChance = (weapHard - vicHard);
  
  // Only proceed if attacker's hardness is higher AND random check passes
  // Also skip if victim is tough
  if ((percentChance(hardChance)) && !victim->isTough()) {
    int eqDamage = weapHard - vicHard;
    
    // Case 1: Weapon hits victim's equipment
    if (weap && vicEq) {
      act("Your $p strikes $N's $P with a solid impact!", 
          FALSE, ch, weap, vicEq, TO_CHAR);
      act("$n's $p strikes your $P with a solid impact!", 
          FALSE, ch, weap, vicEq, TO_VICT);
      act("$n's $p strikes $N's $P with a solid impact!", 
          FALSE, ch, weap, vicEq, TO_NOTVICT);
      vicEq->damageItem(eqDamage);
      if (vicEq->getStructPoints() <= 0) {
        vicEq->makeScraps();
        delete vicEq;
      }  
    }
    // Case 2: Weapon hits victim's body part
    else if (weap && !vicEq) {
      sstring bodyPart = victim->describeBodySlot(vicLimb);
      
      act(format("Your $p strikes $N's %s with a solid impact!") % bodyPart, 
          FALSE, ch, weap, victim, TO_CHAR);
      act(format("$n's $p strikes your %s with a solid impact!") % bodyPart, 
          FALSE, ch, weap, victim, TO_VICT);
      act(format("$n's $p strikes $N's %s with a solid impact!") % bodyPart, 
          FALSE, ch, weap, victim, TO_NOTVICT);
      
      if (victim->isLimbFlags(vicLimb, PART_BRUISED)) {
        victim->incrementBruiseStack(vicLimb, 100);
      } else {
        victim->rawBruise(vicLimb, 100, SILENT_NO, CHECK_IMMUNITY_NO);
      }
    }
    // Case 3: Body part hits victim's equipment
    else if (!weap && vicEq) {
      act("Your blow strikes $N's $p with a solid impact!", 
          FALSE, ch, vicEq, victim, TO_CHAR);
      act("$n's blow strikes your $p with a solid impact!", 
          FALSE, ch, vicEq, victim, TO_VICT);
      act("$n's blow strikes $N's $p with a solid impact!", 
          FALSE, ch, vicEq, victim, TO_NOTVICT);
      
      vicEq->damageItem(eqDamage);
      if (vicEq->getStructPoints() <= 0) {
        vicEq->makeScraps();
        delete vicEq;
      }
    }
    // Case 4: Body part hits victim's body part
    else if (!weap && !vicEq) {
      sstring victimPart = victim->describeBodySlot(vicLimb);
      
      act(format("Your blow strikes $N's %s with a solid impact!") % victimPart, 
          FALSE, ch, nullptr, victim, TO_CHAR);
      act(format("$n's blow strikes your %s with a solid impact!") % victimPart, 
          FALSE, ch, nullptr, victim, TO_VICT);
      act(format("$n's blow strikes $N's %s with a solid impact!") % victimPart, 
          FALSE, ch, nullptr, victim, TO_NOTVICT);
      
      if (victim->isLimbFlags(vicLimb, PART_BRUISED)) {
        victim->incrementBruiseStack(vicLimb, 100);
      } else {
        victim->rawBruise(vicLimb, 100, SILENT_NO, CHECK_IMMUNITY_NO);
      }
    }
    return true;
  }
  return false;  // Return false if no impact occurred
}
int spikesBreak(TBeing* victim, TBeing* ch, TObj* obj) {
  int dam = ::number(1, 4);
  if (!obj)
    return false;

  if ((obj->isObjStat(ITEM_SPIKED)) && percentChance(25) && victim->isTough()) {
    obj->addToStructPoints(-dam);
    obj->addToMaxStructPoints(-1);
    act("Your $o catches on $N!", FALSE, ch, obj, victim, TO_CHAR);
    act("$n's $o catches on $N!", FALSE, ch, obj, victim, TO_NOTVICT);
    act("$n's $o catches as it makes impact with you!", FALSE, ch, obj, victim,
      TO_VICT);
    act("Some spikes break off, damaging $n's $o!", FALSE, ch, obj, nullptr,
      TO_ROOM, ANSI_GRAY);
    act("Some spikes break off, damaging your $o!", FALSE, ch, obj, nullptr,
      TO_CHAR, ANSI_GRAY);

    if (obj->getMaxStructPoints() <= 0) {
      obj->makeScraps();
      return true;
    }

    if (auto* weapon = dynamic_cast<TGenWeapon*>(obj)) {
      weapon->addToCurSharp(-dam);
      weapon->addToMaxSharp(-1);
      act("The impact mars the edge of $n's $o!", FALSE, ch, obj, nullptr,
        TO_ROOM);
      act("The impact mars the edge of your $o!", FALSE, ch, obj, nullptr,
        TO_CHAR);

      if (weapon->getMaxSharp() <= 0) {
        weapon->makeScraps();
        return true;
      }
    }

    if (percentChance(25)) {
      obj->remObjStat(ITEM_SPIKED);
      act("$n's $p looks less dangerous now.", FALSE, ch, obj, nullptr, TO_ROOM,
        ANSI_GRAY);
      act("Your $p looks less dangerous now.", FALSE, ch, obj, nullptr, TO_CHAR,
        ANSI_GRAY);
    }
    return true;
  }
  return false;
}

int spikesHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT limb) {
  // Check for valid parameters
  if (!victim || limb == WEAR_NOWHERE) {
    return false;
  }

  if (!obj->isObjStat(ITEM_SPIKED) && !obj->isSpiked()) {
    return false;
  }

  if (victim->isLimbFlags(limb, PART_MISSING)) {
    return false;
  }

  if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED, limb)) {
    // No bleeding for undead or immune creatures
    return false;
  }

  if (victim->isLimbFlags(limb, PART_BLEEDING)) {
    act(format("Blood spatters as the spikes on your $o sink into $N's bleeding %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_CHAR);
    act(format("Blood spatters as the spikes on $n's $o sink into $N's bleeding %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_NOTVICT);
    act(format("Blood spatters as the spikes on $n's $o sink into your bleeding %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_VICT);
    
    // Increment the bleed stack
    victim->incrementBleedStack(limb, 250);
  } else {   
    act(format("The spikes on $p tear open a <R>bloody wound<1> in $N's %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_NOTVICT);
    act(format("The spikes on $p tear open a <R>bloody wound<1> in your %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_VICT);
    act(format("The spikes on $p tear open a <R>bloody wound<1> in $N's %s!") % 
        victim->describeBodySlot(limb), FALSE, ch, obj, victim, TO_CHAR);
     
    victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  }
  
  spikesBreak(victim, ch, obj);
  return true;
}
