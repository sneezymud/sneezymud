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
#include "obj.h"


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

  setWeaponType(GET_BITS(x3, 7, 8), 0);
  setWeaponFreq(GET_BITS(x3, 15, 8), 0);

  setWeaponType(GET_BITS(x3, 23, 8), 1);
  setWeaponFreq(GET_BITS(x3, 31, 8), 1);

  setWeaponType(GET_BITS(x4, 7, 8), 2);
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

void TGenWeapon::setWeaponType(int n, int which) {
  if (n < WEAPON_TYPE_NONE || n >= WEAPON_TYPE_MAX) {
    vlogf(LOG_BUG,
      format("Invalid weapon type %d on %s, resetting to NONE") % n % getName());
    n = WEAPON_TYPE_NONE;
  }
  weapon_type[which] = (weaponT)n;
}

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
    a += format("Attack Type:        %-8s") %
         attack_hit_text[getWtype(wt) - TYPE_MIN_HIT].singular;
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


int thornsHit(TBeing* victim, TBeing* ch, wearSlotT chLimb, wearSlotT vicLimb) {
  // Check for valid parameters
  if (!victim || !ch || vicLimb == WEAR_NOWHERE || !ch->affectedBySpell(SPELL_THORNFLESH)) {
    return 0;
  }

  if (victim->isLimbFlags(vicLimb, PART_MISSING)) {
    return 0;
  }

  int dam = ::number(1, 10);

  if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED, vicLimb)) {
    // No bleeding for undead or immune creatures
    return dam;
  }
  // Check for victim's limb hardness against attacker's limb hardness
  int vicLimbHardness = getHardnessSpec(victim, vicLimb);
  int chLimbHardness = getHardnessSpec(ch, chLimb);

  // Calculate chance based on hardness difference
  int hardnessDiff = chLimbHardness - vicLimbHardness;
  
  // Only proceed if attacker's hardness is higher AND random check passes
  if (hardnessDiff <= 0) {
    return dam;
  }
  
  // Use hardness difference as percentage chance
  if (!::percentChance(hardnessDiff)) {
    return dam;
  }
  
  if (victim->isLimbFlags(vicLimb, PART_BLEEDING)) {
    static constexpr const char* bleeding_msg =
      "Blood spatters as the thorns on %s %s sink into %s bleeding %s!";

    sstring chBodyPart = ch->describeBodySlot(chLimb);
    sstring vicBodyPart = victim->describeBodySlot(vicLimb);
    act(format(bleeding_msg) % "your" % chBodyPart % "$N's" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_CHAR);
    act(format(bleeding_msg) % "$n's" % chBodyPart % "$N's" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_NOTVICT);
    act(format(bleeding_msg) % "$n's" % chBodyPart % "your" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_VICT);

    // Increment the bleed stack
    victim->incrementBleedStack(vicLimb, 250);
  } else {
    static constexpr const char* wound_msg =
      "The thorns on %s %s tear open a <R>bloody wound<1> in %s %s!";

    sstring chBodyPart = ch->describeBodySlot(chLimb);
    sstring vicBodyPart = victim->describeBodySlot(vicLimb);
    act(format(wound_msg) % "your" % chBodyPart % "$N's" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_CHAR);
    act(format(wound_msg) % "$n's" % chBodyPart % "$N's" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_NOTVICT);
    act(format(wound_msg) % "$n's" % chBodyPart % "your" % vicBodyPart,
        FALSE, ch, nullptr, victim, TO_VICT);

    victim->rawBleed(vicLimb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  return dam;
}

int hardHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT vicLimb, wearSlotT chLimb) {
  if (!victim || !ch || vicLimb == WEAR_NOWHERE) {
    return 0;
  }
  TObj *weap = dynamic_cast<TObj*>(ch->equipment[chLimb]);
  TObj *vicEq = dynamic_cast<TObj*>(victim->equipment[vicLimb]);
  int vicHard = vicEq ? material_nums[vicEq->getMaterial()].hardness : 0;
  int weapHard = weap ? material_nums[weap->getMaterial()].hardness : 0;
  if (!weap) {
    weapHard = getHardnessSpec(ch, chLimb);
  }
  
  if (!vicEq) {
    vicHard = getHardnessSpec(victim, vicLimb);
  }
  
  int dam = ::number(1, 10);

  int hardChance = (weapHard - vicHard);
  if ((percentChance(hardChance)) && !victim->isTough()) {
      int eqDamage = (weapHard-vicHard)/10;
      // Case 1: Weapon hits victim's equipment
    if (weap && vicEq) {
      static constexpr const char* weap_vs_eq_msg =
        "%s $p strikes %s $P with a solid impact!";
      dam += weap->getWeight()/4;

      act(format(weap_vs_eq_msg) % "Your" % "$N's", FALSE, ch, weap, vicEq, TO_CHAR);
      act(format(weap_vs_eq_msg) % "$n's" % "your", FALSE, ch, weap, vicEq, TO_VICT);
      act(format(weap_vs_eq_msg) % "$n's" % "$N's", FALSE, ch, weap, vicEq, TO_NOTVICT);
      vicEq->damageItem(eqDamage);

      if (vicEq->getStructPoints() <= 0 && !vicEq->makeScraps()) {
         delete vicEq;
      }

    }
    // Case 2: Weapon hits victim's body part
    else if (weap && !vicEq) {
      static constexpr const char* weap_vs_body_msg =
        "%s $p strikes %s %s with a solid impact!";
      dam += weap->getWeight()/4;

      sstring bodyPart = victim->describeBodySlot(vicLimb);
      act(format(weap_vs_body_msg) % "Your" % "$N's" % bodyPart, FALSE, ch, weap, victim, TO_CHAR);
      act(format(weap_vs_body_msg) % "$n's" % "your" % bodyPart, FALSE, ch, weap, victim, TO_VICT);
      act(format(weap_vs_body_msg) % "$n's" % "$N's" % bodyPart, FALSE, ch, weap, victim, TO_NOTVICT);
      if (victim->isLimbFlags(vicLimb, PART_BRUISED)) {
        victim->incrementBruiseStack(vicLimb, 100);
      } else {
        victim->rawBruise(vicLimb, 100, SILENT_NO, CHECK_IMMUNITY_NO);
      }
    }
    // Case 3: Body part hits victim's equipment
    else if (!weap && vicEq) {
      static constexpr const char* body_vs_eq_msg =
        "%s %s strikes %s $p with a solid impact!";

      sstring bodyPart = ch->describeBodySlot(chLimb);
      act(format(body_vs_eq_msg) % "Your" % bodyPart % "$N's", FALSE, ch, vicEq, victim, TO_CHAR);
      act(format(body_vs_eq_msg) % "$n's" % bodyPart % "your", FALSE, ch, vicEq, victim, TO_VICT);
      act(format(body_vs_eq_msg) % "$n's" % bodyPart % "$N's", FALSE, ch, vicEq, victim, TO_NOTVICT);
      vicEq->damageItem(eqDamage);
      if (vicEq->getStructPoints() <= 0 && !vicEq->makeScraps()) {
         delete vicEq;
      }
    }
    // Case 4: Body part hits victim's body part
    else {
      static constexpr const char* body_vs_body_msg =
        "%s %s strikes %s %s with a solid impact!";

      sstring attackerPart = ch->describeBodySlot(chLimb);
      sstring victimPart = victim->describeBodySlot(vicLimb);
      act(format(body_vs_body_msg) % "Your" % attackerPart % "$N's" % victimPart, FALSE, ch, nullptr, victim, TO_CHAR);
      act(format(body_vs_body_msg) % "$n's" % attackerPart % "your" % victimPart, FALSE, ch, nullptr, victim, TO_VICT);
      act(format(body_vs_body_msg) % "$n's" % attackerPart % "$N's" % victimPart, FALSE, ch, nullptr, victim, TO_NOTVICT);
      if (victim->isLimbFlags(vicLimb, PART_BRUISED)) {
        victim->incrementBruiseStack(vicLimb, 100);
      } else {
        victim->rawBruise(vicLimb, 100, SILENT_NO, CHECK_IMMUNITY_NO);
      }
    }
    
  }
  return dam;  
}
int spikesBreak(TBeing* victim, TBeing* ch, TObj* obj) {
  int dam = ::number(1, 4);
  if (!obj)
    return 0;

  if ((obj->isObjStat(ITEM_SPIKED)) && percentChance(25) && victim->isTough()) {
    static constexpr const char* catch_msg = "$n's $o catches on $N's $o!";
    static constexpr const char* spikes_break_msg = "Some spikes break off, damaging $n's $o!";

    obj->addToStructPoints(-dam);
    obj->addToMaxStructPoints(-1);
    act(format(catch_msg) % "Your" % "$N", FALSE, ch, obj, victim, TO_CHAR);
    act(format(catch_msg) % "$n's" % "$N", FALSE, ch, obj, victim, TO_NOTVICT);
    act(format(catch_msg) % "$n's" % "$N", FALSE, ch, obj, victim, TO_VICT);
    act(format(spikes_break_msg) % "$n's", FALSE, ch, obj, nullptr, TO_ROOM, ANSI_GRAY);
    act(format(spikes_break_msg) % "your", FALSE, ch, obj, nullptr, TO_CHAR, ANSI_GRAY);

    if (obj->getMaxStructPoints() <= 0) {
      if (!obj->makeScraps()){
        delete obj;
        obj = nullptr;
      }
      return true;
    }

    if (auto* weapon = dynamic_cast<TGenWeapon*>(obj)) {
      static constexpr const char* mars_edge_msg = "The impact mars the edge of %s $o!";

      weapon->addToCurSharp(-dam);
      weapon->addToMaxSharp(-1);
      act(format(mars_edge_msg) % "$n's", FALSE, ch, obj, nullptr, TO_ROOM);
      act(format(mars_edge_msg) % "your", FALSE, ch, obj, nullptr, TO_CHAR);

      if (weapon->getMaxSharp() <= 0) {
        if (!weapon->makeScraps()){
          delete weapon;
          weapon = nullptr;
        }
          return true;
      }
    }

    if (percentChance(25)) {
      static constexpr const char* less_dangerous_msg = "%s $p looks less dangerous now.";

      obj->remObjStat(ITEM_SPIKED);
      act(format(less_dangerous_msg) % "$n's", FALSE, ch, obj, nullptr, TO_ROOM, ANSI_GRAY);
      act(format(less_dangerous_msg) % "Your", FALSE, ch, obj, nullptr, TO_CHAR, ANSI_GRAY);
    }
    return true;
  }
  return false;
}

int impactSpec(TBeing* ch, TBeing* victim, wearSlotT damSource, wearSlotT pos) {
  // Get the object at the damage source (if any)
  if (!ch || !victim || pos == WEAR_NOWHERE) {
    return 0;
  }
  TObj* obj = dynamic_cast<TObj*>(ch->equipment[damSource]);

  if (obj) {
    // There is equipment on damSource
    if (obj->isSpiked()) {
      // Equipment has spikes - use spikesHit
      return spikesHit(victim, ch, obj, pos);
    }
      // Equipment has no spikes - use hardHit
      return hardHit(victim, ch, obj, pos, damSource);
    
  } else {
    // No equipment on damSource
    if (ch->affectedBySpell(SPELL_THORNFLESH)) {
      // Has thornflesh - use thornsHit
      return thornsHit(victim, ch, damSource, pos);
    }
      // No thornflesh - use hardHit
      return hardHit(victim, ch, nullptr, pos, damSource);
    
  }
}

int spikesHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT limb) {
  // Check for valid parameters
  if (!victim || !ch || !obj || limb == WEAR_NOWHERE) {
    return 0;
  }

  if (!obj->isSpiked()) {
    return 0;
  }

  if (victim->isLimbFlags(limb, PART_MISSING)) {
    return 0;
  }

  int dam = ::number(1, 10) + obj->getWeight() / 4;

  if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED, limb)) {
    // No bleeding for undead or immune creatures
    return dam;
  }

  if (victim->isLimbFlags(limb, PART_BLEEDING)) {
    static constexpr const char* spikes_bleeding_msg =
      "Blood spatters as the spikes on %s $o sink into %s bleeding %s!";

    sstring vicBodyPart = victim->describeBodySlot(limb);
    act(format(spikes_bleeding_msg) % "your" % "$N's" % vicBodyPart, FALSE, ch, obj, victim, TO_CHAR);
    act(format(spikes_bleeding_msg) % "$n's" % "$N's" % vicBodyPart, FALSE, ch, obj, victim, TO_NOTVICT);
    act(format(spikes_bleeding_msg) % "$n's" % "your" % vicBodyPart, FALSE, ch, obj, victim, TO_VICT);

    // Increment the bleed stack
    victim->incrementBleedStack(limb, 250);
  } else {
    static constexpr const char* spikes_wound_msg =
      "The spikes on $p tear open a <R>bloody wound<1> in %s %s!";

    sstring vicBodyPart = victim->describeBodySlot(limb);
    act(format(spikes_wound_msg) % "$N's" % vicBodyPart, FALSE, ch, obj, victim, TO_CHAR);
    act(format(spikes_wound_msg) % "$N's" % vicBodyPart, FALSE, ch, obj, victim, TO_NOTVICT);
    act(format(spikes_wound_msg) % "your" % vicBodyPart, FALSE, ch, obj, victim, TO_VICT);

    victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  }
  
  spikesBreak(victim, ch, obj);
  return dam;
}
