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
  TObj *obj = nullptr, *poison = nullptr;
  sstring weaponName, poisonName;
  int rc;

  if (!doesKnowSkill(SKILL_POISON_WEAPON)) {
    sendTo("You know nothing about poisoning weapons.\n\r");
    return FALSE;
  }
  if (checkBusy())
    return FALSE;

  // Parse first two arguments
  weaponName = arg.word(0);
  poisonName = arg.word(1);

  if (weaponName.empty() ||
      !(obj = generic_find_obj(weaponName, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                               this))) {
    sendTo("Poison what?\n\r");
    return FALSE;
  }

  if (!doesKnowSkill(SKILL_POISON_WEAPON) && !(dynamic_cast<TArrow *>(obj))) {
    sendTo("You are only skilled at poisoning arrows.\n\r");
    return FALSE;
  }

  // Handle remove command
  if (poisonName == "remove") {
    TBaseCup *container = nullptr;
    int waterNeeded =
        obj->getVolume() / 100;        // Scale water needed to weapon size
    waterNeeded = max(1, waterNeeded); // Ensure we need at least 1 unit

    if (!checkWaterUsage(this, waterNeeded, &container)) {
      sendTo(format("You need more water to clean off %s.\n\r") %
             obj->getName());
      return FALSE;
    }

    // If we found a container (not using room/weather water), consume the water
    if (container) {
      container->addToDrinkUnits(-waterNeeded);
    }

    rc = poisonWeapon(this, obj, nullptr);
    if (rc)
      addSkillLag(SKILL_POISON_WEAPON, rc);
    return rc;
  }

  // Handle normal poisoning
  if (poisonName.empty()) {
    sendTo("What do you want to apply to the weapon?\n\r");
    return FALSE;
  }

  if (!(poison = generic_find_obj(poisonName, FIND_OBJ_INV | FIND_OBJ_EQUIP,
                                  this))) {
    sendTo("You can't find that liquid container.\n\r");
    return FALSE;
  }

  if (fight()) {
    sendTo("You're a little too busy at the moment to try that.\n\r");
    return FALSE;
  }

  rc = poisonWeapon(this, obj, poison);
  if (rc)
    addSkillLag(SKILL_POISON_WEAPON, rc);
  return rc;
}

int TThing::poisonMePoison(TBeing *ch, TBaseWeapon *) {
  act("$p isn't the proper kind of poison for this.", FALSE, ch, this, 0,
      TO_CHAR);
  return FALSE;
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

int TBaseCup::poisonMePoison(TBeing *ch, TBaseWeapon *weapon) {
  int j;
  sstring s;
  spellNumT skill = SKILL_POISON_WEAPON;

  if (getDrinkUnits() <= 0) {
    act("$p seems not to have anything in it.", TRUE, ch, this, 0, TO_CHAR);
    return FALSE;
  }
  int bKnown = ch->getSkillValue(skill);

  if (ch->bSuccess(bKnown, skill)) {
    for (j = 0; j < MAX_SWING_AFFECT; j++) {
      if (weapon->isPoisoned()) {
        ch->sendTo("That weapon is already affected by poison!\n\r");
        return FALSE;
      }
    }

    weapon->setPoison(getDrinkType());

    s = format("You coat $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(s, FALSE, ch, weapon, NULL, TO_CHAR);
    s = format("$n coats $p with %s.") % liquidInfo[getDrinkType()]->name;
    act(s, FALSE, ch, weapon, NULL, TO_ROOM);
  } else {
    if (critFail(ch, skill) != CRIT_F_NONE) {
      act("You slip up and cut yourself with $p!", FALSE, ch, weapon, NULL,
          TO_CHAR);
      act("$n slips up and cuts $mself with $p!", FALSE, ch, weapon, NULL,
          TO_ROOM);

      act("There was something nasty on that $o!", FALSE, ch, weapon, ch,
          TO_VICT, ANSI_RED);
      act("You inflict something nasty on yourself!", FALSE, ch, weapon, ch,
          TO_CHAR, ANSI_RED);
      act("There was something nasty on that $o!", FALSE, ch, weapon, ch,
          TO_NOTVICT, ANSI_RED);

      doLiqSpell(ch, ch, getDrinkType(), 1);
    } else {
      weapon->setPoison(LIQ_WATER);

      s = format("You coat $p with %s.") % liquidInfo[getDrinkType()]->name;
      act(s, FALSE, ch, weapon, NULL, TO_CHAR);
      s = format("$n coats $p with %s.") % liquidInfo[getDrinkType()]->name;
      act(s, FALSE, ch, weapon, NULL, TO_ROOM);
    }
  }

  addToDrinkUnits(-1);
  return TRUE;
}

int poisonWeapon(TBeing *ch, TThing *weapon, TThing *poison) {
  return weapon->poisonWeaponWeapon(ch, poison);
}

int TThing::poisonWeaponWeapon(TBeing *ch, TThing *) {
  ch->sendTo("You can't poison that!  It's not a weapon!\n\r");
  return FALSE;
}
