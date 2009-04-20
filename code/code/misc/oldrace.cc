//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// oldrace.cc - old functions related to mobile race
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "room.h"
#include "being.h"
#include "materials.h"

/* remove these races
    case RACE_UNCERT:
*/

void TBeing::setRacialStuff()
{
  // set immunities from race
  immuneTypeT itt;
  for (itt = MIN_IMMUNE; itt < MAX_IMMUNES; itt++) {
    // default to the base racial setting.
    // if they are both negative, use the most negative.
    // if they are both positive, use the most positive
    // if they oppose, add the two (so they can cancel eath other out)
    byte amt = getMyRace()->getImmunities().getImmunity(itt);
    if (amt == 0)
      amt = getImmunity(itt);
    else if (amt < 0 && getImmunity(itt) < 0)
      amt = min(amt, (byte)getImmunity(itt));
    else if (amt > 0 && getImmunity(itt) > 0)
      amt = max(amt, (byte)getImmunity(itt));
    else
      amt = max(min(100, amt + getImmunity(itt)), -100);

    setImmunity(itt, amt);
  }

  // add some other immunities based on special conditions
  if (IS_SET(specials.act, ACT_GHOST)) {
    SET_BIT(specials.affectedBy, AFF_LEVITATING);
    setImmunity(IMMUNE_SKIN_COND, 100);
    setImmunity(IMMUNE_BONE_COND, 100);
  }
  if (getMyRace()->hasNoBones()) {
    setImmunity(IMMUNE_BONE_COND, 100);
  }
  if (IS_SET(specials.act, ACT_SKELETON)) {
    setImmunity(IMMUNE_BONE_COND, -100);
  }
  if(isVampire()){
      SET_BIT(specials.affectedBy, AFF_TRUE_SIGHT);
  }
  if (isUndead()) {
    addToLight(-GetMaxLevel() / 5);
    visionBonus = max((int)visionBonus, (-getLight() + 1));
    setImmunity(IMMUNE_DRAIN,100);
    setImmunity(IMMUNE_BLEED,100);
    setImmunity(IMMUNE_POISON,100);
    setImmunity(IMMUNE_SLEEP,100);
    setImmunity(IMMUNE_DISEASE,100);
    setImmunity(IMMUNE_SUFFOCATION,100);
    setImmunity(IMMUNE_FEAR,100);
  }
  switch (getMyRace()->getRace()) {
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_GNOME:
    case RACE_DROW:
    case RACE_MFLAYER:
    case RACE_TROLL:
    case RACE_ORC:
    case RACE_GOBLIN:
    case RACE_KOBOLD:
    case RACE_BUGBEAR:
    case RACE_HOBGOBLIN:
    case RACE_GNOLL:
    case RACE_BAT:
    case RACE_VAMPIREBAT:
    case RACE_INSECT:
    case RACE_FLYINSECT:
    case RACE_ANT:
    case RACE_ARACHNID:
      SET_BIT(specials.affectedBy, AFF_INFRAVISION);
      break;
    case RACE_ORB:
    case RACE_DJINN:
      // this is regarded as a natural flying ability
      SET_BIT(specials.affectedBy, AFF_FLYING);
    break; 
    case RACE_FISHMAN:
    case RACE_MERMAID:
    case RACE_FISH:
    case RACE_OCTOPUS:
    case RACE_CRUSTACEAN:
    case RACE_TURTLE:
      SET_BIT(specials.affectedBy, AFF_WATERBREATH);
      SET_BIT(specials.affectedBy, AFF_SWIM);
      break;
    case RACE_DEVIL:
      SET_BIT(specials.affectedBy, AFF_TRUE_SIGHT);
      SET_BIT(specials.affectedBy, AFF_DETECT_MAGIC);
      SET_BIT(specials.affectedBy, AFF_DETECT_INVISIBLE);
      break;
    case RACE_KUOTOA:
      SET_BIT(specials.affectedBy, AFF_WATERBREATH);
      SET_BIT(specials.affectedBy, AFF_SWIM);
      SET_BIT(specials.affectedBy, AFF_INFRAVISION);
      SET_BIT(specials.affectedBy, AFF_TRUE_SIGHT);
      break;
    case RACE_WYVERN:
      SET_BIT(specials.affectedBy, AFF_INFRAVISION);
      SET_BIT(specials.affectedBy, AFF_DETECT_MAGIC);
      break;
    case RACE_ELEMENT:
      // this is regarded as a natural flying ability
      if (isname("[air]", name))
        SET_BIT(specials.affectedBy, AFF_FLYING); 
      break;
    case RACE_CANINE:
    case RACE_AMPHIB:
    case RACE_FROG:
    case RACE_FROGMAN:
    case RACE_BEAR:
    case RACE_WEASEL:
    case RACE_BADGER:
    case RACE_OTTER:
    case RACE_BEAVER:
    case RACE_SAHUAGIN:
      SET_BIT(specials.affectedBy, AFF_SWIM);
      break;
    case RACE_HORSE:
    case RACE_PEGASUS:
    case RACE_DRAGON:
      setMaxMove(getMaxMove() + 150);
      setMove(moveLimit());
      break;
    case RACE_UNCERT:
    case RACE_NORACE:
      vlogf(LOG_LOW, format("%s had race %s (%d)") %  getName() %
             getMyRace()->getSingularName() %getRace());
      break;
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_OGRE:
    case RACE_LYCANTH:
    case RACE_BIRDMAN:
    case RACE_VAMPIRE:
    case RACE_UNDEAD:
    case RACE_DINOSAUR:
    case RACE_BIRD:
    case RACE_GIANT:
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_DEMON:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_PRIMATE:
    case RACE_FAERIE:
    case RACE_GOLEM:
    case RACE_PANTATH:
    case RACE_RODENT:
    case RACE_TYTAN:
    case RACE_WOODELF:
    case RACE_FELINE:
    case RACE_REPTILE:
    case RACE_MOSS:
    case RACE_BOVINE:
    case RACE_GOAT:
    case RACE_SHEEP:
    case RACE_DEER:
    case RACE_SQUIRREL:
    case RACE_RABBIT:
    case RACE_PIG:
    case RACE_BOAR:
    case RACE_GIRAFFE:
    case RACE_CENTIPEDE:
    case RACE_MOUND:
    case RACE_PIERCER:
    case RACE_MANTICORE:
    case RACE_GRIFFON:
    case RACE_SPHINX:
    case RACE_SHEDU:
    case RACE_LAMMASU:
    case RACE_PHOENIX:
    case RACE_DRAGONNE:
    case RACE_HIPPOGRIFF:
    case RACE_RUST_MON:
    case RACE_LION:
    case RACE_TIGER:
    case RACE_LEOPARD:
    case RACE_COUGAR:
    case RACE_ELEPHANT:
    case RACE_RHINO:
    case RACE_NAGA:
    case RACE_OTYUGH:
    case RACE_OX:
    case RACE_GREMLIN:
    case RACE_OWLBEAR:
    case RACE_CHIMERA:
    case RACE_SATYR:
    case RACE_DRYAD:
    case RACE_MINOTAUR:
    case RACE_GORGON:
    case RACE_BASILISK:
    case RACE_LIZARD_MAN:
    case RACE_CENTAUR:
    case RACE_GOPHER:
    case RACE_LAMIA:
    case RACE_PYGMY:
    case RACE_BAANTA:
    case RACE_MIMIC:
    case RACE_MEDUSA:
    case RACE_PENGUIN:
    case RACE_OSTRICH:
    case RACE_TROG:
    case RACE_COATL:
    case RACE_SNAKE:
    case RACE_SIMAL:
    case RACE_HIPPOPOTAMUS:
    case RACE_ANGEL:
    case RACE_BANSHEE:
    case RACE_WYVELIN:
    case RACE_RATMEN:
    case MAX_RACIAL_TYPES:
      break;
  }

  // faction checking
  if (/*getMyRace()->getRace() == RACE_ORC ||*/
      getMyRace()->getRace() == RACE_KOBOLD) {
    if (!isCult())
      vlogf(LOG_LOW, format("%s: Bad faction for race %s (should be 2)") %  getName() %
               getMyRace()->getSingularName());
  }
  if (/*getMyRace()->getRace() == RACE_TROLL ||*/
      getMyRace()->getRace() == RACE_TYTAN) {
    if (!isBrother())
      vlogf(LOG_LOW, format("%s: Bad faction for race %s (should be 1)") %  getName() %
               getMyRace()->getSingularName());
  }
  if (getMyRace()->getRace() == RACE_WOODELF ||
      getMyRace()->getRace() == RACE_DRYAD ||
      getMyRace()->getRace() == RACE_CENTAUR ||
      getMyRace()->getRace() == RACE_SATYR) {
    if (!isSnake())
      vlogf(LOG_LOW, format("%s: Bad faction for race %s (should be 3)") %  getName() %
               getMyRace()->getSingularName());
  }
}

bool TBeing::isAnimal() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isAnimal() ? TRUE : FALSE;
}

bool TBeing::isVampire() const
{
  if(hasQuestBit(TOG_VAMPIRE))
    return TRUE;
  if(getRace() == RACE_VAMPIRE ||
     getRace() == RACE_VAMPIREBAT)
    return TRUE;
  return FALSE;
}

bool TBeing::isUndead() const
{
  if (hasQuestBit(TOG_VAMPIRE))
    return TRUE;
  if (IS_SET(specials.act, ACT_SKELETON))
    return TRUE;
  if (IS_SET(specials.act, ACT_ZOMBIE))
    return TRUE;
  if (IS_SET(specials.act, ACT_GHOST))
    return TRUE;
  return getMyRace()->isUndead() ? TRUE : FALSE;
}

bool TBeing::isVeggie() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isVeggie() ? TRUE : FALSE;
}

bool TBeing::isOther() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isOther() ? TRUE : FALSE;
}

bool TBeing::isGiantish() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isGiantish() ? TRUE : FALSE;
}

bool TBeing::isLycanthrope() const
{
  if(hasQuestBit(TOG_LYCANTHROPE))
    return TRUE;

  return getMyRace()->isLycanthrope() ? TRUE : FALSE;
}

bool TBeing::isDiabolic() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isDiabolic() ? TRUE : FALSE;
}

bool TBeing::isReptile() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isReptile() ? TRUE : FALSE;
}

// this is for knows
bool TBeing::isPeople() const
{
  // prevent undead from flagging true
  if (IS_SET(specials.act, ACT_SKELETON))
    return FALSE;
  else if (IS_SET(specials.act, ACT_ZOMBIE))
    return FALSE;
  else if (IS_SET(specials.act, ACT_GHOST))
    return FALSE;

  return getMyRace()->isPeople() ? TRUE : FALSE;
}

bool TBeing::isExtraPlanar() const
{
  return getMyRace()->isExtraPlanar() ? TRUE : FALSE;
}

TBeing *TBeing::findDiffZoneSameRace()
{
  int num;
  TBeing *t;

  num = ::number(1, 100);

  for (t = character_list; t; t = t->next, num--) {
    if (isSameRace(t) && !t->isPc() && !num) {

      if (roomp->getZoneNum() != t->roomp->getZoneNum())
        return t;
    }
  }
  return NULL;
}

// this should be places char can actually wear equipment
// should be a subset of actual limbs
// default case is all slots except secondary limbs
int TBeing::validEquipSlot(wearSlotT i)
{
  if (isLimbFlags(i, PART_TRANSFORMED))
    return FALSE;

  switch (getMyRace()->getBodyType()) {
    case BODY_INSECTOID:
    case BODY_ANT:
    case BODY_DJINN:
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_LAMMASU:
    case BODY_DRAGONNE:
    case BODY_WYVERN:
    case BODY_HIPPOGRIFF:
    case BODY_CHIMERA:
    case BODY_CENTIPEDE:
    case BODY_ORB:
    case BODY_PIG:
    case BODY_LION:
    case BODY_TURTLE:
    case BODY_FROG:
    case BODY_BAANTA:
    case BODY_AMPHIBEAN:
      return (i == WEAR_HEAD);
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
    case BODY_WYVELIN:
#if 0
      return ((i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L) ||
              (i == WEAR_HAND_R) || (i == WEAR_HAND_L) ||
              (i == WEAR_BACK) ||
              (i == WEAR_BODY) || (i == WEAR_WAIST) ||
              (i == WEAR_NECK) || (i == WEAR_HEAD) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L));
#else
      // probably want to restrict this: outfit lynx frogskin
      return ((i == WEAR_HEAD) || (i == WEAR_NECK));
#endif
    case BODY_DRAGON:
      return ((i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L) ||
              (i == WEAR_HAND_R) || (i == WEAR_HAND_L) ||
              (i == WEAR_BACK) ||
              (i == WEAR_BODY) || (i == WEAR_WAIST) ||
              (i == WEAR_NECK) || (i == WEAR_HEAD) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L));
    case BODY_KUOTOA:
      return ((i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_BACK) ||
              (i == WEAR_BODY) || (i == WEAR_WAIST) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_WRIST_R) || (i == WEAR_WRIST_L));
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_COATL:
    case BODY_NAGA:
      return ((i == WEAR_BODY) || (i == WEAR_HEAD));
    case BODY_MERMAID:
      return ((i == WEAR_HEAD) || (i == WEAR_BODY) ||
              (i == WEAR_LEG_R) || (i == WEAR_NECK) ||
              (i == WEAR_FOOT_R) ||
              (i == WEAR_HAND_R) || (i == WEAR_HAND_L) ||
              (i == WEAR_WRIST_R) || (i == WEAR_WRIST_L) ||
              (i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_FINGER_R) || (i == WEAR_FINGER_L) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_BACK) || (i == WEAR_WAIST));
    case BODY_OCTOPUS:
    case BODY_SPIDER:
      return ((i == WEAR_HEAD) || 
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L) ||
              (i == WEAR_LEG_L) || (i == WEAR_LEG_L) || (i == WEAR_FOOT_L) ||
              (i == WEAR_FOOT_R) || (i == HOLD_LEFT) || (i == HOLD_RIGHT) ||
              (i == WEAR_BODY));
    case BODY_CRUSTACEAN:
      return ((i == WEAR_ARM_L) || (i == WEAR_ARM_R) ||
              (i == WEAR_LEG_L) || (i == WEAR_LEG_L) || (i == WEAR_HAND_L) ||
              (i == WEAR_HAND_R) || (i == HOLD_LEFT) || (i == HOLD_RIGHT) ||
              (i == WEAR_BODY) || 
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L));
    case BODY_PARASITE:
    case BODY_SLIME:
      return (i == WEAR_BODY);
    case BODY_TREE:
    case BODY_VEGGIE:
      return ((i == WEAR_BODY) || (i == WEAR_ARM_L) || (i == WEAR_ARM_R));
    case BODY_BIRD:
    case BODY_BAT:
      return ((i == WEAR_BODY) || (i == HOLD_RIGHT) || 
              (i == WEAR_ARM_L) || (i == WEAR_ARM_R) ||
              (i == WEAR_HEAD) || (i == HOLD_LEFT));
    case BODY_CENTAUR:
    case BODY_SIMAL:
    case BODY_LAMIA:
      return ((i == WEAR_HAND_R) || (i == WEAR_HAND_L) ||
              (i == WEAR_WRIST_R) || (i == WEAR_WRIST_L) ||
              (i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_FINGER_R) || (i == WEAR_FINGER_L) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_BACK) || (i == WEAR_BODY) || 
              (i == WEAR_HEAD) || (i == WEAR_NECK));
    case BODY_ELEMENTAL:
    case BODY_MOSS:
    case BODY_PIERCER:
    case BODY_NONE:
    case BODY_MIMIC:
    case BODY_GOLEM:  // automatons and golems : makes their AC too good
      return FALSE;   // can't wear anywhere
    case BODY_HUMANOID:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
    case BODY_DEMON:
    case BODY_FROGMAN:
    case BODY_MEDUSA:
    case BODY_FISHMAN:
      return ((i == WEAR_HAND_R) || (i == WEAR_HAND_L) ||
              (i == WEAR_WRIST_R) || (i == WEAR_WRIST_L) ||
              (i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_FINGER_R) || (i == WEAR_FINGER_L) ||
              (i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_WAIST) ||
              (i == WEAR_BACK) || (i == WEAR_BODY) || 
              (i == WEAR_HEAD) || (i == WEAR_NECK));
    case BODY_OTYUGH:
      return ((i == WEAR_ARM_R) || (i == WEAR_ARM_L) ||
              (i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L) ||
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_FOOT_R) ||
              (i == WEAR_BACK) || (i == WEAR_BODY) || 
              (i == WEAR_HEAD));
    case BODY_FOUR_HOOF:
    case BODY_ELEPHANT:
#if 0
      return ((i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L) ||
              (i == WEAR_BACK) ||
              (i == WEAR_BODY) || (i == WEAR_WAIST) ||
              (i == WEAR_NECK) || (i == WEAR_HEAD) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L));
#else
      // probably want to restrict this: outfit lynx frogskin
      return ((i == WEAR_HEAD) || (i == WEAR_NECK));
#endif
    case BODY_PEGASUS:
#if 0
      return ((i == WEAR_LEG_R) || (i == WEAR_LEG_L) ||
              (i == WEAR_EX_LEG_R) || (i == WEAR_EX_LEG_L) ||
              (i == WEAR_EX_FOOT_R) || (i == WEAR_EX_FOOT_L) ||
              (i == WEAR_BACK) ||
              (i == WEAR_BODY) || (i == WEAR_WAIST) ||
              (i == WEAR_NECK) || (i == WEAR_HEAD) ||
              (i == HOLD_RIGHT) || (i == HOLD_LEFT) ||
              (i == WEAR_FOOT_R) || (i == WEAR_FOOT_L) ||
              (i == WEAR_ARM_R) || (i == WEAR_ARM_L));
#else
      // probably want to restrict this: outfit lynx frogskin
      return ((i == WEAR_HEAD) || (i == WEAR_NECK));
#endif
    case MAX_BODY_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Bogus body type (%d) in validEquipSlot") %  getMyRace()->getBodyType());
  return FALSE;
}

const sstring TBeing::bogus_slot_worn(wearSlotT i) const
{
//  vlogf(LOG_BUG, format("%s had bogus slot (%d) worn.") %  getName() % i);
  vlogf(LOG_BUG, format("%s had bogus slot (%d) worn.") %  getName() % i);
  return "Worn on BOGUS slot - Bug this!";
}

const sstring TBeing::defaultEquipmentSlot(wearSlotT i) const
{
  switch (i) {
    case WEAR_EX_LEG_L:
      if (equipment[i] == equipment[WEAR_EX_LEG_R])
        return bogus_slot_worn(i);
      else
        return "Worn on back, left leg";
    case WEAR_EX_LEG_R:
      if (equipment[i] == equipment[WEAR_EX_LEG_L])
        return "Worn on back legs";
      else
        return "Worn on back, right leg";
    case WEAR_LEG_R:
      if (equipment[i] == equipment[WEAR_LEG_L])
        return "Worn on legs";
      else
        return "Worn on right leg";
    case WEAR_LEG_L:
      if (!(equipment[i] == equipment[WEAR_LEG_R]))
        return "Worn on left leg";
      else
        return bogus_slot_worn(i);
    case WEAR_FINGER_R:
      return "Right ring finger";
    case WEAR_FINGER_L:
      return "Left ring finger";
    case WEAR_NECK:
      return "Worn around neck";
    case WEAR_BODY:
      return "Worn on body";
    case WEAR_HEAD:
      return "Worn on head";
    case WEAR_EX_FOOT_L:
      return "Worn on back, left foot";
    case WEAR_EX_FOOT_R:
      return "Worn on back, right foot";
    case WEAR_FOOT_R:
      return "Worn on right foot";
    case WEAR_FOOT_L:
      return "Worn on left foot";
    case WEAR_HAND_R:
      return "Worn on right hand";
    case WEAR_HAND_L:
      return "Worn on left hand";
    case WEAR_ARM_R:
      return "Worn on right arm";
    case WEAR_ARM_L:
      return "Worn on left arm";
    case WEAR_BACK:
      return "Worn on back";
    case WEAR_WAIST:
      return "Worn around waist";
    case WEAR_WRIST_R:
      return "Worn on right wrist";
    case WEAR_WRIST_L:
      return "Worn on left wrist";
    case HOLD_RIGHT:
      if (equipment[i] == equipment[HOLD_LEFT])
        return "Held in both hands";
      else
        return "Held in right hand";
    case HOLD_LEFT:
      if (!(equipment[i] == equipment[HOLD_RIGHT]))
        return "Held in left hand";
      else
        return bogus_slot_worn(i);
    default:
      return bogus_slot_worn(i);
  }
}

const sstring TBeing::bogus_slot(wearSlotT i) const
{
//  vlogf(LOG_BUG, format("%s had bogus slot (%d) used.") %  getName() % i);
  vlogf(LOG_BUG, format("%s had bogus slot (%d) used.") %  getName() % i);
  return "BOGUS slot - Bug this!";
}

const sstring TBeing::default_body_slot(wearSlotT i) const
{
  switch (i) {
    case WEAR_FINGER_R:
      return "right finger";
    case WEAR_FINGER_L:
      return "left finger";
    case WEAR_NECK:
      return "neck";
    case WEAR_BODY:
      return "body";
    case WEAR_HEAD:
      return "head";
    case WEAR_EX_LEG_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return bogus_slot(i);
    case WEAR_LEG_R:
      return "right leg";
    case WEAR_LEG_L:
      return "left leg";
    case WEAR_FOOT_R:
      return "right foot";
    case WEAR_FOOT_L:
      return "left foot";
    case WEAR_HAND_R:
      return "right hand";
    case WEAR_HAND_L:
      return "left hand";
    case WEAR_ARM_R:
      return "right arm";
    case WEAR_ARM_L:
      return "left arm";
    case WEAR_BACK:
      return "back";
    case WEAR_WAIST:
      return "waist";
    case WEAR_WRIST_R:
      return "right wrist";
    case WEAR_WRIST_L:
      return "left wrist";
    case HOLD_LEFT:
      return "left hand";
    case HOLD_RIGHT:
      return "right hand";
    default:
      return "somewhere";
  }
}

const sstring TBeing::describeBodySlot(wearSlotT i) const
{
  char buf[160];

  if (IS_SET(specials.act, ACT_SKELETON)) 
    sprintf(buf, "skeletal %s", describeBodySlot2(i).c_str());
  else if (IS_SET(specials.act, ACT_ZOMBIE)) 
    sprintf(buf, "rotting %s", describeBodySlot2(i).c_str());
  else if (IS_SET(specials.act, ACT_GHOST)) 
    sprintf(buf, "ghostly %s", describeBodySlot2(i).c_str());
  else if (isLimbFlags(i, PART_TRANSFORMED)) {
    sprintf(buf, "%s", describeTransBodySlot(i).c_str());
  } else 
    sprintf(buf, describeBodySlot2(i).c_str());

  return buf;
}

unsigned short TBeing::getMaterial(wearSlotT pos) const
{
  if((hasQuestBit(TOG_PEGLEG_R) && pos==WEAR_LEG_R) ||
     (hasQuestBit(TOG_PEGLEG_L) && pos==WEAR_LEG_L))
    return MAT_WOOD;

  if((hasQuestBit(TOG_HOOK_HAND_R) && pos==WEAR_HAND_R) ||
     (hasQuestBit(TOG_HOOK_HAND_L) && pos==WEAR_HAND_L))
    return MAT_STEEL;

  return TThing::getMaterial();
}

const sstring TBeing::describeBodySlot2(wearSlotT i) const
{
  switch (getMyRace()->getBodyType()) {
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_GOLEM:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
      if(hasQuestBit(TOG_PEGLEG_R) && i==WEAR_LEG_R)
	return "right pegleg";
      if(hasQuestBit(TOG_PEGLEG_L) && i==WEAR_LEG_L)
	return "left pegleg";
      if(hasQuestBit(TOG_HOOK_HAND_R) && i==WEAR_HAND_R)
	return "right hook hand";
      if(hasQuestBit(TOG_HOOK_HAND_L) && i==WEAR_HAND_L)
	return "left hook hand";
      
      return default_body_slot(i);
    case BODY_OTYUGH:
      switch (i) {
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_LEG_R:
        case WEAR_FOOT_R:
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return default_body_slot(i);
        case WEAR_HEAD:
          return "eye stalk";
        case WEAR_ARM_R:
          return "right stalk";
        case WEAR_ARM_L:
          return "left stalk";
        case WEAR_EX_LEG_R:
          return "back leg";
        case WEAR_EX_FOOT_R:
          return "back foot";
        default:
          return bogus_slot(i);
      }
    case BODY_ANT:
    case BODY_INSECTOID:
      switch (i) {
        // based on ant: all 6 legs hook to thorax (front section)
        // all major organs are in abdomen (rear section)
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
          return default_body_slot(i);
	case WEAR_EX_LEG_R:
          return "back, right leg";
	case WEAR_EX_LEG_L:
          return "back, left leg";
	case WEAR_EX_FOOT_L:
          return "middle, left leg";
	case WEAR_EX_FOOT_R:
          return "middle, right leg";
        case WEAR_WAIST:
          return "thorax";
        case WEAR_BODY:
          return "abdomen";
        default:
          return bogus_slot(i);
      }
    case BODY_PIERCER:
      return "body";
    case BODY_MOSS:
      return "growth";
    case BODY_ELEMENTAL:
      if (isname("[air]", name))
        return "air"; 
      else if (isname("[earth]", name))
        return "earth"; 
      else if (isname("[fire]", name))
        return "fire"; 
      else if (isname("[water]", name))
        return "water"; 
      else
        return "body";
    case BODY_KUOTOA:
      switch (i) {
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return "fore arm";
        case WEAR_FOOT_R:
          return "webbed, right foot";
        case WEAR_FOOT_L:
          return "webbed, left foot";
        case WEAR_WAIST:
          return "fish-like tail";
        case WEAR_HAND_L:
        case HOLD_LEFT:
          return "webbed, left claw";
        case WEAR_HAND_R:
        case HOLD_RIGHT:
          return "webbed, right claw";
        case WEAR_NECK:
          return "thick neck";
        case WEAR_HEAD:
          return "fish head";
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_CRUSTACEAN:
      switch (i) {
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return "fore arm";
        case WEAR_BODY:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
          return default_body_slot(i);
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_HAND_L:
        case HOLD_LEFT:
          return "left claw";
        case WEAR_HAND_R:
        case HOLD_RIGHT:
          return "right claw";
        default:
          return bogus_slot(i);
      }
    case BODY_DJINN:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return bogus_slot(i);
        case WEAR_LEG_R:
        case WEAR_FOOT_R:
          return "smoke";
        default:
          return default_body_slot(i);
      }
    case BODY_MERMAID:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return bogus_slot(i);
        case WEAR_LEG_R:
          return "tail";
        case WEAR_FOOT_R:
          return "tail fin";
        default:
          return default_body_slot(i);
      }
    case BODY_FISHMAN:
      switch (i) {
        case WEAR_HAND_L:
          return "finned, left hand";
        case WEAR_HAND_R:
          return "finned, right hand";
        case WEAR_FOOT_L:
          return "finned, left foot";
        case WEAR_FOOT_R:
          return "finned, right foot";
        case WEAR_NECK:
          return "gilled neck";
        case WEAR_BODY:
          return "scaled body";
        case WEAR_BACK:
          return "scaled back";
        case WEAR_HEAD:
          return "scaled head";
        default:
          return default_body_slot(i);
      }
    case BODY_FROGMAN:
      switch (i) {
        case WEAR_HAND_L:
          return "webbed, left hand";
        case WEAR_HAND_R:
          return "webbed, right hand";
        case WEAR_FOOT_L:
          return "webbed, left foot";
        case WEAR_FOOT_R:
          return "webbed, right foot";
        default:
          return default_body_slot(i);
      }
    case BODY_MANTICORE:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, right, front leg";
        case WEAR_LEG_L:
          return "lion-like, left, front leg";
        case WEAR_EX_LEG_R:
          return "lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "bat-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "bat-like, left wing";
        case WEAR_WAIST:
          return "dragon-like tail";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BACK:
          return "lion-like back";
        case WEAR_BODY:
          return "lion-like body";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_GRIFFON:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, front, right leg";
        case WEAR_LEG_L:
          return "lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "eagle-like, left wing";
        case WEAR_WAIST:
          return "lion-like tail";
        case WEAR_HEAD:
          return "eagle-like head";
        case WEAR_BACK:
          return "lion-like back";
        case WEAR_BODY:
          return "lion-like body";
        case WEAR_NECK:
          return "eagle-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_SHEDU:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "bull-like, right hoof";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "bull-like, left hoof";
        case WEAR_LEG_R:
          return "bull-like, front, right leg";
        case WEAR_LEG_L:
          return "bull-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "bull-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "bull-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "bull-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "bull-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "bird-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "bird-like, left wing";
        case WEAR_WAIST:
          return "bull-like tail";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BACK:
          return "bull-like back";
        case WEAR_BODY:
          return "bull-like body";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_SPHINX:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, front, right leg";
        case WEAR_LEG_L:
          return "lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "eagle-like, left wing";
        case WEAR_WAIST:
          return "lion-like tail";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BACK:
          return "lion-like back";
        case WEAR_BODY:
          return "lion-like body";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_CENTAUR:
      switch (i) {
        case HOLD_RIGHT: 
        case WEAR_HAND_R:
          return "right hand";
        case HOLD_LEFT:
        case WEAR_HAND_L:
          return "left hand";
        case WEAR_FOOT_R:
          return "front, right hoof";
        case WEAR_FOOT_L:
          return "front, left hoof";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_EX_FOOT_R:
          return "back, right hoof";
        case WEAR_EX_FOOT_L:
          return "back, left hoof";
        case WEAR_ARM_R:
          return "right arm";
        case WEAR_ARM_L:
          return "left arm";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BODY:
          return "human-like body";
        case WEAR_BACK:
          return "human-like back";
        case WEAR_WAIST:
          return "horse-like body";
        case WEAR_WRIST_R:
          return "right wrist";
        case WEAR_WRIST_L:
          return "left wrist";
        case WEAR_FINGER_R:
          return "right finger";
        case WEAR_FINGER_L:
          return "left finger";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_SIMAL:
      switch (i) {
        case HOLD_RIGHT: 
        case WEAR_HAND_R:
          return "right hand";
        case HOLD_LEFT:
        case WEAR_HAND_L:
          return "left hand";
        case WEAR_FOOT_R:
          return "front, right paw";
        case WEAR_FOOT_L:
          return "front, left paw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_EX_FOOT_R:
          return "back, right paw";
        case WEAR_EX_FOOT_L:
          return "back, left paw";
        case WEAR_ARM_R:
          return "right arm";
        case WEAR_ARM_L:
          return "left arm";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BODY:
          return "feline-like body";
        case WEAR_BACK:
          return "feline-like back";
        case WEAR_WAIST:
          return "feline-like body";
        case WEAR_WRIST_R:
          return "right wrist";
        case WEAR_WRIST_L:
          return "left wrist";
        case WEAR_FINGER_R:
          return "right finger";
        case WEAR_FINGER_L:
          return "left finger";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_LAMIA:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_HAND_R:
          return "right hand";
        case HOLD_LEFT:
        case WEAR_HAND_L:
          return "left hand";
        case WEAR_FOOT_R:
          return "front, right paw";
        case WEAR_FOOT_L:
          return "front, left paw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_EX_FOOT_R:
          return "back, right paw";
        case WEAR_EX_FOOT_L:
          return "back, left paw";
        case WEAR_ARM_R:
          return "right arm";
        case WEAR_ARM_L:
          return "left arm";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BODY:
          return "human-like body";
        case WEAR_BACK:
          return "human-like back";
        case WEAR_WAIST:
          return "lion-like body";
        case WEAR_WRIST_R:
          return "right wrist";
        case WEAR_WRIST_L:
          return "left wrist";
        case WEAR_FINGER_R:
          return "right finger";
        case WEAR_FINGER_L:
          return "left finger";
        case WEAR_NECK:
          return "human-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_LAMMASU:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, front, right leg";
        case WEAR_LEG_L:
          return "lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "eagle-like, left wing";
        case WEAR_WAIST:
          return "lion-like tail";
        case WEAR_HEAD:
          return "human-like head";
        case WEAR_BACK:
          return "lion-like back";
        case WEAR_BODY:
          return "lion-like body";
        case WEAR_NECK:
          return "lion-like mane";
        default:
          return bogus_slot(i);
      }
    case BODY_WYVERN:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "right claw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "left claw";
        case WEAR_HAND_L:
          return "left talon";
        case WEAR_HAND_R:
          return "right talon";
        case WEAR_LEG_R:
          return "right leg";
        case WEAR_LEG_L:
          return "left leg";
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case WEAR_WAIST:
          return "barbed tail";
        case WEAR_BACK:
        case WEAR_BODY:
        case WEAR_NECK:
        case WEAR_HEAD:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_DRAGONNE:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right claw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left claw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_EX_FOOT_L:
          return "lion-like, back, left claw";
        case WEAR_EX_FOOT_R:
          return "lion-like, back, right claw";
        case WEAR_ARM_R:
          return "dragon-like, right wing";
        case WEAR_ARM_L:
          return "dragon-like, left wing";
        case WEAR_WAIST:
          return "tail";
        case WEAR_NECK:
          return "lion-like mane";
        case WEAR_HEAD:
          return "lion-like head";
        case WEAR_BACK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_HIPPOGRIFF:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, front, right leg";
        case WEAR_LEG_L:
          return "lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "horse-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "horse-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "horse-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "horse-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "eagle-like, left wing";
        case WEAR_WAIST:
          return "horse-like tail";
        case WEAR_HEAD:
          return "eagle-like head";
        case WEAR_BACK:
          return "horse-like back";
        case WEAR_BODY:
          return "horse-like body";
        case WEAR_NECK:
          return "eagle-like neck";
        default:
          return bogus_slot(i);
      }
    case BODY_CHIMERA:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "lion-like, right paw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "lion-like, left paw";
        case WEAR_LEG_R:
          return "lion-like, front, right leg";
        case WEAR_LEG_L:
          return "lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "goat-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "goat-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "goat-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "goat-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "dragon-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "dragon-like, left wing";
        case WEAR_WAIST:
          return "goat-like tail";
        case WEAR_HEAD:
          return "lion-like head";
        case WEAR_BACK:
          return "goat-like back";
        case WEAR_BODY:
          return "lion-like body";
        case WEAR_NECK:
          return "lion-like mane";
        default:
          return bogus_slot(i);
      }
    case BODY_DRAGON:
      switch (i) {
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "right claw";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "left claw";
        case WEAR_HAND_L:
          return "left talon";
        case WEAR_HAND_R:
          return "right talon";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_EX_FOOT_L:
          return "back, left claw";
        case WEAR_EX_FOOT_R:
          return "back, right claw";
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case WEAR_WAIST:
          return "tail";
        case WEAR_BACK:
        case WEAR_BODY:
        case WEAR_NECK:
        case WEAR_HEAD:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_COATL:
      switch (i) {
        case WEAR_BODY:
        case WEAR_HEAD:
          return default_body_slot(i);
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        default:
          return bogus_slot(i);
      }
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_NAGA:
      if ((i == WEAR_BODY) || (i == WEAR_HEAD))
        return default_body_slot(i);

      return bogus_slot(i);
    case BODY_SPIDER:
    case BODY_CENTIPEDE:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_R:
        case WEAR_EX_FOOT_L:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
          return "leg";
        case HOLD_LEFT:
          return "left feeler";
        case HOLD_RIGHT:
          return "right feeler";
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_HEAD:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_OCTOPUS:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_ARM_L:
        case WEAR_ARM_R:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case HOLD_LEFT:
        case HOLD_RIGHT:
          return "tentacle";
        case WEAR_BODY:
        case WEAR_HEAD:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_BIRD:
      switch (i) {
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case HOLD_LEFT:
          return "left talon";
        case HOLD_RIGHT:
          return "right talon";
        case WEAR_WAIST:
          return "tail feathers";
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }

    case BODY_BAT:
      switch (i) {
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case HOLD_LEFT:
          return "left talon";
        case HOLD_RIGHT:
          return "right talon";
      case WEAR_WAIST:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
      
    case BODY_TREE:
      switch (i) {
        case WEAR_BODY:
          return "trunk";
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return "branches";
        default:
          return bogus_slot(i);
      }
    case BODY_PARASITE:
    case BODY_SLIME:
      if (i == WEAR_BODY)
        return default_body_slot(i);
      return bogus_slot(i);
    case BODY_ORB:
      if (i == WEAR_BODY)
        return "orb-like body";
      return bogus_slot(i);
    case BODY_VEGGIE:
      if ((i == WEAR_BODY) || (i == WEAR_ARM_L) || (i == WEAR_ARM_R)) 
        return default_body_slot(i);
      return bogus_slot(i);
    case BODY_DEMON:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "right wing";
        case WEAR_EX_LEG_L:
          return "left wing";
        default:
          return default_body_slot(i);
      }
    case BODY_LION:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right paw";
        case WEAR_FOOT_L:
          return "front, left paw";
        case WEAR_EX_FOOT_R:
          return "back, right paw";
        case WEAR_EX_FOOT_L:
          return "back, left paw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left paw";
        case HOLD_RIGHT:
          return "right paw";
        case WEAR_WAIST:
          return "tail";
        case WEAR_NECK:
          return "mane";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_BACK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right paw";
        case WEAR_FOOT_L:
          return "front, left paw";
        case WEAR_EX_FOOT_R:
          return "back, right paw";
        case WEAR_EX_FOOT_L:
          return "back, left paw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
        case WEAR_HAND_L:
          return "left paw";
        case HOLD_RIGHT:
        case WEAR_HAND_R:
          return "right paw";
        case WEAR_WAIST:
          return "tail";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_NECK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_PIG:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right foot";
        case WEAR_FOOT_L:
          return "front, left foot";
        case WEAR_EX_FOOT_R:
          return "back, right foot";
        case WEAR_EX_FOOT_L:
          return "back, left foot";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left foot";
        case HOLD_RIGHT:
          return "right foot";
        case WEAR_WAIST:
          return "tail";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_NECK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_TURTLE:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right claw";
        case WEAR_FOOT_L:
          return "front, left claw";
        case WEAR_EX_FOOT_R:
          return "back, right claw";
        case WEAR_EX_FOOT_L:
          return "back, left claw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left claw";
        case HOLD_RIGHT:
          return "right claw";
        case WEAR_BODY:
        case WEAR_BACK:
          return "shell";
        case WEAR_HEAD:
        case WEAR_NECK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_PEGASUS:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right hoof";
        case WEAR_FOOT_L:
          return "front, left hoof";
        case WEAR_EX_FOOT_R:
          return "back, right hoof";
        case WEAR_EX_FOOT_L:
          return "back, left hoof";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left hoof";
        case HOLD_RIGHT:
          return "right hoof";
        case WEAR_WAIST:
          return "tail";
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_NECK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_FOUR_HOOF:
    case BODY_ELEPHANT:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right hoof";
        case WEAR_FOOT_L:
          return "front, left hoof";
        case WEAR_EX_FOOT_R:
          return "back, right hoof";
        case WEAR_EX_FOOT_L:
          return "back, left hoof";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left hoof";
        case HOLD_RIGHT:
          return "right hoof";
        case WEAR_WAIST:
          return "tail";
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_NECK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_BAANTA:
      switch (i) {
        case WEAR_FOOT_R:
          return "right clawed foot";
        case WEAR_FOOT_L:
          return "left clawed foot";
        case HOLD_LEFT:
        case WEAR_HAND_L:
          return "left clawed hand";
        case HOLD_RIGHT:
        case WEAR_HAND_R:
          return "right clawed hand";
        case WEAR_LEG_R:
          return "right leg";
        case WEAR_LEG_L:
          return "left leg";
        case WEAR_WAIST:
          return "tail";
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_NECK:
        case WEAR_BODY:
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_AMPHIBEAN:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "webbed, right hand";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "webbed, left hand";
        case WEAR_EX_FOOT_R:
          return "webbed, right foot";
        case WEAR_EX_FOOT_L:
          return "webbed, left foot";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_WAIST:
          return "tail";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_NECK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_FROG:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return "webbed, right hand";
        case HOLD_LEFT:
        case WEAR_FOOT_L:
          return "webbed, left hand";
        case WEAR_EX_FOOT_R:
          return "webbed, right foot";
        case WEAR_EX_FOOT_L:
          return "webbed, left foot";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_NECK:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_MIMIC:
      switch (i) {
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case BODY_MEDUSA:
      switch (i) {
        case WEAR_HEAD:
          return "head covered by snake-hair";
        default:
          return default_body_slot(i);
      }
    case BODY_WYVELIN:
      switch (i) {
        case WEAR_EX_LEG_R:
          return "back, right leg";
        case WEAR_EX_LEG_L:
          return "back, left leg";
        case WEAR_FOOT_R:
          return "front, right paw";
        case WEAR_FOOT_L:
          return "front, left paw";
        case WEAR_EX_FOOT_R:
          return "back, right paw";
        case WEAR_EX_FOOT_L:
          return "back, left paw";
        case WEAR_LEG_R:
          return "front, right leg";
        case WEAR_LEG_L:
          return "front, left leg";
        case HOLD_LEFT:
          return "left paw";
        case HOLD_RIGHT:
          return "right paw";
        case WEAR_WAIST:
          return "tail";
        case WEAR_ARM_R:
          return "right wing";
        case WEAR_ARM_L:
          return "left wing";
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_NECK:
        case WEAR_BODY:
          return default_body_slot(i);
        default:
          return bogus_slot(i);
      }
    case MAX_BODY_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Unknown body type (%d) body slot") %  getMyRace()->getBodyType());
  return default_body_slot(i);
}

const sstring TBeing::describeEquipmentSlot(wearSlotT i) const
{
  if (isLimbFlags(i, PART_TRANSFORMED)) {
    return describeTransEquipSlot(i);
  }

  switch (getMyRace()->getBodyType()) {
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_GOLEM:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
      if(hasQuestBit(TOG_PEGLEG_R) && i==WEAR_LEG_R)
	return "Worn on right pegleg";
      if(hasQuestBit(TOG_PEGLEG_L) && i==WEAR_LEG_L)
	return "Worn on left pegleg";
      if(hasQuestBit(TOG_HOOK_HAND_R) && i==WEAR_HAND_R)
	return "Worn on right hook hand";
      if(hasQuestBit(TOG_HOOK_HAND_L) && i==WEAR_HAND_L)
	return "Worn on left hook hand";
      
      return defaultEquipmentSlot(i);
    case BODY_OTYUGH:
      switch (i) {
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_LEG_R:
        case WEAR_FOOT_R:
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return defaultEquipmentSlot(i);
        case WEAR_HEAD:
          return "Worn about eye stalk";
        case WEAR_ARM_R:
          return "Worn on right stalk";
        case WEAR_ARM_L:
          return "Worn on left stalk";
        case WEAR_EX_LEG_R:
          return "Worn on back leg";
        case WEAR_EX_FOOT_R:
          return "Worn on back foot";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_INSECTOID:
    case BODY_ANT:
      switch (i) {
        case WEAR_HEAD:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
	case WEAR_EX_LEG_R:
	case WEAR_EX_LEG_L:
          return defaultEquipmentSlot(i);
	case WEAR_EX_FOOT_L:
          return "middle, left leg";
	case WEAR_EX_FOOT_R:
          return "middle, right leg";
        case WEAR_BACK:
          return "Worn about thorax";
        case WEAR_WAIST:
          return "Worn on thorax";
        case WEAR_BODY:
          return "Worn about abdomen";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_PIERCER:
      return defaultEquipmentSlot(i);
    case BODY_MOSS:
      return "Entangled in the growth";
    case BODY_ELEMENTAL:
      if (isname("[air]", name))
        return "Suspended in air";
      else if (isname("[earth]", name))
        return "Embedded in earth";
      else if (isname("[fire]", name))
        return "Surrounded by fire";
      else if (isname("[water]", name))
        return "Under the water";
      else
        return "body";
    case BODY_KUOTOA:
      switch (i) {
        case WEAR_ARM_L:
          return "Worn on left fore arm";
        case WEAR_ARM_R:
          return "Worn on right fore arm";
        case WEAR_WAIST:
          return "Worn on fish-like tail";
        case HOLD_LEFT:
          return "Held in webbed, left claw";
        case HOLD_RIGHT:
          return "Held in webbed, right claw";
        case WEAR_BODY:
        case WEAR_BACK:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_CRUSTACEAN:
      switch (i) {
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return "Worn about fore arm";
        case WEAR_BODY:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
          return defaultEquipmentSlot(i);
        case WEAR_EX_LEG_R:
          return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on back, left leg";
        case WEAR_HAND_L:
        case HOLD_LEFT:
          return "Held in left claw";
        case WEAR_HAND_R:
        case HOLD_RIGHT:
          return "Held in right claw";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_DJINN:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
        case WEAR_FOOT_R:
          return "Hanging in smoke";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_MERMAID:
      switch (i) {
        case WEAR_LEG_L:
        case WEAR_FOOT_L:
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          return "Worn on tail";
        case WEAR_FOOT_R:
          return "Worn on tail fin";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_FISHMAN:
      switch (i) {
        case WEAR_HAND_L:
          return "Worn on finned, left hand";
        case WEAR_HAND_R:
          return "Worn on finned, right hand";
        case WEAR_FOOT_L:
          return "Worn on finned, left foot";
        case WEAR_FOOT_R:
          return "Worn on finned, right foot";
        case WEAR_NECK:
          return "Worn on gilled neck";
        case WEAR_BODY:
          return "Worn on scaled body";
        case WEAR_BACK:
          return "Worn on scaled back";
        case WEAR_HEAD:
          return "Worn on scaled head";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_FROGMAN:
      switch (i) {
        case WEAR_HAND_L:
          return "Worn on webbed, left hand";
        case WEAR_HAND_R:
          return "Worn on webbed, right hand";
        case WEAR_FOOT_L:
          return "Worn on webbed, left foot";
        case WEAR_FOOT_R:
          return "Worn on webbed, right foot";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_MANTICORE:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on bat-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on bat-like, left wing";
        case WEAR_WAIST:
          return "Worn on dragon-like tail";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BACK:
          return "Worn on lion-like back as saddle";
        case WEAR_BODY:
          return "Worn on lion-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_GRIFFON:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on eagle-like, left wing";
        case WEAR_WAIST:
          return "Worn on lion-like tail";
        case WEAR_HEAD:
          return "Worn on eagle-like head";
        case WEAR_BACK:
          return "Worn on lion-like back as saddle";
        case WEAR_BODY:
          return "Worn on lion-like body";
        case WEAR_NECK:
          return "Worn on eagle-like neck";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_SHEDU:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in bull-like, right hoof";
        case WEAR_FOOT_R:
          return "Worn on bull-like, right hoof";
        case HOLD_LEFT:
          return "Held in bull-like, left hoof";
        case WEAR_FOOT_L:
          return "Worn on bull-like, left hoof";
        case WEAR_LEG_R:
          return "Worn on bull-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on bull-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on bull-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on bull-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on bull-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on bull-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on bird-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on bird-like, left wing";
        case WEAR_WAIST:
          return "Worn on bull-like tail";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BACK:
          return "Worn on bull-like back as saddle";
        case WEAR_BODY:
          return "Worn on bull-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_SPHINX:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on eagle-like, left wing";
        case WEAR_WAIST:
          return "Worn on lion-like tail";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BACK:
          return "Worn on lion-like back as saddle";
        case WEAR_BODY:
          return "Worn on lion-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_CENTAUR:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right hoof";
        case WEAR_FOOT_L:
          return "Worn on front, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right hoof";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left hoof";
        case WEAR_WAIST:
          return "Worn on horse-like body";
        case WEAR_BACK:
          return "Worn on human-like back";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BODY:
          return "Worn on human-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
        case WEAR_HAND_R:
        case WEAR_HAND_L:
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case HOLD_LEFT:
        case HOLD_RIGHT:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_SIMAL:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right paw";
        case WEAR_FOOT_L:
          return "Worn on front, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right paw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left paw";
        case WEAR_WAIST:
          return "Worn on feline-like body";
        case WEAR_BACK:
          return "Worn on feline-like back";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BODY:
          return "Worn on feline-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
        case WEAR_HAND_R:
        case WEAR_HAND_L:
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case HOLD_LEFT:
        case HOLD_RIGHT:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_LAMIA:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right paw";
        case WEAR_FOOT_L:
          return "Worn on front, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right paw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left paw";
        case WEAR_WAIST:
          return "Worn on lion-like body";
        case WEAR_BACK:
          return "Worn on human-like back";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BODY:
          return "Worn on human-like body";
        case WEAR_NECK:
          return "Worn on human-like neck";
        case WEAR_FINGER_R:
        case WEAR_FINGER_L:
        case WEAR_HAND_R:
        case WEAR_HAND_L:
        case WEAR_WRIST_R:
        case WEAR_WRIST_L:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case HOLD_LEFT:
        case HOLD_RIGHT:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_LAMMASU:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on lion-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on lion-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on lion-like, back, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on lion-like, back, right paw";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on eagle-like, left wing";
        case WEAR_WAIST:
          return "Worn on lion-like tail";
        case WEAR_HEAD:
          return "Worn on human-like head";
        case WEAR_BACK:
          return "Worn on lion-like back as saddle";
        case WEAR_BODY:
          return "Worn on lion-like body";
        case WEAR_NECK:
          return "Worn on lion-like mane";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_WYVERN:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in right claw";
        case WEAR_FOOT_R:
          return "Worn on right claw";
        case HOLD_LEFT:
          return "Held in left claw";
        case WEAR_FOOT_L:
          return "Worn on left claw";
        case WEAR_HAND_L:
          return "Worn on left talon";
        case WEAR_HAND_R:
          return "Worn on right talon";
        case WEAR_LEG_R:
          return "Worn on right leg";
        case WEAR_LEG_L:
          return "Worn on left leg";
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case WEAR_WAIST:
          return "Worn on barbed tail";
        case WEAR_BACK:
        case WEAR_BODY:
        case WEAR_NECK:
        case WEAR_HEAD:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_DRAGONNE:
      switch (i) {
        case HOLD_RIGHT:
          return "Clutched in lion-like, right claw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right claw";
        case HOLD_LEFT:
          return "Clutched in lion-like, left claw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left claw";
        case WEAR_LEG_R:
          return "Worn on front, right leg";
        case WEAR_LEG_L:
          return "Worn on front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on lion-like, back, left claw";
        case WEAR_EX_FOOT_R:
          return "Worn on lion-like, back, right claw";
        case WEAR_ARM_R:
          return "Worn on dragon-like, right wing";
        case WEAR_ARM_L:
          return "Worn on dragon-like, left wing";
        case WEAR_WAIST:
          return "Around a tail";
        case WEAR_NECK:
          return "Worn about lion-like mane";
        case WEAR_HEAD:
          return "Worn on lion-like head";
        case WEAR_BACK:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_HIPPOGRIFF:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on horse-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on horse-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on horse-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on horse-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on eagle-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on eagle-like, left wing";
        case WEAR_WAIST:
          return "Worn on horse-like tail";
        case WEAR_HEAD:
          return "Worn on eagle-like head";
        case WEAR_BACK:
          return "Worn on horse-like back as saddle";
        case WEAR_BODY:
          return "Worn on horse-like body";
        case WEAR_NECK:
          return "Worn on eagle-like neck";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_CHIMERA:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in lion-like, right paw";
        case WEAR_FOOT_R:
          return "Worn on lion-like, right paw";
        case HOLD_LEFT:
          return "Held in lion-like, left paw";
        case WEAR_FOOT_L:
          return "Worn on lion-like, left paw";
        case WEAR_LEG_R:
          return "Worn on lion-like, front, right leg";
        case WEAR_LEG_L:
          return "Worn on lion-like, front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on goat-like, back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on goat-like, back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on goat-like, back, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on goat-like, back, right hoof";
        case WEAR_HAND_R:
        case WEAR_ARM_R:
          return "Worn on dragon-like, right wing";
        case WEAR_HAND_L:
        case WEAR_ARM_L:
          return "Worn on dragon-like, left wing";
        case WEAR_WAIST:
          return "Worn on goat-like tail";
        case WEAR_HEAD:
          return "Worn on lion-like head";
        case WEAR_BACK:
          return "Worn on goat-like back as saddle";
        case WEAR_BODY:
          return "Worn on lion-like body";
        case WEAR_NECK:
          return "Worn on lion-like mane";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_DRAGON:
      switch (i) {
        case HOLD_RIGHT:
          return "Held in right claw";
        case WEAR_FOOT_R:
          return "Worn on right claw";
        case HOLD_LEFT:
          return "Held in right claw";
        case WEAR_FOOT_L:
          return "Worn on left claw";
        case WEAR_HAND_L:
          return "Worn on left talon";
        case WEAR_HAND_R:
          return "Worn on right talon";
        case WEAR_LEG_R:
          return "Worn on front, right leg";
        case WEAR_LEG_L:
          return "Worn on front, left leg";
        case WEAR_EX_LEG_R:
          return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          return "Worn on back, left leg";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left claw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right claw";
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_BODY:
        case WEAR_NECK:
        case WEAR_HEAD:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_COATL:
      switch (i) {
        case WEAR_BODY:
        case WEAR_HEAD:
          return defaultEquipmentSlot(i);
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_NAGA:
      if ((i == WEAR_BODY) || (i == WEAR_HEAD))
        return defaultEquipmentSlot( i);
      return bogus_slot_worn(i);
    case BODY_SPIDER:
    case BODY_CENTIPEDE:
      switch (i) {
        case WEAR_BACK:
        case WEAR_HEAD:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_L:
        case WEAR_EX_FOOT_R:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
          return "Worn on a leg";
        case HOLD_RIGHT:
        case HOLD_LEFT:
          return "Grasped by an appendage";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_OCTOPUS:
      switch (i) {
        case WEAR_HEAD:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_ARM_L:
        case WEAR_ARM_R:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
          return "Around a tentacle";
        case HOLD_RIGHT:
        case HOLD_LEFT:
          return "Grasped in tentacle";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_BIRD:
      switch (i) {
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case HOLD_LEFT:
          return "Held in left talon";
        case HOLD_RIGHT:
          return "Held in right talon";
        case WEAR_WAIST:
          return "Worn on tail feathers";
        case WEAR_BODY:
        case WEAR_HEAD:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_BAT:
      switch (i) {  
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case HOLD_LEFT:
          return "Held in left talon";
        case HOLD_RIGHT:
          return "Held in right talon";
        case WEAR_WAIST:
        case WEAR_BODY:
        case WEAR_HEAD:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
   case BODY_TREE:
      switch (i) {
        case WEAR_BODY:
          return "Worn on trunk";
        case WEAR_ARM_L:
        case WEAR_ARM_R:
          return "Worn on branches";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_PARASITE:
    case BODY_SLIME:
    case BODY_ORB:
      if (i == WEAR_BODY)
        return defaultEquipmentSlot(i);
      return bogus_slot_worn(i);
    case BODY_VEGGIE:
      if ((i == WEAR_BODY) || (i == WEAR_ARM_L) || (i == WEAR_ARM_R))
        return defaultEquipmentSlot(i);
      return bogus_slot_worn(i);
    case BODY_DEMON:
      switch (i) {
        case WEAR_EX_LEG_L:
          return "Worn on left wing";
        case WEAR_EX_LEG_R:
          return "Worn on right wing";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_LION:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right paw";
        case WEAR_FOOT_L:
          return "Worn on front, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right paw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left paw";
        case HOLD_LEFT:
          return "Held in left paw";
        case HOLD_RIGHT:
          return "Held in right paw";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_NECK:
          return "Worn on mane";
        case WEAR_HEAD:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right paw";
        case WEAR_FOOT_L:
          return "Worn on front, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right paw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left paw";
        case WEAR_HAND_L:
          return "Worn on left paw";
        case WEAR_HAND_R:
          return "Worn on right paw";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_PIG:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right foot";
        case WEAR_FOOT_L:
          return "Worn on front, left foot";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right foot";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left foot";
        case HOLD_LEFT:
          return "Held by left foot";
        case HOLD_RIGHT:
          return "Held by right foot";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_TURTLE:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right claw";
        case WEAR_FOOT_L:
          return "Worn on front, left claw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right claw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left claw";
        case HOLD_LEFT:
          return "Held in left claw";
        case HOLD_RIGHT:
          return "Held in right claw";
        case WEAR_BACK:
        case WEAR_BODY:
          return "Worn on shell as saddle";
        case WEAR_HEAD:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_PEGASUS:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right hoof";
        case WEAR_FOOT_L:
          return "Worn on front, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right hoof";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left hoof";
        case HOLD_LEFT:
          return "Held in left hoof";
        case HOLD_RIGHT:
          return "Held in right hoof";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_FOUR_HOOF:
    case BODY_ELEPHANT:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right hoof";
        case WEAR_FOOT_L:
          return "Worn on front, left hoof";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right hoof";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left hoof";
        case HOLD_LEFT:
          return "Held in left hoof";
        case HOLD_RIGHT:
          return "Held in right hoof";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_BAANTA:
      switch (i) {
        case WEAR_HEAD:
          return defaultEquipmentSlot(i);
        case WEAR_BACK:
          return "Worn on back as saddle";
        default:
          return bogus_slot_worn(i);
      }
    case BODY_AMPHIBEAN:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on webbed, right hand";
        case WEAR_FOOT_L:
          return "Worn on webbed, left hand";
        case WEAR_EX_FOOT_R:
          return "Worn on webbed, right foot";
        case WEAR_EX_FOOT_L:
          return "Worn on webbed, left foot";
        case HOLD_LEFT:
          return "Held in webbed, left hand";
        case HOLD_RIGHT:
          return "Held in webbed, right hand";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_FROG:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on webbed, right hand";
        case WEAR_FOOT_L:
          return "Worn on webbed, left hand";
        case WEAR_EX_FOOT_R:
          return "Worn on webbed, right foot";
        case WEAR_EX_FOOT_L:
          return "Worn on webbed, left foot";
        case HOLD_LEFT:
          return "Held in webbed, left hand";
        case HOLD_RIGHT:
          return "Held in webbed, right hand";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_HEAD:
        case WEAR_NECK:
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_MIMIC:
      switch (i) {
        case WEAR_BODY:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case BODY_MEDUSA:
      switch (i) {
        case WEAR_HEAD:
          return "Worn over snake-hair covered head";
        default:
          return defaultEquipmentSlot(i);
      }
    case BODY_WYVELIN:
      switch (i) {
        case WEAR_EX_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on back legs";
          else
            return "Worn on back, right leg";
        case WEAR_EX_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on back, left leg";
          return bogus_slot_worn(i);
        case WEAR_LEG_R:
          if (equipment[i] == equipment[WEAR_LEG_L])
            return "Worn on front legs";
          else
            return "Worn on front, right leg";
        case WEAR_LEG_L:
          if (!(equipment[i] == equipment[WEAR_LEG_R]))
            return "Worn on front, left leg";
          return bogus_slot_worn(i);
        case WEAR_FOOT_R:
          return "Worn on front, right paw";
        case WEAR_FOOT_L:
          return "Worn on front, left paw";
        case WEAR_EX_FOOT_R:
          return "Worn on back, right paw";
        case WEAR_EX_FOOT_L:
          return "Worn on back, left paw";
        case HOLD_LEFT:
          return "Held in left paw";
        case HOLD_RIGHT:
          return "Held in right paw";
        case WEAR_BACK:
          return "Worn on back as saddle";
        case WEAR_WAIST:
          return "Worn on tail";
        case WEAR_ARM_R:
          return "Worn on right wing";
        case WEAR_ARM_L:
          return "Worn on left wing";
        case WEAR_HEAD:
        case WEAR_BODY:
        case WEAR_NECK:
          return defaultEquipmentSlot(i);
        default:
          return bogus_slot_worn(i);
      }
    case MAX_BODY_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Unknown body type (%d) equip slot") %  getMyRace()->getBodyType());
  return defaultEquipmentSlot(i);
}

// returns TRUE if slot is connected to body
// returns false if there's a missing part in the chain
int TBeing::defaultLimbConnections(wearSlotT slot)
{
  int flags = PART_MISSING;

  switch (slot) {
    case WEAR_BODY:
      return TRUE;
      break;
    case WEAR_HEAD:
      if (!defaultLimbConnections(WEAR_NECK))
        return FALSE;
      else if (isLimbFlags(WEAR_NECK, flags))
        return FALSE;
      break;
    case WEAR_NECK:
    case WEAR_ARM_L:
    case WEAR_ARM_R:
    case WEAR_WAIST:
    case WEAR_BACK:
      if (!defaultLimbConnections(WEAR_BODY))
        return FALSE;
      else if (isLimbFlags(WEAR_BODY, flags))
        return FALSE;
      break;
    case WEAR_WRIST_L:
      if (!defaultLimbConnections(WEAR_ARM_L))
        return FALSE;
      else if (isLimbFlags(WEAR_ARM_L, flags))
        return FALSE;
      break;
    case WEAR_HAND_L:
      if (!defaultLimbConnections(WEAR_WRIST_L))
        return FALSE;
      else if (isLimbFlags(WEAR_WRIST_L, flags))
        return FALSE;
      break;
    case WEAR_FINGER_L:
    case HOLD_LEFT:
      if (!defaultLimbConnections(WEAR_HAND_L))
        return FALSE;
      else if (isLimbFlags(WEAR_HAND_L, flags))
        return FALSE;
      break;
    case WEAR_WRIST_R:
      if (!defaultLimbConnections(WEAR_ARM_R))
        return FALSE;
      else if (isLimbFlags(WEAR_ARM_R, flags))
        return FALSE;
      break;
    case WEAR_HAND_R:
      if (!defaultLimbConnections(WEAR_WRIST_R))
        return FALSE;
      else if (isLimbFlags(WEAR_WRIST_R, flags))
        return FALSE;
      break;
    case WEAR_FINGER_R:
    case HOLD_RIGHT:
      if (!defaultLimbConnections(WEAR_HAND_R))
        return FALSE;
      else if (isLimbFlags(WEAR_HAND_R, flags))
        return FALSE;
      break;
    case WEAR_LEG_L:
    case WEAR_LEG_R:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      if (!defaultLimbConnections(WEAR_WAIST))
        return FALSE;
      else if (isLimbFlags(WEAR_WAIST, flags))
        return FALSE;
      break;
    case WEAR_EX_FOOT_R:
      if (!defaultLimbConnections(WEAR_EX_LEG_R))
        return FALSE;
      else if (isLimbFlags(WEAR_EX_LEG_R, flags))
        return FALSE;
      break;
    case WEAR_EX_FOOT_L:
      if (!defaultLimbConnections(WEAR_EX_LEG_L))
        return FALSE;
      else if (isLimbFlags(WEAR_EX_LEG_L, flags))
        return FALSE;
      break;
    case WEAR_FOOT_L:
      if (!defaultLimbConnections(WEAR_LEG_L))
        return FALSE;
      else if (isLimbFlags(WEAR_LEG_L, flags))
        return FALSE;
      break;
    case WEAR_FOOT_R:
      if (!defaultLimbConnections(WEAR_LEG_R))
        return FALSE;
      else if (isLimbFlags(WEAR_LEG_R, flags))
        return FALSE;
      break;
    default:
      vlogf(LOG_BUG,format("Error in defaultLimbConnections: slot %d") %  slot);
      return FALSE;
  }
  return TRUE;
}

int TBeing::limbConnections(wearSlotT slot)
{
  mud_assert(slot >= MIN_WEAR && slot < MAX_WEAR,
      "Bad slot in limbConnections %s, %d", getName(), slot);

  if (slotChance(slot) <= 0) {
    vlogf(LOG_BUG,format("Bogus slot (%d) on char %s") % slot %getName());
    return FALSE;
  }
  if (slot == WEAR_BODY)
    return TRUE;

  switch (getMyRace()->getBodyType()) {
    case BODY_NONE:
    case BODY_HUMANOID:
    case BODY_GOLEM:
    case BODY_OWLBEAR:
    case BODY_MINOTAUR:
    case BODY_PIERCER:
    case BODY_MOSS:
    case BODY_ELEMENTAL:
    case BODY_PARASITE:
    case BODY_SLIME:
    case BODY_ORB:
    case BODY_VEGGIE:
    case BODY_MIMIC:
    case BODY_MEDUSA:
      return defaultLimbConnections(slot);
    case BODY_INSECTOID:
    case BODY_ANT:
      switch (slot) {
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_R:
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_WAIST);
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_OTYUGH:
      switch (slot) {
        case WEAR_HEAD:
        case WEAR_BACK:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_EX_LEG_R:
          return limbConnections(WEAR_BODY);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_KUOTOA:
      switch (slot) {
        case WEAR_ARM_L:
        case WEAR_ARM_R:
        case WEAR_WAIST:
        case WEAR_NECK:
        case WEAR_BACK:
          return limbConnections(WEAR_BODY);
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        case WEAR_HAND_L:
        case HOLD_LEFT:
          return limbConnections(WEAR_ARM_L);
        case WEAR_HAND_R:
        case HOLD_RIGHT:
          return limbConnections(WEAR_ARM_L);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_LEG_L:
        case WEAR_LEG_R:
          return limbConnections(WEAR_WAIST);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_CRUSTACEAN:
      switch (slot) {
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
          return limbConnections(WEAR_BODY);
        case HOLD_RIGHT:
          return limbConnections(WEAR_HAND_R);
        case HOLD_LEFT:
          return limbConnections(WEAR_HAND_L);
        case WEAR_HAND_L:
          return limbConnections(WEAR_ARM_L);
        case WEAR_HAND_R:
          return limbConnections(WEAR_ARM_R);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_DJINN:
    case BODY_MERMAID:
    case BODY_FROGMAN:
    case BODY_FISHMAN:
      return defaultLimbConnections(slot);
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_LAMMASU:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_DRAGON:
      switch (slot) {
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_ARM_R:   // wings
        case WEAR_ARM_L:
          return limbConnections(WEAR_BACK);
        case HOLD_LEFT:
          return limbConnections(WEAR_ARM_L);
        case HOLD_RIGHT:
          return limbConnections(WEAR_ARM_R);
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
          return limbConnections(WEAR_BODY);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_CENTAUR:
    case BODY_SIMAL:
    case BODY_LAMIA:
      switch (slot) {
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BODY);
        case WEAR_FINGER_R:
        case HOLD_RIGHT:
          return limbConnections(WEAR_HAND_R);
        case WEAR_FINGER_L:
        case HOLD_LEFT:
          return limbConnections(WEAR_HAND_L);
        case WEAR_HAND_R:
          return limbConnections(WEAR_WRIST_R);
        case WEAR_HAND_L:
          return limbConnections(WEAR_WRIST_L);
        case WEAR_WRIST_R:
          return limbConnections(WEAR_ARM_R);
        case WEAR_WRIST_L:
          return limbConnections(WEAR_ARM_L);
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
          return limbConnections(WEAR_WAIST);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_WYVERN:
      switch (slot) {
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_ARM_R:   // wings
        case WEAR_ARM_L:
          return limbConnections(WEAR_BACK);
        case HOLD_LEFT:
          return limbConnections(WEAR_ARM_L);
        case HOLD_RIGHT:
          return limbConnections(WEAR_ARM_R);
        case WEAR_LEG_R:
        case WEAR_LEG_L:
          return limbConnections(WEAR_BODY);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_PEGASUS:
      switch (slot) {
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_FOOT_L:
        case HOLD_LEFT:
          return limbConnections(WEAR_LEG_L);
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BACK);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_CHIMERA:
    case BODY_LION:
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
    case BODY_PIG:
    case BODY_FOUR_HOOF:
    case BODY_ELEPHANT:
    case BODY_TURTLE:
    case BODY_AMPHIBEAN:
    case BODY_FROG:
      switch (slot) {
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_FOOT_L:
        case HOLD_LEFT:
          return limbConnections(WEAR_LEG_L);
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_COATL:
      switch (slot) {
        case WEAR_HEAD:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_NAGA:
      switch (slot) {
        case WEAR_HEAD:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_SPIDER:
    case BODY_CENTIPEDE:
      switch (slot) {
        case WEAR_HEAD:
        case WEAR_BACK:
          return limbConnections(WEAR_BODY);
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_EX_FOOT_R:
        case WEAR_EX_FOOT_L:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
        case HOLD_RIGHT:
        case HOLD_LEFT:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_OCTOPUS:
      switch (slot) {
        case WEAR_HEAD:
          return limbConnections(WEAR_BODY);
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_FOOT_L:
        case WEAR_FOOT_R:
        case HOLD_RIGHT:
        case HOLD_LEFT:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }      
    case BODY_BIRD:
    case BODY_BAT:
      switch (slot) {
        case WEAR_LEG_L:
        case WEAR_LEG_R:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
        case WEAR_NECK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case HOLD_LEFT:
          return limbConnections(WEAR_ARM_L);
        case HOLD_RIGHT:
          return limbConnections(WEAR_ARM_R);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      } 
    case BODY_TREE:
      switch (slot) {
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BODY);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_DEMON:
      switch (slot) {
        case WEAR_EX_LEG_R:    // wings
        case WEAR_EX_LEG_L:
          return limbConnections(WEAR_BACK);
        default:
          return defaultLimbConnections(slot);
      }
    case BODY_BAANTA:
      switch (slot) {
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_LEG_R:
        case WEAR_LEG_L:
          return limbConnections(WEAR_WAIST);
        case WEAR_FOOT_L:
          return limbConnections(WEAR_LEG_L);
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_HAND_L:
          return limbConnections(WEAR_ARM_L);
        case WEAR_HAND_R:
          return limbConnections(WEAR_ARM_R);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case BODY_WYVELIN:
      switch (slot) {
        case WEAR_EX_LEG_R:
        case WEAR_EX_LEG_L:
        case WEAR_LEG_R:
        case WEAR_LEG_L:
        case WEAR_NECK:
        case WEAR_BACK:
        case WEAR_WAIST:
          return limbConnections(WEAR_BODY);
        case WEAR_HEAD:
          return limbConnections(WEAR_NECK);
        case WEAR_FOOT_L:
        case HOLD_LEFT:
          return limbConnections(WEAR_LEG_L);
        case HOLD_RIGHT:
        case WEAR_FOOT_R:
          return limbConnections(WEAR_LEG_R);
        case WEAR_EX_FOOT_R:
          return limbConnections(WEAR_EX_LEG_R);
        case WEAR_EX_FOOT_L:
          return limbConnections(WEAR_EX_LEG_L);
        case WEAR_ARM_R:
        case WEAR_ARM_L:
          return limbConnections(WEAR_BACK);
        default:
          vlogf(LOG_BUG,format("bogus check on %s for slot %d") % 
             getMyRace()->getSingularName() % slot);
          return FALSE;
      }
    case MAX_BODY_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Bogus body type (%d) in limb connections") %  getMyRace()->getBodyType());
  return defaultLimbConnections(slot);
}

spellNumT TBeing::getFormType() const
{
  int num;

  num = ::number(1, 100);
  switch (getMyRace()->getRace()) {
    case RACE_CRUSTACEAN:
      return TYPE_CLAW;
    case RACE_LYCANTH:
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_RODENT:
    case RACE_REPTILE:
    case RACE_BANSHEE:
    case RACE_FELINE:
    case RACE_TURTLE:
    case RACE_LION:
    case RACE_TIGER:
    case RACE_LEOPARD:
    case RACE_COUGAR:
    case RACE_BAT:
    case RACE_VAMPIREBAT:
    case RACE_KUOTOA:
    case RACE_BAANTA:
    case RACE_RATMEN:
    case RACE_WYVELIN:
      if (num <= 33)
        return (TYPE_BITE);
      else
        return (TYPE_CLAW);
    case RACE_INSECT:
    case RACE_FLYINSECT:
    case RACE_ANT:
    case RACE_ARACHNID:
      if (num <= 50)
        return (TYPE_BITE);
      else
        return (TYPE_STING);
    case RACE_FISH:
    case RACE_SNAKE:
    case RACE_COATL:
    case RACE_NAGA:
    case RACE_CANINE:
    case RACE_FROG:
      return (TYPE_BITE);
    case RACE_BIRDMAN:
    case RACE_BIRD:
    case RACE_PHOENIX:
      if (num < 35)
        return (TYPE_BEAK);
      else
        return (TYPE_CLAW);
    case RACE_GIANT:
    case RACE_GOLEM:
    case RACE_TYTAN:
    case RACE_ELEPHANT:
    case RACE_RHINO:
      if (num <= 30)
        return (TYPE_BLUDGEON);
      else if (num <= 60)
        return (TYPE_WALLOP);
      else
        return (TYPE_STRIKE);
    case RACE_ELEMENT:
      if (num <= 80) {
        if (isname("[air]", name))
          return TYPE_AIR;
        else if (isname("[earth]", name))
          return TYPE_EARTH;
        else if (isname("[fire]", name))
          return TYPE_FIRE;
        else if (isname("[water]", name))
          return TYPE_WATER;
      }
      return (TYPE_POUND);
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_TROLL:
      return (TYPE_CLAW);
    case RACE_TREE:
      return (TYPE_SMITE);
    case RACE_MFLAYER:
      if (num <= 60)
        return (TYPE_WHIP);
      else if (num < 80)
        return (TYPE_BITE);
      else
        return (TYPE_HIT);
    case RACE_WYVERN:
      if (num <= 60)
        return (TYPE_CLAW);
      else if (num <= 80)
        return (TYPE_BITE);
      else
        return (TYPE_STING);
    case RACE_PRIMATE:
    case RACE_AMPHIB:
    case RACE_BEAR:
    case RACE_DRAGONNE:
    case RACE_HIPPOGRIFF:
    case RACE_RUST_MON:
    case RACE_MOUND:
    case RACE_OTYUGH:
    case RACE_CHIMERA:
    case RACE_BASILISK:
      if (num <= 70)
        return (TYPE_HIT);
      else
        return (TYPE_BITE);
    case RACE_MANTICORE:
    case RACE_SHEDU:
    case RACE_SPHINX:
    case RACE_LAMMASU:
    case RACE_GRIFFON:
      if (num < 30)
        return (TYPE_BITE);
      else if (num < 90)
        return (TYPE_CLAW);
      else
        return (TYPE_HIT);
    case RACE_MEDUSA:
      if (num < 10)
        return (TYPE_BITE);
      else if (num < 70)
        return (TYPE_CLAW);
      else
        return (TYPE_HIT);
    case RACE_SQUIRREL:
    case RACE_DEER:
    case RACE_WEASEL:
    case RACE_GOAT:
    case RACE_SHEEP:
    case RACE_GIRAFFE:
    case RACE_PIG:
    case RACE_BOAR:
    case RACE_RABBIT:
    case RACE_BADGER:
    case RACE_OTTER:
    case RACE_BEAVER:
    case RACE_GOPHER:
    case RACE_SAHUAGIN:
      if (num <= 30)
        return (TYPE_HIT);
      else
        return (TYPE_BITE);
    case RACE_UNDEAD:
    case RACE_OWLBEAR:
    case RACE_HIPPOPOTAMUS:
      if (num <= 60)
        return (TYPE_BEAT);
      else
        return (TYPE_BITE);
    case RACE_PANTATH:
      if (num <= 70)
        return (TYPE_BEAT);
      else if (num <= 90)
        return (TYPE_STING);
      else
        return (TYPE_BITE);
    case RACE_HORSE:
    case RACE_PEGASUS:
    case RACE_BOVINE:
    case RACE_OX:
      if (num <= 20)
        return (TYPE_BITE);
      else
        return (TYPE_STRIKE);
    case RACE_OCTOPUS:
    case RACE_DJINN:
      if (num <= 20)
        return (TYPE_THRASH);
      else if (num <= 30)
        return (TYPE_BITE);
      else if (num <= 60)
        return (TYPE_WHIP);
      else if (num <= 90)
        return (TYPE_FLAIL);
      else
        return (TYPE_POUND);
    case RACE_PIERCER:
      return TYPE_PIERCE;
    case RACE_ORB:
    case RACE_MIMIC:
      return TYPE_POUND;
    case RACE_NORACE:
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_OGRE:
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_VEGGIE:
    case RACE_FAERIE:
    case RACE_WOODELF:
    case RACE_MOSS:
    case RACE_CENTIPEDE:
    case RACE_GREMLIN:
    case RACE_SATYR:
    case RACE_DRYAD:
    case RACE_MINOTAUR:
    case RACE_GORGON:
    case RACE_LIZARD_MAN:
    case RACE_CENTAUR:
    case RACE_SIMAL:
    case RACE_LAMIA:
    case RACE_PYGMY:
    case RACE_PENGUIN:
    case RACE_OSTRICH:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_GNOME:
    case RACE_ORC:
    case RACE_FROGMAN:
    case RACE_GOBLIN:
    case RACE_TROG:
    case RACE_ANGEL:
    case RACE_DROW:
    case RACE_MERMAID:
    case RACE_FISHMAN:
    case RACE_UNCERT:
    case RACE_BUGBEAR:
    case RACE_KOBOLD:
    case RACE_GNOLL:
    case RACE_HOBGOBLIN:
    case RACE_VAMPIRE:
    case MAX_RACIAL_TYPES:
      return TYPE_HIT;
  }
  return (TYPE_HIT);
}

// this determines if mob dies when it leaves water
bool TBeing::isAquatic() const
{
  return getMyRace()->isAquatic();
}

bool TBeing::isFourLegged() const
{
  return getMyRace()->isFourLegged();
}

bool TBeing::isWinged() const
{
  return getMyRace()->isWinged();
}

bool TBeing::isColdBlooded() const
{
  if (isUndead())
    return TRUE;

  return getMyRace()->isColdBlooded();
}

int TBeing::slotChance(wearSlotT slot) const
{
  int old_slot;

  mud_assert(slot >= MIN_WEAR && slot < MAX_WEAR,
      "Bad slot in slotChance %s, %d", getName(), slot);

  // the slot_chance struct uses an old-style ordering structure 
  // Fortunately, the mapping function we have in place for writing files can
  // be used to get around this.
  old_slot = mapSlotToFile( slot);

  return slot_chance[getMyRace()->getBodyType()][old_slot];
}

