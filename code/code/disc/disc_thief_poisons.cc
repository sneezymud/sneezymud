#include "disc_thief_poisons.h"
#include "being.h"
#include "combat.h"
#include "disc_thief_murder.h"
#include "disease.h"
#include "extern.h"
#include "handler.h"
#include "liquids.h"
#include "monster.h"
#include "obj_arrow.h"
#include "obj_base_cup.h"
#include "obj_base_weapon.h"
#include "obj_general_weapon.h"
#include "obj_tool.h"
#include "room.h"

int TBeing::doPoisonWeapon(sstring arg) {
  TObj* obj = nullptr;
  TObj* poison = nullptr;
  int rc;

  if (!doesKnowSkill(SKILL_POISON_WEAPON)) {
    sendTo("You know nothing about poisoning weapons.\n\r");
    return false;
  }
  if (checkBusy())
    return false;

  sstring targetName = arg.word(0);
  sstring poisonName = arg.word(1);

  if (targetName.empty() ||
      !(obj = generic_find_obj(targetName, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                               this))) {
    sendTo("Poison what?\n\r");
    return false;
  }

  // Must be a weapon, arrow, or spiked item
  bool isWeapon = dynamic_cast<TBaseWeapon*>(obj) != nullptr;
  bool isArrow = dynamic_cast<TArrow*>(obj) != nullptr;
  bool isSpiked = obj->isSpiked();

  if (!isWeapon && !isArrow && !isSpiked) {
    sendTo("You can only poison weapons and spiked items.\n\r");
    return false;
  }

  // Handle remove command
  if (poisonName == "remove") {
    TBaseCup* container = nullptr;
    int waterNeeded = std::max(1, obj->getVolume() / 100);

    if (!obj->isPoisoned()) {
      sendTo("That isn't poisoned.\n\r");
      return false;
    }

    if (!checkWaterUsage(this, waterNeeded, &container)) {
      sendTo(format("You need more water to clean off %s.\n\r") %
        obj->getName());
      return false;
    }

    obj->clearPoison();

    if (container) {
      container->addToDrinkUnits(-waterNeeded);
      act("You use some water from $P to wash the poison off $p.",
        false, this, obj, container, TO_CHAR);
      act("$n uses some water from $P to wash the poison off $p.",
        false, this, obj, container, TO_ROOM);
    } else {
      act("You use the surrounding water to wash the poison off $p.",
        false, this, obj, nullptr, TO_CHAR);
      act("$n uses the surrounding water to wash the poison off $p.",
        false, this, obj, nullptr, TO_ROOM);
    }

    addSkillLag(SKILL_POISON_WEAPON, 1);
    return true;
  }

  // Handle normal poisoning
  if (poisonName.empty()) {
    sendTo("What do you want to apply to it?\n\r");
    return false;
  }

  if (!(poison = generic_find_obj(poisonName, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                                  this))) {
    sendTo("You can't find that liquid container.\n\r");
    return false;
  }

  if (fight()) {
    sendTo("You're a little too busy at the moment to try that.\n\r");
    return false;
  }

  rc = poisonWeapon(this, obj, poison);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  addSkillLag(SKILL_POISON_WEAPON, 1);
  return rc;
}

int TThing::poisonMePoison(TBeing* ch, TObj*) {
  act("You can't use $p as a poison.", false, ch, this, nullptr, TO_CHAR);
  return false;
}

void addPoisonDefaults(affectedData *aff, int level, int duration) {
  aff->type = SPELL_POISON;
  aff->bitvector = AFF_POISON;
  aff->renew = -1;
  aff->level = level;
  aff->duration = duration;
}

// this is ugly as hell
bool addPoison(affectedData aff[5], liqTypeT liq, int level, int duration) {
  addPoisonDefaults(&aff[4], level, duration);
  aff[4].type = AFFECT_DISEASE;
  aff[4].modifier = DISEASE_POISON;
  aff[4].location = APPLY_NONE;

  switch (liq) {
  case LIQ_POISON_CAMAS:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_DEX;
    aff[0].modifier = -20;
    addPoisonDefaults(&aff[1], level, duration);
    aff[1].location = APPLY_AGI;
    aff[1].modifier = -20;
    addPoisonDefaults(&aff[2], level, duration);
    aff[2].location = APPLY_STR;
    aff[2].modifier = -20;
    addPoisonDefaults(&aff[3], level, duration);
    aff[3].location = APPLY_SPE;
    aff[3].modifier = -20;
    break;
  case LIQ_POISON_ANGEL:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_VISION;
    aff[0].modifier = -5;
    addPoisonDefaults(&aff[1], level, duration);
    aff[1].location = APPLY_FOC;
    aff[1].modifier = -20;
    addPoisonDefaults(&aff[2], level, duration);
    aff[2].location = APPLY_WIS;
    aff[2].modifier = -20;
    break;
  case LIQ_POISON_JIMSON:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_VISION;
    aff[0].modifier = -10;
    addPoisonDefaults(&aff[1], level, duration);
    aff[1].location = APPLY_FOC;
    aff[1].modifier = -20;
    break;
  case LIQ_POISON_HEMLOCK:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_STR;
    aff[0].modifier = -20;
    addPoisonDefaults(&aff[1], level, duration);
    aff[1].location = APPLY_INT;
    aff[1].modifier = -20;
    addPoisonDefaults(&aff[2], level, duration);
    aff[2].location = APPLY_FOC;
    aff[2].modifier = -20;
    break;
  case LIQ_POISON_MONKSHOOD:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_STR;
    aff[0].modifier = -20;
    break;
  case LIQ_POISON_GLOW_FISH:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_CAN_BE_SEEN;
    aff[0].modifier = -10;
    break;
  case LIQ_POISON_SCORPION:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_INT;
    aff[0].modifier = -20;
    addPoisonDefaults(&aff[1], level, duration);
    aff[1].location = APPLY_SPE;
    aff[1].modifier = -40;
    break;
  case LIQ_POISON_VIOLET_FUNGUS:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_IMMUNITY;
    aff[0].modifier = IMMUNE_SLEEP;
    aff[0].modifier2 = -30;
    break;
  case LIQ_POISON_DEVIL_ICE:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_IMMUNITY;
    aff[0].modifier = IMMUNE_HEAT;
    aff[0].modifier2 = -20;
    break;
  case LIQ_POISON_FIREDRAKE:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_IMMUNITY;
    aff[0].modifier = IMMUNE_COLD;
    aff[0].modifier2 = -20;
    break;
  case LIQ_POISON_INFANT:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_IMMUNITY;
    aff[0].modifier = IMMUNE_DRAIN;
    aff[0].modifier2 = -20;
    break;
  case LIQ_POISON_PEA_SEED:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_SPE;
    aff[0].modifier = -20;
    break;
  case LIQ_POISON_ACACIA:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_STR;
    aff[0].modifier = -40;
    break;
  case LIQ_POISON_STANDARD:
    addPoisonDefaults(&aff[0], level, duration);
    aff[0].location = APPLY_STR;
    aff[0].modifier = -20;
    break;
  default:
    return false;
    break;
  }
  return true;
}

int TBaseCup::poisonMePoison(TBeing* ch, TObj* target) {
  if (getDrinkUnits() <= 0) {
    act("$p is already empty.", true, ch, this, nullptr, TO_CHAR);
    return false;
  }

  if (getDrinkType() == LIQ_WATER || getDrinkType() == LIQ_SALTWATER) {
    ch->sendTo(format("%s isn't something you can poison with.\n\r") %
      sstring(liquidInfo[getDrinkType()]->name).cap());
    return false;
  }

  int bKnown = ch->getSkillValue(SKILL_POISON_WEAPON);

  if (ch->bSuccess(bKnown, SKILL_POISON_WEAPON)) {
    if (target->isPoisoned()) {
      ch->sendTo("That is already affected by poison!\n\r");
      return false;
    }

    target->setPoison(getDrinkType());

    auto msg = format("You coat $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(msg, false, ch, target, nullptr, TO_CHAR);
    msg = format("$n coats $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(msg, false, ch, target, nullptr, TO_ROOM);
  } else {
    if (critFail(ch, SKILL_POISON_WEAPON) != CRIT_F_NONE) {
      act("You slip up and nick yourself on $p!", false, ch, target, nullptr,
        TO_CHAR);
      act("$n slips up and nicks $mself on $p!", false, ch, target, nullptr,
        TO_ROOM);

      act("You inflict something nasty on yourself!", false, ch, target, nullptr,
        TO_CHAR, ANSI_RED);
      act("$n inflicts something nasty on $mself!", false, ch, target, nullptr,
        TO_ROOM, ANSI_RED);

      int rc = doLiqSpell(ch, ch, getDrinkType(), 1);
      if (IS_SET(rc, VICTIM_DEAD | CASTER_DEAD))
        return DELETE_THIS;
    } else {
      auto msg = format("You try to coat $p with %s, but it doesn't take.") % liquidInfo[getDrinkType()]->name;
      act(msg, false, ch, target, nullptr, TO_CHAR);
      msg = format("$n tries to coat $p with %s, but it doesn't seem to take.") % liquidInfo[getDrinkType()]->name;
      act(msg, false, ch, target, nullptr, TO_ROOM);
    }
  }

  addToDrinkUnits(-1);
  return true;
}

int poisonWeapon(TBeing* ch, TThing* target, TThing* poison) {
  return target->poisonWeaponWeapon(ch, poison);
}

int TThing::poisonWeaponWeapon(TBeing* ch, TThing*) {
  ch->sendTo("You can't poison that!\n\r");
  return false;
}
